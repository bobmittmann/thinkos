/*************************************************************************
* Name:        huffman.c
* Author:      Marcus Geelnard
* Description: Huffman coder/decoder implementation.
* Reentrant:   Yes
*
* This is a very straight forward implementation of a Huffman coder and
* decoder.
*
* Primary flaws with this primitive implementation are:
*  - Slow bit stream implementation
*  - Maximum tree depth of 32 (the coder aborts if any code exceeds a
*    size of 32 bits). If I'm not mistaking, this should not be possible
*    unless the input buffer is larger than 2^32 bytes, which is not
*    supported by the coder anyway (max 2^32-1 bytes can be specified with
*    an unsigned 32-bit integer).
*
* On the other hand, there are a few advantages of this implementation:
*  - The Huffman tree is stored in a very compact form, requiring only
*    10 bits per symbol (for 8 bit symbols), meaning a maximum of 320
*    bytes overhead.
*  - The code should be fairly easy to follow, if you are familiar with
*    how the Huffman compression algorithm works.
*
* Possible improvements (probably not worth it):
*  - Partition the input data stream into blocks, where each block has
*    its own Huffman tree. With variable block sizes, it should be
*    possible to find locally optimal Huffman trees, which in turn could
*    reduce the total size.
*  - Allow for a few different predefined Huffman trees, which could
*    reduce the size of a block even further.
*-------------------------------------------------------------------------
* Copyright (c) 2003-2006 Marcus Geelnard
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would
*    be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
* Marcus Geelnard
* marcus.geelnard at home.se
*************************************************************************/

#include <stdint.h>

/*************************************************************************
* Types used for Huffman coding
*************************************************************************/

typedef struct {
    uint8_t *BytePtr;
    uint32_t  BitPos;
} huff_bitstream_t;

typedef struct {
    int Symbol;
    uint32_t Count;
    uint32_t Code;
    uint32_t Bits;
} huff_sym_t;

typedef struct huff_encodenode_struct huff_encodenode_t;

struct huff_encodenode_struct {
    huff_encodenode_t *ChildA, *ChildB;
    int Count;
    int Symbol;
};

typedef struct huff_decodenode_struct huff_decodenode_t;

struct huff_decodenode_struct {
    huff_decodenode_t *ChildA, *ChildB;
    int Symbol;
};


/*************************************************************************
* Constants for Huffman decoding
*************************************************************************/

/* The maximum number of nodes in the Huffman tree is 2^(8+1)-1 = 511 */
#define MAX_TREE_NODES 511



/*************************************************************************
*                           INTERNAL FUNCTIONS                           *
*************************************************************************/


/*************************************************************************
* _huffman_initbitstream() - Initialize a bitstream.
*************************************************************************/

static void _huffman_initbitstream(huff_bitstream_t *stream, 
								   uint8_t const buf[])
{
	stream->BytePtr  = (uint8_t *)buf;
	stream->BitPos   = 0;
}


/*************************************************************************
* _huffman_readbit() - Read one bit from a bitstream.
*************************************************************************/

static uint32_t _huffman_readbit( huff_bitstream_t *stream )
{
  uint32_t  x, bit;
  uint8_t *buf;

  /* Get current stream state */
  buf = stream->BytePtr;
  bit = stream->BitPos;

  /* Extract bit */
  x = (*buf & (1<<(7-bit))) ? 1 : 0;
  bit = (bit+1) & 7;
  if( !bit )
  {
    ++ buf;
  }

  /* Store new stream state */
  stream->BitPos = bit;
  stream->BytePtr = buf;

  return x;
}


/*************************************************************************
* _huffman_read8bits() - Read eight bits from a bitstream.
*************************************************************************/

static uint32_t _huffman_read8bits( huff_bitstream_t *stream )
{
  uint32_t  x, bit;
  uint8_t *buf;

  /* Get current stream state */
  buf = stream->BytePtr;
  bit = stream->BitPos;

  /* Extract byte */
  x = (*buf << bit) | (buf[1] >> (8-bit));
  ++ buf;

  /* Store new stream state */
  stream->BytePtr = buf;

  return x;
}


/*************************************************************************
* _huffman_WriteBits() - Write bits to a bitstream.
*************************************************************************/

