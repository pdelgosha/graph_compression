#include "bitstream.h"

/*!
  Some examples: n = 1, bits = 1|0000000 (followed by 3 zero bytes)
  n = 12, bits = 1100|0000 (followed by 3 zero bytes)
  n = 255633, bits = 11111001 10100100 01|000000 (followed by a zero byte)
*/
bit_pipe::bit_pipe(const unsigned int &n){
  bits.resize(1);
  bits[0] = n;
  last_bits = nu_bits(n);
  bits[0] <<= BIT_INT - last_bits; // so that n appears in the MSB place 
}

bit_pipe::bit_pipe(const mpz_class& n){
  size_t n_bits = mpz_sizeinbase(n.get_mpz_t(), 2);
  size_t size = n_bits / BIT_INT + 1; // how many unsigned int chunks we need
  bits.resize(size);
  mpz_export(&bits[0],
             &size,
             1, // order can be 1 for most significant word first or -1 for least significant first
             BYTE_INT, // size: each word will be size bytes and
             0, // Within each word endian can be 1 for most significant byte first, -1 for least significant first, or 0 for the native endianness of the host CPU.
             0, // The most significant nails bits of each word are unused and set to zero, this can be 0 to produce full words.
             n.get_mpz_t());
  bits.resize(size);
  last_bits = BIT_INT; // at the moment LSB of n is the LSB bit of the rightmost chunk
  // but we need the MSB of n to be the MSB of the leftmost chunk
  // in order to do this, we must shift left
  // but how much? it is related to the remainder of bit count in n with respect to BIT_INT
  int rem = n_bits % BIT_INT; // the remainder 
  if (rem != 0){
    // if remainder is zero, nothing should be done
    // otherwise, shift left BIT_INT - rem bits
    shift_left(BIT_INT - rem);
  }
  
}

/*!
  Example: if bits is 11111111 11111000 and n = 5, then bits becomes 00000111 11111111 11000000 (trailing zero bytes are not shown in this example)
*/
void bit_pipe::shift_right(int n){
  if (n == 0)
    return; // nothing to do
  if (n >= BIT_INT){
    bits.insert(bits.begin(), n / BIT_INT, 0); // n/BIT_INT bytes each zero will be added
    shift_right(n%BIT_INT);
    return;
  }
  // when we arrive at this line, n must be strictly less than BIT_INT and strictly bigger than zero, i.e. 0 < n < BIT_INT
  unsigned int mask = mask_gen(n); // mask is going to be n many ones (in LSB), e.g. if n = 3, mask is 00000111, this is useful in carrying over LSB of left bytes to the right bytes
  unsigned int carry_current = 0; // carry over of left bytes to the right. For instance, if we want to shift 11111111 3 bits to the right, it becomes 00011111 but a carry over 111 must be added to the byte to the right. This is initially zero
  unsigned int carry_prev = 0; // the same concept, but for the previous byte (to the left of me). 
  for (int i=0;i<bits.size();i++){
    carry_current = bits[i] & mask; // find carryover bits for current byte
    bits[i] >>= n; // shift the current byte
    carry_prev <<= (BIT_INT-n); // put the previous carryover bits in place to be added to the current byte
    bits[i] |= carry_prev; // add the carryover to the current byte
    carry_prev = carry_current; // the current byte is the previous byte for the next byte
  }
    
  if (n > (BIT_INT - last_bits)){
    // the LSB bits of the last chunk must fall into a new chunk, so I should push_back a new chunk, which is zero for now
    carry_prev <<= (BIT_INT-n);
    bits.push_back(carry_prev); // the last byte is the last carryover shifted to the left 
  }
  last_bits += n;
  if (last_bits > BIT_INT)
    last_bits -= BIT_INT;
}


