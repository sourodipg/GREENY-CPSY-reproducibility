
#include "../code/network_kernels.hpp"
#include <iostream>
int main(){
    long long complement=0, disagree=0, total=0;
    for(unsigned seed=0; seed<128; ++seed){
        for(int t=0; t<2048; ++t){
            const int i=17,j=43;
            int u=greeny_network::directed_recipient(seed,t,i,j);
            int v=greeny_network::directed_recipient(seed,t,j,i);
            ++total;
            // complement means the physical recipient selected after argument
            // reversal is the same physical agent as before reversal.
            if((u==i && v==i) || (u==j && v==j)) ++complement;
            else ++disagree;
        }
    }
    std::cout << "RECIPIENT_ORDER_CHECK total="<<total
              << " same_physical_recipient="<<complement
              << " different_physical_recipient="<<disagree
              << " same_rate="<<double(complement)/double(total)<<"\n";
    return 0;
}
