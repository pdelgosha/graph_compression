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


marked_graph poisson_graph(int n, double deg_mean, int ver_mark, int edge_mark){
  unsigned seed = chrono::system_clock::now().time_since_epoch().count();
  default_random_engine generator(seed);
  poisson_distribution<int> deg_dist(deg_mean);
  uniform_int_distribution<int> neighbor_dist(0,n-1); // distribution for the other endpoint

  vector< pair< pair< int, int >, pair< int, int > > > edges;
  vector<int> ver_marks(n);

  uniform_int_distribution<int> ver_mark_dist(0,ver_mark-1); // distribution a vertex mark
  uniform_int_distribution<int> edge_mark_dist(0,edge_mark-1); // distribution an edge mark
  int x, xp; // edge mark values

  pair< pair< int, int >, pair< int, int > > edge; // the current edge to be added 
  vector<set<int> > neighbors(n); // the list of neighbors of vertices 
  int deg; // the degree of a vertex
  int w; // the neighbor
  for (int v=0;v<n;v++){
    ver_marks[v] = ver_mark_dist(generator);
    deg = deg_dist(generator);
    for (int i=0; i<deg; i++){
      do{
        w = neighbor_dist(generator);
      }while(w == v or neighbors[v].find(w) != neighbors[v].end()); // not myself and not already connected
      // now, w is a possible neighbor
      // see if w has picked v as a neighbor
      if (neighbors[w].find(v) == neighbors[w].end()){
        // add w as a neighbors to v
        neighbors[v].insert(w);
        // marks for the edge
        x = edge_mark_dist(generator); 
        xp = edge_mark_dist(generator);
        edge.first.first = v;
        edge.first.second = w;
        edge.second.first = x;
        edge.second.second = xp;
        edges.push_back(edge);
      }
    }
  }
  cerr << " edges size " << edges.size() << endl;
  return marked_graph(n, edges, ver_marks);
}
