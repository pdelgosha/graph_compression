#ifndef __GRPAH_MESSAGE__
#define __GRPAH_MESSAGE__

#include <vector>
#include <map>
#include "marked_graph.h"
using namespace std;


//! this class takes care of message passing on marked graphs.
/*!
  This graph has a reference to a marked_graph object for which we perform message passing to find edge types. The edge types are discovered up to depth h-1, and with degree parameter Delta, where h and Delta are member objects. Star type messages (which roughly speaking corresponds to places where there is a vertex in the h neighborhood has degree more than delta) are vectors of size 2, first coordinate being -1, and the second being the edge mark component (towards the 'me' vertex). 

  _Sample Usage_

  ~~~~~~~~~{.cpp}
  marked_graph G;
  ... //define G
  int h = 10;
  int Delta = 5; 
  graph_message M(G, h, Delta);
  ~~~~~~~~~

 */
class graph_message{
  int h; //!< the depth up to which we do message passing (the type of edges go through depth h-1)
  int Delta; //!< the maximum degree threshold

  //! performs the message passing algorithm and updates the messages array accordingly
  void update_messages(const marked_graph&);

  //! update message_dict and message_list
  void update_message_dictionary(const marked_graph&); 
public:
  //const marked_graph & G; //!< reference to the marked graph for which we do message passing
  vector<vector<vector<vector<int> > > > messages; //!< messages[v][i][t] is the message at time t from vertex v towards its ith neighbor (in the order given by adj_list of vertex i in graph G). Messages will be useful to find edge types

  map<vector<int>, int> message_dict; //!< the message dictionary (at depth t=h-1), which maps each message to its corresponding index in the dictionary
  vector<vector<int> > message_list; //!< the list of messages present in the graph, stored in an order consistent with message_dict, i.e. for a message m, if messsage_dict[m] = i, then message_list[i] = m. This is constructed in such a way that message_list[message_dict[x]] = x. This is sorted in reverse order so that all * messages (those messages starting with -1) go to the end of the list. Star type messages (which roughly speaking corresponds to places where there is a vertex in the h neighborhood has degree more than delta) are vectors of size 2, first coordinate being -1, and the second being the edge mark component (towards the 'me' vertex). 

  //! constructor, given reference to a graph
  graph_message(const marked_graph& graph, int depth, int max_degree){
    h = depth;
    Delta = max_degree;
    update_messages(graph); // do message passing
  }

  //! default constructor
  graph_message() {}
};







//! used for sorting messages 
bool pair_compare(const pair<vector<int> , int>& , const pair<vector<int>, int>& );





//! this class defines a colored graph, which is obtained from a simple marked graph and the color of edges come from the type of edges 
/*!
  quick member overview:

  - h and Delta are parameters that determine depth and maximum degree to form edge types,
  - M is a member with type graph_message that is used to form edge types,
  - nu_vertices: number of vertices in the graph
  - adj_list: the adjacency list of vertices, which also includes edge colors
  - adj_location: map for finding where neighbors of vertices are in the adjacency list
  - ver_type: a vector for each vertex, containing mark + vectorized degree matrix
  - ver_type_dict: dictionary mapping vertex mark + degree matrix to integer
  - ver_type_list: list of "distinct" vertex types
  - ver_type_int: vertex types converted to integers

  __Sample Usage__

  ~~~~~~~~~{.cpp}
  marked_graph G;
  ... //define G
  int h = 10;
  int Delta = 5;
  colored_graph C(G, h, Delta);
  ~~~~~~~~~

 */
class colored_graph{
public:
  //const marked_graph & G; //!< the marked graph from which this is created
  int h; //!< the depth up to which look at edge types
  int Delta; //!< the maximum degree threshold
  graph_message M; //!< we use the message passing algorithm of class graph_message to find out edge types
  int L; //!< the number of colors, excluding the star colors 
  int nu_vertices; //!< the number of vertices in the graph.
  vector<vector<pair<int, pair<int, int> > > > adj_list; //!< adj_list[i] is the list of edges connected to vertex i, each of the format (other endpoint, color component towards i, color component towards other endpoint). Therefore, the color of an edge between v and its ith neighbor is of the form (adj_list[v][i].second.first, adj_list[v][i].second.second)

  vector<map<int,int> > adj_location;  //!< adj_location[v] for \f$0 \leq v < n\f$, is a map, where adj_location[v][w] denotes the index in adj_list[v] where the information regarding the edge between v and w is stored. Hence, adj_location[v][w] does not exist if w is not adjacent to v, and adj_list[v][adj_location[v][w]] is the edge between v and w

  vector<vector<int> >  ver_type; //!< vertex mark and the colored degree matrix of each vertex. For a vertex v,  ver_type[v] is a vector of size \f$1 + L \times L\f$, where the first entry is the vertex mark, and the rest is the colored degree matrix row by row. Here, \f$L\f$ denotes the number of non star type colors.  Note that all star type colors are in the range [L, M.message_list.size()).

  map<vector<int>, int > ver_type_dict; //!< the dictionary mapping vertex types to integers, obtained from the ver_type array defined above

  vector<vector<int> > ver_type_list; //!< the list of all distinct vertex types, obtained from the ver_type array. 



  vector<int> ver_type_int; //!< vertex type converted to integers, using the ver_type_dict map, i.e. ver_type_int[v] = ver_type_dict[ver_type[v]]

  vector<int> is_star_vertex; //!< for \f$0 \leq v < n\f$, is_star_vertex[v] is true if vertex v has at least one star typed edge connected to it
  vector<int> star_vertices; //!< the (sorted) list of star_vertices, where a star vertex is the one which has at least one star type vertex connected to it.

  //! constructor from a graph, depth and maximum degree parameters
 colored_graph(const marked_graph& graph, int depth, int max_degree): M(graph, depth, max_degree), h(depth), Delta(max_degree)
  {
    init(graph); // initialize other variables 
  }

  //! default constructor
  colored_graph() {}

  //! initializes other variables. Here, G is the reference to the marked graph based on which this object is being created 
  void init(const marked_graph& G);
};


#endif //__GRPAH_MESSAGE__