void bit_pipe::shift_left(int n){
  if (n < 0){
    cerr << " ERROR: bit_pipe::shift_left called for negative value " << n << endl;
    return;
  }
  if (n >= BIT_INT){
    // we need to remove a number of bytes
    int bytes_to_remove = n / BIT_INT; // these many bytes must be remove
    bits.erase(bits.begin(), bits.begin() + bytes_to_remove);
    n = n % BIT_INT;
  }
  if (n == 0)
    return;

  // when we reach at this line, we have 1 <= n <= 7
  unsigned int mask = mask_gen(n) << (BIT_INT-n); // n bits in MSB for carryover masking
  unsigned int carry; // carryover to the left byte
  for (int i=0;i<bits.size(); i++){
    carry = (mask & bits[i]) >> (BIT_INT-n); // bring it to the right
    if ( i> 0)
      bits[i-1] |= carry; // add carry to the left guy
    bits[i] <<= n; 
  }

  // now, deal with last_bits
  last_bits -= n;
  if (last_bits <= 0){
    // means that the rightmost byte must vanish
    last_bits += BIT_INT;
    bits.pop_back(); // remove the last byte
  }
}




ostream& operator << (ostream& o, const bit_pipe& B){
  if (B.bits.size()==0){
    o << "<>";
    return o;
  }
  o << "<";
  for (int i=0;i<(B.bits.size()-1); i++){ // the last byte requires special handling
    bitset<BIT_INT> b(B.bits[i]);
    o << b << " ";
  }
  unsigned int last_byte = B.bits[B.bits.size()-1];
    
  for (int k=BIT_INT;k>(BIT_INT-B.last_bits);k--){ // starting from MSB bit to LSB for existing bits
    if (last_byte & (1<<(k-1)))
      o << "1";
    else
      o << "0";
  }
  o << "|"; // to show the place of the last bit
  for (int k=BIT_INT-B.last_bits; k>=1; k--){
    if (last_byte &(1<<(k-1)))
      o << "1";
    else
      o << "0";
  }
  o << ">";
  return o;
}



/*!
  Example: if this is <1100|0000> and B is <11110000 1111|0000> then this becomes <11110000 11111100> (trailing zero bytes not shown in example)
*/
void bit_pipe::append_left(const bit_pipe& B){
  if (B.size() == 0) // nothing should be done, B is empty
    return;
  int B_res = B.residue(); // number of incomplete bits in B
  if (B_res == BIT_INT){
    // B has complete chunks, so I just need to insert chunks of B at the beginning of my chunks
    bits.insert(bits.begin(), B.chunks().begin(), B.chunks().end());
    return; // all set!
  }
  // B has a residue
  // so I need to shift myself to the right and then append
  shift_right(B_res);
  // then, my leftmost chunk must be combined with the rightmost chunk of B:
  bits[0] |= B[B.size()-1];
  // then insert all but the rightmost chunk of B at my left
  bits.insert(bits.begin(), B.chunks().begin(), B.chunks().end()-1);
}


bit_pipe operator << (const bit_pipe& B, int n){
  bit_pipe ans = B;
  ans.shift_left(n);
  return ans;
}

bit_pipe operator >> (const bit_pipe& B, int n){
  bit_pipe ans = B;
  ans.shift_right(n);
  return ans;
}


unsigned int& bit_pipe::operator [](int n){
  if (n < 0 or n >= bits.size()){
    cerr << " ERROR: bit_pipe::operator [] called for value out of range " << n << " the range is [0, " << bits.size()-1 << "]" << endl;
  }
  return bits[n];
}


const unsigned int& bit_pipe::operator [](int n)const{
  if (n < 0 or n >= bits.size()){
    cerr << " ERROR: bit_pipe::operator [] called for value out of range " << n << " the range is [0, " << bits.size()-1 << "]" << endl;
  }
  return bits[n];
}

