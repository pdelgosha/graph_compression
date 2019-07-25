#include "bipartite_graph_compression.h"


//==============================
// b_graph_encoder
//==============================

void b_graph_encoder::init(const b_graph& G)
{
  // initializing beta
  beta = G.get_right_degree_sequence();

  // initializing the Fenwick tree
  U = reverse_fenwick_tree(beta);

  if (a != G.get_left_degree_sequence() or b != G.get_right_degree_sequence())
    cerr << " WARNING b_graph_encoder::init : vectors a and/or b do not match with the degree sequences of the given bipartite graph  " << endl;

}



pair<mpz_class, mpz_class> b_graph_encoder::compute_N(int i, int j, const b_graph& G)
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
    ans = compute_N(i,t, G);
    Nit = ans.first;
    lit = ans.second;
    St = U.sum(0);

    // right subinterval
    ans = compute_N(t+1, j, G);
    Ntj = ans.first;
    ltj = ans.second;
    Sj = U.sum(0);

    mpz_class rtj; // r_{t+1, j}
    mpz_class prod_afac = prod_factorial(a, t+1, j);; // the product of a_k! for t + 1 <= k <= j


    rtj = compute_product(St, St - Sj, 1) / prod_afac; 

    Nij = Nit * rtj + lit * Ntj;
    lij = lit * ltj;
    return pair<mpz_class, mpz_class>(Nij, lij);
  }
}


pair<mpz_class, mpz_class> b_graph_encoder::compute_N_new(const b_graph& G){
  //logger::item_start("bip init");
  int n_l = G.nu_left_vertices(); // number of left vertices
  int n_bits = 0;
  int n_copy = n_l;
  while (n_copy > 0){
    n_bits ++;
    n_copy >>= 1;
  }
  n_bits += 2;

  vector<pair<int, int> > call_stack(2 * n_bits);
  vector<pair<mpz_class, mpz_class> > return_stack(2 * n_bits); // first = N, second = l
  vector<int> status_stack(2 * n_bits);
  vector<int> St_stack(2 * n_bits); // stack to store values of St
 
  call_stack[0] = pair<int, int> (0,n_l-1); // i j 
  status_stack[0] = 0; // newly added
 
  int call_size = 1; // the size of the call stack
  int return_size = 0; // the size of the return stack

  int i, j, t, Sj;
  int status;
 
  vector<int> gamma; // forward list  of the graph

  mpz_class rtj, prod_afac, Nit_rtj, lit_Ntj, bin;
  //logger::item_stop("bip init");
  while(call_size > 0){
    //cerr << " call_size " << call_size << endl;
    i = call_stack[call_size-1].first;
    j = call_stack[call_size-1].second;
    if (i==j){
      //logger::item_start("bip enc i = j");
      return_stack[return_size].first = 0; // N_{i,j} is initialized with 0
      return_stack[return_size].second = 1; // l_{i,j} is initialized with 1
      gamma = G.get_adj_list(i);
      for (int k=0;k<a[i];k++){
        //logger::item_start("bip enc i = j binomial");
        bin = binomial(U.sum(1+gamma[k]), a[i] - k);
        //logger::item_stop("bip enc i = j binomial");

        //logger::item_start("bip enc i = j arithmetic");
        return_stack[return_size].first += return_stack[return_size].second * bin;
        return_stack[return_size].second *= beta[gamma[k]];
        //logger::item_start("bip enc i = j arithmetic");
        beta[gamma[k]] --;
        U.add(gamma[k],-1);
      }
      return_size ++;
      call_size --;
      //logger::item_stop("bip enc i = j");
    }else{
      //logger::item_start("bip enc i neq j");
      t = (i+j)/2;
      status = status_stack[call_size - 1];
      //logger::item_start("bip enc stacking 0 1");
      if (status == 0){
        // newly added, left node must be called
        call_stack[call_size].first = i;
        call_stack[call_size].second = t;
        status_stack[call_size-1] = 1; // left is called
        status_stack[call_size] = 0; // newly added
        call_size++;
      }
      if (status == 1){
        // left is returned
        St_stack[call_size-1] = U.sum(0);
        // call the right child
        call_stack[call_size].first = t+1;
        call_stack[call_size].second = j;
        status_stack[call_size-1] = 2; //right is called
        status_stack[call_size] = 0;  // newly called
        call_size ++;
      }
      //logger::item_stop("bip enc stacking 0 1");
      if (status == 2){
        Sj = U.sum(0);
        //logger::item_start("bip enc i neq j prod_factorial");
        prod_afac = prod_factorial(a, t+1, j); // the product of a_k! for t + 1 <= k <= j
        //logger::item_stop("bip enc i neq j prod_factorial");

        //logger::item_start("bip enc i neq j compute_product");
        rtj = compute_product(St_stack[call_size-1], St_stack[call_size-1] - Sj, 1) / prod_afac;
        //logger::item_stop("bip enc i neq j compute_product");

        //logger::item_start("bip enc i neq j arithmetic");
        Nit_rtj = return_stack[return_size-2].first * rtj;
        lit_Ntj = return_stack[return_size-2].second * return_stack[return_size-1].first;
        return_stack[return_size-2].first = Nit_rtj + lit_Ntj; // Nij
        return_stack[return_size-2].second = return_stack[return_size-2].second * return_stack[return_size-1].second; // lij
        //logger::item_stop("bip enc i neq j arithmetic");
        return_size --; // pop 2 add 1
        call_size --;
      }
      //logger::item_stop("bip enc i neq j");
    }

  }
  if (return_size != 1){
    cerr << " error: bip compute_N return_size is not 1 it is " << return_size << endl;
  }
  return return_stack[0];
}



