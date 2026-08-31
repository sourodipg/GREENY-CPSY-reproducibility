// BranchN extension only. The frozen GREENY source is included verbatim so that
// population construction, parameter values, and coordinate generation come
// from the same implementation used by the frozen executable. No production
// file is modified.
#define main greeny_frozen_main
#include "../../../../code/symmetric_network_matrix.cpp"
#undef main

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

namespace {

double pearson(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2) return 0.0;
    const double n = static_cast<double>(x.size());
    double sx=0, sy=0, sxx=0, syy=0, sxy=0;
    for (size_t i=0;i<x.size();++i) {
        sx += x[i]; sy += y[i]; sxx += x[i]*x[i]; syy += y[i]*y[i]; sxy += x[i]*y[i];
    }
    const double vx=sxx/n-(sx/n)*(sx/n), vy=syy/n-(sy/n)*(sy/n);
    const double c=sxy/n-(sx/n)*(sy/n);
    const double d=std::sqrt(std::max(0.0,vx*vy));
    return d>0 ? c/d : 0.0;
}

double covariance(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.size() < 2) return 0.0;
    const double n=static_cast<double>(x.size());
    double sx=0,sy=0,sxy=0;
    for(size_t i=0;i<x.size();++i){sx+=x[i];sy+=y[i];sxy+=x[i]*y[i];}
    return sxy/n-(sx/n)*(sy/n);
}

std::vector<int> rank_order_from_z(const std::vector<double>& z) {
    std::vector<int> ord(z.size());
    std::iota(ord.begin(), ord.end(), 0);
    std::sort(ord.begin(), ord.end(), [&](int a,int b){ return z[a]<z[b] || (z[a]==z[b] && a<b); });
    return ord;
}

struct Summary {
    double mean_local_A=0, mean_local_V=0;
    double local_A_assort=0, local_V_assort=0;
    double local_A_to_V=0, local_V_to_A=0;
    double local_pair_cov_A=0, local_pair_cov_V=0;
    double local_cross_cov_AV=0;
    double mean_local_within_cov_AV=0;
    double mean_rank_gap=0;
};

Summary compute(unsigned seed, int offset) {
    Params P=frozen();
    P.n_families=1000; P.family_size=4; P.max_steps=12000; P.shock_mean_interval=500.0; P.symptom_window=0.06;
    const int N=P.n_families*P.family_size;
    Population pop=build_population(P,seed);
    const std::vector<double> z=greeny_network::social_latent(N,unsigned(seed + offset));
    const std::vector<int> ord=rank_order_from_z(z);
    std::vector<int> rank(N,-1); for(int r=0;r<N;++r) rank[ord[r]]=r;

    std::vector<double>A(N),V(N),localA(N),localV(N),crossAtoV(N),crossVtoA(N);
    std::vector<double> pairA_i,pairA_j,pairV_i,pairV_j,cross1,cross2;
    pairA_i.reserve(N);pairA_j.reserve(N);pairV_i.reserve(N);pairV_j.reserve(N);cross1.reserve(N);cross2.reserve(N);
    double gap_sum=0;
    for(int i=0;i<N;++i){
        A[i]=pop.agents[i].anx; V[i]=pop.agents[i].avo;
        const int r=rank[i];
        const int il=ord[(r-1+N)%N], ir=ord[(r+1)%N];
        localA[i]=0.5*(A[il]+A[ir]);
        localV[i]=0.5*(V[il]+V[ir]);
        crossAtoV[i]=0.5*(V[il]+V[ir]);
        crossVtoA[i]=0.5*(A[il]+A[ir]);
        // Edge endpoint values for local assortativity/covariance.
        pairA_i.push_back(A[i]); pairA_j.push_back(0.5*(A[il]+A[ir]));
        pairV_i.push_back(V[i]); pairV_j.push_back(0.5*(V[il]+V[ir]));
        cross1.push_back(A[i]); cross2.push_back(0.5*(V[il]+V[ir]));
        gap_sum += std::fabs(z[i]-z[il]) + std::fabs(z[i]-z[ir]);
    }

    // Local neighborhood trait covariance is computed from the three trait
    // points {i, left-neighbor, right-neighbor} and then averaged over agents.
    double within_cov_sum=0;
    for(int i=0;i<N;++i){
        const int r=rank[i]; const int il=ord[(r-1+N)%N], ir=ord[(r+1)%N];
        const double ma=(A[i]+A[il]+A[ir])/3.0, mv=(V[i]+V[il]+V[ir])/3.0;
        within_cov_sum += ((A[i]-ma)*(V[i]-mv)+(A[il]-ma)*(V[il]-mv)+(A[ir]-ma)*(V[ir]-mv))/3.0;
    }
    Summary s;
    for(int i=0;i<N;++i){s.mean_local_A+=localA[i];s.mean_local_V+=localV[i];}
    s.mean_local_A/=N;s.mean_local_V/=N;
    s.local_A_assort=pearson(A,localA);
    s.local_V_assort=pearson(V,localV);
    s.local_A_to_V=pearson(A,localV);
    s.local_V_to_A=pearson(V,localA);
    s.local_pair_cov_A=covariance(pairA_i,pairA_j);
    s.local_pair_cov_V=covariance(pairV_i,pairV_j);
    s.local_cross_cov_AV=covariance(cross1,cross2);
    s.mean_local_within_cov_AV=within_cov_sum/N;
    s.mean_rank_gap=gap_sum/(2.0*N);
    return s;
}

}

int main(int argc,char**argv){
    try {
        if(argc<3) throw std::runtime_error("usage: neighborhood_trait_probe SEED OFFSET");
        const unsigned seed=static_cast<unsigned>(std::stoul(argv[1]));
        const int offset=std::stoi(argv[2]);
        const auto s=compute(seed,offset);
        std::cout<<std::setprecision(17)
                 <<seed<<","<<offset<<","<<s.mean_local_A<<","<<s.mean_local_V<<","<<s.local_A_assort<<","<<s.local_V_assort<<","<<s.local_A_to_V<<","<<s.local_V_to_A<<","<<s.local_pair_cov_A<<","<<s.local_pair_cov_V<<","<<s.local_cross_cov_AV<<","<<s.mean_local_within_cov_AV<<","<<s.mean_rank_gap<<"\n";
        return 0;
    } catch(const std::exception& e){ std::cerr<<"ERROR: "<<e.what()<<"\n"; return 2; }
}
