#ifndef __MARKED_GRAPH_COMPRESSION__
#define __MARKED_GRAPH_COMPRESSION__


#include <vector>
#include "marked_graph.h"
#include "graph_message.h"
#include "simple_graph.h"
#include "bipartite_graph.h"
#include "simple_graph_compression.h"
#include "bipartite_graph_compression.h"
#include "time_series_compression.h"

using namespace std;

class marked_graph_compressed
{
 public:
  int n; //!< the number of vertices
  int L; //!< the number of half edge colors (non star type)
  pair<vector<int>, mpz_class> star_vertices; //!< the compressed form of the star_vertices list
  map<pair<int, int> , vector<vector<int> > > star_edges; //!< for each pair of edge marks x,x', and integer \f$k\f$, star_edges[pair<int,int>(x,x')][k] is a list of neighbors \f$w\f$ of the \f$k\f$th star vertex (say \f$v\f$) so that \f$v\f$ shares a star edge with \f$w\f$ so that the mark towards \f$v\f$ is x and the mark towards \f$w\f$ is xp. 
  vector<vector<int> > message_list; //!< a list of messages which appear in graph (not star type), so its size is L
  vector<vector<int> > ver_type_list; //!< the list of all vertex types that appear in the graph, where the type of a vertex is a vector of size \f$1+L \times L\f$ with the first index being the vertex mark, and the rest being the color degree matrix stored row by row. This can be considered as a dictionary to convert actual vertex types to integers (if ver_type_list[i] = a, then the integer i represents vertex type a) and subsequently compress it as a time series and store in the ver_types member. 

  pair<vector<int>, mpz_class> ver_types; //!< the compressed form of vertex types, where the type of a vertex is the index with respect to ver_type_list of the list of size \f$1 + L \times L\f$ encapsulating the vertex mark and the

  map<pair<int, int>, mpz_class> part_bgraph; //!< compressed form of partition bipartite graphs corresponding to colors in \f$C_<\f$. For a pair \f$0 \leq t < t' < L\f$ of half edge types, part_bgraph[pair<int, int>(t,t')] is the compressed form of the bipartite graph with n left and right nodes, where a left node i is connected to a right node j if there is an edge connecting i to j with type t towards i and type t' towards j

  map<int, pair<mpz_class, vector<int> > > part_graph; //!< compressed form of partition graphs corresponding to colors in \f$C_=\f$. For a half edge type t, part_graph[t] is the compressed form of the simple unmarked graph with n vertices, where a node i is connected to a node j where there is an edge between i and j in the original graph with color (t,t)

  void clear();
};

class marked_graph_encoder
{
  int h;
  int delta;
  int n; //!< number of vertices, this is set when a graph G is given to be encoded
  colored_graph C; //!< the auxiliary object to extract edge types
  int L; //!< the number of edge colors, excluding star types 
  vector<int> is_star_vertex; //!< for \f$0 \leq v < n\f$, is_star_vertex[v] is 1 if there is at least one star type edge connected to v and 0 otherwise.
  vector<int> star_vertices; //!< the list of star vertices

  map<pair<int, int>, b_graph> part_bgraph; //!< for \f$0 \leq t < t' < L\f$, part_bgraph[pair<int, int> (t,t')] is a bipartite graph with n left vertex and n right vertex. In this bipartite graph, a left vertex i is connected to a right vertex j iff there is an edge in the graph between vertices i and j with a half edge type towards i equal to t and a half edge type towards j equal to t'.
  map<int, graph> part_graph; //!< for \f$0 \leq t < L\f$, part_graph[i] is a simple unmarked graph with n vertices. In this graph, vertices i and j are connected in the original graph with an edge with half edge types t in both directions i and j. 

  marked_graph_compressed compressed; //!< the compressed version of the given graph in encode function

  void encode_star_vertices();
  void extract_edge_types(const marked_graph&);
  void encode_star_edges();
  void encode_vertex_types();
  void extract_partition_graphs();
  void encode_partition_bgraphs();
  void encode_partition_graphs();
 public:

 marked_graph_encoder(int h_, int delta_): h(h_), delta(delta_) {}
  marked_graph_compressed encode(const marked_graph& G);
  
};


#endif