pair<mpz_class, mpz_class> b_graph_encoder::compute_N_new_r(const b_graph& G){
  //logger::item_start("bip init");
  int n_l = G.nu_left_vertices(); // number of left vertices
  int n_bits = 0;
  int n_copy = n_l;
  while (n_copy > 0){
    n_bits ++;
    n_copy >>= 1;
  }
  n_bits += 2;

  vector<pair<int, int> > call_stack(2 * n_bits);
  vector<pair<mpz_class, mpz_class> > return_stack(2 * n_bits); // first = N, second = l
  vector<mpz_class> r_stack(2 * n_bits); // stack of r values

  vector<int> status_stack(2 * n_bits);
  //vector<int> St_stack(2 * n_bits); // stack to store values of St
 
  call_stack[0] = pair<int, int> (0,n_l-1); // i j 
  status_stack[0] = 0; // newly added
 
  int call_size = 1; // the size of the call stack
  int return_size = 0; // the size of the return stack

  int i, j, t, Sj;
  int status;
 
  vector<int> gamma; // forward list  of the graph

  mpz_class rtj, prod_afac, Nit_rtj, lit_Ntj, bin;
  //logger::item_stop("bip init");
  while(call_size > 0){
    //cerr << " call_size " << call_size << endl;
    i = call_stack[call_size-1].first;
    j = call_stack[call_size-1].second;
    if (i==j){
      //logger::item_start("bip enc i = j");
      return_stack[return_size].first = 0; // N_{i,j} is initialized with 0
      return_stack[return_size].second = 1; // l_{i,j} is initialized with 1
      r_stack[return_size] = binomial(U.sum(0), a[i]); // r_i = \binom{S_i}{a_i}, s_i = U.sum(0)
      gamma = G.get_adj_list(i);
      for (int k=0;k<a[i];k++){
        //logger::item_start("bip enc i = j binomial");
        bin = binomial(U.sum(1+gamma[k]), a[i] - k);
        //logger::item_stop("bip enc i = j binomial");

        //logger::item_start("bip enc i = j arithmetic");
        return_stack[return_size].first += return_stack[return_size].second * bin;
        return_stack[return_size].second *= beta[gamma[k]];
        //logger::item_start("bip enc i = j arithmetic");
        beta[gamma[k]] --;
        U.add(gamma[k],-1);
      }
      return_size ++;
      call_size --;
      //logger::item_stop("bip enc i = j");
    }else{
      //logger::item_start("bip enc i neq j");
      t = (i+j)/2;
      status = status_stack[call_size - 1];
      //logger::item_start("bip enc stacking 0 1");
      if (status == 0){
        // newly added, left node must be called
        call_stack[call_size].first = i;
        call_stack[call_size].second = t;
        status_stack[call_size-1] = 1; // left is called
        status_stack[call_size] = 0; // newly added
        call_size++;
      }
      if (status == 1){
        // left is returned
        //St_stack[call_size-1] = U.sum(0);
        // call the right child
        call_stack[call_size].first = t+1;
        call_stack[call_size].second = j;
        status_stack[call_size-1] = 2; //right is called
        status_stack[call_size] = 0;  // newly called
        call_size ++;
      }
      //logger::item_stop("bip enc stacking 0 1");
      if (status == 2){
        //Sj = U.sum(0);
        //logger::item_start("bip enc i neq j prod_factorial");
        //prod_afac = prod_factorial(a, t+1, j); // the product of a_k! for t + 1 <= k <= j
        //logger::item_stop("bip enc i neq j prod_factorial");

        //logger::item_start("bip enc i neq j compute_product");
        //rtj = compute_product(St_stack[call_size-1], St_stack[call_size-1] - Sj, 1) / prod_afac;
        //logger::item_stop("bip enc i neq j compute_product");

        //logger::item_start("bip enc i neq j arithmetic");
        Nit_rtj = return_stack[return_size-2].first * r_stack[return_size-1]; 
        lit_Ntj = return_stack[return_size-2].second * return_stack[return_size-1].first;
        return_stack[return_size-2].first = Nit_rtj + lit_Ntj; // Nij
        return_stack[return_size-2].second = return_stack[return_size-2].second * return_stack[return_size-1].second; // lij
        r_stack[return_size - 2] = r_stack[return_size-2] * r_stack[return_size-1];
        //logger::item_stop("bip enc i neq j arithmetic");
        return_size --; // pop 2 add 1
        call_size --;
      }
      //logger::item_stop("bip enc i neq j");
    }

  }
  if (return_size != 1){
    cerr << " error: bip compute_N return_size is not 1 it is " << return_size << endl;
  }
  return return_stack[0];
}


