#ifndef GREENY_NETWORK_KERNELS_HPP
#define GREENY_NETWORK_KERNELS_HPP

#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>
#include <numeric>

namespace greeny_network {

struct PairID { int a=-1, b=-1; friend bool operator==(const PairID& x,const PairID& y){return x.a==y.a&&x.b==y.b;} friend bool operator!=(const PairID& x,const PairID& y){return !(x==y);} };

inline uint64_t splitmix64(uint64_t x){
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

inline uint64_t key(uint64_t seed,int t,uint64_t stream=0){
    uint64_t x=seed ^ (0xD1B54A32D192ED03ULL + stream*0x9E3779B97F4A7C15ULL);
    x ^= uint64_t(t)*0x94D049BB133111EBULL;
    return splitmix64(x);
}

// One fixed exogenous coordinate per agent.  Only its induced rank ordering
// is used by metric-local matching.
inline std::vector<double> social_latent(int N, unsigned seed){
    std::mt19937 rng(seed + 0x6A09E667u);
    std::uniform_real_distribution<double> U(0.0,1.0);
    std::vector<double> z(N);
    for(double &v:z) v=U(rng);
    return z;
}

inline std::vector<PairID> random_matching(int N,unsigned seed,int t){
    if(N%2) throw std::runtime_error("N must be even");
    std::vector<int> v(N); std::iota(v.begin(),v.end(),0); uint64_t s=key(seed,t,11);
    for(int k=N-1;k>0;--k){s=splitmix64(s);int j=int(s%uint64_t(k+1));std::swap(v[k],v[j]);}
    std::vector<PairID> out;out.reserve(N/2);for(int q=0;q<N;q+=2)out.push_back({v[q],v[q+1]});return out;
}

inline std::vector<PairID> metric_matching(const std::vector<int>&ord,unsigned seed,int t){
    const int N=int(ord.size()); if(N%2) throw std::runtime_error("N must be even");
    const int shift=int(splitmix64(key(seed,t,17))%uint64_t(N));
    std::vector<PairID> out;out.reserve(N/2);
    for(int q=0;q<N;q+=2) out.push_back({ord[(q+shift)%N],ord[(q+1+shift)%N]});
    return out;
}

// D-condition designation.  The selector is deterministic given seed, time,
// and unordered pair identity, while remaining balanced between the members.
inline int directed_recipient(unsigned seed,int t,int i,int j){
    const uint64_t pair_key = uint64_t((uint32_t)((i+1)*2654435761u+(j+1)));
    const uint64_t rr=splitmix64(key(seed,t,31)^pair_key);
    return (rr&1ULL)?i:j;
}

} // namespace greeny_network

#endif
