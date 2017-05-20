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

#ifdef PYTHON_LIB

#include "utils.h"
#define ERR_BUFFER_SIZE     512

void
myError (const char *file, const int line, const char *fnct, const char *frm,
     ...)
{
    size_t written = 0;
    char buffer [ERR_BUFFER_SIZE];
    va_list ap;

    va_start (ap, frm);

    written = snprintf (buffer, ERR_BUFFER_SIZE, "*** error in file %s, line %d, function %s:\n", file,
	   line, fnct);
    if (written < 0)
        buffer [0] = '\0';
    else if (written < ERR_BUFFER_SIZE-1) {
        if (vsnprintf (buffer + written, ERR_BUFFER_SIZE - written, frm, ap) < 0)
            buffer[written] = '\0';
    }

    PyErr_SetString(InternalError, buffer);
}

void
warning (const char *file, const int line, const char *fnct, const char *frm,
	 ...)
{
}

#else

void
myError (const char *file, const int line, const char *fnct, const char *frm,
     ...)
{
  va_list ap;

  va_start (ap, frm);
  fprintf (stderr, "*** error in file %s, line %d, function %s:\n", file,
	   line, fnct);
  vfprintf (stderr, frm, ap);
  fprintf (stderr, "\n");
}

void
warning (const char *file, const int line, const char *fnct, const char *frm,
	 ...)
{
  va_list ap;

  va_start (ap, frm);
  fprintf (stderr, "*** warning in file %s, line %d, function %s:\n*** ",
	   file, line, fnct);
  vfprintf (stderr, frm, ap);
  fprintf (stderr, "\n");
}

#endif

void *
xmalloc (const char *file, const int line, const char *fnct,
	 const size_t size)
{
  void *ptr;

  ptr = malloc (size);
  if (ptr == NULL)
    {
      myError (file, line, fnct, "memory allocation failed");
      exit (-1);
    }
  return ptr;
}

void *
xcalloc (const char *file, const int line, const char *fnct,
	 const size_t nmemb, const size_t size)
{
  void *ptr;

  ptr = calloc (nmemb, size);
  if (ptr == NULL)
    {
      myError (file, line, fnct, "memory allocation failed");
      exit (-1);
    }
  return ptr;
}

void *
xrealloc (const char *file, const int line, const char *fnct, void *ptr,
	  const size_t size)
{
  ptr = realloc (ptr, size);
  if (ptr == NULL)
    {
      myError (file, line, fnct, "memory allocation failed");
      exit (-1);
    }
  return ptr;
}

FILE *
xfopen (const char *file, const int line, const char *fnct, const char *name,
	const char *mode)
{
  FILE *fp;

  fp = fopen (name, mode);
  if (fp == NULL)
    {
      myError (file, line, fnct, "could not open file %s in mode %s", name,
	     mode);
      exit (-1);
    }
  return fp;
}

int
hamming_weight (uint64_t val)
{
  int res;
  int i;

  res = 0;
  for (i = 0; i < 64; i++)
    {
      res += (int) (val & UINT64_C (1));
      val >>= 1;
    }
  return res;
}

int
hamming_distance (uint64_t val1, uint64_t val2)
{
  return hamming_weight (val1 ^ val2);
}