void obitstream::write(){
  if (buffer.bits.size() > 1){
    // write the first chunks to the output
    fwrite(&buffer.bits[0], sizeof(unsigned int), buffer.bits.size()-1, f);
    // add the number of chunks written to chunks_written
    chunks_written += buffer.bits.size() -1;
    // then, remove the first buffer.bits.size()-1 chunks which were written to the output
    buffer.bits.erase(buffer.bits.begin(), buffer.bits.begin() + buffer.bits.size()-1); 
  }
}

/*!
  \param n: bits to be written to the output in the form of an unsigned int (4 bytes of data)
  \param nu_bits: number of bits, counted from LSB of n, to write to the output. For instance if n = 1 and nu_bits = 1, a single bit with value 1 is written 
*/
void obitstream::write_bits(unsigned int n, unsigned int nu_bits){
  unsigned int buffer_backup = 0; // the backup of the remaining chunk in 
  int buffer_res = 0; // number of bits remaining in the buffer
  if (buffer.bits.size() != 0){
    buffer_backup = buffer.bits[0];
    buffer_res = buffer.last_bits;
  }
  buffer.bits.resize(1);
  buffer.bits[0] = n << (BIT_INT - nu_bits); // shift left so that exactly nu_bits many bits are in the buffer
  buffer.last_bits = nu_bits;
  buffer.shift_right(buffer_res); // open up space for the residual of the previous operation
  buffer.bits[0] |= buffer_backup; // add the residual 
  write(); // write the buffer to the output
}


obitstream& obitstream::operator << (const unsigned int& n){
  if (buffer.bits.size() > 1){
    cerr << " ERROR: buffer has more than 1 chunk! " << endl;
  }
  unsigned int buffer_backup = 0; // the backup of the remaining chunk in 
  int buffer_res = 0; 
  if (buffer.bits.size() != 0){
    buffer_backup = buffer.bits[0];
    buffer_res = buffer.last_bits;
  }
  elias_delta_encode(n+1, buffer); // find the delta encoded version of n + 1 
  buffer.shift_right(buffer_res); // open up space for the residual of the previous operation
  buffer.bits[0] |= buffer_backup; // add the residual
  write();
  return *this;
}

obitstream& obitstream::operator << (const mpz_class& n){
  if (buffer.bits.size() > 1){
    cerr << " ERROR: buffer has more than 1 chunk! " << endl;
  }
  unsigned int buffer_backup = 0; // the backup of the remaining chunk in 
  int buffer_res = 0; 
  if (buffer.bits.size() != 0){
    buffer_backup = buffer.bits[0];
    buffer_res = buffer.last_bits;
  }
  elias_delta_encode(n+1, buffer); // find the delta encoded version of n + 1 
  buffer.shift_right(buffer_res); // open up space for the residual of the previous operation
  buffer.bits[0] |= buffer_backup; // add the residual
  write();
  return *this;
}

void obitstream::close(){
  if (buffer.bits.size() > 0){
    fwrite(&buffer.bits[0], sizeof(unsigned int), buffer.bits.size(), f);
    buffer.bits.clear();
    buffer.last_bits = 0;
  }
  fclose(f);
}


void ibitstream::read_chunk(){
  fread(&buffer, sizeof(unsigned int), 1, f);
  //cout << " in read chunk  buffer = " << bitset<32>(buffer) << endl;
  head_mask = 1 << (BIT_INT - 1); // pointing to the MSB which is the first bit to consider
  head_place = BIT_INT;
}


