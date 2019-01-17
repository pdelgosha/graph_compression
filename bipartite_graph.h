#ifndef __BIPARTITE_GRAPH__
#define __BIPARTITE_GRAPH__

#include <iostream>
#include <vector>
using namespace std;

//! simple unmarked bipartite graph
class b_graph{
  int n; //!< the number of vertices in each side, so we have 2n vertices in total
  vector<vector<int> > adj_list; //!< adjacency list for left vertices, where for \f$0 \leq v < n \f$, adj_list[v] is a sorted list of right vertices connected to v.
  vector<int> left_deg_seq; //!< degree sequence for left vertices, where left_deg_seq[v] is the degree of the left node v
  vector<int> right_deg_seq; //!< degree sequence for right vertices, where left_deg_seq[v] is the degree of the right node v
public:
  //! a constructor
  /*!
    This constructor takes the list of adjacent vertices and the right degree sequence, and constructs an object.
    \param list list[v] for a left node v is the list of right nodes w connected to v. This list does not have to be sorted 
    \param right_deg right_deg[v] is the degree of the right node v
  */
  b_graph(vector<vector<int> > list, vector<int> right_deg);

  //! returns the adjacency list of a given left vertex 
  vector<int> get_adj_list(int v) const;

  //! returns the  degree of a right vertex v
  int get_right_degree(int v) const;

  //! returns the  degree of a right vertex v
  int get_left_degree(int v) const;

  //! return the right degree sequence
  vector<int> get_right_degree_sequence() const;

  //! return the left degree sequence
  vector<int> get_left_degree_sequence() const;

  //! returns the number of vertices
  int nu_vertices() const;

  //! printing the graph to the output
  friend ostream& operator << (ostream& o, const b_graph& G);

  //! comparing two graphs for equality
  friend bool operator == (const b_graph& G1, const b_graph& G2);


  //! comparing for inequality
  friend bool operator != (const b_graph& G1, const b_graph& G2);
};


#endif
