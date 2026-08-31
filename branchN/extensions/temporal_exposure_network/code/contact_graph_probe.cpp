#define main greeny_original_main
#include "../../../../code/symmetric_network_matrix.cpp"
#undef main

#include <fstream>
#include <iomanip>
#include <numeric>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

using greeny_network::PairID;

static double pearson_unweighted(const std::vector<double>& x,const std::vector<double>& y){
    if(x.size()!=y.size()||x.size()<2) return std::numeric_limits<double>::quiet_NaN();
    double mx=std::accumulate(x.begin(),x.end(),0.0)/x.size(), my=std::accumulate(y.begin(),y.end(),0.0)/y.size();
    double a=0,b=0,c=0; for(size_t i=0;i<x.size();++i){double dx=x[i]-mx,dy=y[i]-my;a+=dx*dy;b+=dx*dx;c+=dy*dy;}
    return (b>0&&c>0)?a/std::sqrt(b*c):std::numeric_limits<double>::quiet_NaN();
}
static double pearson_weighted(const std::vector<double>& x,const std::vector<double>& y,const std::vector<double>& w){
    if(x.size()!=y.size()||x.size()!=w.size()||x.empty()) return std::numeric_limits<double>::quiet_NaN();
    double sw=std::accumulate(w.begin(),w.end(),0.0); if(sw<=0) return std::numeric_limits<double>::quiet_NaN();
    double mx=0,my=0; for(size_t i=0;i<x.size();++i){mx+=w[i]*x[i];my+=w[i]*y[i];} mx/=sw; my/=sw;
    double a=0,b=0,c=0; for(size_t i=0;i<x.size();++i){double dx=x[i]-mx,dy=y[i]-my;a+=w[i]*dx*dy;b+=w[i]*dx*dx;c+=w[i]*dy*dy;}
    return (b>0&&c>0)?a/std::sqrt(b*c):std::numeric_limits<double>::quiet_NaN();
}
static double mean_abs_diff(const std::vector<double>& x,const std::vector<double>& y){double s=0;for(size_t i=0;i<x.size();++i)s+=std::fabs(x[i]-y[i]);return x.empty()?0:s/x.size();}
static double weighted_mean_abs_diff(const std::vector<double>& x,const std::vector<double>& y,const std::vector<double>& w){double s=0,sw=0;for(size_t i=0;i<x.size();++i){s+=w[i]*std::fabs(x[i]-y[i]);sw+=w[i];}return sw? s/sw:0;}

int main(int argc,char**argv){
    try{
        if(argc<4) throw std::runtime_error("usage: contact_graph_probe <seed> <coordinate_offset> <steps> [N] [families]");
        const unsigned seed=static_cast<unsigned>(std::stoul(argv[1]));
        const int offset=std::stoi(argv[2]), steps=std::stoi(argv[3]);
        const int N=argc>4?std::stoi(argv[4]):4000, families=argc>5?std::stoi(argv[5]):1000;
        if(N<=0||N%2||families<=0||N%families) throw std::runtime_error("invalid N/families");
        Params P=frozen();P.n_families=families;P.family_size=N/families;
        auto pop=build_population(P,seed);
        std::vector<double> z=greeny_network::social_latent(N,unsigned(seed+g_coordinate_seed_offset+offset));
        std::vector<int> ord(N);std::iota(ord.begin(),ord.end(),0);std::sort(ord.begin(),ord.end(),[&](int a,int b){return z[a]<z[b] || (z[a]==z[b]&&a<b);});
        // Exact aggregate M graph: only shift parity matters for an even N.
        long long parity_count[2]={0,0};
        for(int t=0;t<steps;++t){int shift=int(greeny_network::splitmix64(greeny_network::key(seed,t,17))%uint64_t(N));++parity_count[shift&1];}
        struct Edge{int a,b;long long w;}; std::vector<Edge> edges;edges.reserve(N);
        const long long w0=parity_count[0], w1=parity_count[1];
        // union of the two alternating perfect matchings is the rank cycle.
        for(int q=0;q<N;++q){int a=ord[q], b=ord[(q+1)%N]; long long w=(q%2==0)?w0:w1; int lo=std::min(a,b),hi=std::max(a,b);edges.push_back({lo,hi,w});}
        std::sort(edges.begin(),edges.end(),[](const Edge&a,const Edge&b){return a.a<b.a||(a.a==b.a&&a.b<b.b);});
        std::vector<double>A(N),V(N);for(int i=0;i<N;++i){A[i]=pop.agents[i].anx;V[i]=pop.agents[i].avo;}
        std::vector<double> ea,eb,va,vb,ww; ea.reserve(edges.size());eb.reserve(edges.size());va.reserve(edges.size());vb.reserve(edges.size());ww.reserve(edges.size());
        std::vector<int> deg(N,0);std::vector<long long> strength(N,0);
        for(const auto&e:edges){ea.push_back(A[e.a]);eb.push_back(A[e.b]);va.push_back(V[e.a]);vb.push_back(V[e.b]);ww.push_back(double(e.w));deg[e.a]++;deg[e.b]++;strength[e.a]+=e.w;strength[e.b]+=e.w;}
        double dsum=0,d2=0,ssum=0,s2=0;for(int d:deg){dsum+=d;d2+=double(d)*d;}for(long long s:strength){ssum+=s;s2+=double(s)*s;}
        double md=dsum/N, sd=std::sqrt(std::max(0.0,d2/N-md*md)); double ms=ssum/N, ss=std::sqrt(std::max(0.0,s2/N-ms*ms));
        double ewmin=*std::min_element(ww.begin(),ww.end()),ewmax=*std::max_element(ww.begin(),ww.end()),ewmean=std::accumulate(ww.begin(),ww.end(),0.0)/ww.size();
        double ewsd=0;for(double w:ww)ewsd+=(w-ewmean)*(w-ewmean);ewsd=std::sqrt(ewsd/ww.size());
        double density=2.0*double(edges.size())/(double(N)*double(N-1));
        double rAu=pearson_unweighted(ea,eb),rVu=pearson_unweighted(va,vb),rAw=pearson_weighted(ea,eb,ww),rVw=pearson_weighted(va,vb,ww);
        double da=mean_abs_diff(ea,eb),dv=mean_abs_diff(va,vb),daw=weighted_mean_abs_diff(ea,eb,ww),dvw=weighted_mean_abs_diff(va,vb,ww);
        // For the cycle N>3, binary triangles do not exist.
        double clustering=0.0;
        std::cout<<std::setprecision(12)
                 <<seed<<","<<offset<<","<<N<<","<<steps<<","<<families<<","<<parity_count[0]<<","<<parity_count[1]<<","<<edges.size()<<","<<density<<","<<1<<","<<md<<","<<sd<<","<<ms<<","<<ss<<","<<ewmin<<","<<ewmax<<","<<ewmean<<","<<((ewmean>0)?ewsd/ewmean:0)<<","<<clustering<<","<<rAu<<","<<rVu<<","<<rAw<<","<<rVw<<","<<da<<","<<dv<<","<<daw<<","<<dvw<<"\n";
        return 0;
    }catch(const std::exception&e){std::cerr<<"ERROR: "<<e.what()<<"\n";return 2;}
}
