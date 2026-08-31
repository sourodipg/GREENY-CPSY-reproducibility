import sympy as sp
x_i,x_j,T,F_i,F_j,wi,wo,wt,ki,kj,ai,aj=sp.symbols("x_i x_j T F_i F_j w_i w_o w_t k_i k_j alpha_i alpha_j")
S_i=(wt*T+ki*(wi*F_i+wo*x_j))/(wt+ki*(wi+wo))
S_j=(wt*T+kj*(wi*F_j+wo*x_i))/(wt+kj*(wi+wo))
X_i=x_i+ai*(S_i-x_i); X_j=x_j+aj*(S_j-x_j)
sub={x_i:x_j,x_j:x_i,F_i:F_j,F_j:F_i,ki:kj,kj:ki,ai:aj,aj:ai}
# After swapping labels, the new first participant's output must equal the old second, and vice versa.
assert sp.simplify(X_i.xreplace(sub)-X_j)==0
assert sp.simplify(X_j.xreplace(sub)-X_i)==0
print("SYMBOLIC SWAP-EQUIVARIANCE PASS")
