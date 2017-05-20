/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>

#include <utils.h>

/* The P permutation table, as in the standard. The first entry (16) is the
 * position of the first (leftmost) bit of the result in the input 32 bits word.
 * */
int p_table[32] = {
  16 , 7  , 20 , 21 , 
  29 , 12 , 28 , 17 , 
  1  , 15 , 23 , 26 , 
  5  , 18 , 31 , 10 , 
  2  , 8  , 24 , 14 , 
  32 , 27 , 3  , 9  , 
  19 , 13 , 30 , 6  , 
  22 , 11 , 4  , 25
};

/* Returns the value of a given bit (0 or 1) of a 32 bits word. Positions are
 * numbered as in the DES standard: 1 is the leftmost and 32 is the rightmost.
 * */
int
get_bit(int position, uint64_t val);

/* Force a given bit of a 32 bits word to 1. Positions are numbered as in the
 * DES standard: 1 is the leftmost and 32 is the rightmost. */
uint64_t
set_bit(int position, uint64_t val);

/* Force a given bit of a 32 bits word to 0. Positions are numbered as in the
 * DES standard: 1 is the leftmost and 32 is the rightmost. */
uint64_t
unset_bit(int position, uint64_t val);

/* Force a given bit of a 32 bits word to a given value. Positions are numbered
 * as in the DES standard: 1 is the leftmost and 32 is the rightmost. */
uint64_t
force_bit(int position, int value, uint64_t val);

/* Applies the P permutation to a 32 bits word and returns the result as another
 * 32 bits word. */
uint64_t
des_p_ta(uint64_t val) {
  uint64_t res;
  int i, j, k;

  res = UINT64_C(0);
  k = 0;
  for(i = 1; i <= 32; i++) {
    if(get_bit(i, val) == 1) {
      for(j = 1; j <= 32; j++) {
        if(p_table[j - 1] == i) { /* C array index starts at 0, not 1 */
          k = j;
        }
      }
      res = set_bit(k, res);
    }
  }
  return res;
}

/* Returns the value of a given bit (0 or 1) of a 32 bits word. Positions are
 * numbered as in the DES standard: 1 is the leftmost and 32 is the rightmost.
 * */
int
get_bit(int position, uint64_t val) {
  int i;

  if(position < 1 || position > 32) {
    ERROR(0, -1, "Invalid bit position (%d)", position);
  }
  for(i = 32; i > position; i--) {
    val = val >> 1;
  }
  val = val & UINT64_C(1);
  return (int)(val);
}

/* Force a given bit of a 32 bits word to 1. Positions are numbered as in the
 * DES standard: 1 is the leftmost and 32 is the rightmost. */
uint64_t
set_bit(int position, uint64_t val) {
  uint64_t tmp;
  int i;

  if(position < 1 || position > 32) {
    ERROR(0, -1, "Invalid bit position (%d)", position);
  }
  tmp = UINT64_C(1);
  for(i = 32; i > position; i--) {
    tmp = tmp << 1;
  }
  val = val | tmp;
  return val;
}

/* Force a given bit of a 32 bits word to 0. Positions are numbered as in the
 * DES standard: 1 is the leftmost and 32 is the rightmost. */
uint64_t
unset_bit(int position, uint64_t val) {
  uint64_t tmp;
  int i;

  if(position < 1 || position > 32) {
    ERROR(0, -1, "Invalid bit position (%d)", position);
  }
  tmp = UINT64_C(1);
  for(i = 32; i > position; i--) {
    tmp = tmp << 1;
  }
  tmp = ~tmp;
  val = val & tmp;
  return val;
}

/* Force a given bit of a 32 bits word to a given value. Positions are numbered
 * as in the DES standard: 1 is the leftmost and 32 is the rightmost. */
uint64_t
force_bit(int position, int value, uint64_t val) {
  if(position < 1 || position > 32) {
    ERROR(0, -1, "Invalid bit position (%d)", position);
  }
  if(value != 0 && value != 1) {
    ERROR(0, -1, "Invalid bit value (%d)", value);
  }
  if(value == 0) {
    return unset_bit(position, val);
  }
  else {
    return set_bit(position, val);
  }
}
