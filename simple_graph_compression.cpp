#include "simple_graph_compression.h"


//==============================
// graph_encoder
//==============================

/*!
  initializes the degree sequence to a_, sets n and logn2, and resizes the Stilde vector
 */
graph_encoder::graph_encoder(const vector<int>& a_){
  a = a_;
  n = a.size();
  int log2n = 0; // log of n in base 2
  int nn = n; // a copy of n
  while (nn>0){
    log2n ++;
    nn = nn >> 1; // divide by 2
  } // eventually, we count the number of bits in n
  log2n --; // we count extra, e.g. when n = 1, we end up having 1, rather than 0
  logn2 = log2n * log2n;

  Stilde.clear();
  Stilde.resize(4 * n / logn2); // look at the explanation of the algorithm, before deriving the bound 16 n, that 4n / \lfloor \log_2 n \rfloor^2 is also an upper bound. After this point, we have used \lfloor \log_2 n \rfloor \geq \log n / 2 to derive the 16 n bound.
  Stilde[0] = 0;
}

void graph_encoder::init(const graph& G)
{
  // initializing the beta sequence
  beta = a; 

  //beta.resize(G.nu_vertices());
  //for (int v=0;v<G.nu_vertices();v++)
  //  beta[v] = G.get_degree(v);  

  // initializing the Fenwick Tree
  U = reverse_fenwick_tree(beta);


  //initializing the partial sum vector Stilde
  Stilde.clear();
  Stilde.resize(4 * n / logn2); // TO CHECK, 2018-10-18_self-compression_Stilde-size-required-2nlogn2.pdf
  Stilde[0] = 0;
}






pair<mpz_class, mpz_class> graph_encoder::compute_N(const graph& G)
{

  int n = G.nu_vertices();
  int n_bits = 0;
  int n_copy = n;
  while (n_copy > 0){
    n_bits ++;
    n_copy >>= 1;
  }
  n_bits += 2;

  vector<pair<pair<int, int>, int > > call_stack(2 * n_bits);
  vector<pair<mpz_class, mpz_class> > return_stack(2 * n_bits); // first = N, second = l
  vector<mpz_class> r_stack(2 * n_bits); // stack of r values 
  vector<int> status_stack(2 * n_bits);
  //vector<int> St_stack(2 * n_bits); // stack to store values of St

  call_stack[0].first = pair<int, int> (0,n-1); // i j 
  call_stack[0].second = 1; // I
  status_stack[0] = 0; // newly added

  int call_size = 1; // the size of the call stack
  int return_size = 0; // the size of the return stack

  int i, j, I, t, Sj;
  int status;

  vector<int> gamma; // forward list  of the graph
  mpz_class zik, lik, rtj; // intermediate variables
  mpz_class Nit_rtj; // result of Nij * rtj
  mpz_class lit_Ntj; // result of lit * Ntj 
  while (call_size > 0){
    // cerr << " printing the whole stack " << endl;
    // for (int k = 0; k<call_size; k++){
    //   cerr << k << " : " << call_stack[k].first.first << " " << call_stack[k].first.second << " I " << call_stack[k].second << "s= " << status_stack[k] << endl;
    // }
    // cerr << " return stack " << endl;
    // for (int k=0;k<return_size;k++)
    //   cerr << k << ": " << return_stack[k].first << " " << return_stack[k].second << endl;
    i = call_stack[call_size-1].first.first;
    j = call_stack[call_size-1].first.second;
    I = call_stack[call_size-1].second;
    if (i==j){
      //logger::item_start("sim enc i = j");
      return_stack[return_size].first = 0; // z_i is initialized with 0
      return_stack[return_size].second = 1; // l_i is initialize with 1
      gamma = G.get_forward_list(i); // the forward adjacency list of vertex i
      r_stack[return_size] = compute_product(U.sum(i) - 1, beta[i], 2); // this is r_i
      // cerr << " i " << i << " j " << j << " gamma: " << endl;
      // for (int k=0;k<gamma.size();k++)
      //   cerr << gamma[k] << " ";
      // cerr << endl;
      // cerr << " beta " << endl;
      // for (int k=0; k<n; k++)
      //   cerr << beta[k] << " ";
      // cerr << endl;

      for (int k=0;k<gamma.size();k++){
        zik = compute_product(U.sum(1+gamma[k]), beta[i] - k, 1); // we are zero based here, so instead of -k + 1, we have -k
        //zik = helper_vars::return_stack[0];
        //cerr << " zik " << zik << endl;
        return_stack[return_size].first += return_stack[return_size].second * zik;
        lik = (beta[i] - k) * beta[gamma[k]]; // we are zero based here, so instead of -k + 1, we have -k
        //cerr << " lik " << lik << endl;
        return_stack[return_size].second *= lik;
        beta[gamma[k]] -- ;
        U.add(gamma[k],-1);
      }
      return_size ++; // establish the return
      call_size --;
      //logger::item_stop("sim enc i = j");
    }else{
      status = status_stack[call_size-1];
      if (status == 0){
        // newly added node, we should call its left child
        t = (i+j) / 2;
        call_stack[call_size].first.first = i;
        call_stack[call_size].first.second = t;
        call_stack[call_size].second = 2*I;
        status_stack[call_size-1] = 1; // left is called
        status_stack[call_size] = 0; // newly added
        call_size++;
      }
      if (status == 1){
        // left is returned
        t = (i+j) / 2;
        //St_stack[call_size-1] = U.sum(t+1);
        if (j - i > logn2)
          Stilde[I] = U.sum(t+1);//St_stack[call_size-1];
        // prepare to call right
        
        call_stack[call_size].first.first = t + 1;
        call_stack[call_size].first.second = j;
        call_stack[call_size].second = 2*I + 1;
        status_stack[call_size-1] = 2; // right is called
        status_stack[call_size] = 0; // newly called
        call_size ++;
      }

      if (status == 2){
        // both are returned, and results can be accessed by the top two elements in return stack
        //Sj = U.sum(j+1);
        //logger::item_start("sim enc i neq j compute_product");
        //rtj = compute_product(St_stack[call_size-1]-1, (St_stack[call_size-1] - Sj)/2, 2);
        //logger::item_stop("sim enc i neq j compute_product");
        //rtj = helper_vars::return_stack[0];
        //cerr << " rtj " << rtj << endl;
        //Nij = Nit * rtj + lit * Ntj ;
        //logger::item_start("sim enc i neq j arithmetic");
        Nit_rtj = return_stack[return_size-2].first * r_stack[return_size-1];
        lit_Ntj = return_stack[return_size-2].second * return_stack[return_size-1].first;
        return_stack[return_size-2].first = Nit_rtj + lit_Ntj; // Nij
        return_stack[return_size-2].second = return_stack[return_size-2].second * return_stack[return_size-1].second; // lij
        r_stack[return_size-2] = r_stack[return_size-2] * r_stack[return_size-1];
        //logger::item_stop("sim enc i neq j arithmetic");
        return_size --; // pop 2 add 1
        call_size --;
      }
    }

  }

  if (return_size != 1){
    cerr << " error: return_size is not 1 it is " << return_size << endl;
  }
  return return_stack[0];
}

