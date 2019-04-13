#include "compression_helper.h"


mpz_class compute_product(int N, int k, int s){
  //cerr << " compute_product  N " << N << " k " << k << " s " << s << endl;

  if (k==1)
    return N;
  if (k == 0) // TO CHECK because there are no terms to compute product
    return 1;

  if (k < 0){
    cerr << " WARNING: compute_product called for k < 0, returning 1, N  " << N << " k " << k << " s " << s << endl;
    return 1;
  }
  if (N - (k-1) * s <= 0){ // the terms go negative
    //cerr << " WARNING: compute_product called for N - (k-1) * s <= 0 " << endl;
    return 0;
  }
  
  // we do this by dividing the terms into two parts
  int m = k / 2; // the middle point
  mpz_class left, right; // each of the half products 
  left = compute_product(N, m, s);
  right = compute_product(N-m * s, k-m, s);
  return left * right;
}


mpz_class binomial(const int n, const int m)
{
  if (n <= 0 or m > n or m <= 0)
    return 0;
  return compute_product(n, m, 1) / compute_product(m, m, 1);
}


mpz_class prod_factorial(const vector<int>& a, int i, int j)
{
  if (i == j){
    return compute_product(a[i], a[i], 1);
  }else{
    int k = (i+j)/2;
    mpz_class x = prod_factorial(a, i, k);
    mpz_class y = prod_factorial(a, k+1, j);
    return x * y;
  }
}
