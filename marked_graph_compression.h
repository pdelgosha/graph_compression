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
  int h; //!< the depth up to which the compression was performed
  int delta; //!< the degree threshold used when compression was performed
  pair<vector<int>, mpz_class> star_vertices; //!< the compressed form of the star_vertices list
  map<pair<int, int> , vector<vector<int> > > star_edges; //!< for each pair of edge marks x,x', and integer \f$k\f$, star_edges[pair<int,int>(x,x')][k] is a list of neighbors \f$w\f$ of the \f$k\f$th star vertex (say \f$v\f$) so that \f$v\f$ shares a star edge with \f$w\f$ so that the mark towards \f$v\f$ is x and the mark towards \f$w\f$ is xp. 

  vector<int> type_mark; //!< for an edge type t, type_mark[t] denotes the mark component of t

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
  //int L; //!< the number of edge colors, excluding star types 
  vector<bool> is_star_vertex; //!< for \f$0 \leq v < n\f$, is_star_vertex[v] is true if there is at least one star type edge connected to v and false otherwise.
  vector<int> star_vertices; //!< the list of star vertices

  map<pair<int, int>, b_graph> part_bgraph; //!< for \f$0 \leq t < t' < L\f$, part_bgraph[pair<int, int> (t,t')] is a bipartite graph with n left vertex and n right vertex. In this bipartite graph, a left vertex i is connected to a right vertex j iff there is an edge in the graph between vertices i and j with a half edge type towards i equal to t and a half edge type towards j equal to t'.

  vector<map<pair<int, int>, int> > part_index; //!< for a vertex \f$0 \leq v < n\f$, if v has a (t,t') edge connected to it. part_index[v][(t,t')] is the index of vertex v in the partition graph (or bipartite graph) corresponding to the pair (t,t'). If t < t', this is the index of the left vertex corresponding to v in the partition bipartite graph, and if t > t', this is the index of the right node corresponding to v in the bipartite partition graph.

  map<pair<int, int>, vector<int> > part_deg; //!< for a pair of types (t,t'), part_deg[(t,t')] is the degree sequence of the nodes in the partition graph corresponding to the pair t,t'. If t < t', this is the degree sequence of the left nodes in the (t,t') partition bipartite graph, while if t > t', this is the degree sequence of the right nodes in the (t', t) partition bipartite graph. Moreover, if t = t', this is the degree sequence of the (t,t) partition graph. 

  map<int, graph> part_graph; //!< for \f$0 \leq t < L\f$, part_graph[i] is a simple unmarked graph with n vertices. In this graph, vertices i and j are connected in the original graph with an edge with half edge types t in both directions i and j. 

  marked_graph_compressed compressed; //!< the compressed version of the given graph in encode function

  //! encodes the star vertices (those vertices with at least one star edge connected to them)
  /*!
    uses time_series_encode to encode the 0-1 sequence of star vertices stored in is_star_vertex to the star_vertices attribute of compressed
   */
  void encode_star_vertices();

  //! Given a marked graph, extracts edge types by updating the colored_graph member C
  void extract_edge_types(const marked_graph&);

  //! Encodes star edges to the star_edges attribute of compressed
  void encode_star_edges();

  //! encodes the type of vertices, where the type of a vertex denotes its mark as well as its degree matrix 
  void encode_vertex_types();

  //! update part_index and part_deg members
  void find_part_index_deg();

  //! by looking at the colored graph C, extract partition graphs (simple and bipartite)
  void extract_partition_graphs();

  //! encode partition bipartite graphs
  void encode_partition_bgraphs();

  //! encodes partition simple graphs 
  void encode_partition_graphs();
 public:

 marked_graph_encoder(int h_, int delta_): h(h_), delta(delta_) {}
  marked_graph_compressed encode(const marked_graph& G);

};


class marked_graph_decoder
{
  int h;
  int delta;
  int n; //!< number of vertices, this is set when a graph G is given to be encoded
  //int L; //!< the number of edge colors, excluding star types 
  vector<int> is_star_vertex; //!< for \f$0 \leq v < n\f$, is_star_vertex[v] is 1 if there is at least one star type edge connected to v and 0 otherwise.
  vector<int> star_vertices; //!< the list of star vertices

  //vector<vector<int> > ver_type; //!< the list of vertex types, where the type of a vertex is an array of size \f$1+L\times L\f$, using the same convention as in the `colored_graph` class.

  map<pair<int, int>, b_graph> part_bgraph; //!< for \f$0 \leq t < t' < L\f$, part_bgraph[pair<int, int> (t,t')] is a bipartite graph with n left vertex and n right vertex. In this bipartite graph, a left vertex i is connected to a right vertex j iff there is an edge in the graph between vertices i and j with a half edge type towards i equal to t and a half edge type towards j equal to t'.
  map<int, graph> part_graph; //!< for \f$0 \leq t < L\f$, part_graph[i] is a simple unmarked graph with n vertices. In this graph, vertices i and j are connected in the original graph with an edge with half edge types t in both directions i and j.

  vector<pair<pair<int, int>, pair<int, int> > > edges; //!< the list of edges in the decoded graph, each index of the form \f$((i,j), (x,y))\f$, where \f$i\f$ and \f$j\f$ are the endpoints and \f$x\f$ and \f$y\f$ are the marks (towards \f$i\f$ and \f$j\f$, respectively).

  vector<int> vertex_marks; //!< the list of vertex marks of the marked graph to be decoded

  vector<map<pair<int, int>, int> > Deg; //!< for a vertex \f$0 \leq v < n\f$, Deg[v] is a map such that Deg[v][(t,t')] is the number of edges connected to v with type \f$(t,t')\f$ (if such an edge exists)

  map<pair<int, int>, vector<int> > part_deg; //!< part_deg[(t,t')] is the degree sequence of the partition graph corresponding to pair of types t and t', if \f$t \neq t'\f$, this is the degree sequence of the side of the graph corresponding to t.

  map<pair<int, int>, vector<int> > origin_index; //!< origin_index[(t,t')][v] gives the original index in the marked graph corresponding to the vertex v in the \f$(t,t')\f$ partition graph. Here, if \f$t \neq t'\f$, v is in the side of the bipartite graph corresponding to t

  void decode_star_vertices(const marked_graph_compressed&);
  void decode_star_edges(const marked_graph_compressed&);
  void decode_vertex_types(const marked_graph_compressed&);
  void find_part_deg_orig_index();
  void decode_partition_graphs(const marked_graph_compressed&);
  void decode_partition_bgraphs(const marked_graph_compressed&);

 public:

  //! constructor
  marked_graph_decoder() {}


  marked_graph decode(const marked_graph_compressed&);
};

#endif
