#include <iostream>
#include <fstream>
#include <vector>

#include "marked_graph.h"
#include "graph_message.h"
#include "fenwick.h"
#include "simple_graph.h"
#include "simple_graph_compression.h"
#include "bipartite_graph.h"
#include "bipartite_graph_compression.h"
#include "time_series_compression.h"
#include "marked_graph_compression.h"
#include "random_graph.h"

using namespace std;

ostream& operator << (ostream& o, const vector<int>& v){
  for (int i=0;i<v.size();i++){
    o << v[i];
    if (i < v.size()-1)
      o << ", ";
  }
  return o;
}

void b_graph_test(){
  vector<int> a = {1,1,2}; // left degree sequence 
  vector<int> b = {2,2}; // right degree sequence

  b_graph G({{0},{1},{0,1}}); // defining the graph

  b_graph_encoder E(a,b); // constructing the encoder object
  mpz_class f = E.encode(G);

  b_graph_decoder D(a, b);
  b_graph Ghat = D.decode(f);

  if (Ghat == G)
    cout << " successfully decoded the graph! " << endl;  
}

void graph_test(){
  vector<int> a = {3,2,2,3};
  vector<vector<int> > list = {{1,2,3},{3},{3},{}};

  graph G(list);

  graph_encoder E(a);
  pair<mpz_class, vector<int> > f = E.encode(G);

  graph_decoder D(a);
  graph Ghat = D.decode(f.first, f.second);
  Ghat = D.decode(f.first, f.second);

  if (Ghat == G)
    cout << " successfully decoded the graph! " << endl;
}

void time_series_compression_test()
{
  vector<int> a = {0,2,3,1,2,1,0,1,0,2,1,0,0,2,1,3,4,5,0};
  int n = a.size();
  time_series_encoder E(n);
  pair<vector<int>, mpz_class > ans = E.encode(a);

  time_series_decoder D(n);
  vector<int> ahat = D.decode(ans);
  if (ahat == a)
    cout << " successfully decoded the original time series! " << endl;
}

void marked_graph_encoder_test()
{
  marked_graph G;
  //ifstream inp("test_graphs/ten_node.txt"); //("test_graphs/hexagon_diagonal_marked.txt");
  //inp >> G;
  int h, delta;
  cout << " h " << endl;
  cin >> h;
  cout << " delta " << endl;
  cin >> delta; 
  G = marked_ER(300,0.01,2 ,2);
  cout << " graph constructed " << endl;
  marked_graph_encoder E(h,delta);
  marked_graph_compressed C = E.encode(G);

  marked_graph_decoder D; 
  marked_graph Ghat = D.decode(C);

  if (Ghat == G)
    cout << " successfully decoded the marked graph :D " << endl;
  else
    cout << " they do not match :(" << endl;

  //cout << " G " << endl;
  //cout << G << endl;
  //cout << " Ghat " << endl;
  //cout << Ghat << endl;
}

void random_graph_test()
{
  marked_graph G = marked_ER(100,1,3,4);
  //cout << G << endl;
}
int main(){
  //marked_graph G;
  //ifstream inp("star_graph.txt");
  //inp >> G;
  //graph_message M(G, 10, 2);
  //M.update_messages();
  //graph_test();
  //time_series_compression_test();
  marked_graph_encoder_test();
  //random_graph_test();

  return 0;
  // vector<vector<int> > list = {{}, {}, {}};

  // b_graph G({{0},{1},{0,1}});
  // // cout << G << endl;
  // // cout << G.nu_left_vertices() << endl;
  // // cout << G.nu_right_vertices() << endl;
  // // cout << G.get_left_degree_sequence() << endl;
  // // cout << G.get_right_degree_sequence() << endl;
  // vector<int> a = G.get_left_degree_sequence();
  // vector<int> b = G.get_right_degree_sequence();
  

  // b_graph_encoder  E(a,b);
  // mpz_class m = E.encode(G);
  // cout << "encoded: " << m << endl;

  // b_graph_decoder D(a, b);
  // b_graph Ghat = D.decode(m);
  // cout << "decoded graph: " << endl << Ghat << endl;

  // if (Ghat == G)
  //   cout << " equal " << endl;
  // return 0;

}
