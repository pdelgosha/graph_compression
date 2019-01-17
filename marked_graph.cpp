#include "marked_graph.h"

marked_graph::marked_graph(int n, vector<pair< pair<int, int> , pair<int, int> > > edges, vector<int> vertex_marks)
{ 
  nu_vertices = n;
  adj_list.resize(n);
  adj_location.resize(n);
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
    adj_location[i][j] = adj_list[i].size() - 1;
    adj_list[j].push_back(pair<int, pair<int, int> > (i, pair<int, int> (y,x)));
    adj_location[j][i] = adj_list[j].size() - 1;
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
