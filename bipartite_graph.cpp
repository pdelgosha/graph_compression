#include "bipartite_graph.h"

b_graph::b_graph(vector<vector<int> > list, vector<int> right_deg)
{
  n = list.size();
  np = right_deg.size(); // the number of right nodes
  adj_list = list;
  left_deg_seq.resize(n);
  // sorting the list
  for (int v=0; v<n; v++){
    sort(adj_list[v].begin(), adj_list[v].end());
    left_deg_seq[v] = adj_list[v].size();
  }
  right_deg_seq = right_deg;
}

b_graph::b_graph(vector<vector<int> > list)
{
  // goal: finding right degrees and calling the above constructor
  // first, we find the number of right nodes
  np = 0; // the number of right nodes
  n = list.size();
  adj_list = list;
  left_deg_seq.resize(n);
  for (int v=0;v<adj_list.size();v++){
    cerr << " v " << v << endl;
    sort(adj_list[v].begin(), adj_list[v].end());
    if (adj_list[v].size() > 0 and adj_list[v][adj_list[v].size()-1] > np)
      np = adj_list[v][adj_list[v].size()-1];
    left_deg_seq[v] = adj_list[v].size();
  }
  np++; // node indexing is zero based
  
  right_deg_seq.resize(np);
  fill(right_deg_seq.begin(), right_deg_seq.end(), 0); // make all elements 0
  for (int v=0;v<list.size();v++)
    for (int i=0;i<list[v].size();i++)
      right_deg_seq[list[v][i]]++;
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


int b_graph::nu_left_vertices() const
{
  return n;
}

int b_graph::nu_right_vertices() const
{
  return np;
}


ostream& operator << (ostream& o, const b_graph& G)
{
  int n = G.nu_left_vertices();
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
  int n1 = G1.nu_left_vertices();
  int n2 = G2.nu_left_vertices();
  
  int np1 = G1.nu_right_vertices();
  int np2 = G2.nu_right_vertices();
  if (n1!= n2 or np1 != np2)
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

