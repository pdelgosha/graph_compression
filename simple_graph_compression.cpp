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


pair<mpz_class, mpz_class> graph_encoder::compute_N(int i, int j, int I, const graph& G)
{
  //cerr << " i, j " << i << " , " << j << endl;
  if (i==j){
    mpz_class zi, li, zik, lik;
    zi = 0;
    li = 1;
    vector<int> x = G.get_forward_list(i); // the forward adjacency list of vertex i
    if (beta[i] != x.size())
      cerr << " DANGER! beta[i] is not the same as x.size()!!" << endl;
    for (int k=0;k<x.size();k++){
      //logger::item_start("simple_cp_1");
      zik = compute_product(U.sum(1+x[k]), beta[i] - k, 1); // we are zero based here, so instead of -k + 1, we have -k
      //logger::item_stop("simple_cp_1");
      //logger::item_start("simple_ar");
      zi += li * zik;
      lik = (beta[i] - k) * beta[x[k]]; // we are zero based here, so instead of -k + 1, we have -k
      li *= lik;
      //logger::item_stop("simple_ar");
      beta[x[k]] --;
      U.add(x[k], -1);
    }
    //cerr << " returning (" << i << " , " << j << ") N " << zi << " l " << li << endl;
    return pair<mpz_class, mpz_class>(zi, li);
  }else{
    int t = (i+j) / 2;
    mpz_class Nit, lit; // \f$N_{i,t}\f$ and \f$l_{[i:t]}\f$
    mpz_class Ntj, ltj; // \f$N_{t+1,j}\f$ and \f$l_{[t+1:j]}\f$
    mpz_class Nij, lij; // \f$N_{i,j}\f$ and \f$l_{[i:j]}\f$
    int St, Sj; // \f$S_{t+1}\f$ and \f$S_{j+1}\f$
    mpz_class rtj; // \f$r_{[t+1:j]}\f$

    pair<mpz_class, mpz_class> return_left = compute_N(i,t, 2*I, G); // calling for the interval i,t
    Nit = return_left.first;
    lit = return_left.second;
    St = U.sum(t+1);

    if (j - i > logn2){// we should save the midpoint sum St
      //cerr << " i " << i << " j " << j << " I " << I << " storing St " << St << endl;
      if (I >= Stilde.size() ){
        cerr << " WARNING graph_encoder::compute_N index I is out of range.  I= " << I << " Stilde.size()= "<< Stilde.size() <<  " i= " << i << " j= " << j << " logn2= " << logn2 << endl;
      }
      Stilde[I] = St;
    }

    pair<mpz_class, mpz_class> return_right = compute_N(t+1, j, 2*I + 1, G); // calling for the interval t+1, j
    Ntj = return_right.first;
    ltj = return_right.second;
    Sj = U.sum(j+1);
    //logger::item_start("simple_cp"); 
    rtj = compute_product(St-1, (St - Sj)/2, 2);
    //logger::item_stop("simple_cp");
    //cerr << "( " << i << " , " << j << ") St " << St << " Sj " << Sj << " rtj " << rtj << endl;
    //logger::item_start("simple_ar");
    Nij = Nit * rtj + lit * Ntj ;
    lij = lit * ltj;
    //logger::item_stop("simple_ar");
    //cerr << " returning (" << i << " , " << j << ") N " << Nij << " l " << lij << endl;
    return pair<mpz_class, mpz_class> (Nij, lij);
  }
}

pair<mpz_class, vector<int> > graph_encoder::encode(const graph& G){
  if (G.get_degree_sequence()!= a)
    cerr << " WARNING graph_encoder::encode : vector a does not match with the degree sequence of the given graph ";
  init(G); // initialize U and beta 
  pair<mpz_class, mpz_class> N_ans  = compute_N(0,G.nu_vertices()-1,1, G);

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