static void _huffman_WriteBits( huff_bitstream_t *stream, uint32_t x,
  uint32_t bits )
{
  uint32_t  bit, count;
  uint8_t *buf;
  uint32_t  mask;

  /* Get current stream state */
  buf = stream->BytePtr;
  bit = stream->BitPos;

  /* Append bits */
  mask = 1 << (bits-1);
  for( count = 0; count < bits; ++ count )
  {
    *buf = (*buf & (0xff^(1<<(7-bit)))) +
            ((x & mask ? 1 : 0) << (7-bit));
    x <<= 1;
    bit = (bit+1) & 7;
    if( !bit )
    {
      ++ buf;
    }
  }

  /* Store new stream state */
  stream->BytePtr = buf;
  stream->BitPos  = bit;
}


/*************************************************************************
* _huffman_hist() - Calculate (sorted) histogram for a block of data.
*************************************************************************/

static void _huffman_hist(const uint8_t in[], huff_sym_t *sym, uint32_t size )
{
  int k;

  /* Clear/init histogram */
  for( k = 0; k < 256; ++ k )
  {
    sym[k].Symbol = k;
    sym[k].Count  = 0;
    sym[k].Code   = 0;
    sym[k].Bits   = 0;
  }

  /* Build histogram */
  for( k = size; k; -- k )
  {
    sym[*in ++].Count ++;
  }
}


/*************************************************************************
* _huffman_StoreTree() - Store a Huffman tree in the output stream and
* in a look-up-table (a symbol array).
*************************************************************************/

static void _huffman_StoreTree( huff_encodenode_t *node, huff_sym_t *sym,
  huff_bitstream_t *stream, uint32_t code, uint32_t bits )
{
  uint32_t sym_idx;

  /* Is this a leaf node? */
  if( node->Symbol >= 0 )
  {
    /* Append symbol to tree description */
    _huffman_WriteBits( stream, 1, 1 );
    _huffman_WriteBits( stream, node->Symbol, 8 );

    /* Find symbol index */
    for( sym_idx = 0; sym_idx < 256; ++ sym_idx )
    {
      if( sym[sym_idx].Symbol == node->Symbol ) break;
    }

    /* Store code info in symbol array */
    sym[sym_idx].Code = code;
    sym[sym_idx].Bits = bits;
    return;
  }
  else
  {
    /* This was not a leaf node */
    _huffman_WriteBits( stream, 0, 1 );
  }

  /* Branch A */
  _huffman_StoreTree( node->ChildA, sym, stream, (code<<1)+0, bits+1 );

  /* Branch B */
  _huffman_StoreTree( node->ChildB, sym, stream, (code<<1)+1, bits+1 );
}


/*************************************************************************
* _huffman_MakeTree() - Generate a Huffman tree.
*************************************************************************/

static void _huffman_MakeTree( huff_sym_t *sym, huff_bitstream_t *stream )
{
  huff_encodenode_t nodes[MAX_TREE_NODES], *node_1, *node_2, *root;
  uint32_t k, num_symbols, nodes_left, next_idx;

  /* Initialize all leaf nodes */
  num_symbols = 0;
  for( k = 0; k < 256; ++ k )
  {
    if( sym[k].Count > 0 )
    {
      nodes[num_symbols].Symbol = sym[k].Symbol;
      nodes[num_symbols].Count = sym[k].Count;
      nodes[num_symbols].ChildA = (huff_encodenode_t *) 0;
      nodes[num_symbols].ChildB = (huff_encodenode_t *) 0;
      ++ num_symbols;
    }
  }

  /* Build tree by joining the lightest nodes until there is only
     one node left (the root node). */
  root = (huff_encodenode_t *) 0;
  nodes_left = num_symbols;
  next_idx = num_symbols;
  while( nodes_left > 1 )
  {
    /* Find the two lightest nodes */
    node_1 = (huff_encodenode_t *) 0;
    node_2 = (huff_encodenode_t *) 0;
    for( k = 0; k < next_idx; ++ k )
    {
      if( nodes[k].Count > 0 )
      {
        if( !node_1 || (nodes[k].Count <= node_1->Count) )
        {
          node_2 = node_1;
          node_1 = &nodes[k];
        }
        else if( !node_2 || (nodes[k].Count <= node_2->Count) )
        {
          node_2 = &nodes[k];
        }
      }
    }

    /* Join the two nodes into a new parent node */
    root = &nodes[next_idx];
    root->ChildA = node_1;
    root->ChildB = node_2;
    root->Count = node_1->Count + node_2->Count;
    root->Symbol = -1;
    node_1->Count = 0;
    node_2->Count = 0;
    ++ next_idx;
    -- nodes_left;
  }

  /* Store the tree in the output stream, and in the sym[] array (the
      latter is used as a look-up-table for faster encoding) */
  if( root )
  {
    _huffman_StoreTree( root, sym, stream, 0, 0 );
  }
  else
  {
    /* Special case: only one symbol => no binary tree */
    root = &nodes[0];
    _huffman_StoreTree( root, sym, stream, 0, 1 );
  }
}