pair<mpz_class, vector<int> > graph_encoder::encode(const graph& G){
  if (G.get_degree_sequence()!= a)
    cerr << " WARNING graph_encoder::encode : vector a does not match with the degree sequence of the given graph ";
  //init(G); // initialize U and beta 
  //pair<mpz_class, mpz_class> N_ans  = compute_N(0,G.nu_vertices()-1,1, G);
  init(G); // re initializing U abd beta for the second test
  pair<mpz_class, mpz_class> N_ans = compute_N(G);
  // init(G);
  // pair<mpz_class, mpz_class> N_ans_2 = compute_N(G);
  // if (N_ans.first == N_ans_2.first and N_ans.second == N_ans_2.second){
  //   cerr << " = " << endl;
  // }else{
  //   cerr << " error N_ans and N_ans_2 are not the same, "  << endl << "N_ans = (" << N_ans.first << " , " << N_ans.second << ") " << endl << "N_ans_2 = (" << N_ans_2.first << " , " << N_ans_2.second << ")" << endl; 
  // }
  //if (N_ans.first != N_ans_2.first or N_ans.second != N_ans_2.second)
  //  cerr << " error N_ans and N_ans_2 are not the same, "  << endl << "N_ans = (" << N_ans.first << " , " << N_ans.second << ") " << endl << "N_ans_2 = (" << N_ans_2.first << " , " << N_ans_2.second << ")" << endl;
  //else
  //  cerr << " N_ans = N_ans_2 " << endl;
  //mpz_class prod_a_factorial = prod_factorial(a, 0,a.size()-1); // \prod_{i=1}^n a_i!
  //if (prod_a_factorial!= N_ans.second)
  //  cerr << " ERROR: not equal " << endl;
  // N_ans.second = \prod_{i=1}^n a_i!
  // we need the ceiling of the ratio of N_ans.first and prod_a_factorial
  bool ceil = false; // if true, we will add one to the integer division
  //logger::item_start("simple_ar");
  if (N_ans.first % N_ans.second != 0)
    ceil = true;
  N_ans.first /= N_ans.second;
  if (ceil)
    N_ans.first ++;
  //logger::item_stop("simple_ar");
  return pair<mpz_class, vector<int> > (N_ans.first, Stilde);
}





//==============================
// graph_decoder
//==============================

graph_decoder::graph_decoder(vector<int> a_)
{
  a = a_;
  n = a.size();

  int log2n = 0; // log of n in base 2
  int nn = n; // a copy of n
  while (nn>0){
    log2n ++;
    nn = nn >> 1; // divide by 2
  } // eventually, we count the number of bits in n
  log2n --; // we count extra, e.g. when n = 1, we end up having 1, rather than 0
  logn2 = log2n * log2n;

  init(); // init x, beta and U
}

