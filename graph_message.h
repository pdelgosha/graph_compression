#ifndef __GRPAH_MESSAGE__
#define __GRPAH_MESSAGE__

#include <vector>
#include <map>
#include <unordered_map>
#include <boost/functional/hash/hash.hpp>
#include "marked_graph.h"
#include "logger.h"

using namespace std;

struct vint_hash{
  size_t operator()(vector<int> const& v) const;
};

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
  vector<vector<vector<int > > > messages; //!< messages[v][i][t] is the integer version of the  message at time t from vertex v towards its ith neighbor (in the order given by adj_list of vertex i in graph G). Messages will be useful to find edge types

  vector<vector<vector<int > > > inward_messages; //!< inward_messages[v][i][t] is the integer version of the  message at time t from the ith neighbor of v towards v (in the order given by adj_list in graph G). 

  vector<unordered_map<vector<int>, int, vint_hash> > message_dict; //!< message_dict[t] for \f$0 \leq t \leq h-1\f$ is the message dictionary at depth t, which maps each message to its corresponding index in the dictionary

  //vector<map<vector<int>, int> > message_dict; //!< message_dict[t] for \f$0 \leq t \leq h-1\f$ is the message dictionary at depth t, which maps each message to its corresponding index in the dictionary

  vector<vector<vector<int> > > message_list; //!< message_list[t] is the list of messages present in the graph at depth t, stored in an order consistent with message_dict[t], i.e. for a message m, if messsage_dict[t][m] = i, then message_list[t][i] = m. This is constructed in such a way that message_list[t][message_dict[t][x]] = x. message_list[h-1] is sorted in reverse order so that all * messages (those messages starting with -1) go to the end of the list. Star type messages (which roughly speaking corresponds to places where there is a vertex in the h neighborhood has degree more than delta) are vectors of size 2, first coordinate being -1, and the second being the edge mark component (towards the 'me' vertex). 


  vector<int> message_mark; //!< for an integer message such as m at depth h-1, message_mark[m] denotes the mark component of the message that corresponds to m. The message corresponds to m is basically message_list[h-1][m] which is of type vector<int> and the last component in this array is the mark component. 

  vector<bool> is_star_message; //!< for a message m (integer type) at depth h-1, is_star_message[m] is true if the corresponding message starts with -1, ans is false otherwise.

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
  int nu_vertices; //!< the number of vertices in the graph.
  vector<vector<pair<int, pair<int, int> > > > adj_list; //!< adj_list[i] is the list of edges connected to vertex i, each of the format (other endpoint, color component towards i, color component towards other endpoint). Therefore, the color of an edge between v and its ith neighbor is of the form (adj_list[v][i].second.first, adj_list[v][i].second.second)

  vector<map<int,int> > adj_location;  //!< adj_location[v] for \f$0 \leq v < n\f$, is a map, where adj_location[v][w] denotes the index in adj_list[v] where the information regarding the edge between v and w is stored. Hence, adj_location[v][w] does not exist if w is not adjacent to v, and adj_list[v][adj_location[v][w]] is the edge between v and w

  vector<map<pair<int, int> , int> > deg; //!< deg[v] for a vertex v is a map, where deg[v][(m, m')] for a pair of non star types  m, m' is the number of edges connected to v with type m towards v and type m' towards the other endpoint. Note that only non star types appear in this map. 

  //map<pair<int, int> , vector<int> > type_vertex_list; //!< type_vertex_list[(m,m')] for a pair of types m and m' is the list of vertices v in the graph with at least one edge adjacent to v with type m towards v and m' towards the other endpoint. type_vertex_list[(m, m')] is guaranteed to be an increasing list.

  vector<vector<int> >  ver_type; //!< for a vertex v, ver_type[v] is a vector<int> and encodes the mark of v and its colored degree in the following way: ver_type[v][0] is the ver_mark of v, ver_type[v][3k+1], ver_type[v][3k+2] and ver_type[3k+3] are \f$m, m'\f$ and \f$n_{m, m'}\f$, where \f$m\f$ and \f$m'\f$ are edge types, and \f$n_{m, m'}\f$ denotes the number of edges connected to v with type \f$(m, m')\f$. The list of \f$m, m'\f$ is sorted (lexicographically) to ensure unique representation. Since we only represent types with nonzero \f$n_{m, m'}\f$, we are effectively giving the nonzero entries of the colored degree matrix, resulting in an improvement over storing the whole degree matrix. 

  map<vector<int>, int > ver_type_dict; //!< the dictionary mapping vertex types to integers, obtained from the ver_type array defined above

  vector<vector<int> > ver_type_list; //!< the list of all distinct vertex types, obtained from the ver_type array. 



  vector<int> ver_type_int; //!< vertex type converted to integers, using the ver_type_dict map, i.e. ver_type_int[v] = ver_type_dict[ver_type[v]]

  vector<bool> is_star_vertex; //!< for \f$0 \leq v < n\f$, is_star_vertex[v] is true if vertex v has at least one star typed edge connected to it
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
