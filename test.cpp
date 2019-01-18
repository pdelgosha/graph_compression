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

using namespace std;

ostream& operator << (ostream& o, const vector<int>& v){
  for (int i=0;i<v.size();i++){
    o << v[i];
    if (i < v.size()-1)
      o << ", ";
  }
  return o;
}

int main(){
  //marked_graph G;
  //ifstream inp("star_graph.txt");
  //inp >> G;
  //graph_message M(G, 10, 2);
  //M.update_messages();
  vector<int> a = {1,1,2}; // left degree sequence 
  vector<int> b = {2,2}; // right degree sequence

  b_graph G({{0},{1},{0,1}}); // defining the graph

  b_graph_encoder E(a,b); // constructing the encoder object
  mpz_class f = E.encode(G);

  b_graph_decoder D(a, b);
  b_graph Ghat = D.decode(f);

  if (Ghat == G)
    cout << " successfully decoded the graph! " << endl;


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
