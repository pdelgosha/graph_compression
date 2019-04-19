#include <iostream>
#include <fstream>
#include <vector>

#include "marked_graph.h"
#include "graph_message.h"
#include "random_graph.h"
#include "logger.h"

using namespace std;

ostream& operator << (ostream& o, const vector<int>& v){
  for (int i=0;i<v.size();i++){
    o << v[i];
    if (i < v.size()-1)
      o << ", ";
  }
  return o;
}


void random_graph_test(){
  //marked_graph G = poisson_graph(10, 2, 2, 2);
  marked_graph G = near_regular_graph(10,2,1,1);
  cout << G;
}

void mp_test(){
  marked_graph G;
  ifstream inp("test_graphs/hexagon_diagonal_marked2.txt");
  inp >> G;
  //G = marked_ER(10000,0.0003,2 ,2);
  //G = poisson_graph(100000,3, 10, 10);
  //cerr << " graph generated " << endl;
  //cerr << " graph generated " << endl;
  //cout << " G " << endl << G << endl;
  colored_graph C(G, 3, 2);

  /*
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
  */
  
}


int main(){
  logger::start();
  //mp_test();
  random_graph_test();
  logger::stop();
  //random_graph_test ();
  return 0;
}
