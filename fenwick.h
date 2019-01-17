
#ifndef __FENWICK__
#define __FENWICK__

#include <vector>


using namespace std;

//! Fenwick tree class
/*!
  this class computes the partial sums of an array. More precisely, we feed it a vector of integers, and it can compute the sum of values up to a certain index efficiently. Moreover, we can change the value of an index. Both these operations are done in \f$O(\log n)\f$ where \f$n\f$ is the size of the array. 
 */
class fenwick_tree{
  //! a one based vector containing sum of values
  /*! sums[k] contains the sum of values in the interval (k-lsb(k), k]. Here
    lsb(k) denotes the rightmost one in k.
  */
  vector<int> sums;
 public:
  //! default constructor
  fenwick_tree()
    {
      sums.resize(0);
    }

  //! constructor, which takes a vector of values and initializes
  fenwick_tree(vector<int>);
  
  //! the size of the array, which is sums.size()-1, since sums is one based
  int size()
  {
    return sums.size() - 1;
  }
  
  /*! gets a (zero based) index k, and add to that value
    \param k the index to be modified, this is zero based
    \param val the value to be added to the above index
  */
  void add(int k, int val);
  
  /*!
    returns the sum of values from 0 to k
    \param k the index up to which (including) the sum is computed
  */
  int sum(int k);
};

//! similar to the fenwick_tree class, but instead of prefix sums, this class computes suffix sums.
/*
  Similar to the Fenwick tree class, but returns sum of values from a given index until the end of the array.
  This is implemented based on the previous fenwick_tree class. 
 */
class reverse_fenwick_tree{
  fenwick_tree FT; //!< member of type fenwick_tree, which saves the partial sums for the reversed array.
public:
  //! default constructor
  reverse_fenwick_tree(){}

  //! constructor which receives values and initializes
  reverse_fenwick_tree(vector<int>);

  /*! gets a (zero based) index k, and add to that value
    \param k the index to be modified, this is zero based
    \param val the value to be added to the above index
  */
  void add(int k, int val);

  //! the number of elements in the original array 
  int size(){
    return FT.size();
  }

  /*!
    returns the sum of values from index k until the end of the array
    \param k the index from which (including) the sum is computed
  */
  int sum(int k);


};




#endif
