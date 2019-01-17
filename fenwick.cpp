#include "fenwick.h"


//==============================
// fenwick_tree
//==============================

fenwick_tree::fenwick_tree(vector<int> vals)
{
  int n = vals.size();
  sums.resize(n+1);
  // initializes at zero
  for (int i=1;i<=n;i++)
    sums[i] = 0;
  for (int i=0;i<n;i++)
    add(i,vals[i]); // add values one by one
}

void fenwick_tree::add(int k, int val)
{
  k = k +1; // the sums vector is one based while the index k was zero based
  while (k < sums.size()){
    sums[k] += val;
    k += (k & -k);
  }
}


int fenwick_tree::sum(int k)
{
  k = k +1; // the sums vector is one based while the index k was zero based
  int sum_computed = 0;
  while (k > 0){
    sum_computed += sums[k];
    k -= (k & -k); // reduce the lsb bit
  }
  return sum_computed;
}


//==============================
// reverse_fenwick_tree
//==============================


reverse_fenwick_tree::reverse_fenwick_tree(vector<int> vals)
{
  reverse(vals.begin(),vals.end()); // reverse the array and then use the previously defined fenwick_tree class
  FT = fenwick_tree(vals);
}

void reverse_fenwick_tree::add(int k, int val)
{
  FT.add(FT.size() - 1 - k, val);
}


int reverse_fenwick_tree::sum(int k)
{
  if (k >= size())
    return 0;
  return FT.sum(FT.size() - 1 - k); 
}
