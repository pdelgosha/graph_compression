#include "random_graph.h"

marked_graph marked_ER(int n, double p, int ver_mark, int edge_mark)
{
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine generator(seed);
  uniform_int_distribution<int> ver_mark_dist(0,ver_mark-1); // distribution a vertex mark
  uniform_int_distribution<int> edge_mark_dist(0,edge_mark-1); // distribution an edge mark
  uniform_real_distribution<double> unif_dist(0.0,1.0);
  double unif;
  int x, xp; // generated marks

  vector<int> ver_marks(n); // vector of size n
  vector<pair< pair<int, int>, pair<int, int> > > edges; // the edge list of the graph 

  for (int v=0; v<n; v++){
    ver_marks[v] = ver_mark_dist(generator);
    for (int w=v+1; w<n;w++){
      unif = unif_dist(generator);
      if (unif < p){ // we put an edge between v and w
        x = edge_mark_dist(generator);
        xp = edge_mark_dist(generator);
        edges.push_back(pair<pair<int, int>, pair<int, int> >(pair<int, int>(v,w), pair<int, int>(x,xp)));
      }
    }
  }
  cout << " marked_ER number of edges " << edges.size() << endl;
  return marked_graph(n, edges, ver_marks);
}
