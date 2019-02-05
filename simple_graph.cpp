#include "simple_graph.h"


/*!
  This constructor takes the list of adjacent vertices and the degree sequence, and constructs an object.
  \param list list[v] is the list of vertices w adjacent to v such that \f$w > v\f$. However, this list does not have to be sorted.
  \param deg deg[v] is the overall degree of the vertex (not only the ones with greater index).
*/
graph::graph(vector<vector<int> > list, vector<int> deg){
  n = list.size();
  forward_adj_list = list;
  // sorting the list
  for (int v=0; v<n; v++)
    sort(forward_adj_list[v].begin(), forward_adj_list[v].end());
  degree_sequence = deg;
}

/*!
  This constructor only takes the forward adjacency list and computes the degree sequence itself
 */
graph::graph(vector<vector<int> > list)
{
  n = list.size();
  forward_adj_list = list;


  // sorting the list
  for (int v=0; v<n; v++)
    sort(forward_adj_list[v].begin(), forward_adj_list[v].end());

  // finding the degree sequence
  // first, removing and resize it
  degree_sequence.clear();
  degree_sequence.resize(n);

  for (int v=0; v<n; v++){
    degree_sequence[v] += forward_adj_list[v].size(); // degree to the right 
    for (int i=0;i<forward_adj_list[v].size();i++) // modifying degree of vertices to the right of v
      degree_sequence[forward_adj_list[v][i]]++;
  }
}

vector<int> graph::get_forward_list(int v) const{
  if (v < 0 or v >= n)
    cerr << "graph::get_forward_list, index v out of range" << endl;
  return forward_adj_list[v];
}

int graph::get_forward_degree(int v) const{
  if (v < 0 or v >= n)
    cerr << "graph::get_forward_degree, index v out of range" << endl;
  return forward_adj_list[v].size();
}

int graph::get_degree(int v) const{
  return degree_sequence[v];
}

vector<int> graph::get_degree_sequence() const{
  return degree_sequence;
}


int graph::nu_vertices() const{
  return n;
}

ostream& operator << (ostream& o, const graph& G)
{
  int n = G.nu_vertices();
  vector<int> list;
  for (int i=0;i<n;i++){
    list = G.get_forward_list(i);
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

bool operator == (const graph& G1, const graph& G2)
{
  int n1 = G1.nu_vertices();
  int n2 = G2.nu_vertices();
  if (n1!= n2)
    return false;
  vector<int> list1, list2;
  for (int v=0; v<n1; v++){
    list1 = G1.get_forward_list(v);
    list2 = G2.get_forward_list(v);
    if (list1 != list2)
      return false;
  }
  return true;
}

bool operator != (const graph& G1, const graph& G2)
{
  return !(G1 == G2);
}


