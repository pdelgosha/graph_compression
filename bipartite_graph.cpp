#include "bipartite_graph.h"

b_graph::b_graph(vector<vector<int> > list, vector<int> right_deg)
{
  n = list.size();
  adj_list = list;
  left_deg_seq.resize(n);
  // sorting the list
  for (int v=0; v<n; v++){
    sort(adj_list[v].begin(), adj_list[v].end());
    left_deg_seq[v] = adj_list[v].size();
  }
  right_deg_seq = right_deg;
}



vector<int> b_graph::get_adj_list(int v) const
{
  if (v < 0 or v >= n)
    cerr << "b_graph::get_adj_list, index v out of range" << endl;
  return adj_list[v];
}


int b_graph::get_right_degree(int v) const
{
  if (v < 0 or v >= n)
    cerr << "b_graph::get_right_degree, index v out of range" << endl;
  return right_deg_seq[v];
}

int b_graph::get_left_degree(int v) const
{
  if (v < 0 or v >= n)
    cerr << "b_graph::get_left_degree, index v out of range" << endl;
  return left_deg_seq[v];
}

vector<int> b_graph::get_right_degree_sequence() const
{
  return right_deg_seq;
}

vector<int> b_graph::get_left_degree_sequence() const
{
  return left_deg_seq;
}


int b_graph::nu_vertices() const
{
  return n;
}


ostream& operator << (ostream& o, const b_graph& G)
{
  int n = G.nu_vertices();
  vector<int> list;
  for (int i=0;i<n;i++){
    list = G.get_adj_list(i);
    o << i << " -> ";
    for (int j=0;j<list.size();j++){
      o << list[j];
      if (j < list.size()-1)
        o << ", ";
    }
    o << endl;
  }
  return o;
}

bool operator == (const b_graph& G1, const b_graph& G2)
{
  int n1 = G1.nu_vertices();
  int n2 = G2.nu_vertices();
  if (n1!= n2)
    return false;
  vector<int> list1, list2;
  for (int v=0; v<n1; v++){
    list1 = G1.get_adj_list(v);
    list2 = G2.get_adj_list(v);
    if (list1 != list2)
      return false;
  }
  return true;
}

bool operator != (const b_graph& G1, const b_graph& G2)
{
  return !(G1 == G2);
}

