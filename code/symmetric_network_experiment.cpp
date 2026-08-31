#include "../parent/reproduction_package/code/greeny/src/params.hpp"
#include "../parent/reproduction_package/code/greeny/src/population.hpp"
#include "../parent/reproduction_package/code/greeny/src/simulate.hpp"
#include <algorithm>
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

struct PairID { unsigned a=0,b=0; bool operator==(const PairID&o) const{return a==o.a&&b==o.b;} };

static uint64_t splitmix64(uint64_t x){
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
static uint64_t key(uint64_t seed,int t,uint64_t stream=0){
    uint64_t x=seed ^ (0xD1B54A32D192ED03ULL + stream*0x9E3779B97F4A7C15ULL);
    x ^= uint64_t(t)*0x94D049BB133111EBULL;
    return splitmix64(x);
}
static std::vector<PairID> matching(int N,unsigned seed,int t){
    std::vector<int> v(N); std::iota(v.begin(),v.end(),0); uint64_t s=key(seed,t,1);
    for(int k=N-1;k>0;--k){s=splitmix64(s); int j=int(s%uint64_t(k+1)); std::swap(v[k],v[j]);}
    std::vector<PairID> out; out.reserve(N/2);
    for(int q=0;q<N;q+=2) out.push_back({unsigned(v[q]),unsigned(v[q+1])});
    return out;
}

static Params frozen(){
    Params P;
    P.alpha0=0.15; P.K=0.3; P.theta0=11.4620520617449; P.p_exp=1.546108328167009;
    P.gamma=0.9631550430923563; P.c_anx=0.248276268816769; P.c_avo=0.7747696473157317;
    P.w_avo_theta=0.4807631904393139; P.dim_rho=0.2544178617628602; P.extra_var=0.06625401676478866;
    P.lambda0=0.004; P.track_frac=0.6583990950417017; P.recover_frac=0.8; P.dose_ref=0.16; P.delta_dose=0.15;
    P.random_forcing=true; P.two_axis=true; P.suspended_episodes=true; P.alpha_floor=0.002;
    P.n_families=1000; P.family_size=4; P.n_seeds=56; P.max_steps=12000;
    P.perception_noise=0.0; P.nucleation_collapse=false; P.arrhenius_update=false;
    return P;
}
static double social_signal(const Params& P,const Population& pop,const std::vector<double>& prev,const std::vector<double>& fam,int i,int j,double target){
    const int f=pop.agents[i].family_id; const double iw=pop.inside_w[f],ow=pop.outside_w[f],tw=std::max(0.01,1.0-iw-ow);
    const double kap=P.two_axis?pop.agents[i].kappa:1.0;
    return (tw*target+kap*(iw*fam[f]+ow*prev[j]))/(tw+kap*(iw+ow));
}

struct NewState{double x=0,debt=0,theta=0,ep_dose=0; long long ep_steps=0,n_collapse=0; int ep_left=0; bool entered=false;};
struct OnlineCorr{long long n=0;double sx=0,sy=0,sxx=0,syy=0,sxy=0;void add(double x,double y){n++;sx+=x;sy+=y;sxx+=x*x;syy+=y*y;sxy+=x*y;}double value()const{if(n<2)return 0;double nn=(double)n,vx=sxx/nn-(sx/nn)*(sx/nn),vy=syy/nn-(sy/nn)*(sy/nn),c=sxy/nn-(sx/nn)*(sy/nn),d=std::sqrt(std::max(0.0,vx*vy));return d>0?c/d:0;}};
struct Metrics{
    double rA=0,rV=0,anx_or=0,av_or=0,rate=0;
    double pair_x_corr=0,pair_q_corr=0,pair_x_corr_post=0,pair_q_corr_post=0,mutual_elev=0;
    double repeat1=0,repeat5=0,repeat20=0;
    double self_loop=0, instant_recip=0, unique_target_frac=0, same_family=0;
};

static double pearson(const std::vector<double>&x,const std::vector<double>&y){
    if(x.size()<2) return 0;
    double sx=0,sy=0,sxx=0,syy=0,sxy=0; double n=x.size();
    for(size_t i=0;i<x.size();++i){sx+=x[i];sy+=y[i];sxx+=x[i]*x[i];syy+=y[i]*y[i];sxy+=x[i]*y[i];}
    double vx=sxx/n-(sx/n)*(sx/n),vy=syy/n-(sy/n)*(sy/n),c=sxy/n-(sx/n)*(sy/n); double d=std::sqrt(std::max(0.0,vx*vy)); return d>0?c/d:0;
}
static void finite_vec(const std::vector<double>&v,const char*n){for(double x:v)if(!std::isfinite(x))throw std::runtime_error(std::string("nonfinite ")+n);}

// Exact parent-structure probe at the frozen GREENY RNG configuration. The parent
// partner RNG is isolated from population construction and, because all
// RNG-consuming optional mechanisms are disabled here, one integer draw occurs
// per agent per timestep exactly as in simulate_once().
static Metrics greeny_graph_probe(const Params&P,unsigned seed){
    const int N=P.n_families*P.family_size;
    Population pop=build_population(P,seed);
    std::mt19937 rng(seed+1000003u); std::uniform_int_distribution<int> pick(0,N-1);
    std::vector<int> last(N,-1), hist_flat(size_t(N)*20,-1), hlen(N,0), partner(N,-1), target_seen(N,0);
    long long total=0,r1=0,r5=0,r20=0,self=0,recip_arcs=0,unique_sum=0,samefam=0;
    for(int t=0;t<P.max_steps;++t){
        std::fill(partner.begin(),partner.end(),-1); std::fill(target_seen.begin(),target_seen.end(),0);
        for(int i=0;i<N;++i){
            int j=pick(rng); partner[i]=j; total++;
            if(j==i) self++;
            if(last[i]==j) r1++;
            int n=hlen[i], base=i*20;
            for(int q=std::max(0,n-5);q<n;++q) if(hist_flat[base+q]==j){r5++;break;}
            for(int q=0;q<n;++q) if(hist_flat[base+q]==j){r20++;break;}
            if(n<20){ hist_flat[base+n]=j; hlen[i]=n+1; } else { for(int q=0;q<19;++q) hist_flat[base+q]=hist_flat[base+q+1]; hist_flat[base+19]=j; hlen[i]=20; }
            last[i]=j; target_seen[j]=1;
            if(j!=i && pop.agents[i].family_id==pop.agents[j].family_id) samefam++;
        }
        for(int i=0;i<N;++i){int j=partner[i]; if(j!=i && partner[j]==i) recip_arcs++;}
        unique_sum += std::accumulate(target_seen.begin(),target_seen.end(),0);
    }
    Metrics m; m.repeat1=double(r1)/total;m.repeat5=double(r5)/total;m.repeat20=double(r20)/total;m.self_loop=double(self)/total;m.instant_recip=double(recip_arcs)/total;m.unique_target_frac=double(unique_sum)/total;m.same_family=double(samefam)/total; return m;
}

static inline double prev_value(const std::vector<double>& prev,int idx,int override_idx,double override_value){return idx==override_idx?override_value:prev[idx];}

static NewState step_one(const Params&P,const Population&pop,const std::vector<double>&prev,const std::vector<double>&fam,int i,int j,double target,const NewState&old,int override_idx=-1,double override_value=0.0){
    NewState z=old; const Agent&a=pop.agents[i];
    if(P.suspended_episodes && old.ep_left>0){
        const int f=a.family_id; const double iw=pop.inside_w[f],ow=pop.outside_w[f],tw=std::max(0.01,1.0-iw-ow),kap=P.two_axis?a.kappa:1.0;
        const double partner_x=prev_value(prev,j,override_idx,override_value);
        const double S=(tw*target+kap*(iw*fam[f]+ow*partner_x))/(tw+kap*(iw+ow));
        z.ep_left=1;z.ep_steps=old.ep_steps+1;z.ep_dose=old.ep_dose+(old.debt/std::max(1e-9,old.theta))/P.shock_mean_interval;
        z.x=old.x+P.track_frac*a.alpha*(S-old.x);z.debt=old.debt*(1.0-a.lam);
        if(z.debt<P.recover_frac*old.theta){double fd=z.ep_dose/std::max(1e-9,P.dose_ref);z.theta=std::min(P.theta_hi_mult*a.theta0_trait,std::max(P.theta_lo_mult*a.theta0_trait,old.theta*std::exp(-P.delta_dose*(fd-1.0))));z.ep_left=0;z.ep_dose=0;}
        return z;
    }
    const double S=social_signal(P,pop,prev,fam,i,j,target); const double d=S-prev_value(prev,i,override_idx,override_value); double alpha=a.alpha;
    const double xnew=prev_value(prev,i,override_idx,override_value)+alpha*d; const double inflow=(1.0-alpha)*std::max(0.0,std::fabs(d)-P.debt_deadband); const double enew=P.gamma*old.debt+inflow;
    z.x=xnew;z.debt=enew;z.entered=(P.suspended_episodes&&enew>=old.theta);if(z.entered){z.ep_left=1;z.ep_dose=0;z.n_collapse=old.n_collapse+1;} else z.ep_left=old.ep_left; return z;
}

static Metrics matched_run(const Params&P,unsigned seed,bool simultaneous){
    const int N=P.n_families*P.family_size; if(N%2)throw std::runtime_error("N must be even");
    Population pop=build_population(P,seed); std::vector<double>T=random_schedule(P,seed+777013u);
    const int burn=int(P.burn_in_frac*P.max_steps), sym_start=std::max(0,P.max_steps-std::max(1,int(P.symptom_window*P.shock_mean_interval)));
    long long cat[3]={0,0,0}, total=0,r1=0,r5=0,r20=0,mut=0,samefam=0;
    std::vector<int>last(N,-1), ring(size_t(N)*20,-1), rlen(N,0); std::vector<double>prev(N),fam(P.n_families); std::vector<NewState>old(N),nw(N);
    OnlineCorr prex,preq,postx,postq;
    for(int t=0;t<P.max_steps;++t){
        std::fill(fam.begin(),fam.end(),0.0);
        for(int i=0;i<N;++i){prev[i]=pop.agents[i].x;fam[pop.agents[i].family_id]+=prev[i];}
        for(double&z:fam)z/=P.family_size;
        for(int i=0;i<N;++i){const Agent&a=pop.agents[i];old[i]={a.x,a.debt,a.theta,a.ep_dose,a.ep_steps,a.n_collapse,a.ep_left,false};nw[i]=old[i];}
        auto pairs=matching(N,seed,t);
        for(const auto&p:pairs){
            const int i=int(p.a),j=int(p.b); total+=2;
            auto rec=[&](int u,int v){int n=rlen[u],base=u*20;if(last[u]==v)r1++;int hit5=0,hit20=0;for(int q=0;q<n;q++){if(ring[base+q]==v){hit20=1;if(q>=std::max(0,n-5))hit5=1;break;}}r5+=hit5;r20+=hit20;if(n<20){ring[base+n]=v;rlen[u]=n+1;}else{for(int q=0;q<19;q++)ring[base+q]=ring[base+q+1];ring[base+19]=v;}last[u]=v;};
            rec(i,j); rec(j,i);
            const double qi=old[i].debt/std::max(1e-9,old[i].theta), qj=old[j].debt/std::max(1e-9,old[j].theta);if(qi>=1&&qj>=1)mut++;if(pop.agents[i].family_id==pop.agents[j].family_id)samefam+=2;
            prex.add(prev[i],prev[j]);preq.add(qi,qj);
            const NewState ni=step_one(P,pop,prev,fam,i,j,T[t],old[i]);
            NewState nj;
            if(simultaneous){nj=step_one(P,pop,prev,fam,j,i,T[t],old[j]);nw[i]=ni;nw[j]=nj;}
            else{nw[i]=ni; nj=step_one(P,pop,prev,fam,j,i,T[t],old[j],i,ni.x);nw[j]=nj;}
            postx.add(nw[i].x,nw[j].x);postq.add(nw[i].debt/std::max(1e-9,nw[i].theta),nw[j].debt/std::max(1e-9,nw[j].theta));
        }
        for(int i=0;i<N;++i){Agent&a=pop.agents[i];const NewState&z=nw[i];a.x=z.x;a.debt=z.debt;a.theta=z.theta;a.ep_left=z.ep_left;a.ep_steps=z.ep_steps;a.ep_dose=z.ep_dose;a.n_collapse=z.n_collapse;if(z.entered&&t>=burn)cat[a.style]++;if(t>=sym_start){a.sym_debt+=a.debt/std::max(1e-9,a.theta);a.sym_abs+=a.debt;a.sym_n++;}}
    }
    std::vector<double>A,V,S;A.reserve(N);V.reserve(N);S.reserve(N);int cnt[3]={0,0,0};for(const auto&a:pop.agents){cnt[a.style]++;A.push_back(a.anx);V.push_back(a.avo);S.push_back(a.sym_n?a.sym_debt/a.sym_n:0.0);}finite_vec(A,"A");finite_vec(V,"V");finite_vec(S,"S");Metrics m;double rates[3]={0,0,0};for(int s=0;s<3;++s)rates[s]=cnt[s]?double(cat[s])/cnt[s]:0;m.anx_or=rates[2]/std::max(1e-12,rates[0]);m.av_or=rates[1]/std::max(1e-12,rates[0]);m.rate=rates[0]/((double)(P.max_steps-burn)/P.shock_mean_interval);m.rA=pearson(A,S);m.rV=pearson(V,S);m.pair_x_corr=prex.value();m.pair_q_corr=preq.value();m.pair_x_corr_post=postx.value();m.pair_q_corr_post=postq.value();m.mutual_elev=double(mut)/std::max(1LL,1LL*P.max_steps*(N/2));m.repeat1=double(r1)/total;m.repeat5=double(r5)/total;m.repeat20=double(r20)/total;m.instant_recip=1.0;m.unique_target_frac=1.0;m.same_family=double(samefam)/total;return m;
}

struct Stat{double mean=0,sd=0,se=0,lo=0,hi=0,p=0,dz=0;};
static Stat stats(const std::vector<double>&d){Stat s;if(d.empty())return s;s.mean=std::accumulate(d.begin(),d.end(),0.0)/d.size();double ss=0;for(double x:d)ss+=(x-s.mean)*(x-s.mean);s.sd=std::sqrt(ss/std::max<size_t>(1,d.size()-1));s.se=s.sd/std::sqrt(double(d.size()));s.lo=s.mean-1.96*s.se;s.hi=s.mean+1.96*s.se;std::mt19937 g(0xDADA5629u);int ge=0;for(int b=0;b<20000;b++){double x=0;for(double v:d)x+=(g()&1)?v:-v;x/=d.size();if(std::fabs(x)>=std::fabs(s.mean))ge++;}s.p=(1.0+ge)/20001.0;s.dz=s.sd>0?s.mean/s.sd:0;return s;}

static void write_results(const std::vector<unsigned>&seeds,const std::vector<Metrics>&seq,const std::vector<Metrics>&sym,const std::vector<Metrics>&green){
    std::ofstream ng("results/network_structure.csv"); ng<<"seed,green_y_repeat1,green_y_repeat5,green_y_repeat20,green_y_self_loop,green_y_instant_recip,green_y_unique_target_frac,green_y_same_family,matched_repeat1,matched_repeat5,matched_repeat20,matched_instant_recip,matched_unique_target_frac,matched_same_family\n";
    std::ofstream o("results/paired.csv");o<<"seed,rA_seq,rA_sym,drA,rV_seq,rV_sym,drV,rate_seq,rate_sym,drate,pair_x_corr_pre_seq,pair_x_corr_pre_sym,pair_q_corr_pre_seq,pair_q_corr_pre_sym,pair_x_corr_post_seq,pair_x_corr_post_sym,pair_q_corr_post_seq,pair_q_corr_post_sym,mutual_seq,mutual_sym,repeat1,repeat5,repeat20,self_loop,instant_recip,unique_target_frac,same_family,greeny_repeat1,greeny_repeat5,greeny_repeat20,greeny_self_loop,greeny_instant_recip,greeny_unique_target_frac,greeny_same_family\n";
    for(size_t i=0;i<seeds.size();++i){ ng<<seeds[i]<<","<<green[i].repeat1<<","<<green[i].repeat5<<","<<green[i].repeat20<<","<<green[i].self_loop<<","<<green[i].instant_recip<<","<<green[i].unique_target_frac<<","<<green[i].same_family<<","<<seq[i].repeat1<<","<<seq[i].repeat5<<","<<seq[i].repeat20<<","<<seq[i].instant_recip<<","<<seq[i].unique_target_frac<<","<<seq[i].same_family<<"\n"; o<<seeds[i]<<","<<seq[i].rA<<","<<sym[i].rA<<","<<sym[i].rA-seq[i].rA<<","<<seq[i].rV<<","<<sym[i].rV<<","<<sym[i].rV-seq[i].rV<<","<<seq[i].rate<<","<<sym[i].rate<<","<<sym[i].rate-seq[i].rate<<","<<seq[i].pair_x_corr<<","<<sym[i].pair_x_corr<<","<<seq[i].pair_q_corr<<","<<sym[i].pair_q_corr<<","<<seq[i].pair_x_corr_post<<","<<sym[i].pair_x_corr_post<<","<<seq[i].pair_q_corr_post<<","<<sym[i].pair_q_corr_post<<","<<seq[i].mutual_elev<<","<<sym[i].mutual_elev<<","<<seq[i].repeat1<<","<<seq[i].repeat5<<","<<seq[i].repeat20<<","<<seq[i].self_loop<<","<<seq[i].instant_recip<<","<<seq[i].unique_target_frac<<","<<seq[i].same_family<<","<<green[i].repeat1<<","<<green[i].repeat5<<","<<green[i].repeat20<<","<<green[i].self_loop<<","<<green[i].instant_recip<<","<<green[i].unique_target_frac<<","<<green[i].same_family<<"\n"; }
    std::vector<double>a,b,c;for(size_t i=0;i<seq.size();++i){a.push_back(sym[i].rA-seq[i].rA);b.push_back(sym[i].rV-seq[i].rV);c.push_back(sym[i].rate-seq[i].rate);}Stat sa=stats(a),sb=stats(b),sc=stats(c);std::ofstream p("results/paired_statistics.csv");p<<"metric,mean,sd,se,ci_lo,ci_hi,paired_permutation_p,d_z\n";p<<"rA,"<<sa.mean<<","<<sa.sd<<","<<sa.se<<","<<sa.lo<<","<<sa.hi<<","<<sa.p<<","<<sa.dz<<"\n";p<<"rV,"<<sb.mean<<","<<sb.sd<<","<<sb.se<<","<<sb.lo<<","<<sb.hi<<","<<sb.p<<","<<sb.dz<<"\n";p<<"rate,"<<sc.mean<<","<<sc.sd<<","<<sc.se<<","<<sc.lo<<","<<sc.hi<<","<<sc.p<<","<<sc.dz<<"\n";
    std::ofstream q("results/summary.csv"); q<<"condition,rA,rV,rate,pair_x_corr,pair_q_corr,mutual_elevation,repeat1,repeat5,repeat20\n";
    double ra1=0,ra2=0,rv1=0,rv2=0,rt1=0,rt2=0,px1=0,px2=0,pq1=0,pq2=0,mu1=0,mu2=0,r1=0,r5=0,r20=0;
    for(size_t i=0;i<seq.size();++i){ra1+=seq[i].rA;ra2+=sym[i].rA;rv1+=seq[i].rV;rv2+=sym[i].rV;rt1+=seq[i].rate;rt2+=sym[i].rate;px1+=seq[i].pair_x_corr;px2+=sym[i].pair_x_corr;pq1+=seq[i].pair_q_corr;pq2+=sym[i].pair_q_corr;mu1+=seq[i].mutual_elev;mu2+=sym[i].mutual_elev;r1+=seq[i].repeat1;r5+=seq[i].repeat5;r20+=seq[i].repeat20;}
    double nn=double(seq.size()); q<<"matched-bidir-sequential,"<<ra1/nn<<","<<rv1/nn<<","<<rt1/nn<<","<<px1/nn<<","<<pq1/nn<<","<<mu1/nn<<","<<r1/nn<<","<<r5/nn<<","<<r20/nn<<"\n"; q<<"matched-symmetric-simultaneous,"<<ra2/nn<<","<<rv2/nn<<","<<rt2/nn<<","<<px2/nn<<","<<pq2/nn<<","<<mu2/nn<<","<<r1/nn<<","<<r5/nn<<","<<r20/nn<<"\n";
}

static int run(int seeds,int threads,int families,int steps,int totalN){
    if(seeds<1||threads<1||families<1||steps<1||totalN<2) throw std::runtime_error("dimensions must be positive");
    if(totalN % families != 0) throw std::runtime_error("totalN must be divisible by families");
    const int family_size = totalN / families;
    if(family_size < 1) throw std::runtime_error("family size must be positive");
    Params P=frozen();P.n_families=families;P.family_size=family_size;P.max_steps=steps;const int N=totalN;if(N%2)throw std::runtime_error("N must be even");
    std::vector<unsigned>sv(seeds);for(int k=0;k<seeds;k++)sv[k]=P.seed0+101u*k;std::vector<Metrics>seq(seeds),sym(seeds),green(seeds);
#ifdef _OPENMP
    omp_set_num_threads(threads);
#pragma omp parallel for schedule(static)
#endif
    for(int k=0;k<seeds;k++){unsigned s=sv[k];seq[k]=matched_run(P,s,false);sym[k]=matched_run(P,s,true);green[k]=greeny_graph_probe(P,s);}
    write_results(sv,seq,sym,green);
    double ra1=0,ra2=0,rv1=0,rv2=0,rt1=0,rt2=0,m1=0,m2=0,g1=0,g2=0,g3=0;for(int i=0;i<seeds;i++){ra1+=seq[i].rA;ra2+=sym[i].rA;rv1+=seq[i].rV;rv2+=sym[i].rV;rt1+=seq[i].rate;rt2+=sym[i].rate;m1+=seq[i].mutual_elev;m2+=sym[i].mutual_elev;g1+=green[i].repeat1;g2+=green[i].repeat5;g3+=green[i].repeat20;}double n=seeds;
    std::cout<<"SYMMETRIC DYAD NETWORK EXPERIMENT seeds="<<seeds<<" threads="<<threads<<" N="<<N<<" steps="<<steps<<" families="<<families<<" family_size="<<family_size<<"\n";
    std::cout<<std::fixed<<std::setprecision(6)<<"MATCHED-BIDIR-SEQUENTIAL rA="<<ra1/n<<" rV="<<rv1/n<<" rate="<<rt1/n<<" mutual_elev="<<m1/n<<"\n";
    std::cout<<"MATCHED-SYMMETRIC-SIMULTANEOUS rA="<<ra2/n<<" rV="<<rv2/n<<" rate="<<rt2/n<<" mutual_elev="<<m2/n<<"\n";
    std::cout<<"GREENY graph probe repeat1="<<g1/n<<" repeat5="<<g2/n<<" repeat20="<<g3/n<<"\n";
    return 0;
}

} // namespace

