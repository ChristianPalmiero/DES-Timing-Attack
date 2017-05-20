/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
*/

/** \file rdtsc_timer.h
 *  A function that returns the current value of the internal timer.
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-08
 *  \attention
 *  Not portable, tested only on Pentium architecture under Linux.
 *
 *  Example of use to time a function 10 times and return the minimum time value as a floating point (double):
 *  \code
 *  uint64_t a, b;
 *  int i;
 *  double t, min;
 *  ...
 *  for(i = 0; i < 10; i++)
 *  {
 *    a = get_rdtsc_timer();
 *    function_to_time();
 *    b = get_rdtsc_timer();
 *    t = (double)(b - a);
 *    if(i == 0 || t < min)
 *    {
 *      min = t;
 *    }
 *  }
 *  return min;
 *  \endcode
 */

#ifndef RDTSC_TIMER_H
#define RDTSC_TIMER_H

#include <stdint.h>

/** A function that returns the current value of the internal timer.
 * \return the timer value as a 64 bits unsigned integer. */
uint64_t get_rdtsc_timer (void);

#endif /* not RDTSC_TIMER_H */
