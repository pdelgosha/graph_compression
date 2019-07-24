#include "compression_helper.h"

mpz_class helper_vars::mul_1;
mpz_class helper_vars::mul_2;
vector<mpz_class> helper_vars::return_stack;
vector<mpz_class> helper_vars::mpz_vec;

mpz_class compute_product_old(int N, int k, int s){
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

  if (k == 2)
    return mpz_class(N) * mpz_class(N-s);
  // we do this by dividing the terms into two parts
  int m = k / 2; // the middle point
  mpz_class left, right; // each of the half products 
  left = compute_product(N, m, s);
  right = compute_product(N-m * s, k-m, s);
  //logger::item_start("cp_mul");
  mpz_class ans = left*right;
  //logger::item_stop("cp_mul");
  return ans;
}

mpz_class compute_product_stack(int N, int k, int s){

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

  if (k == 2){
    helper_vars::mul_1 = N;
    helper_vars::mul_2 = N - s;
    return helper_vars::mul_1 * helper_vars::mul_2;
  }

  logger::item_start("CP body");

  int k_bits = 0; // roughly , the number of bits in k, the depth of the stack during run time
  int k_copy = k;
  while (k_copy > 0){
    k_bits ++;
    k_copy >>= 1;
  }
  k_bits += 2; 
  vector<pair<int, int> > call_stack(2 * k_bits);
  //cout << " 2 * k_bits " << 2 * k_bits << endl; 
  int call_pointer = 0; // size of the call pointer, so the top index is call_pointer - 1
  vector<int> status_stack(2 * k_bits); // 0: first meet, 1: to return 
  //vector<mpz_class> return_stack(2 * k_bits);
  helper_vars::return_stack.resize(2*k_bits);
  int return_pointer = 0;

  call_stack[call_pointer] = pair<int, int> (N, k);
  status_stack[call_pointer] = 0;
  call_pointer ++;

  int m;
  int N_now, k_now; // N and k for the current stack element
  
  while (call_pointer > 0){
    N_now = call_stack[call_pointer-1].first;
    k_now = call_stack[call_pointer-1].second;
    //cout << "call_pointer = " << call_pointer << " N = " << N_now << " k = " << k_now << " stat = " << status_stack[call_pointer-1] << endl;
    //cout << " the whole stack " << endl;
    //for (int i=0;i<call_pointer; i++){
    //  cout << call_stack[i].first << " , " << call_stack[i].second << " " << status_stack[i] << endl;
    //}
    if (status_stack[call_pointer-1] == 1){ // we should multiply two top elements in the return stack
      // to collect two top elements in return stack and multiply them
      logger::item_start("CP arithmetic");
      helper_vars::return_stack[return_pointer-2] = helper_vars::return_stack[return_pointer-2] * helper_vars::return_stack[return_pointer-1];
      logger::item_stop("CP arithmetic");
      return_pointer--; // remove two items, add one item
      call_pointer --;
    }else{
      //cout << " else " << endl;
      if(k_now == 1){
        // to return the corresponding N
        helper_vars::return_stack[return_pointer++] = call_stack[call_pointer-1].first;
        call_pointer --; // pop this element
      }
      if (k_now == 2){
        helper_vars::mul_1 = N_now;
        helper_vars::mul_2 = N_now - s;
        logger::item_start("CP arithmetic");
        helper_vars::return_stack[return_pointer++] = helper_vars::mul_1 * helper_vars::mul_2;
        logger::item_stop("CP arithmetic");
        call_pointer --; 
      }
      if (k_now > 2){
        m = k_now / 2;
        status_stack[call_pointer-1] = 1; // when return to this state, we know that we should aggregate 
        call_stack[call_pointer] = pair<int, int>(N_now, m);
        status_stack[call_pointer] = 0; // just added
        call_stack[call_pointer+1] = pair<int, int>(N_now - m*s, k_now - m );
        status_stack[call_pointer+1]  = 0;
        call_pointer += 2; 
      }
    }
  }
  // make sure there is exactly one element in return stack
  if (return_pointer != 1){
    cerr << " return pointer is not zero";
  }
  logger::item_stop("CP body");
  return helper_vars::return_stack[0]; // the top element remaining in the return stack 
}



