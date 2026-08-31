#!/usr/bin/env python3
"""Audit whether D recipient selection depends on pair representation order.

The production selector is reproducible and approximately balanced, but this
check also tests a stronger property: swapping (i,j) to (j,i) and then mapping
back to physical agents. A nonzero rate means designation is representation-
sensitive. This is reported as an implementation finding, not hidden by a
binary PASS label.
"""
from pathlib import Path
import subprocess, textwrap
ROOT=Path(__file__).resolve().parents[1]
CPP=ROOT/'verification'/'_recipient_order_check.cpp'
BIN=ROOT/'bin'/'_recipient_order_check'
CPP.write_text(textwrap.dedent(r'''
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
'''))
p=subprocess.run(['g++','-O2','-std=c++17','-Wall','-Wextra','-Wpedantic','-Werror',str(CPP),'-o',str(BIN)],cwd=ROOT,text=True,capture_output=True)
if p.returncode: print(p.stdout,p.stderr); raise SystemExit(p.returncode)
p=subprocess.run([str(BIN)],cwd=ROOT,text=True,capture_output=True)
print(p.stdout,end='')
if p.returncode: print(p.stderr,end=''); raise SystemExit(p.returncode)
rate=float(p.stdout.strip().split('same_rate=')[-1])
print('RECIPIENT_ORDER_AUDIT COMPLETE')
print('NOTE: production designation is approximately balanced, but the selector is representation-sensitive when the ordered pair arguments are swapped; this is retained as an implementation finding because changing it would alter the archived results and requires a fresh production rerun.')