unsigned int ibitstream::read_bits(unsigned int k){

  //cerr << " read bits with k = " << k << endl;
  if (k < 1 or k > BIT_INT){
    cerr << "ERROR: ibitstream::read_bits called with k out of range, k = " << k << endl;
  }
  if (head_place == 0)// no bits left
    read_chunk();
  if (head_place >= k){ // head_place is effectively the number of unread bits remaining in the buffer
    //cerr << " head_place >= k head_mask = " << head_mask << " head_place = " << head_place << endl;
    // there are enough number of bits in the current buffer to read
    // mask the input
    unsigned int mask = mask_gen(k); // k ones
    // now we should shift mask to start at head_place
    mask <<= (head_place - k);
    unsigned int ans = buffer & mask; // mask out the corresponding bits
    ans >>= (head_place - k); // bring it back to LSB

    // we need to shift head k bits to the right
    // in some compilers, >>= 32 does strange things, in fact it does nothing. To avoid that, I shift k - 1 bits and then an extra 1 bit
    head_mask >>= k - 1;
    head_mask >>= 1;
    head_place -= k;
    //cerr << " after head_mask " << head_mask << " head_place = " << head_place << endl;
    return ans;
  }else{
    // there is not enough bits in the current buffer.
    // So we should read head_place many bits from the current buffer
    // then read another chunk from input file
    // and then read k - head_place bits from the new buffer
    // we do these two steps recursively
    // but first need to store the number of bits we will have to read in the future, since these variables will be modified later:
    unsigned int future_bits = k - head_place;
    unsigned int a = read_bits(head_place); // the bits from the current buffer
    read_chunk();
    unsigned int b = read_bits(future_bits); // bits from the next buffer
    // now we need to combine these
    // in order to do so, we need to shift a to the left and combine with b
    // but the number of bits we need to shift a is exactly future bits
    a <<= future_bits;
    return a | b;
  }
}

void ibitstream::read_bits_append(int k, bit_pipe& B){
  //cout << " read_bits called k = " << k << " head_place = " << head_place << endl;
  // by assumption, when calling this function, B has full chunks (last_bits is either zero so BIT_INT)
  if (k == 0)
    return; // nothing remains to be done
  if (head_place == 0){
    // we are over with the current bits in the buffer
    // so we need to load a few chunks from the input
    // we should append k / BIT_INT full chunks to B and then k % BIT_INT bits from the next chunk
    unsigned int full_chunks = k / BIT_INT;
    if (full_chunks > 0){
      B.bits.resize(B.bits.size() + full_chunks);
      fread(&B.bits[B.bits.size() - full_chunks], sizeof(unsigned int), full_chunks, f); // read full_chunks many chunks
      B.last_bits = BIT_INT; // the last chunk contains full bits 
    }
    unsigned int res_bits = k % BIT_INT; // the remaining bits to be read
    if (res_bits > 0){
      // we need to read an extra res bits
      unsigned int res = read_bits (res_bits); // read res many bits
      // we should shift res_bits so that its MSB is the leftmost bits of the chunk
      res <<= (BIT_INT - res_bits);
      B.bits.push_back(res);
      B.last_bits = res_bits;
    }
  }else{
    if (k <= head_place){
      // there are enough bits to read
      unsigned int a = read_bits(k);
      // no need to shift a since we need LSB of a to be in the rightmost bit
      B.bits.push_back(a);
      B.last_bits = BIT_INT;
    }else{
      // read head_place bits and call again
      unsigned int future_read; // number of bits to read in future after calling the read_bits function below
      future_read = k - head_place; 
      unsigned int a = read_bits(head_place);
      B.bits.push_back(a);
      B.last_bits = BIT_INT;
      read_bits_append(future_read, B); // read the remaining bits 
    }
  }

  B.shift_right(BIT_INT - B.last_bits); // so that LSB of B is the rightmost bit of the lats chunk.

  // this is important to make sure that B is correctly representing an integer and can be converted to mpz_class
  // TODO issue of 2^k - 1 correct
}

