#ifndef __SIMPLE_GRAPH_COMPRESSION__
#define __SIMPLE_GRAPH_COMPRESSION__

#include <vector>
#include <math.h>
#include "simple_graph.h"
#include "compression_helper.h"
#include "fenwick.h"
#include "logger.h"

//! Encodes a simple unmarked graph
/*!
  Encodes a simple graph in the set of graphs with a given degree sequence a. Therefore, to construct an encoder object, we need to specify this degree sequence as a vector of int. For instance (in c++11)

  ~~~~~~~~~{.cpp}
  vector<int> a = {3,2,2,3};
  graph_encoder E(a);
  ~~~~~~~~~

  constructs an encode object E which is capable of encoding graphs having 4 nodes with degrees 3, 2, 2, 3 (in order). Hence, assume that we have defined such a graph by giving forward adjacency list:

  ~~~~~~~~~{.cpp}
  graph G({1,2,3},{3},{3},{});
  ~~~~~~~~~

  Note that G has a degree sequences which is equal to a. Then, we can use E to encode G as follows:

  ~~~~~~~~~{.cpp}
  pair<mpz_class, vector<int> > f = E.encode(G);
  ~~~~~~~~~

  In this way, the encode converts G to a pair stored in f, where its first part is an integer, and the second part is the array of integers \f$\tilde{S}\f$. Later on, we can use f to decode G.
 */
class graph_encoder{
  //const graph& G; //!< the simple unmarked graph which is going to be encoded
  int n; //!< the number of vertices 
  vector<int> a; //!< the degree sequence
  vector<int> beta; //!<  When compute_N is called for \f$i \leq j\f$, for \f$i \leq v \leq n\f$, we have \f$\beta_v = d_v(i)\f$.
  reverse_fenwick_tree U; //!< a Fenwick tree which encodes the forward degrees to the right. When compute_N is called for \f$i \leq j\f$, for \f$i \leq v\f$, we have \f$U_v = \sum_{k=v}^n d_k(i)\f$.
  vector<int> Stilde; //!< Summation of forward degrees at \f$n / \log^2 n\f$ many points.
  int logn2; //!< \f$\lfloor \log_2 n \rfloor^2\f$ where \f$n\f$ is the number of vertices

 public:
  //! constructor
  graph_encoder(const vector<int>& a_);


  //! initializes beta and U, clears Stilde for a fresh use
  void init(const graph& G);
  
  //! computes \f$N_{i,j}(G)\f$
  /*!
    \param i,j the interval for which we compute \f$N_{i,j}(G)\f$
    \param I The integer index corresponding to the current interval (follows a heap convention).
    \param G the reference to the simple graph G
    \return A pair, where the first component is \f$N_{i,j}(G)\f$ and the second component is \f$l_{i,j}(G)\f$.
   */

  pair<mpz_class, mpz_class> compute_N(int i, int j, int I, const graph& G);

  //! Encodes the graph and returns N together with Stilde
  /*!
    \param G: reference to the graph to encode 
    \return A pair, where the first component is \f$\lceil N(G) / \prod_{i=1}^n a_i! \rceil\f$ where \f$N(G) = N_{0,n-1}(G)\f$ and \f$a\f$ is the degree sequence of the graph, and the second component is the vector Stilde which stores partial mid sum of intervals and has length roughly \f$n / \log^2 n\f$
   */
  pair<mpz_class, vector<int> > encode(const graph& G);

};

//! Decodes a simple unmarked graph
/*!
  Decodes a simple graph given its encoded version. We assume that the decoder knows the degree sequences of the encoded graph, hence these sequences must be given when a decoder object is being constructed. For instance, borrowing the degree sequence of the example we used to explain the graph_encoder class:
  
  ~~~~~~~~~~{.cpp}
  vector<int> a = {3,2,2,3};
  b_graph_decoder D(a);
  ~~~~~~~~~~

 Then, if variable f of type `pair<mpz_class, vector<int> >` is obtained from a graph_encoder class, we can reconstruct the graph using f:

 ~~~~~~~~~{.cpp}
 graph Ghat = D.decode(f.first, f.second);
 ~~~~~~~~~

 Then, the graph Ghat will be equal to the graph G. Here is a full example showing the procedure of compression and decompression together:

 ~~~~~~~~~~{.cpp}
 vector<int> a = {3,2,2,3}; // degree sequence
     
 graph G({1,2,3},{3},{3},{}); // defining the graph

 graph_encoder E(a); // constructing the encoder object
 pair<mpz_class, vector<int> > f = E.encode(G);

 graph_decoder D(a);
 graph Ghat = D.decode(f.first, f.second);

 if (Ghat == G)
    cout << " we successfully reconstructed the graph! " << endl;
 ~~~~~~~~~~

 */
class graph_decoder{
  vector<int> a; //!< the degree sequence of the graph.
  int n; //!< the number of vertices, which is a.size()
  int logn2; //!< \f$\lfloor \log_2 n \rfloor^2\f$
  vector<vector<int> > x; //!< the forward adjacency list of the decoded graph
  vector<int> beta; //!< the sequence \f$\vec{\beta}\f$, where after decoding vertex \f$i\f$, for \f$i \leq v\leq n\f$ we have \f$\beta_v = d_v(i)\f$.
  reverse_fenwick_tree U; //!< a Fenwick tree initialized with the degree sequence a, and after decoding vertex \f$i\f$, for \f$i \leq v\f$, we have \f$U_v = \sum_{k=v}^{n-1} d_k(i)\f$.
  vector<int> tS;  //!< the \f$\tilde{S}\f$ vector, which stores the partial sums for the midpoints of intervals with length more than \f$\log^2 n\f$.
public:
  //! constructor given the degree sequence
  graph_decoder(vector<int> a_);

  //! initializes x to be empty vector of size n, and U and beta by a
  void init();

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
