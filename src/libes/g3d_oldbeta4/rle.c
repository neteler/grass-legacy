#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <rpc/types.h>
#include <rpc/xdr.h>
#include "G3d_intern.h"

#define G_254_SQUARE 64516
#define G_254_TIMES_2 508

#define G_RLE_OUTPUT_CODE(code) (*((unsigned char *) dst++) = (code))
#define G_RLE_INPUT_CODE(codeP) (*(codeP) = *((unsigned char *) src++))

/*---------------------------------------------------------------------------*/

static int
G_rle_codeLength (length)

     int length;

{
  register int lPrime;
  int codeLength;

  if (length == -1) return 2;
  if (length < 254) return 1;
  if (length < G_254_TIMES_2) return 2;
  if (length < G_254_SQUARE) return 3;
  
  codeLength = 0;
  lPrime = length;
  while ((lPrime = lPrime / 254) != 0) codeLength++;
  return codeLength + 2;
}

/*---------------------------------------------------------------------------*/

static char *
rle_length2code (length, dst)

     int length;
     char *dst;

{
  register int lPrime;

  if (length == -1) { /* stop code */
    G_RLE_OUTPUT_CODE (255);
    G_RLE_OUTPUT_CODE (255);

    return dst;
  }

  if (length < 254) {
    G_RLE_OUTPUT_CODE (length);

    return dst;
  }

  if (length < G_254_TIMES_2) { /* length == 254 + a; a < 254 */
    G_RLE_OUTPUT_CODE (255);
    G_RLE_OUTPUT_CODE (length % 254);

    return dst;
  }

  if (length < G_254_SQUARE) {/* length = 254 * b + a; b, a < 254 */
    G_RLE_OUTPUT_CODE (254);  /* this if-clause included for efficiency only */
    G_RLE_OUTPUT_CODE (length / 254);
    G_RLE_OUTPUT_CODE (length % 254);

    return dst;
  }

  /* length = 254 ^ c + 254 * b + a; b, a < 254 */

  lPrime = length;
  while ((lPrime = lPrime / 254) != 0) G_RLE_OUTPUT_CODE (254);
    
  length %= G_254_SQUARE;

  G_RLE_OUTPUT_CODE (length / 254);
  G_RLE_OUTPUT_CODE (length % 254);

  return dst;
}
  
/*---------------------------------------------------------------------------*/

static char *
rle_code2length (src, length)

     char *src;
     int *length;

{
  int code;

  if (G_RLE_INPUT_CODE (length) < 254) return src; /* length < 254 */

  if (*length == 255) { /* length == 254 + a; a < 254 */
    if (G_RLE_INPUT_CODE (length) == 255) {
      *length = -1;
      return src;
    }

    *length += 254;
    return src;
  }

  G_RLE_INPUT_CODE (&code);
  if (code < 254) { /* length = 254 * b + a; b, a < 254 */ 
    G_RLE_INPUT_CODE (length);/* this if-clause included for efficiency only */
    *length += 254 * code;

    return src;
  }

  /* length = 254 ^ c + 254 * b + a; b, a < 254 */

  *length = G_254_SQUARE;
  while (G_RLE_INPUT_CODE (&code) == 254) *length *= 254;
  
  *length += 254 * code;
  G_RLE_INPUT_CODE (&code);
  *length += code;
  
  return src;
}

/*---------------------------------------------------------------------------*/

int
G_rle_count_only (src, nofElts, eltLength)

     char *src;
     int nofElts, eltLength;

{
  int length, nofEqual;
  char *head, *tail, *headStop, *headStop2;

  if (nofElts <= 0) G3d_fatalError ("trying to encode 0-length list");

  length = 0;
  nofEqual = 1;
  head = src + eltLength;
  tail = src;

  headStop = src + nofElts * eltLength;

  while (head != headStop) {
    headStop2 = head + eltLength;

    while (head != headStop2) {
      if (*head != *tail) {
	length += G_rle_codeLength (nofEqual) + eltLength;
	nofEqual = 1;
	tail = headStop2 - eltLength;
	break;
      }
      head++;
      tail++;
    }
    
    if (head == headStop2) {
      nofEqual++;
      continue;
    }

    head = headStop2;
  }
  length += G_rle_codeLength (nofEqual) + eltLength;

  return length + G_rle_codeLength (-1);
}

/*---------------------------------------------------------------------------*/

void
G_rle_encode (src, dst, nofElts, eltLength)

     char *src, *dst;
     int nofElts, eltLength;

{
  int length, nofEqual;
  char *head, *tail, *headStop, *headStop2;

  if (nofElts <= 0) G3d_fatalError ("trying to encode 0-length list");

  length = 0;
  nofEqual = 1;
  head = src + eltLength;
  tail = src;

  headStop = src + nofElts * eltLength;

  while (head != headStop) {
    headStop2 = head + eltLength;

    while (head != headStop2) {
      if (*head != *tail) {
	dst = rle_length2code (nofEqual, dst);
	tail = headStop2 - eltLength * (nofEqual + 1);
	head = tail + eltLength;
/*	fprintf (stdout,"equal %d char %d\n", nofEqual, *tail);*/
	while (tail != head) *dst++ = *tail++;
	length += G_rle_codeLength (nofEqual) + eltLength;
	nofEqual = 1;
	tail = headStop2 - eltLength;
	break;
      }
      head++;
      tail++;
    }
    
    if (head == headStop2) {
      nofEqual++;
      continue;
    }

    head = headStop2;
  }

  dst = rle_length2code (nofEqual, dst);
  tail = headStop - eltLength * nofEqual;
  head = tail + eltLength;
  while (tail != head) *dst++ = *tail++;
  length += G_rle_codeLength (nofEqual) + eltLength;
  dst = rle_length2code (-1, dst);
  length += G_rle_codeLength (-1);
  rle_code2length (dst - 2, &nofEqual);
}

/*---------------------------------------------------------------------------*/

void
G_rle_decode (src, dst, nofElts, eltLength, lengthEncode, lengthDecode)

     char *src, *dst;
     int nofElts, eltLength;
     int *lengthEncode, *lengthDecode;

{
  int nofEqual;
  char *src2, *srcStop, *src2Stop, *dstFirst;

  srcStop = src + nofElts * eltLength;
  dstFirst = dst;

  while (src != srcStop) {
    src = rle_code2length (src, &nofEqual);

    if (nofEqual == -1) {
      *lengthEncode = src - (srcStop - nofElts * eltLength);
      *lengthDecode = dst - dstFirst;
      return;
    }

    while (nofEqual--) {
      src2 = src;
      src2Stop = src2 + eltLength;
      while (src2 != src2Stop) *dst++ = *src2++;
    }
    src += eltLength;
  }

  G3d_fatalError ("G_rle_decode: string ends prematurely");
}

/*---------------------------------------------------------------------------*/

test_rle ()

{
  char c[100];
  int length;

  do {
    fprintf (stdout,"length? ");
    scanf ("%d", &length);
    fprintf (stdout,"length = %d\n", length);
    fprintf (stdout,"codeLength %d   ", G_rle_codeLength (length));
    (void) rle_length2code (length, c);
    length = 0;
    (void) rle_code2length (c, &length);
    fprintf (stdout,"output length %d\n\n", length);
  } while (1);
}
  
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
