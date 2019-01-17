#ifndef __BIPARTITE_GRAPH_COMPRESSION__
#define __BIPARTITE_GRAPH_COMPRESSION__

#include <iostream>
#include <vector>
#include "compression_helper.h"
#include "bipartite_graph.h"
#include "fenwick.h"

using namespace std;

//! Encodes a simple unmarked bipartite graph
class b_graph_encoder
{
  const b_graph& G; //!< the simple unmarked bipartite graph to be encoded
  vector<int> beta; //!< when compute_N is called for \f$i \leq j\f$, for \f$ i \leq v \leq n\f$, we have beta[v] = \f$b_v(i)\f$
  vector<int> a; //!< the degree sequence for the left nodes
  vector<int> b; //!< the degree sequence for the right nodes
  reverse_fenwick_tree U; //!< a Fenwick tree which encodes the degree of right nodes. When compute_N is called for \f$i \leq j\f$, for \f$i \leq v \leq n\f$, we have U.sum[v] \f$= \sum_{k = v}^n b_k(i)\f$.
public:
  //! constructor
  b_graph_encoder(const b_graph& Gin): G(Gin)
  {
    init();
  }

  //! initializes beta and U, and a as the left degree vector of G, and b as the right degree vector of G
  void init();


  //! computes \f$N_{i,j}(G)\f$
  /*!
    \param i,j the interval for which we compute \f$N_{i,j}(G)\f$
    \return A pair, where the first component is \f$N_{i,j}(G)\f$, and the second component is \f$l_{i,j}(G)\f$
   */
  pair<mpz_class, mpz_class> compute_N(int i, int j);


  //! encodes the given bipartite graph and returns an integer in the specified range
  mpz_class encode();

};

//! Decodes a simple unmarked bipartite graph
class b_graph_decoder
{
  int n; //!< number of vertices
  vector<int> a; //!< left degree sequence
  vector<int> b; //!< right degree sequence
  vector<vector<int> > x;//!< the adjacency list of left nodes for the decoded graph
  reverse_fenwick_tree U; //!< reverse Fenwick tree initialized with the right degree sequence b, and after decoding vertex \f$i\f$, for \f$i \leq v\f$, we have \f$U_v = \sum_{k=v}^{n-1} b_k(i)\f$
  reverse_fenwick_tree W; //!< keeping partial sums for the degree sequence a. More precisely, for \f$0 \leq v < n\f$, we have \f$W_v = \sum_{k=v}^{n-1} a_k\f$ 
  vector<int> beta; //!< the sequence \f$\vec{\beta}\f$, where after decoding vertex \f$i\f$, for \f$i \leq v \leq n\f$, we have \f$beta_v = b_v(i)\f$

public:
  //! constructor
  b_graph_decoder(vector<int> a_, vector<int> b_);

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
    \param f which is \f$\lceil N(G) / \prod b_v! \rceil
    \return the decoded bipartite graph \f$G\f$
   */
  b_graph decode(mpz_class f);
};



#endif
