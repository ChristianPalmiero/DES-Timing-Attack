/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
*/

#include <stdint.h>

uint64_t
get_rdtsc_timer (void)
{
  unsigned long long x;

/*  __asm__ volatile  (".byte 0x0f, 0xa2" : "=A"(x)); */
  __asm__ volatile (".byte 0x0f, 0x31":"=A" (x));
  return x;
}