void graph_decoder::init()
{
  x.clear();
  x.resize(n);
  beta = a;
  U = reverse_fenwick_tree(a);
}

graph graph_decoder::decode(mpz_class f, vector<int> tS_)
{
  init(); // make x, U and beta ready for decoding 
  tS = tS_;
  //mpz_class prod_a_factorial = 1; // \prod_{i=1}^n a_i!
  //for (int i=0; i<a.size();i++)
  //  prod_a_factorial *= compute_product(a[i], a[i], 1);

  mpz_class prod_a_factorial = prod_factorial(a, 0,a.size()-1); // \prod_{i=0}^{n-1} a_i!
  mpz_class tN = f * prod_a_factorial;
  decode_interval(0,n-1,1,tN,0);
  return graph(x, a);
}

pair<mpz_class, mpz_class> graph_decoder::decode_node (int i, mpz_class tN)
{
  //cerr << " decode node " << i << " tN " << tN << endl;
  //cerr << " beta[i] " << beta[i] << endl;
  //cerr << " beta " << endl;
  //for (int k = i; k< n;k++)
  //  cerr << k << " " << beta[k] << endl;
  //cerr << " U " << endl;
  //for (int k=i;k<n;k++)
  //  cerr << k << " " << U.sum(k) << endl;

  if (beta[i] == 0)
    return pair<mpz_class, mpz_class> (0,1);

  mpz_class li = 1; // l_i(G)
  mpz_class Ni = 0; // N_{i,i}(G)
  int f, g; // endpoints for the binary search
  int t; // midpoint for the binary search
  mpz_class zik, lik; 
  for (int k=0;k<beta[i];k++){
    if (k==0)
      f = i+1;
    else
      f = x[i][k-1]+1;
    g = n-1;
    while(g > f){
      //cerr << " f , g " << f << " " << g << endl;
      t = (f+g)/2;
      // binary search:
      if(compute_product(U.sum(t+1), beta[i] - k, 1) <= tN)
        g = t;
      else
        f = t+1;
    }
    x[i].push_back(f);
    zik = compute_product(U.sum(x[i][k]+1), beta[i] - k, 1);
    Ni += li * zik;
    lik = (beta[i] - k) * beta[x[i][k]];
    li *= lik;
    tN -= zik;
    tN /= lik;
    U.add(x[i][k],-1);
    beta[x[i][k]] --;
  }
  //cerr << " decoded for " << i << " x: " << endl;
  //for (int j=0;j<x[i].size(); j++)
  //  cerr << x[i][j] << " " ;
  //cerr << endl;
  return pair<mpz_class, mpz_class> (Ni, li);
}


pair<mpz_class, mpz_class> graph_decoder::decode_interval(int i, int j, int I, mpz_class tN, int Sj)
{
  //cerr << " decode interval " << i << " " << j << " tN " << tN << endl;
  if (i == j)
    return decode_node(i, tN);

  // sweeping for zero nodes

  int t; // place to break
  int St; // S_{t+1}
  if ((j-i) > logn2){
    //cerr << " long interval I = " << I << endl;
    t = (i+j) / 2; // break at middle, since we have \tilde{S}
    St = tS[I]; // looking at the \f$\tilde{S}\f$ vector
  }else{
    //cerr << " short interval " << endl;
    t = i;
    St = U.sum(i) - 2 * beta[i];
  }

  //cerr << " decode interval " << i << " " << j << " t " << t << " St " << St << " Sj " << Sj << endl;
  mpz_class rtj; // \f$t_{t+1, j}\f$
  mpz_class tNit; // \f$\tilde{N}_{i,t}\f$ for the left decoder
  mpz_class tNtj; // \f$\tilde{N}_{t+1, j}\f$ for the right decoder 
  mpz_class Nit; // the true N_{i,t} returned by the left decoder
  mpz_class lit; // the true l_{i,t} returned by the left decoder
  mpz_class Ntj; // the true N_{t+1, j} returned by the right decoder
  mpz_class ltj; // the true l_{t+1,j} returned by the right decoder
  mpz_class Nij; // the true N_{i,j} to return
  mpz_class lij; // the true l_{i,j} to return

  pair<mpz_class, mpz_class> ans; // returned by subintervals 


  rtj = compute_product(St - 1, (St - Sj)/2, 2);
  //cerr << " interval " << i << " " << j << " t " << t << " St " << St << " rtj " << rtj << endl;
  tNit = tN / rtj;

  // calling the left decoder 
  ans = decode_interval(i,t,2*I,tNit, St); 
  Nit = ans.first;
  lit = ans.second;

  // reducing the contribution of the left decoder to prepare for the right decoder
  tNtj = (tN - Nit * rtj) / lit;

  // calling the right decoder
  ans = decode_interval(t+1, j, 2*I + 1, tNtj, Sj);
  Ntj = ans.first;
  ltj = ans.second;

  // preparing Nij and lij to return
  Nij = Nit * rtj + lit * Ntj;
  lij = lit * ltj;
  return pair<mpz_class, mpz_class> (Nij, lij);
}