void ibitstream::read_bits(int k, bit_pipe& B){
  //cerr << " read_bits " << k << endl;
  // assumption: B is empty bit_pipe
  if (B.bits.size() != 0 or B.last_bits != 0){
    cerr << " ERROR: ibitstream::read_bits(int k, bit_pipe& B) must be called with an empty bit_pipe, a nonempty bitpipe is given with B.bits.size() = " << B.bits.size() << endl;
  }
  read_bits_append(k, B);
  // there might be a few zero chunks at the beginning of B which are redundant, we remove them here
  // the number of nonzero chunks is exactly the floor of k / BIT_INT
  int nonzero_chunks = k / BIT_INT;
  if (k % BIT_INT != 0)
    nonzero_chunks ++; // take the floor
  //cerr << " nonzero_chunks " << nonzero_chunks << endl;
  if (nonzero_chunks < B.bits.size())
    B.bits.erase(B.bits.begin(), B.bits.begin() + B.bits.size() - nonzero_chunks);
}

ibitstream& ibitstream::operator >> (unsigned int& n){
  // implement Elias delta decoding
  //bitset<32> B(buffer);
  //cerr << " buffer " << B << endl;
  //cerr << " head " << bitset<32>(head_mask) << endl;
  //cerr << " head position " << head_place << endl;
  
  unsigned int L = 0;
  while (!read_bit()){
    // read until reach one
    L++;
  }
  //cerr << " L " << L << endl;
  
  unsigned int N;
  if (L == 0){// special case, avoid going over further calculations
    n = 0; // we had subtracted one when encoding 
    return *this;
  }
  N = read_bits(L); // read L digits
  
  N += (1<<L); // we must add 2^L
  N --; // this was N + 1
  
  n = read_bits(N); // read N digits
  n += (1 << N); // we must add 2^N
  n --; // when we encoded, in order to get a positive integer, we added one, now we subtract one
  return *this;
}

ibitstream& ibitstream::operator >> (mpz_class& n){
  unsigned int L = 0;
  //cout << "head_place " << head_place << endl;
  //cout << "head_mask  " << head_mask << endl;
  while (!read_bit()){
    // read until reach one
    L++;
  }

  //cout << " L = " << L << endl;
  unsigned int N;
  if (L == 0){// special case, avoid going over further calculations
    n = 0; // we had subtracted one when encoding 
    return *this;
  }

  N = read_bits(L); // read L digits
  
  N += (1<<L); // we must add 2^L
  N --; // this was N + 1

  // we must read N bits and form n based on that

  bit_pipe B;
  //cout << " N " << N << endl;
  read_bits(N, B);
  //cout << " B first " << B << endl;
  // we should add a leading 1 to B
  // in order to do so, we should consider 2 cases:
  if (N % BIT_INT == 0){
    // this is the tricky case, since B now contains full chunks and there is no room to add the leading 1
    // so we need to insert a chunk at the beginning and place the leading bit there
    // since the leading bit will be in the rightmost bit in this case, the value of the initial chunk is 1 in this case
    B.bits.insert(B.bits.begin(), 1);
  }else{
    // in this case, the lading bit will be placed in the first chunk of B
    B.bits[0] |= (1 << (N % BIT_INT));
  }
  //cout << " B " << B << endl;
  //cout << B.bits[0] << endl;

  // construct the mpz_clas
  mpz_import(n.get_mpz_t(),
             B.bits.size(), // the number of words
             1, // order: 1 means first significant word first
             sizeof(unsigned int), // each word is this many bytes
             0, // endian can be 1 for most significant byte first, -1 for least significant first, or 0 for the native endianness of the host CPU.
             0, // nails
             &B.bits[0]); //&B.bits[0]);

  n --; // when encoding, we added 1 to make sure it is positive
  return *this;
}


bool ibitstream::read_bit(){
  if (head_mask == 0){ // nothing is in buffer
    //cerr << " read a chunk " << endl;
    read_chunk();
  }
  bool ans = head_mask & buffer; // look at the value of buffer at the bit where the head_mask is pointing to
  head_mask >>= 1; // go one bit to the right
  head_place --;
  //cerr << " read bit " << ans << endl;
  return ans;
}



