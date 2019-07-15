#ifndef __BIPARTITE_GRAPH_COMPRESSION__
#define __BIPARTITE_GRAPH_COMPRESSION__

#include <iostream>
#include <vector>
#include "compression_helper.h"
#include "bipartite_graph.h"
#include "fenwick.h"

using namespace std;

//! Encodes a simple unmarked bipartite graph
/*!
  Encodes a simple bipartite graph in the set of bipartite graphs with given left degree sequence a and right degree sequence b. Therefore, to construct an encoder object, we need to specify these two degree sequences as vectors of int. For instance (in c++11)

  ~~~~~~~~~{.cpp}
  vector<int> a = {1,1,2};
  vector<int> b = {2,2};
  b_graph_encoder E(a,b);
  ~~~~~~~~~

 constructs an encode object E which is capable of encoding bipartite graphs having 3 left nodes with degrees 1, 1, 2 (in order) and 2 right nodes with degrees 2,2 (in order). Hence, assume that we have defined such a bipartite graph by giving adjacency list:

 ~~~~~~~~~{.cpp}
 b_graph G({{0},{1},{0,1}});
 ~~~~~~~~~

 Note that G has left and right degree sequences which are equal to a and b, respectively. Then, we can use E to encode G as follows:

 ~~~~~~~~~{.cpp}
 mpz_class f = E.encode(G);
 ~~~~~~~~~

 In this way, the encode converts G to an integer stored in f. Later on, we can use f to decode G.
 */
class b_graph_encoder
{
  //const b_graph& G; //!< the simple unmarked bipartite graph to be encoded
  vector<int> beta; //!< when compute_N is called for \f$i \leq j\f$, for \f$ i \leq v \leq n\f$, we have beta[v] = \f$b_v(i)\f$
  vector<int> a; //!< the degree sequence for the left nodes
  vector<int> b; //!< the degree sequence for the right nodes
  reverse_fenwick_tree U; //!< a Fenwick tree which encodes the degree of right nodes. When compute_N is called for \f$i \leq j\f$, for \f$i \leq v \leq n\f$, we have U.sum[v] \f$= \sum_{k = v}^n b_k(i)\f$.
public:

  //! constructor
 b_graph_encoder(vector<int> a_, vector<int> b_): a(a_), b(b_) {}
  
  //! initializes beta and U
  void init(const b_graph& G);


  //! computes \f$N_{i,j}(G)\f$
  /*!
    \param i,j the interval for which we compute \f$N_{i,j}(G)\f$
    \param G reference to the bipartite graph for which we compute N
    \return A pair, where the first component is \f$N_{i,j}(G)\f$, and the second component is \f$l_{i,j}(G)\f$
   */
  pair<mpz_class, mpz_class> compute_N(int i, int j, const b_graph& G);

  pair<mpz_class, mpz_class> compute_N_new(const b_graph& G);

  //! encodes the given bipartite graph \f$G\f$ and returns an integer in the specified range
  mpz_class encode(const b_graph& G);

};

//! Decodes a simple unmarked bipartite graph
/*!
  Decodes a simple bipartite graph given its encoded integer. We assume that the decoder knows the left and right degree sequences of the encoded graph, hence these sequences must be given when a decoder object is being constructed. For instance, borrowing the degree sequences of the example we used to explain the b_graph_encoder class:

  ~~~~~~~~~{.cpp}
  vector<int> a = {1,1,2};
  vector<int> b = {2,2};
  b_graph_decoder D(a,b);
  ~~~~~~~~~

 Then, if variable f of type mpz_class is obtained from a b_graph_encoder class, we can reconstruct the graph using f:

 ~~~~~~~~~{.cpp}
 b_graph Ghat = D.decode(f);
 ~~~~~~~~~

 Then, the graph Ghat will be equal to the graph G. Here is a full example showing the procedure of compression and decompression together:

 ~~~~~~~~~{.cpp}
 vector<int> a = {1,1,2}; // left degree sequence 
 vector<int> b = {2,2}; // right degree sequence
 
 b_graph G({{0},{1},{0,1}}); // defining the graph
 
 b_graph_encoder E(a,b); // constructing the encoder object
 mpz_class f = E.encode(G);
 
 b_graph_decoder D(a, b);
 b_graph Ghat = D.decode(f);
 
 if (Ghat == G)
    cout << " we successfully reconstructed the graph! " << endl;
 ~~~~~~~~~
 
 */
class b_graph_decoder
{
  int n; //!< number of left vertices
  int np; //!< number of right vertices
  vector<int> a; //!< left degree sequence
  vector<int> b; //!< right degree sequence
  vector<vector<int> > x;//!< the adjacency list of left nodes for the decoded graph
  reverse_fenwick_tree U; //!< reverse Fenwick tree initialized with the right degree sequence b, and after decoding vertex \f$i\f$, for \f$0 \leq v <n'\f$, we have \f$U_v = \sum_{k=v}^{n'-1} b_k(i)\f$
  reverse_fenwick_tree W; //!< keeping partial sums for the degree sequence a. More precisely, for \f$0 \leq v < n\f$, we have \f$W_v = \sum_{k=v}^{n-1} a_k\f$ 
  vector<int> beta; //!< the sequence \f$\vec{\beta}\f$, where before decoding vertex \f$i\f$, for \f$0 \leq v < n'\f$, we have \f$\beta_v = b_v(i)\f$

public:
  //! constructor
  b_graph_decoder(vector<int> a_, vector<int> b_);

  //! initializes x as empty list of size n, beta as b, U with b and W with a
  void init();

  //! decodes the connectivity list of a left node \f$0 \leq i < n\f$ given \f$\tilde{N}_{i,i}\f$
  /*!
    \param i: the vertex to be decoded
    \param tN: \f$\tilde{N}_{i,i}\f$
    \return decodes the connectivity list and updates the x member, and returns a pair, where the first component is \f$N_{i,i}(G)\f$ and the second component is \f$l_{i}(G)\f$
   */
  pair<mpz_class, mpz_class> decode_node(int i, mpz_class tN);

  //! decodes the connectivity list of left vertices \f$i \leq v \leq j\f$ given \f$\tilde{N}_{i,j}\f$
  /*!
    \param i,j endpoints of the interval
    \param tN \f$\tilde{N}_{i,j}\f$
    \return decodes the connectivity list of vertices in the range and updated member x. Furthermore, returns a pair where the first component is \f$N_{i,j}(G)\f$ and the second is \f$l_{i,j}(G)\f$
   */
  pair<mpz_class, mpz_class> decode_interval(int i, int j, mpz_class tN);

  //! decodes the bipartite graph given the encoded integer
  /*!
    \param f which is \f$\lceil N(G) / \prod b_v! \rceil\f$
    \return the decoded bipartite graph \f$G\f$
   */
  b_graph decode(mpz_class f);
};


#endif
