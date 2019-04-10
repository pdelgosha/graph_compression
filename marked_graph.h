#ifndef __MARKED_GRAPH__
#define __MARKED_GRAPH__

#include<iostream>
#include<vector>
#include<map>
#include<fstream>

using namespace std;

//! simple marked graph
/*!
  This class stores a simple marked graph where each vertex carries a mark, and each edge carries two marks, one towards each of its endpoints. The mark of each vertex and each edge is a nonnegative integer.
 */
class marked_graph{ 
public:
  int nu_vertices; //!< number of vertices in the graph 
  vector<vector<pair<int, pair<int, int> > > > adj_list; //!< adj_list[i] is the list of edges connected to vertex i, each of the format (other endpoint, mark towards i, mark towards other endpoint)
  //vector<map<int,int> > adj_location;  //!< adj_location[v] for \f$0 \leq v < n\f$, is a map, where adj_location[v][w] denotes the index in adj_list[v] where the information regarding the edge between v and w is stored. Hence, adj_location[v][w] does not exist if w is not adjacent to v, and adj_list[v][adj_location[v][w]] is the edge between v and w

  vector<vector<int> > index_in_neighbor; //!< index_in_neighbor[v][i] is the index of vertex v in the adjacency list of the ith neighbor of v

  vector<int> ver_mark; //!< ver_mark[i] is the mark of vertex i
  
  //! default constructor
  marked_graph()
    {
      nu_vertices = 0;
    }
  
  //! constructs a marked graph based on edges lists and vertex marks.
  /*!
    \param n the number of vertices in the graph
    \param edges a vector, where each element is of the form \f$((i,j), (x,y))\f$ where \f$i \neq j\f$ denotes the endpoints of the edge, \f$x\f$ is the mark towards \f$i\f$ and \f$y\f$ is the mark towards \f$j\f$
    \param vertex_marks is a vector of size n, where vertex_marks[i] is the mark of vertex i
   */
  marked_graph(int n, vector<pair< pair<int, int> , pair<int, int> > > edges, vector<int> vertex_marks);

  //! checks whether two marked graphs are the same.
  /*!
    two marked graphs are said to be the same if: 1) they have the same number of vertices, 2) vertex marks match and 3) each vertex has the same set of neighbors with matching marks.
   */
  friend bool operator== (const marked_graph& G1, const marked_graph& G2);

  //! checks whether two marked graphs are not equal
  friend bool operator!= (const marked_graph& G1, const marked_graph& G2);

  //! prints a marked graph to the output 
  friend ostream& operator<< (ostream& o, const marked_graph& G);
};

//! inputs a marked_graph
/*!
  The input format is as follows:
  1) number of vertices
  2) a list of vertex marks as nonnegative integers
  3) number of edges
  4) for each edge: write \f$i j x y\f$, where \f$i\f$ and \f$j\f$ are the endpoints (here, \f$0 \leq i, j \leq n-1 \f$ with \f$n\f$ being the number of vertices), \f$x\f$ is the mark towards \f$i\f$ and \f$y\f$ is the mark towards \f$j\f$ (both nonnegative integers)
  Example:
  2
  1 2
  1
  0 1 1 2
  which is a graph with 2 vertices, the mark of vertex 0 is 1 and the mark of vertex 1 is 2, there is one edge between these two vertices with mark 1 towards 0 and mark 2 toward s 1
 */
istream& operator>>(istream& inp, marked_graph& G);


//! this is to help comparing two marked graphs. The inputs would resemble two edge information, of the form (j, (x, y)), where j is the other endpoint, and x and y are marks. We want to sort them with respect to the neighbor index j. 
bool edge_compare(const pair<int, pair<int, int> >& a, pair<int, pair<int, int> >& b);


#endif // __MARKED_GRAPH__