/*!
  This is in fact nothing but \f$\lfloor \log_2 n \rfloor + 1\f$
*/
unsigned int nu_bits(unsigned int n){
  int nu_bits = 0;
  unsigned int n_copy = n;
  while (n_copy > 0){
    nu_bits ++;
    n_copy >>= 1;
  }
  return nu_bits; 
}


/*!
  Example: n = 1 -> 00000001, n = 7 -> 01111111
*/
unsigned int mask_gen(int n){
  if (n < 1 or n > BIT_INT){
    cerr << " ERROR: mask_gen called for n outside the range [1,BIT_INT] " << endl;
    return 0;
  }
  unsigned int mask = 1; 
  for (int i=1; i<n; i++){
    mask <<= 1;
    mask += 1;
  }
  return mask;
}

bit_pipe elias_delta_encode(const unsigned int &n){
  if (n == 0){
    cerr << " ERROR: elias delta called for 0, input must be a positive integer" << endl;
  }
  // first, find number of bits in n 
  int n_bits = nu_bits(n); // or equivalently \f$\lfloor \log_2 n \rfloor + 1\f$
  int L = nu_bits(n_bits) - 1; // this is \f$\floor \og_2 (N+1) \rfloor\f$ where \f$N = \lfloor \log_2 n \rfloor\f$

  bit_pipe N(n_bits); // binary representation
  N.shift_right(L); // it is as if I write L zeros followed by binary representation of N
  bit_pipe n_pipe(n); // binary representation of n
  n_pipe.shift_left(1); // remove the leading 1
  n_pipe.append_left(N);
  return n_pipe;
}

void elias_delta_encode(const unsigned int &n, bit_pipe& B){
  if (n == 0){
    cerr << " ERROR: elias delta called for 0, input must be a positive integer" << endl;
  }
  // first, find number of bits in n 
  int n_bits = nu_bits(n); // or equivalently \f$\lfloor \log_2 n \rfloor + 1\f$
  int L = nu_bits(n_bits) - 1; // this is \f$\floor \og_2 (N+1) \rfloor\f$ where \f$N = \lfloor \log_2 n \rfloor\f$

  bit_pipe N(n_bits); // binary representation
  N.shift_right(L); // it is as if I write L zeros followed by binary representation of N
  B = bit_pipe(n); // binary representation of n
  B.shift_left(1); // remove the leading 1
  B.append_left(N);
}


bit_pipe elias_delta_encode(const mpz_class& n){
  if (n == 0){
    cerr << " ERROR: elias delta called for 0, input must be a positive integer" << endl;
  }
  // first, find number of bits in n 
  int n_bits = mpz_sizeinbase(n.get_mpz_t(), 2); // number of bits in n
  int L = nu_bits(n_bits) - 1; // this is \f$\floor \og_2 (N+1) \rfloor\f$ where \f$N = \lfloor \log_2 n \rfloor\f$

  bit_pipe N(n_bits); // binary representation
  N.shift_right(L); // it is as if I write L zeros followed by binary representation of N
  bit_pipe n_pipe(n); // binary representation of n
  n_pipe.shift_left(1); // remove the leading 1
  n_pipe.append_left(N);
  return n_pipe;
}

void elias_delta_encode(const mpz_class& n, bit_pipe& B){
  if (n == 0){
    cerr << " ERROR: elias delta called for 0, input must be a positive integer" << endl;
  }
  // first, find number of bits in n 
  int n_bits = mpz_sizeinbase(n.get_mpz_t(), 2); // number of bits in n
  int L = nu_bits(n_bits) - 1; // this is \f$\floor \og_2 (N+1) \rfloor\f$ where \f$N = \lfloor \log_2 n \rfloor\f$

  bit_pipe N(n_bits); // binary representation
  N.shift_right(L); // it is as if I write L zeros followed by binary representation of N
  B = bit_pipe(n); // binary representation of n
  B.shift_left(1); // remove the leading 1
  B.append_left(N);
}
