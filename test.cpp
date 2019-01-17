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

int main(){
  marked_graph G;
  ifstream inp("star_graph.txt");
  inp >> G;
  graph_message M(G, 10, 2);
  M.update_messages();
}
