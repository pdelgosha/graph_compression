#ifndef __SIMPLE_GRAPH_COMPRESSION__
#define __SIMPLE_GRAPH_COMPRESSION__

#include <vector>
#include <math.h>
#include "simple_graph.h"
#include "compression_helper.h"
#include "fenwick.h"

//! Encodes a simple unmarked graph
/*!
  This class has a reference to a simple unmarked graph, and encodes it using the counting algorithm which counts the number of configurations resulting in a graph lexicographically smaller than the reference graph. It is assumed that both the encode and the decode know the number of vertices and also the degree profile of the graph.
 */
class graph_encoder{
  const graph& G; //!< the simple unmarked graph which is going to be encoded
  vector<int> beta; //!<  When compute_N is called for \f$i \leq j\f$, for \f$i \leq v \leq n\f$, we have \f$\beta_v = d_v(i)\f$.
  reverse_fenwick_tree U; //!< a Fenwick tree which encodes the forward degrees to the right. When compute_N is called for \f$i \leq j\f$, for \f$i \leq v\f$, we have \f$U_v = \sum_{k=v}^n d_k(i)\f$.
  vector<int> Stilde; //!< Summation of forward degrees at \f$n / \log^2 n\f$ many points.
  int logn2; //!< the integer part of \f$\log^2 n\f$ where \f$n\f$ is the number of vertices in \f$G\f$.
 public:
  //! constructor
 graph_encoder(const graph& Gin): G(Gin)
  {
    init();
  }
  
  //! initializes beta and U, logn2, and resizes Stilde.
  void init();
  
  //! computes \f$N_{i,j}(G)\f$
  /*!
    \param i,j the interval for which we compute \f$N_{i,j}(G)\f$
    \param I The integer index corresponding to the current interval (follows a heap convention).
    \return A pair, where the first component is \f$N_{i,j}(G)\f$ and the second component is \f$l_{i,j}(G)\f$.
   */

  pair<mpz_class, mpz_class> compute_N(int i, int j, int I);

  //! Encodes the graph and returns N together with Stilde
  /*!
    \return A pair, where the first component is \f$\lceil N(G) / \prod_{i=1}^n a_i! \rceil\f$ where \f$N(G) = N_{0,n-1}(G)\f$ and \f$a\f$ is the degree sequence of the graph, and the second component is the vector Stilde which stores partial mid sum of intervals and has length roughly \f$n / \log^2 n\f$
   */
  pair<mpz_class, vector<int> > encode();

};

//! Decodes a simple unmarked graph
/*!
  This class received a number \f$\tilde{N}\f$ and finds a simple unmarked graph.
  We assume that the degree sequence of the graph is known as well.
 */
class graph_decoder{
  vector<int> a; //!< the degree sequence of the graph.
  int n; //!< the number of vertices, which is a.size()
  int logn2; //!< the integer part of \f$\log^2 n\f$
  vector<vector<int> > x; //!< the forward adjacency list of the decoded graph
  vector<int> beta; //!< the sequence \f$\vec{\beta}\f$, where after decoding vertex \f$i\f$, for \f$i \leq v\leq n\f$ we have \f$\beta_v = d_v(i)\f$.
  reverse_fenwick_tree U; //!< a Fenwick tree initialized with the degree sequence a, and after decoding vertex \f$i\f$, for \f$i \leq v\f$, we have \f$U_v = \sum_{k=v}^{n-1} d_k(i)\f$.
  vector<int> tS;  //!< the \f$\tilde{S}\f$ vector, which stores the partial sums for the midpoints of intervals with length more than \f$\log^2 n\f$.
public:
  //! constructor given the degree sequence
  graph_decoder(vector<int> a_);

  //! given \f$\tilde{N}\f$ and a vector \f$\tilde{S}\f$, decodes the graph and returns an object of type graph
  graph decode(mpz_class f, vector<int> tS_);

  //! decode the node i
  /*!
    \param i the vertex index
    \param tN \f$\tilde{N}_{i,i}\f$
    \return a pair \f$(N_{i,i}, l_i)\f$ where \f$l_i = l_i(G)\f$ and \f$N_{i,i} = N_{i,i}(G)\f$
   */
  pair<mpz_class, mpz_class> decode_node (int i, mpz_class tN);


  //! decodes the interval \f$[i,j]\f$ with interval index \f$I\f$.
  /*!
    \param i,j intervals endpoints
    \param I the index of the interval
    \param tN \f$\tilde{N}_{i,j}\f$
    \param Sj \f$S_{j+1}\f$
    \return a pair \f$N_{i,j}, l_{i,j}\f$ where \f$N_{i,j} = N_{i,j}(G)\f$ and \f$l_{i,j} = l_{i,j}(G)\f$
   */
  pair<mpz_class, mpz_class> decode_interval(int i, int j, int I, mpz_class tN, int Sj);

};


#endif 
