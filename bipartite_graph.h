#ifndef __BIPARTITE_GRAPH__
#define __BIPARTITE_GRAPH__

#include <iostream>
#include <vector>
using namespace std;

//! simple unmarked bipartite graph
/*!
  A simple unmarked bipartite graph with n left nodes and np right nodes. There are two ways to define such an object.

  1. through adjacency list which is a vector<vector<int> > of size n (number of left nodes) where each element is a vector of adjacent right vertices (does not have to be sorted). Note that both left and right vertex indices are 0 based. For instance, (in c++11 notation), if list = {{0},{1},{0,1}}, the graph has 3 left nodes and 2 right nodes, left node 0 is connected to right node 0, left node 1 is connected to right node 1, and left node 2 is connected to right nodes 0 and 1.

         vector<vector<int> > list = {{0},{1},{0,1}};
         b_graph G(list);

  2. through adjacency list and right degree vector. Adjacency list is as explained above, and the extra information of right degree vector is just to help construct the object more easily. For instance, with list = {{0},{1},{0,1}}, we have right_deg = {1,2}, which means that the degree of the right node 0 is 1 while the degree of the right node 1 is 2.

         vector<vector<int> > list = {{0},{1},{0,1}};
         vector<int> right_deg = {1,2}
         b_graph G(list, right_deg);

 */
class b_graph{
  int n; //!< the number of left vertices
  int np; //!< the number of right vertices
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

  //! a constructor
  /*!
    This constructor takes the list of adjacent vertices 
    \param list list[v] for a left node v is the list of right nodes w connected to v. This list does not have to be sorted 
  */
  b_graph(vector<vector<int> > list);


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

  //! returns the number of left vertices
  int nu_left_vertices() const;

  //! returns the number of right vertices
  int nu_right_vertices() const;

  //! printing the graph to the output
  friend ostream& operator << (ostream& o, const b_graph& G);

  //! comparing two graphs for equality
  friend bool operator == (const b_graph& G1, const b_graph& G2);


  //! comparing for inequality
  friend bool operator != (const b_graph& G1, const b_graph& G2);
};


#endif
