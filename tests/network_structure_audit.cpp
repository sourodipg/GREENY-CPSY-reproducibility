#include "../code/network_kernels.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <vector>

int main() {
    // A. One-sided recipient symmetry: the deterministic selector must not
    // prefer the first or second pair member over a large audit set.
    long long left=0,right=0;
    for(unsigned seed=0; seed<128; ++seed){
        for(int t=0; t<2048; ++t){
            const int i=17, j=43;
            const int u=greeny_network::directed_recipient(seed,t,i,j);
            if(u==i) ++left;
            else if(u==j) ++right;
            else throw std::runtime_error("invalid directed recipient");
        }
    }
    const double p_left=double(left)/double(left+right);
    if(std::fabs(p_left-0.5) > 0.01)
        throw std::runtime_error("directed recipient imbalance exceeds 1 percentage point");

    // B. The metric construction depends on rank(z), not the numeric scale of z.
    constexpr int N=4000;
    auto z=greeny_network::social_latent(N,12345u);
    std::vector<int> a(N),b(N);
    std::iota(a.begin(),a.end(),0);
    std::iota(b.begin(),b.end(),0);
    std::sort(a.begin(),a.end(),[&](int i,int j){return z[i]<z[j] || (z[i]==z[j]&&i<j);});
    std::sort(b.begin(),b.end(),[&](int i,int j){const double zi=z[i]*z[i]*z[i],zj=z[j]*z[j]*z[j];return zi<zj || (zi==zj&&i<j);});
    if(a!=b) throw std::runtime_error("strictly monotone coordinate transform changed the rank ordering");
    for(int t: {0,1,2,17,100,999}){
        auto pa=greeny_network::metric_matching(a,777u,t);
        auto pb=greeny_network::metric_matching(b,777u,t);
        if(pa!=pb) throw std::runtime_error("monotone coordinate transform changed metric ledger");
    }

    // C. Direct recurrence audit: the local rule itself should generate the
    // advertised strong recurrence without running the dynamical model.
    const int T=4000;
    std::vector<int> last(N,-1);
    std::vector<std::vector<int>> hist(N);
    long long n=0,r1=0,r5=0,r20=0;
    std::vector<int> ord=a;
    for(int t=0;t<T;++t){
        auto pairs=greeny_network::metric_matching(ord,777u,t);
        for(const auto& p:pairs){
            auto visit=[&](int u,int v){
                ++n;
                if(last[u]==v) ++r1;
                int h=std::min<int>(20,hist[u].size());
                for(int q=1;q<=5 && q<=h;++q) if(hist[u][hist[u].size()-q]==v){++r5;break;}
                for(int q=1;q<=20 && q<=h;++q) if(hist[u][hist[u].size()-q]==v){++r20;break;}
                last[u]=v;
                hist[u].push_back(v);
                if(hist[u].size()>20) hist[u].erase(hist[u].begin());
            };
            visit(p.a,p.b); visit(p.b,p.a);
        }
    }
    const double rr1=double(r1)/n, rr5=double(r5)/n, rr20=double(r20)/n;
    if(!(rr1>0.49 && rr1<0.51)) throw std::runtime_error("metric repeat1 is outside the expected local-pair regime");
    if(!(rr5>0.95)) throw std::runtime_error("metric repeat5 is unexpectedly weak");
    if(!(rr20>0.99)) throw std::runtime_error("metric repeat20 is unexpectedly weak");

    std::cout << "STRUCTURE AUDIT PASS\n";
    std::cout << "DIRECTED_RECIPIENT_COUNTS left="<<left<<" right="<<right<<" p_left="<<p_left<<"\n";
    std::cout << "LATENT_RANK_INVARIANCE PASS transform=z^3\n";
    std::cout << "METRIC_RECURRENCE_DIRECT repeat1="<<rr1<<" repeat5="<<rr5<<" repeat20="<<rr20<<"\n";
    return 0;
}
