#include <iostream>
#include <fstream>
#include <vector>

#include "marked_graph.h"
#include "graph_message.h"
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


void mp_test(){
  marked_graph G;
  ifstream inp("test_graphs/hexagon_diagonal_marked.txt");
  inp >> G;
  //G = marked_ER(10000,0.0003,2 ,2);
  //cerr << " graph generated " << endl;
  cout << " G " << endl << G << endl;
  colored_graph C(G, 2, 2);

  
  int n = C.nu_vertices;
  for (int v=0; v<n; v++){
    cout << v << " : ";
    for (int i=0;i<C.adj_list[v].size();i++){
      cout << C.adj_list[v][i].first << " ( " << C.adj_list[v][i].second.first << " , " << C.adj_list[v][i].second.second << " ) ";
    }
    cout << endl;
  }

  cout << " message marks " << endl;
  for (int m=0; m<C.M.message_mark.size(); m++){
    cout << m << " mark = " << C.M.message_mark[m] << " star " << C.M.is_star_message[m] << endl;
  }
  
}


int main(){
  mp_test();
  return 0;
}
