#include "time_series_compression.h"

void time_series_encoder::init_alph_size(const vector<int>& x)
{
  alph_size = 0;
  for (int i=0;i<x.size();i++){
    if (x[i] > alph_size)
      alph_size = x[i];
    if (x[i] < 0)
      cerr << " WARNING: time_series_encoder::encode called for a vector with negative entries " << endl;
  }

  alph_size ++; // the array is zero based 
}

void time_series_encoder::init_freq(const vector<int>& x)
{
  freq.clear();
  freq.resize(alph_size); //assuming that alph_size is already set
  for (int i=0;i<x.size();i++)
    freq[x[i]] ++;
}

void time_series_encoder::init_G(const vector<int>& x)
{
  //initializing the adjacency list
  vector<vector<int> > list;
  list.resize(n);
  for (int i=0;i<x.size();i++)
    list[i] = vector<int>({x[i]}); // list[i] has a single member, which is x[i]. In other words, the left vertex i has only one right neighbor, which is precisely x[i]
  G = b_graph(list, freq); // construct G based on its adjacency list, and the right degree sequence which is freq
}

pair<vector<int>, mpz_class> time_series_encoder::encode(const vector<int>& x)
{
  //check whether x is a compatible sequence
  if (x.size()!= n)
    cerr << " WARNING: time_series_encoder::encode, called for a vector with size different from n, x.size() = " << x.size() << endl;

  // initialize alph_size, freq and G
  //logger::item_start("time series init alph size");
  init_alph_size(x);
  //logger::item_stop("time series init alph size");
  //logger::item_start("time series init freq");
  init_freq(x);
  //logger::item_stop("time series init freq");

  //logger::item_start("time series init G");
  init_G(x);
  //logger::item_stop("time series init G");
  
  // initializing a b_graph_encoder
  vector<int> left_deg(n, 1); // the left degree sequence
  b_graph_encoder E(left_deg, freq); // the right degree sequence is freq
  //logger::item_start("time series encode");
  mpz_class f = E.encode(G);
  //logger::item_stop("time series encode");
  pair<vector<int>, mpz_class> ans;
  ans.first = freq;
  ans.second  = f;
  return ans;
}







//==================================================
// time_series_decoder
//==================================================



vector<int> time_series_decoder::decode(pair<vector<int>, mpz_class> E)
{
  freq = E.first;
  mpz_class f = E.second;
  vector<int> left_deg(n,1); // the left degree sequence
  b_graph_decoder D(left_deg, freq); // the bipartite graph decoder to convert f to G
  G = D.decode(f);

  // reconstructing the original sequence given G
  vector<int> x(n);
  vector<int> adj_list;
  for (int i=0;i<n;i++){
    adj_list = G.get_adj_list(i);
    x[i] = adj_list[0];
  }
  return x;
}
