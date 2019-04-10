#include "marked_graph.h"

marked_graph::marked_graph(int n, vector<pair< pair<int, int> , pair<int, int> > > edges, vector<int> vertex_marks)
{ 
  nu_vertices = n;
  adj_list.resize(n);
  //adj_location.resize(n);
  index_in_neighbor.resize(n);
  // modify the edges if necessary so that in each element of the form ((i,j), (x, x')), we have i < j. This is important when forming adjacency lists so that the list of each vertex is sorted
  for (int i=0;i<edges.size();i++){
    if (edges[i].first.first > edges[i].first.second){
      // swap the edge endpoints and represent it in the other direction
      swap(edges[i].first.first, edges[i].first.second);
      // also, we should swap the mark components
      swap(edges[i].second.first, edges[i].second.second);
    }
  }
  sort(edges.begin(), edges.end()); // so that the adjacency list is sorted
  for (int k=0; k<edges.size(); k++){
    // (i,j) are endpoints if the edge
    // (x,y) are marks, x towards i and y towards j
    int i = edges[k].first.first;
    int j = edges[k].first.second;
    int x = edges[k].second.first;
    int y = edges[k].second.second;
    if (i <0 || i >= n || j < 0 || j >= n || i ==j)
      cerr << " ERROR: graph::graph(n, edges) received an invalid pair of edges with n = " << n << " : (" << i << " , " << j << " )" << endl;
    adj_list[i].push_back(pair<int, pair<int, int> > (j, pair<int, int> (x,y)));
    //adj_location[i][j] = adj_list[i].size() - 1;
    adj_list[j].push_back(pair<int, pair<int, int> > (i, pair<int, int> (y,x)));
    //adj_location[j][i] = adj_list[j].size() - 1;
    index_in_neighbor[i].push_back(adj_list[j].size()-1);
    index_in_neighbor[j].push_back(adj_list[i].size()-1);
  }


  ver_mark = vertex_marks;
}

istream& operator>>(istream& inp, marked_graph& G)
{
  int nu_vertices;
  inp >> nu_vertices;

  vector<int> ver_marks;
  ver_marks.resize(nu_vertices);
  for (int i=0;i<nu_vertices;i++)
    inp >> ver_marks[i];

  int nu_edges;
  inp >> nu_edges;
  vector<pair< pair<int, int> , pair<int, int> > > edges;
  edges.resize(nu_edges);
  for (int i=0;i<nu_edges;i++)
    inp >> edges[i].first.first >> edges[i].first.second >> edges[i].second.first >> edges[i].second.second;

  G = marked_graph(nu_vertices, edges, ver_marks);

  return inp;
}

bool operator== (const marked_graph& G1, const marked_graph& G2)
{
  if (G1.nu_vertices != G2.nu_vertices)
    return false;
  int n = G1.nu_vertices; // number of vertices of the two graphs
  vector< pair< int, pair< int, int > > > l1, l2; // the adjacency list of a vertex in two graphs for comparison. 
  for (int v=0;v<n;v++){
    if (G1.ver_mark[v] != G2.ver_mark[v]) // mark of each vertex should be the same
      return false;
    if (G1.adj_list[v].size() != G2.adj_list[v].size()) // each vertex must have the same degree in two graphs
      return false;
    l1 = G1.adj_list[v];
    l2 = G2.adj_list[v];
    sort(l1.begin(), l1.end(), edge_compare); // sort with respect to the other endpoint
    sort(l2.begin(), l2.end(), edge_compare);
    if (l1 != l2) // after sorting, the lists must match
      return false;
  }
  return true;
}

bool operator!= (const marked_graph& G1, const marked_graph& G2)
{
  return !(G1 == G2);
}

bool edge_compare(const pair<int, pair<int, int> >& a, pair<int, pair<int, int> >& b)
{
  return a.first < b.first;
}


ostream& operator<< (ostream& o, const marked_graph& G)
{
  o << " number of vertices " << G.nu_vertices << endl;
  vector<pair<int, pair<int, int> > > l; // the adjacency list of a vertex
  for (int v=0; v<G.nu_vertices; v++){
    o << " vertex " << v << " mark " << G.ver_mark[v] << endl;
    //o << " adj list (connections to vertices with greater index): format (j, (x,y))" << endl;
    o << " adj list " << endl;
    l = G.adj_list[v];
    sort(l.begin(), l.end(), edge_compare);
    for (int i=0;i<l.size();i++){
      if (l[i].first > v)
        o << " (" << l[i].first << ", (" << l[i].second.first << ", " << l[i].second.second << ")) ";
    }
    o << endl << endl;
  }
  return o;
}