mpz_class b_graph_encoder::encode(const b_graph& G)
{
  if (a != G.get_left_degree_sequence() or b != G.get_right_degree_sequence())
    cerr << " WARNING b_graph_encoder::encoder : vectors a and/or b do not match with the degree sequences of the given bipartite graph  " << endl;

  //init(G); // initialize U and beta for G
  //pair<mpz_class, mpz_class> ans = compute_N(0,G.nu_left_vertices()-1, G);
  //init(G);
  //logger::item_start("bip enc compute N");
  //pair<mpz_class, mpz_class> ans = compute_N_new(G);
  //logger::item_stop("bip enc compute N");

  init(G);
  //logger::item_start("bip enc compute N new r");
  pair<mpz_class, mpz_class> ans = compute_N_new_r(G);
  //logger::item_stop("bip enc compute N new r");
  // if (ans.first == ans2.first and ans.second == ans2.second){
  //   cout << " = " << endl;
  // }else{
  //   cout << " != " << endl;
  // }
  //if (ans.first!= ans_2.first or ans.second != ans_2.second){
  //  cerr << " bip ans != ans_2 ans = (" << ans.first << " , " << ans.second <<  " ) ans_2 = (" << ans_2.first << " , " << ans_2.second << " ) "  << endl;
  //}//else{
  //cerr << " the same! ans = (" << ans.first << " , " << ans.second <<  " ) ans_2 = (" << ans_2.first << " , " << ans_2.second << " ) "  << endl;
  //}
  //mpz_class prod_b_factorial = prod_factorial(b, 0, b.size()-1); // \prod_{i=0}^{n-1} b_i

  //if (prod_b_factorial != ans.second)
  //  cerr << "EEEEEEEEEEEEEEEEEEEEEEEEE prod_b_factorial != ans.second" << endl;

  bool ceil = false;
  //logger::item_start("bip enc ceil");
  if (ans.first % ans.second != 0)
    ceil = true;
  //logger::item_stop("bip enc ceil");

  //logger::item_start("bip enc final div");
  ans.first /= ans.second;
  //logger::item_stop("bip enc final div");
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
  np = b.size();
  init();
}

void b_graph_decoder::init()
{
  x.clear();
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
  x[i].clear(); // make sure nothing is in the list to be decoded
  for (int k=0;k<a[i];k++){
    // finding x[i][k]
    if (k==0)
      f = 0;
    else
      f = 1 + x[i][k-1];
    g = np-1;
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
  mpz_class prod_b_factorial = prod_factorial(b, 0, np-1);
  mpz_class tN = f * prod_b_factorial;
  decode_interval(0,n-1,tN);
  return b_graph(x, b);
}
