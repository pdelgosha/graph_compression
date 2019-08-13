#ifndef __BITSTREAM__
#define __BITSTREAM__

#include <iostream>
#include <gmpxx.h>
#include <vector>

using namespace std;

const unsigned int BYTE_INT = sizeof(unsigned int); // number of bytes in an unsigned int data type
const unsigned int BIT_INT = 8 * sizeof(unsigned int); // the number of bits in an unsigned int data type



//! A sequence of arbitrary number of bits
/*!
  The bit_pipe class implements an arbitrary sequence of bits. This is useful for example when we want to use Elias delta code to write some integer to the output. This can lead to storage efficiencies, since in such cases we will need to work with incomplete bytes.

  The bits vector stores an array of chunks, each having 4 bytes (32 bits). For instance the sequence of bits <11001100110011> is stored as a single chunk <11001100110011|000000000000000000> of size 32 where the | sign shows that the remaining zeros are residuals (not part of data). This is stored as the last_bits variable. In this example, last_bits is 14 because there are 14 bits of data in the last chunk.
 */
class bit_pipe{
 private:
  vector<unsigned int> bits; //!< a vector of chunks, each of size 4 bytes. This represents an arbitrary sequence of bits
  int last_bits; //!< the number of bits in the last chunk (the last chunk starts from MSB, so the BIT_INT - last_bits many bits to the right (LSB) are empty and should be zero)
 public:
  bit_pipe(){bits.resize(0); last_bits = 0;}
  
  //! constructor given an integer
  bit_pipe(const unsigned int &n);

  //! constructor given an mpz_class object 
  bit_pipe(const mpz_class& n);

  //! shifts n bits to the right.
  void shift_right(int n);

  //! shift everything n bits to the left
  void shift_left(int n);

  //! write to the output
  friend ostream& operator << (ostream& o, const bit_pipe& B);

  //! return the number of chunks
  int size()const{return bits.size();}

  //! returns the number of residual bits in the last chunk
  int residue()const{return last_bits;}

  //! returns const reference to the bit sequence (object bits)
  const vector<unsigned int>& chunks() const{return bits;}

  //! append B to the left of me
  void append_left(const bit_pipe& B);

  //! shifts bits in B n bits to the left
  friend bit_pipe operator << (const bit_pipe& B, int n);

  //! shifts bits in B n bits to the right
  friend bit_pipe operator >> (const bit_pipe& B, int n);

  //! returns a reference to the nth chunk
  unsigned int& operator [](int n);

  //! returns a (const) reference to the nth chunk (const)
  const unsigned int& operator [](int n)const;

  friend class obitstream;
  friend class ibitstream;
};


//! handles writing bitstreams to binary files
/*!
  When trying to write to binary files, we sometimes need to write less than a byte, or a few bytes followed by say 2 bits. This is not possible unless we turn those 2 bits to 8 bits by basically adding 6 zeros. . However, if we want to do a lot of such operations, this can result in space inefficiencies. To avoid this, we can concatenate the bitstreams together and perhaps gain a lot in space. This class also handles Elias delta encoding of unsigned int and mpz_class. The way it is done is to buffer the data, write complete bytes to the output, and keeping the residuals for future operations.

  In order to make sure that the carry over from the last operation is also written to the output, we should call the close() function. 
 */
class obitstream{
 private:
  bit_pipe buffer; //!< a bit_pipe carrying the buffered data 
  FILE* f; //!< pointer to the binary output file
  //! writes complete chunks to the output
  void write(); 
 public:
  //! constructor 
  obitstream(string file_name){
    f = fopen(file_name.c_str(), "wb+");
  }

  //! write the bits given as unsigned int to the output
  void write_bits(unsigned int n, unsigned int nu_bits);

  //! uses Elias delta code to write a nonnegative integer to the output. In order to make sure that n >= 1, we effectively encode n + 1 instead 
  obitstream& operator << (const unsigned int& n);

  //! uses Elias delta code to write a nonnegative mpz_class integer to the output. In order to make sure that n >= 1, we effectively encode n + 1 instead 
  obitstream& operator << (const mpz_class& n);

  //! closes the session by writing the remaining chunk to the output (if any) and closing the file pointer f
  void close(); 
};

//! deals with reading bit streams from binary files, this is the reverse of obitstream
class ibitstream{
 private:
  FILE* f; //!< pointer to input binary file
  unsigned int buffer; //!< the last chunk read from the input
  unsigned int head_mask; //!< the place of the head bit in buffer, represented in terms of mask. So if we are in the LSB, head_mask is one, if we are in two bit left of LSB, this is 4 so on. When this is zero, it means the buffer is expired and we should probably read one more chunk from the input file
  unsigned int head_place; //!< the place of head represented in terms of integer, LSB is 1, left of LSB is 2, 2 left of LSB is 3 and so on. head_place is effectively the number of unread bits remaining in the buffer.

 public:

  //! reads one chunk (4 bytes) from the input file and stores it in buffer
  void read_chunk();

  //! read k bits from the input, interpret it as integer (first bit read is MSB) and return its value. Here, k must be in the range 1 <= k <= BIT_INT
  unsigned int read_bits(unsigned int k);

  //! reads k bits from input and stores in the given bit_pipe. \f$k \geq 1\f$ is arbitrary. The bits are stored in the bit_pipe so that can be interpreted as integer (e.g. mpz_class) so the LSB is located in the rightmost bit of the rightmost chunk (unlike the usual bit_pipe situation). We assume that the B given here is an empty bit_pipe
  void read_bits(int k, bit_pipe& B);

  //! similar to read_bits, but B does not have to be empty and the result will be appended to B (this is used in order to recursively implement read_bits)
  void read_bits_append(int k, bit_pipe& B);

  //! read one bit from input and return true if its value is 1 and false otherwise.
  bool read_bit();

  ibitstream(string file_name){
    f = fopen(file_name.c_str(), "rb+");
    buffer =0;
    head_mask = 0;
    head_place = 0;
  }

  //! reads an unsigned int from the input using Elias delta decoding and saves it in the reference given 
  ibitstream& operator >> (unsigned int& n);

  //! reads a nonnegative mpz_class integer using Elias delta decoding and stores in the reference given
  ibitstream& operator >> (mpz_class& n);

  //! closes the session by closing the input file
  void close(){fclose(f);}
};


//! returns number of bits in a positive integer n, e.g. 3 has 3 bits, 12 has 4 bits, and 0 has 0 bits.
unsigned int nu_bits(unsigned int n);

//! generates a binary mask with n consecutive ones in LSB
unsigned int mask_gen(int n);

//! returns the Elias delta representation of an integer in bit_pipe format
bit_pipe elias_delta_encode(const unsigned int &n);

//! performs Elias delta encode for an integer, and stores the results in the given reference to bit_pipe objects

void elias_delta_encode(const unsigned int &n, bit_pipe& B);

//! returns the Elias delta representation of an mpz_class in bit_pipe format
bit_pipe elias_delta_encode(const mpz_class &n);

//! performs Elias delta encoding on n, and stores the results in the given reference to bit_pipe objects
void elias_delta_encode(const mpz_class &n, bit_pipe& B);


#endif


