#ifndef __SIMPLE_GRAPH__
#define __SIMPLE_GRAPH__

#include <iostream>
#include <vector>
using namespace std;

//! simple unmarked graph
class graph{
  int n; //!< the number of vertices in the graph
  vector<vector<int> > forward_adj_list; //!< for a vertex \f$0 \leq v < n\f$, forward_adj_list[v] is a vector containing vertices w such that are adjacent to v and also \f$w > v\f$, i.e. the adjacent vertices in the forward direction. For such v, forward_adj_list[v] is sorted increasing.
  vector<int> degree_sequence; //!< the degree sequence of the graph, where the degree of a vertex is the number of all edges connected to it (not just the ones with greater index).
public:

  //! default constructor
 graph(): n(0) {}


  //! a constructor
  graph(vector<vector<int> > list, vector<int> deg);

  //! constructor, given only the forward adjacency list 
  graph(vector<vector<int> > list);

  //! returns the forward adjacency list of a given vertex 
  vector<int> get_forward_list(int v) const;

  //! returns the forward degree of a vertex v
  int get_forward_degree(int v) const;

  //! returns the overall degree of a vertex
  int get_degree(int v) const;

  //! returns the whole degree sequence
  vector<int> get_degree_sequence() const;

  //! the number of vertices in the graph 
  int nu_vertices() const;

  //! printing the graph to the output
  friend ostream& operator << (ostream& o, const graph& G);

  //! comparing two graphs for equality
  friend bool operator == (const graph& G1, const graph& G2);

  //! comparing for inequality
  friend bool operator != (const graph& G1, const graph& G2);

};


#endif
