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


// this is another way of implementing compute_product, which splits the terms even / odd. But I did not see much improvement with this!
/*
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
  int m = (k+1) / 2; // the middle point
  mpz_class left, right; // each of the half products 
  left = compute_product(N, m, 2*s);
  right = compute_product(N-s, k-m, 2*s);
  return left * right;
}
*/

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


void bit_string_write(FILE* f, const string& s){
  // find out the number of bytes
  int ssize = s.size();
  int nu_bytes = ssize / 8;

  if (ssize % 8 != 0) // an incomplete byte is required
    nu_bytes++;

  fwrite(&ssize, sizeof(ssize), 1, f); // first, write down how many bytes are coming.

  stringstream ss;
  ss << s;

  bitset<8> B;
  unsigned char c; 
  while (ss >> B){
    c = B.to_ulong();
    fwrite(&c, sizeof(c), 1, f);
  }
}


string bit_string_read(FILE* f){
  int nu_bytes;
  int ssize;
  // read the number of bytes to read
  fread(&ssize, sizeof(ssize), 1, f);
  //cerr << " ssize " << ssize << endl;
  nu_bytes = ssize / 8;
  if (ssize % 8 != 0)
    nu_bytes ++;

  int last_byte_size = ssize % 8;
  if (last_byte_size == 0)
    last_byte_size  = 8;

  unsigned char c;
  bitset<8> B;
  string s; 
  for (int i=0;i<nu_bytes;i++){
    fread(&c, sizeof(c), 1, f);
    B = c;
    //cout << B << endl;
    if (i < nu_bytes -1){
      s += B.to_string();
    }else{
      s += B.to_string().substr(8-last_byte_size, last_byte_size);
    }
  }
  return s;
}