int main(int argc,char**argv){try{std::string mode=argc>1?argv[1]:"smoke";int seeds=argc>2?std::stoi(argv[2]):1,threads=argc>3?std::stoi(argv[3]):1,families=argc>4?std::stoi(argv[4]):10,steps=argc>5?std::stoi(argv[5]):120,totalN=argc>6?std::stoi(argv[6]):40;if(mode=="verify-ledger"){const int N=40;for(unsigned s=1;s<=3;s++)for(int t=0;t<20;t++){auto a=matching(N,s,t),b=matching(N,s,t);if(a!=b)throw std::runtime_error("ledger non-determinism");std::vector<int>seen(N);for(auto p:a){if(p.a>=unsigned(N)||p.b>=unsigned(N)||p.a==p.b)throw std::runtime_error("invalid pair");seen[p.a]++;seen[p.b]++;}for(int x:seen)if(x!=1)throw std::runtime_error("not a perfect matching");}std::cout<<"LEDGER INVARIANTS PASS\n";return 0;}if(mode=="smoke"||mode=="compare")return run(seeds,threads,families,steps,totalN);throw std::runtime_error("mode smoke|compare|verify-ledger");}catch(const std::exception&e){std::cerr<<"ERROR: "<<e.what()<<"\n";return 2;}}
