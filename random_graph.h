#ifndef __RANDOM_GRAPH__
#define __RANDOM_GRAPH__

#include "marked_graph.h"
#include <random>
#include <chrono>
#include <vector>
#include <set>
using namespace std;

//! generates a marked Erdos Renyi graph
/*!
  \param n: the number of vertices 
  \param p: probability of an edge being present
  \param ver_mark: the number of possible vertex marks
  \param edge_mark: the number of possible edge marks
  \return a random marked graph, where each edge is independently present with probability p, each vertex has a random integer mark in the range [0,ver_mark), and each edge has two random integers marks in the range [0,edge_mark)
 */

marked_graph marked_ER(int n, double p, int ver_mark, int edge_mark);

//! generates a random graph where roughly speaking, the degree of a vertex is Poisson
/*!
  \param n: the number of vertices
  \param deg_mean: mean of Poisson
  \param ver_mark: the number of possible vertex marks
  \param edge_mark: the number of possible edge marks
  \return A random graph, where each vertex chooses its degree according to Poisson(deg_mean), then picks neighbors uniformly at random, and connects to them (if the neighbors have not already connected to them, if some of the neighbors I pick are already connected to me, I just don't do anything). Vertex and edge marks are picked independently and uniformly.
 */

marked_graph poisson_graph(int n, double deg_mean, int ver_mark, int edge_mark);


#endif
