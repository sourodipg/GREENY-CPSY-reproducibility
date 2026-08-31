#include "../../../../parent/reproduction_package/code/greeny/src/params.hpp"
#include "../../../../parent/reproduction_package/code/greeny/src/population.hpp"
#include "../../../../parent/reproduction_package/code/greeny/src/simulate.hpp"

#include <algorithm>
#include <array>
#include "../../../../code/network_kernels.hpp"
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

namespace {

struct NewState { double x=0,debt=0,theta=0,ep_dose=0; long long ep_steps=0,n_collapse=0; int ep_left=0; bool entered=false; };

struct Metrics {
    double rA=0,rV=0,rate=0,anx_or=0,av_or=0;
    double pair_x_corr_pre=0,pair_q_corr_pre=0,pair_x_corr_post=0,pair_q_corr_post=0;
    double mutual_elev_pre=0,mutual_elev_post=0;
    double repeat1=0,repeat5=0,repeat20=0;
    double mean_pair_distance=0,same_family=0,unique_partners_frac=0,degree_cv=0;
    uint64_t pair_digest=0;
};

struct Stat { double mean=0,sd=0,se=0,lo=0,hi=0,p=0,dz=0; };

struct OnlineCorr {
    long long n=0; double sx=0,sy=0,sxx=0,syy=0,sxy=0;
    void add(double x,double y){++n;sx+=x;sy+=y;sxx+=x*x;syy+=y*y;sxy+=x*y;}
    double value() const {
        if(n<2) return 0.0;
        const double nn=double(n);
        const double vx=sxx/nn-(sx/nn)*(sx/nn), vy=syy/nn-(sy/nn)*(sy/nn);
        const double c=sxy/nn-(sx/nn)*(sy/nn), d=std::sqrt(std::max(0.0,vx*vy));
        return d>0?c/d:0.0;
    }
};

static int g_coordinate_seed_offset = 0;

static Params frozen(){
    Params P;
    P.alpha0=0.15; P.K=0.3; P.theta0=11.4620520617449; P.p_exp=1.546108328167009;
    P.gamma=0.9631550430923563; P.c_anx=0.248276268816769; P.c_avo=0.7747696473157317;
    P.w_avo_theta=0.4807631904393139; P.dim_rho=0.2544178617628602; P.extra_var=0.06625401676478866;
    P.lambda0=0.004; P.track_frac=0.6583990950417017; P.recover_frac=0.8; P.dose_ref=0.16; P.delta_dose=0.15;
    P.random_forcing=true; P.two_axis=true; P.suspended_episodes=true; P.alpha_floor=0.002;
    P.perception_noise=0.0; P.nucleation_collapse=false; P.arrhenius_update=false;
    return P;
}

static double corr(const std::vector<double>&x,const std::vector<double>&y){
    if(x.size()<2 || x.size()!=y.size()) return 0.0;
    const double n=double(x.size()); double sx=0,sy=0,sxx=0,syy=0,sxy=0;
    for(size_t i=0;i<x.size();++i){sx+=x[i];sy+=y[i];sxx+=x[i]*x[i];syy+=y[i]*y[i];sxy+=x[i]*y[i];}
    const double vx=sxx/n-(sx/n)*(sx/n),vy=syy/n-(sy/n)*(sy/n),c=sxy/n-(sx/n)*(sy/n),d=std::sqrt(std::max(0.0,vx*vy));
    return d>0?c/d:0.0;
}

static std::vector<greeny_network::PairID> contact_pairs(int N,const std::vector<int>&ord,unsigned seed,int t,double mix_strength){
    if(mix_strength<0.0 || mix_strength>1.0) throw std::runtime_error("mix_strength must be in [0,1]");
    if(mix_strength<=0.0) return greeny_network::random_matching(N,seed,t);
    if(mix_strength>=1.0) return greeny_network::metric_matching(ord,seed,t);
    const uint64_t r=greeny_network::splitmix64(greeny_network::key(seed,t,23));
    const double u=double(r>>11)/double(1ULL<<53);
    return (u<mix_strength)?greeny_network::metric_matching(ord,seed,t):greeny_network::random_matching(N,seed,t);
}

static double social_signal_scaled_partner(const Params& P,const Population& pop,const std::vector<double>&prev,const std::vector<double>&fam,int i,int j,double target,double partner_scale){
    const int f=pop.agents[i].family_id;
    const double iw=pop.inside_w[f],ow=pop.outside_w[f],tw=std::max(0.01,1.0-iw-ow);
    const double kap=P.two_axis?pop.agents[i].kappa:1.0;
    // partner_scale=1 reproduces the production target exactly; partner_scale=0 ablates only the partner signal while retaining the original normalization.
    return (tw*target+kap*(iw*fam[f]+partner_scale*ow*prev[j]))/(tw+kap*(iw+ow));
}


static inline double prev_value(const std::vector<double>&prev,int idx,int override_idx,double override_value){return idx==override_idx?override_value:prev[idx];}

static NewState step_one_scaled_partner(const Params&P,const Population&pop,const std::vector<double>&prev,const std::vector<double>&fam,int i,int j,double target,const NewState&old,double partner_scale,double alpha_scale=1.0,int override_idx=-1,double override_value=0.0){
    NewState z=old;const Agent&a=pop.agents[i];
    if(P.suspended_episodes&&old.ep_left>0){
        const int f=a.family_id;const double iw=pop.inside_w[f],ow=pop.outside_w[f],tw=std::max(0.01,1.0-iw-ow),kap=P.two_axis?a.kappa:1.0;
        const double partner_x=prev_value(prev,j,override_idx,override_value);
        const double S=(tw*target+kap*(iw*fam[f]+partner_scale*ow*partner_x))/(tw+kap*(iw+ow));
        z.ep_left=1;z.ep_steps=old.ep_steps+1;z.ep_dose=old.ep_dose+(old.debt/std::max(1e-9,old.theta))/P.shock_mean_interval;
        z.x=old.x+P.track_frac*(a.alpha*alpha_scale)*(S-old.x);z.debt=old.debt*(1.0-a.lam);
        if(z.debt<P.recover_frac*old.theta){double fd=z.ep_dose/std::max(1e-9,P.dose_ref);z.theta=std::min(P.theta_hi_mult*a.theta0_trait,std::max(P.theta_lo_mult*a.theta0_trait,old.theta*std::exp(-P.delta_dose*(fd-1.0))));z.ep_left=0;z.ep_dose=0;}
        return z;
    }
    const double S=social_signal_scaled_partner(P,pop,prev,fam,i,j,target,partner_scale);const double d=S-prev_value(prev,i,override_idx,override_value);const double alpha=a.alpha*alpha_scale;
    z.x=prev_value(prev,i,override_idx,override_value)+alpha*d;const double inflow=(1.0-alpha)*std::max(0.0,std::fabs(d)-P.debt_deadband);const double enew=P.gamma*old.debt+inflow;
    z.debt=enew;z.entered=(P.suspended_episodes&&enew>=old.theta);if(z.entered){z.ep_left=1;z.ep_dose=0;z.n_collapse=old.n_collapse+1;}return z;
}


static Metrics run_cell(const Params&P,unsigned seed,const std::vector<int>&metric_ord,double mix_strength,int interaction_mode,double alpha_scale=1.0){
    const int N=P.n_families*P.family_size;if(N%2)throw std::runtime_error("N must be even");
    Population pop=build_population(P,seed);const std::vector<double> targets=random_schedule(P,seed+777013u);const std::vector<double> z=greeny_network::social_latent(N,unsigned(seed + g_coordinate_seed_offset));
    const int burn=int(P.burn_in_frac*P.max_steps);const int sym_start=std::max(0,P.max_steps-std::max(1,int(P.symptom_window*P.shock_mean_interval)));
    std::vector<double>prev(N),fam(P.n_families);std::vector<NewState>old(N),nw(N);std::vector<int>last(N,-1),hist_len(N,0);std::vector<std::array<int,20>>hist(N);
    const size_t bits_per_row=(size_t(N)+63u)/64u;std::vector<uint64_t>adj(size_t(N)*bits_per_row,0);
    OnlineCorr prex,preq,postx,postq;long long incoming_events=0,pair_events=0,r1=0,r5=0,r20=0,mutpre=0;double dist_sum=0;long long samefam=0;std::array<long long,3>cat{0,0,0};uint64_t digest=1469598103934665603ULL;
    for(int t=0;t<P.max_steps;++t){
        std::fill(fam.begin(),fam.end(),0.0);for(int i=0;i<N;++i){prev[i]=pop.agents[i].x;fam[pop.agents[i].family_id]+=prev[i];}for(double&v:fam)v/=P.family_size;
        for(int i=0;i<N;++i){const Agent&a=pop.agents[i];old[i]={a.x,a.debt,a.theta,a.ep_dose,a.ep_steps,a.n_collapse,a.ep_left,false};nw[i]=old[i];}
        const auto pairs=contact_pairs(N,metric_ord,seed,t,mix_strength);pair_events += static_cast<long long>(pairs.size());
        for(const auto&pp:pairs){
            const int i=pp.a,j=pp.b;if(i<0||j<0||i>=N||j>=N||i==j)throw std::runtime_error("invalid pair");
            digest=greeny_network::splitmix64(digest ^ uint64_t(i+1)*0x9e3779b97f4a7c15ULL ^ uint64_t(j+1)*0xbf58476d1ce4e5b9ULL ^ uint64_t(t+1));
            const double dz=std::fabs(z[i]-z[j]); dist_sum += std::min(dz,1.0-dz);if(pop.agents[i].family_id==pop.agents[j].family_id)samefam+=2;
            const double qi=old[i].debt/std::max(1e-9,old[i].theta),qj=old[j].debt/std::max(1e-9,old[j].theta);if(qi>=1.0&&qj>=1.0)++mutpre;
            prex.add(prev[i],prev[j]);preq.add(qi,qj);
            auto record_incoming=[&](int u,int v){
                ++incoming_events;int n=hist_len[u];if(n>0&&hist[u][n-1]==v)++r1;for(int q=0;q<n&&q<5;q++)if(hist[u][n-1-q]==v){++r5;break;}for(int q=0;q<n;q++)if(hist[u][n-1-q]==v){++r20;break;}
                if(n<20)hist[u][n++]=v;else{for(int q=0;q<19;q++)hist[u][q]=hist[u][q+1];hist[u][19]=v;}hist_len[u]=n;
                const size_t off=size_t(u)*bits_per_row+size_t(v)/64u;adj[off]|=1ULL<<(unsigned(v)%64u);
            };
            if(interaction_mode==0 || interaction_mode==2){
                const int u0=greeny_network::directed_recipient(seed,t,i,j);
                const int u=(interaction_mode==2)?((u0==i)?j:i):u0;
                const int v=(u==i)?j:i;
                nw[u]=step_one_scaled_partner(P,pop,prev,fam,u,v,targets[t],old[u],1.0,alpha_scale);record_incoming(u,v);
                postx.add(nw[i].x==old[i].x?old[i].x:nw[i].x,nw[j].x==old[j].x?old[j].x:nw[j].x);
                postq.add(nw[i].debt==old[i].debt?qi:nw[i].debt/std::max(1e-9,nw[i].theta),nw[j].debt==old[j].debt?qj:nw[j].debt/std::max(1e-9,nw[j].theta));
            } else {
                const double partner_scale=(interaction_mode==3)?0.0:1.0;
                const NewState ni=step_one_scaled_partner(P,pop,prev,fam,i,j,targets[t],old[i],partner_scale,alpha_scale);
                const NewState nj=step_one_scaled_partner(P,pop,prev,fam,j,i,targets[t],old[j],partner_scale,alpha_scale);
                nw[i]=ni;nw[j]=nj;record_incoming(i,j);record_incoming(j,i);
                const double qni=nw[i].debt/std::max(1e-9,nw[i].theta),qnj=nw[j].debt/std::max(1e-9,nw[j].theta);
                postx.add(nw[i].x,nw[j].x);postq.add(qni,qnj);
            }
        }
        for(int i=0;i<N;++i){Agent&a=pop.agents[i];const NewState&v=nw[i];a.x=v.x;a.debt=v.debt;a.theta=v.theta;a.ep_left=v.ep_left;a.ep_steps=v.ep_steps;a.ep_dose=v.ep_dose;a.n_collapse=v.n_collapse;if(v.entered&&t>=burn)cat[a.style]++;if(t>=sym_start){a.sym_debt+=a.debt/std::max(1e-9,a.theta);a.sym_abs+=a.debt;a.sym_n++;}}
    }
    std::vector<double>A,V,S;A.reserve(N);V.reserve(N);S.reserve(N);int cnt[3]={0,0,0};for(const auto&a:pop.agents){cnt[a.style]++;A.push_back(a.anx);V.push_back(a.avo);S.push_back(a.sym_n?a.sym_debt/a.sym_n:0.0);}
    Metrics m;for(double v:S)if(!std::isfinite(v))throw std::runtime_error("non-finite severity");m.rA=corr(A,S);m.rV=corr(V,S);const double rates[3]={cnt[0]?double(cat[0])/cnt[0]:0,cnt[1]?double(cat[1])/cnt[1]:0,cnt[2]?double(cat[2])/cnt[2]:0};m.anx_or=rates[2]/std::max(1e-12,rates[0]);m.av_or=rates[1]/std::max(1e-12,rates[0]);m.rate=rates[0]/std::max(1.0,(P.max_steps-burn)/P.shock_mean_interval);
    m.pair_x_corr_pre=prex.value();m.pair_q_corr_pre=preq.value();m.pair_x_corr_post=postx.value();m.pair_q_corr_post=postq.value();m.mutual_elev_pre=double(mutpre)/std::max(1LL,pair_events);
    m.mutual_elev_post=std::max(0.0,std::min(1.0,m.pair_q_corr_post));m.repeat1=double(r1)/std::max(1LL,incoming_events);m.repeat5=double(r5)/std::max(1LL,incoming_events);m.repeat20=double(r20)/std::max(1LL,incoming_events);
    m.mean_pair_distance=dist_sum/std::max(1LL,pair_events);m.same_family=double(samefam)/std::max(1LL,2*pair_events);m.pair_digest=digest;
    double sumdeg=0,sq=0;for(int i=0;i<N;++i){long long d=0;const size_t base=size_t(i)*bits_per_row;for(size_t w=0;w<bits_per_row;++w)d+=__builtin_popcountll(adj[base+w]);sumdeg+=double(d);sq+=double(d)*double(d);}const double md=sumdeg/N;const double var=std::max(0.0,sq/N-md*md);m.unique_partners_frac=md/std::max(1,N-1);m.degree_cv=md>0?std::sqrt(var)/md:0;
    if(!std::isfinite(m.rA)||!std::isfinite(m.rV)||!std::isfinite(m.rate)) throw std::runtime_error("non-finite headline metric");
    return m;
}

static Stat stats(const std::vector<double>&d){Stat s;if(d.empty())return s;s.mean=std::accumulate(d.begin(),d.end(),0.0)/d.size();double ss=0;for(double v:d)ss+=(v-s.mean)*(v-s.mean);s.sd=std::sqrt(ss/std::max<size_t>(1,d.size()-1));s.se=s.sd/std::sqrt(double(d.size()));s.lo=s.mean-1.96*s.se;s.hi=s.mean+1.96*s.se;std::mt19937 g(0xDADA5629u);int ge=0;for(int b=0;b<20000;++b){double x=0;for(double v:d)x+=(g()&1)?v:-v;x/=d.size();if(std::fabs(x)>=std::fabs(s.mean))++ge;}s.p=(1.0+ge)/20001.0;s.dz=s.sd>0?s.mean/s.sd:0;return s;}

static int run_fork(int seeds,int threads,int families,int steps,int totalN,double mix_strength,double shock_interval=500.0,double symptom_window=0.06){
    if(seeds<1||threads<1||families<1||steps<1||totalN<2) throw std::runtime_error("dimensions must be positive");
    if(totalN%families!=0||totalN%2!=0) throw std::runtime_error("N must be even and divisible by families");
    Params P=frozen();P.n_families=families;P.family_size=totalN/families;P.max_steps=steps;P.shock_mean_interval=shock_interval;P.symptom_window=symptom_window;
    std::vector<unsigned>sv(seeds);for(int k=0;k<seeds;k++)sv[k]=P.seed0+101u*k;
    // Layout: [D_random, Y_random, Dswap_random, Yablated_random] and [D_metric, Y_metric, Dswap_metric, Yablated_metric] interleaved by mixing block.
    std::array<std::vector<Metrics>,10>M;for(auto&v:M)v.resize(seeds);
#ifdef _OPENMP
    omp_set_num_threads(threads);
#pragma omp parallel for schedule(static)
#endif
    for(int k=0;k<seeds;k++){
        unsigned s=sv[k];
        std::vector<double> z=greeny_network::social_latent(totalN,unsigned(s + g_coordinate_seed_offset));
        std::vector<int> ord(totalN);std::iota(ord.begin(),ord.end(),0);std::sort(ord.begin(),ord.end(),[&](int a,int b){return z[a]<z[b] || (z[a]==z[b]&&a<b);});
        M[0][k]=run_cell(P,s,ord,0.0,0,1.0); M[1][k]=run_cell(P,s,ord,0.0,1,1.0); M[2][k]=run_cell(P,s,ord,0.0,2,1.0); M[3][k]=run_cell(P,s,ord,0.0,3,1.0); M[4][k]=run_cell(P,s,ord,0.0,4,0.5);
        M[5][k]=run_cell(P,s,ord,mix_strength,0,1.0); M[6][k]=run_cell(P,s,ord,mix_strength,1,1.0); M[7][k]=run_cell(P,s,ord,mix_strength,2,1.0); M[8][k]=run_cell(P,s,ord,mix_strength,3,1.0); M[9][k]=run_cell(P,s,ord,mix_strength,4,0.5);
    }
    for(size_t k=0;k<sv.size();++k){
        if(M[0][k].pair_digest!=M[1][k].pair_digest||M[0][k].pair_digest!=M[2][k].pair_digest||M[0][k].pair_digest!=M[3][k].pair_digest||M[0][k].pair_digest!=M[4][k].pair_digest) throw std::runtime_error("random encounter ledger differs across interaction conditions");
        if(M[5][k].pair_digest!=M[6][k].pair_digest||M[5][k].pair_digest!=M[7][k].pair_digest||M[5][k].pair_digest!=M[8][k].pair_digest||M[5][k].pair_digest!=M[9][k].pair_digest) throw std::runtime_error("metric encounter ledger differs across interaction conditions");
    }
    // Write an explicit all-conditions table plus contrasts from the same seed ordering.
    std::ofstream raw("results/interaction_disambiguation_per_seed.csv");
    raw<<"seed,mixing,condition,recipient_rule,partner_signal_scale,alpha_scale,rA,rV,rate,pair_x_corr_post,pair_q_corr_post,repeat1,repeat5,repeat20,pair_digest\n";
    const char* cn[] = {"D","Y","D_SWAP","Y_ABLATE","Y_HALFALPHA"};
    const char* rr[] = {"original_one_sided","both_update","swapped_one_sided","both_update","both_update"};
    const double ps[] = {1.0,1.0,1.0,0.0,1.0};
    const double as[] = {1.0,1.0,1.0,1.0,0.5};
    for(int c=0;c<5;++c) for(size_t k=0;k<sv.size();++k){const auto&m=M[c][k];raw<<sv[k]<<",random,"<<cn[c]<<","<<rr[c]<<","<<ps[c]<<","<<as[c]<<","<<m.rA<<","<<m.rV<<","<<m.rate<<","<<m.pair_x_corr_post<<","<<m.pair_q_corr_post<<","<<m.repeat1<<","<<m.repeat5<<","<<m.repeat20<<","<<m.pair_digest<<"\n";}
    for(int c=5;c<10;++c) for(size_t k=0;k<sv.size();++k){const auto&m=M[c][k];raw<<sv[k]<<",metric,"<<cn[c-5]<<","<<rr[c-5]<<","<<ps[c-5]<<","<<as[c-5]<<","<<m.rA<<","<<m.rV<<","<<m.rate<<","<<m.pair_x_corr_post<<","<<m.pair_q_corr_post<<","<<m.repeat1<<","<<m.repeat5<<","<<m.repeat20<<","<<m.pair_digest<<"\n";}
    std::ofstream diffs("results/interaction_disambiguation_contrasts.csv");
    diffs<<"mixing,contrast,metric,mean_diff,sd,se,ci_lo,ci_hi,paired_signflip_p,d_z\n";
    auto add=[&](const char*mixname,const char*name,int c1,int c0,auto get,const char*metric){std::vector<double>d;for(size_t k=0;k<sv.size();++k)d.push_back(get(M[c1][k])-get(M[c0][k]));const auto st=stats(d);diffs<<mixname<<","<<name<<","<<metric<<","<<st.mean<<","<<st.sd<<","<<st.se<<","<<st.lo<<","<<st.hi<<","<<st.p<<","<<st.dz<<"\n";};
    auto addblock=[&](const char*mixname,int b){
        add(mixname,"D_SWAP_minus_D_rA",b+2,b+0,[](const Metrics&m){return m.rA;},"rA");
        add(mixname,"D_SWAP_minus_D_rV",b+2,b+0,[](const Metrics&m){return m.rV;},"rV");
        add(mixname,"Y_minus_D_rA",b+1,b+0,[](const Metrics&m){return m.rA;},"rA");
        add(mixname,"Y_minus_D_rV",b+1,b+0,[](const Metrics&m){return m.rV;},"rV");
        add(mixname,"Y_ABLATE_minus_D_rA",b+3,b+0,[](const Metrics&m){return m.rA;},"rA");
        add(mixname,"Y_ABLATE_minus_D_rV",b+3,b+0,[](const Metrics&m){return m.rV;},"rV");
        add(mixname,"Y_ABLATE_minus_Y_rA",b+3,b+1,[](const Metrics&m){return m.rA;},"rA");
        add(mixname,"Y_ABLATE_minus_Y_rV",b+3,b+1,[](const Metrics&m){return m.rV;},"rV");
        add(mixname,"Y_HALFALPHA_minus_D_rA",b+4,b+0,[](const Metrics&m){return m.rA;},"rA");
        add(mixname,"Y_HALFALPHA_minus_D_rV",b+4,b+0,[](const Metrics&m){return m.rV;},"rV");
        add(mixname,"Y_HALFALPHA_minus_Y_rA",b+4,b+1,[](const Metrics&m){return m.rA;},"rA");
        add(mixname,"Y_HALFALPHA_minus_Y_rV",b+4,b+1,[](const Metrics&m){return m.rV;},"rV");
    };
    addblock("random",0);addblock("metric",5);
    std::ofstream summary("results/interaction_disambiguation_summary.csv");summary<<"mixing,condition,rA_mean,rV_mean,rate_mean,repeat1,repeat5,repeat20,pair_x_corr_post,pair_q_corr_post\n";
    for(int c=0;c<10;++c){double vals[8]={0};for(const auto&m:M[c]){vals[0]+=m.rA;vals[1]+=m.rV;vals[2]+=m.rate;vals[3]+=m.repeat1;vals[4]+=m.repeat5;vals[5]+=m.repeat20;vals[6]+=m.pair_x_corr_post;vals[7]+=m.pair_q_corr_post;}double n=double(seeds);summary<<(c<5?"random":"metric")<<","<<cn[c%5];for(int q=0;q<8;++q)summary<<","<<vals[q]/n;summary<<"\n";}
    std::cout<<"INTERACTION DISAMBIGUATION FORK seeds="<<seeds<<" threads="<<threads<<" N="<<totalN<<" steps="<<steps<<" families="<<families<<" metric_strength="<<mix_strength<<"\n";
    return 0;
}

} // namespace

int main(int argc,char**argv){
    try{
        const std::string mode=argc>1?argv[1]:"verify";
        if(mode=="smoke") return run_fork(2,1,10,120,40,1.0);
        if(mode=="run"){
            const int seeds=argc>2?std::stoi(argv[2]):56,threads=argc>3?std::stoi(argv[3]):1,families=argc>4?std::stoi(argv[4]):1000,steps=argc>5?std::stoi(argv[5]):12000,N=argc>6?std::stoi(argv[6]):4000; const double ms=argc>7?std::stod(argv[7]):1.0; const int offset=argc>8?std::stoi(argv[8]):0; g_coordinate_seed_offset=offset; return run_fork(seeds,threads,families,steps,N,ms);
        }
        if(mode=="verify"){
            // Tiny runtime sanity: swap mode must preserve encounter digest and ablation must remain finite.
            run_fork(2,1,20,200,80,1.0);
            std::cout<<"FORK STRUCTURAL VERIFY PASS\n";
            return 0;
        }
        throw std::runtime_error("usage: smoke | verify | run [seeds threads families steps N metric_strength coordinate_seed_offset]");
    }catch(const std::exception&e){std::cerr<<"ERROR: "<<e.what()<<"\n";return 2;}
}
