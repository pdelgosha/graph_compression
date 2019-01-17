#ifndef __COMPRESSION_HELPER__
#define __COMPRESSION_HELPER__

#include <iostream>
#include <gmpxx.h>
#include <vector>

using namespace std;

//! This function computes the product of consecutive integers separated by a given iteration 
/*!
  \param N The first term in the product
  \param k the number of terms in the product
  \param s the iteration
  \return the product N \f$ N \times (N-s) \times (N-2s) \times \dots \times (N-(k-1)s) \f$
*/
mpz_class compute_product(mpz_class N, mpz_class k, int s);


//! computes the binomial coefficient n choose m = n! / m! (n-m)!
/*!
  \param n integer
  \param m integer
  \return the binomial coefficient n! / m! (n-m)!. If n <= 0, or  m > n, or m <= 0, returns 0
*/
mpz_class binomial(const mpz_class n, const mpz_class m);


//! computes the product of factorials in a vector given a range
/*!
  \param a vector of integers
  \param i,j endpoints of the interval
  \return \f$\prod_{v = i}^j a_v ! \f$
*/
mpz_class prod_factorial(const vector<int>& a, int i, int j);


#endif
