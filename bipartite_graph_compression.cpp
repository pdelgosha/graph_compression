#include "bipartite_graph_compression.h"


//==============================
// b_graph_encoder
//==============================

void b_graph_encoder::init()
{
  // initializing beta
  beta = G.get_right_degree_sequence();

  // initializing the Fenwick tree
  U = reverse_fenwick_tree(beta);

  //initializing a as the left degree vector
  a = G.get_left_degree_sequence();

  //initializing b as the right degree vector
  b = G.get_right_degree_sequence();
    
}



  pair<mpz_class, mpz_class> b_graph_encoder::compute_N(int i, int j)
  {
    mpz_class Nij = 0;
    mpz_class lij = 1;
    if (i == j){
      vector<int> x = G.get_adj_list(i); // the adjacency list of the vertex
      for (int k=0;k<a[i];k++){
        Nij += lij * binomial(U.sum(1+x[k]), a[i] - k);
        lij *= beta[x[k]];
        beta[x[k]] --;
        U.add(x[k],-1);
      }
      return pair<mpz_class, mpz_class> (Nij, lij);
    }else{
      int t = (i+j)/ 2;
      mpz_class Nit, lit; // for the left subinterval i, j
      mpz_class Ntj, ltj; // for the right subinterval t+1, j
      mpz_class Nij, lij; // to return
      int St; // S_{t+1}
      int Sj; // S_{j+1}

      pair<mpz_class, mpz_class> ans; // for collecting the results from subintervals

      // left subinterval
      ans = compute_N(i,t);
      Nit = ans.first;
      lit = ans.second;
      St = U.sum(0);

      // right subinterval
      ans = compute_N(t+1, j);
      Ntj = ans.first;
      ltj = ans.second;
      Sj = U.sum(0);

      mpz_class rtj; // r_{t+1, j}
      mpz_class prod_afac = 1; // the product of a_k! for t + 1 <= k <= j
      for (int k = t+1; k <= j; k++)
        prod_afac *= compute_product(a[k], a[k], 1);
      rtj = compute_product(St, St - Sj, 1) / prod_afac; 

      Nij = Nit * rtj + lit * Ntj;
      lij = lit * ltj;
      return pair<mpz_class, mpz_class>(Nij, lij);
    }
  }



mpz_class b_graph_encoder::encode()
{
  pair<mpz_class, mpz_class> ans = compute_N(0,G.nu_vertices()-1);

  mpz_class prod_b_factorial = prod_factorial(b, 0, b.size()-1); // \prod_{i=0}^{n-1} b_i

  bool ceil = false;
  if (ans.first % prod_b_factorial != 0)
    ceil = true;
  ans.first /= prod_b_factorial;
  if (ceil)
    ans.first ++;
  return ans.first;
}


//==============================
// b_graph_decoder
//==============================

b_graph_decoder::b_graph_decoder(vector<int> a_, vector<int> b_)
{
  a = a_;
  b = b_;
  n = a.size();
  x.resize(n);
  beta = b;
  U = reverse_fenwick_tree(b);
  W = reverse_fenwick_tree(a);
}


pair<mpz_class, mpz_class> b_graph_decoder::decode_node(int i, mpz_class tN)
{
  mpz_class li = 1;
  mpz_class Ni = 0;
  int f, g; // endpoints of the interval for binary search
  int v;
  mpz_class y; // helper 
  for (int k=0;k<a[i];k++){
    // finding x[i][k]
    if (k==0)
      f = 0;
    else
      f = 1 + x[i][k-1];
    g = n-1;
    while (g > f){
      v = (f+g)/2;
      if (binomial(U.sum(1+v) , a[i] - k) <= tN)
        g = v;
      else
        f = v + 1;
    }
    x[i].push_back(f); // decoded the kth connection of vertex i
    y = binomial(U.sum(1+x[i][k]), a[i] - k);
    tN = (tN - y) / beta[x[i][k]];
    Ni += li * y;
    li *= beta[x[i][k]];
    beta[x[i][k]] --;
    U.add(x[i][k], -1);
  }
  return pair<mpz_class, mpz_class>(Ni, li);
}


pair<mpz_class, mpz_class> b_graph_decoder::decode_interval(int i, int j, mpz_class tN)
{
  if (i==j)
    return decode_node(i,tN);
  int k = (i+j)/ 2; // midpoint to break
  int Wk = W.sum(k+1);
  int Wj = W.sum(j+1);
  mpz_class rkj = compute_product(Wk, Wk - Wj, 1) / prod_factorial(a, k+1, j); // r_{t+1, j}
  mpz_class tNik = tN / rkj; // \tilde{N}_{i,k}
  pair<mpz_class, mpz_class> ans; // to keep the return for each subinterval

  // calling the left subinterval 
  ans = decode_interval(i,k,tNik);

  // preparing for the right subinterval
  mpz_class Nik = ans.first;
  mpz_class lik = ans.second;
  mpz_class tNkj = (tN - Nik * rkj) / lik; // \tilde{N}_{k+1, j}

  // calling the right subinterval
  ans = decode_interval(k+1, j, tNkj);
  mpz_class Nkj = ans.first;
  mpz_class lkj = ans.second;
  mpz_class Nij = Nik * rkj + lik * Nkj;
  mpz_class lij = lik * lkj;
  return pair<mpz_class, mpz_class> (Nij, lij);
}


b_graph b_graph_decoder::decode(mpz_class f)
{
  mpz_class prod_b_factorial = prod_factorial(b, 0, n-1);
  mpz_class tN = f * prod_b_factorial;
  decode_interval(0,n-1,tN);
  return b_graph(x, b);
}