/*************************************************************************
* _huffman_recovertree() - Recover a Huffman tree from a bitstream.
*************************************************************************/

static huff_decodenode_t * _huffman_recovertree(huff_decodenode_t *nodes, 
												huff_bitstream_t *stream, 
												uint32_t *nodenum )
{
  huff_decodenode_t * this_node;

  /* Pick a node from the node array */
  this_node = &nodes[*nodenum];
  *nodenum = *nodenum + 1;

  /* Clear the node */
  this_node->Symbol = -1;
  this_node->ChildA = (huff_decodenode_t *) 0;
  this_node->ChildB = (huff_decodenode_t *) 0;

  /* Is this a leaf node? */
  if (_huffman_readbit(stream)) {
    /* Get symbol from tree description and store in lead node */
    this_node->Symbol = _huffman_read8bits( stream );
    return this_node;
  }

  /* Get branch A */
  this_node->ChildA = _huffman_recovertree(nodes, stream, nodenum);

  /* Get branch B */
  this_node->ChildB = _huffman_recovertree(nodes, stream, nodenum);

  return this_node;
}



/*************************************************************************
*                            PUBLIC FUNCTIONS                            *
*************************************************************************/


/*************************************************************************
* huffman_compress() - Compress a block of data using a Huffman coder.
*  in     - Input (uncompressed) buffer.
*  out    - Output (compressed) buffer. This buffer must be 384 bytes
*           larger than the input buffer.
*  insize - Number of input bytes.
* The function returns the size of the compressed data.
*************************************************************************/

int huffman_compress(const uint8_t in[], uint8_t out[], uint32_t insize)
{
  huff_sym_t       sym[256], tmp;
  huff_bitstream_t stream;
  uint32_t     k, total_bytes, swaps, symbol;

  /* Do we have anything to compress? */
  if( insize < 1 ) return 0;

  /* Initialize bitstream */
  _huffman_initbitstream(&stream, out);

  /* Calculate and sort histogram for input data */
  _huffman_hist( in, sym, insize );

  /* Build Huffman tree */
  _huffman_MakeTree( sym, &stream );

  /* Sort histogram - first symbol first (bubble sort) */
  do
  {
    swaps = 0;
    for( k = 0; k < 255; ++ k )
    {
      if( sym[k].Symbol > sym[k+1].Symbol )
      {
        tmp      = sym[k];
        sym[k]   = sym[k+1];
        sym[k+1] = tmp;
        swaps    = 1;
      }
    }
  }
  while( swaps );

  /* Encode input stream */
  for( k = 0; k < insize; ++ k )
  {
    symbol = in[k];
    _huffman_WriteBits( &stream, sym[symbol].Code,
                        sym[symbol].Bits );
  }

  /* Calculate size of output data */
  total_bytes = (int)(stream.BytePtr - out);
  if( stream.BitPos > 0 )
  {
    ++ total_bytes;
  }

  return total_bytes;
}


/*************************************************************************
* huffman_uncompress() - Uncompress a block of data using a Huffman
* decoder.
*  in      - Input (compressed) buffer.
*  out     - Output (uncompressed) buffer. This buffer must be large
*            enough to hold the uncompressed data.
*  insize  - Number of input bytes.
*  outsize - Number of output bytes.
*************************************************************************/

void huffman_uncompress(const uint8_t in[], uint8_t out[], uint32_t insize, 
						uint32_t outsize)
{
  huff_decodenode_t nodes[MAX_TREE_NODES], *root, *node;
  huff_bitstream_t stream;
  uint32_t k, node_count;
  uint8_t *buf;

  /* Do we have anything to decompress? */
  if (insize < 1) 
	  return;

  /* Initialize bitstream */
  _huffman_initbitstream(&stream, in);

  /* Recover Huffman tree */
  node_count = 0;
  root = _huffman_recovertree(nodes, &stream, &node_count);

  /* Decode input stream */
  buf = out;
  for (k = 0; k < outsize; ++k) {
    /* Traverse tree until we find a matching leaf node */
    node = root;
    while (node->Symbol < 0) {
      /* Get next node */
      if (_huffman_readbit(&stream))
        node = node->ChildB;
      else
        node = node->ChildA;
    }

    /* We found the matching leaf node and have the symbol */
    *buf++ = (uint8_t)node->Symbol;
  }
}