void compute_product_void(int N, int k, int s){
  //cerr << " void called N " << N << " k " << k << " s " << s << endl;
  if (k==1){
    helper_vars::return_stack.resize(1);
    helper_vars::return_stack[0] = N;//return N;
    return;
  }
  if (k == 0){ // TO CHECK because there are no terms to compute product
    helper_vars::return_stack.resize(1);
    helper_vars::return_stack[0] = 1;//return 1;
    return;
  }

  if (k < 0){
    cerr << " WARNING: compute_product called for k < 0, returning 1, N  " << N << " k " << k << " s " << s << endl;
    helper_vars::return_stack.resize(1);
    helper_vars::return_stack[0] = 1;//return 1;
    return;
  }
  if (N - (k-1) * s <= 0){ // the terms go negative
    //cerr << " WARNING: compute_product called for N - (k-1) * s <= 0 " << endl;
    helper_vars::return_stack.resize(1);
    helper_vars::return_stack[0] = 0; //return 0;
    return;
  }

  if (k == 2){
    helper_vars::mul_1 = N;
    helper_vars::mul_2 = N - s;
    helper_vars::return_stack.resize(1);
    helper_vars::return_stack[0] = helper_vars::mul_1 * helper_vars::mul_2;
    return;
  }

  int k_bits = 0; // roughly , the number of bits in k, the depth of the stack during run time
  int k_copy = k;
  while (k_copy > 0){
    k_bits ++;
    k_copy >>= 1;
  }
  k_bits += 2; 
  vector<pair<int, int> > call_stack(2 * k_bits);
  //cout << " 2 * k_bits " << 2 * k_bits << endl; 
  int call_pointer = 0; // size of the call pointer, so the top index is call_pointer - 1
  vector<int> status_stack(2 * k_bits); // 0: first meet, 1: to return 
  //vector<mpz_class> return_stack(2 * k_bits);
  helper_vars::return_stack.resize(2*k_bits);
  int return_pointer = 0;

  call_stack[call_pointer] = pair<int, int> (N, k);
  status_stack[call_pointer] = 0;
  call_pointer ++;

  int m;
  int N_now, k_now; // N and k for the current stack element
  
  while (call_pointer > 0){
    N_now = call_stack[call_pointer-1].first;
    k_now = call_stack[call_pointer-1].second;
    //cout << "call_pointer = " << call_pointer << " N = " << N_now << " k = " << k_now << " stat = " << status_stack[call_pointer-1] << endl;
    //cout << " the whole stack " << endl;
    //for (int i=0;i<call_pointer; i++){
    //  cout << call_stack[i].first << " , " << call_stack[i].second << " " << status_stack[i] << endl;
    //}
    if (status_stack[call_pointer-1] == 1){ // we should multiply two top elements in the return stack
      // to collect two top elements in return stack and multiply them
      helper_vars::return_stack[return_pointer-2] = helper_vars::return_stack[return_pointer-2] * helper_vars::return_stack[return_pointer-1];
      return_pointer--; // remove two items, add one item
      call_pointer --;
    }else{
      //cout << " else " << endl;
      if(k_now == 1){
        // to return the corresponding N
        helper_vars::return_stack[return_pointer++] = call_stack[call_pointer-1].first;
        call_pointer --; // pop this element
      }
      if (k_now == 2){
        helper_vars::mul_1 = N_now;
        helper_vars::mul_2 = N_now - s;
        helper_vars::return_stack[return_pointer++] = helper_vars::mul_1 * helper_vars::mul_2;
        call_pointer --; 
      }
      if (k_now > 2){
        m = k_now / 2;
        status_stack[call_pointer-1] = 1; // when return to this state, we know that we should aggregate 
        call_stack[call_pointer] = pair<int, int>(N_now, m);
        status_stack[call_pointer] = 0; // just added
        call_stack[call_pointer+1] = pair<int, int>(N_now - m*s, k_now - m );
        status_stack[call_pointer+1]  = 0;
        call_pointer += 2; 
      }
    }
  }
  // make sure there is exactly one element in return stack
  if (return_pointer != 1){
    cerr << " return pointer is not zero";
  }
  //return helper_vars::return_stack[0]; // the top element remaining in the return stack 
}

mpz_class compute_product(int N, int k, int s){
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

  if (k == 2){
    helper_vars::mul_1 = N;
    helper_vars::mul_2 = N - s;
    return helper_vars::mul_1 * helper_vars::mul_2;
  }

  helper_vars::mpz_vec.resize(k);
  for (int i=0;i<k;i++)
    helper_vars::mpz_vec[i] = N - i * s;

  int step_size, to_mul; 

  for (step_size = 2, to_mul = 1; to_mul < k; step_size <<=1, to_mul <<=1){
    for (int i=0; i<k; i+=step_size){
      if (i+to_mul < k)
        helper_vars::mpz_vec[i] *= helper_vars::mpz_vec[i+to_mul];
    }
  }
  return helper_vars::mpz_vec[0];
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
