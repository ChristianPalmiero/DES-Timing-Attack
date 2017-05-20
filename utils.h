/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
*/

/** \file utils.h
 *  Some utility functions and macros.
 *
 *  When available the macros are much simpler to use, so if they fit your  needs, do not even look at the functions.
 *
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-08
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/** Print an error message and exit with -1 status. \e s is the desired exit status, followed by a printf-like formating string and an optional printf-like variable number of arguments. Example: if file foo.c contains:
 * \code
 * void bar(void)
 *   {
 *     ...
 *     A = 15;
 *     ERROR (-1, "invalid value of parameter A: %d", A);
 *     ...
 *   }
 * \endcode
 * and the ERROR macro (at line 47 of foo.c) is executed, it will print the message:<br>
\verbatim
*** error in file foo.c, line 47, function bar:
*** invalid value of parameter A: 15
\endverbatim
 * on the standard error and then exit with status -1. */
#ifdef PYTHON_LIB
#include <python2.7/Python.h>

PyObject *InternalError;

/* If the code is compiled for python, we don't want to just exit, we wish to raise an exception. To do that, instead of calling exit, we set a new exception (by calling PyErr_SetString) in the error function and we return from the function that raised an error. Generic return for any function is impossible so we need to pass to the macro a value that the function should return in case of error. If the call to the function that raised an error was nested inside another C function (not one of the wrappers define in Py_* files), we should detect that an error occurred in the caller and return in turn from the function. To handle this case we add the __check_forward_exception macro that does nothing when not compiled with -GPYTHON_LIB since it's only useful when an error occurred, which would have already called exit for a "traditional" compilation.
 */
#define ERROR(retval, exitval, s,...) { myError(__FILE__, __LINE__, __func__, s, ##__VA_ARGS__); return retval; }
#define __check_forward_exception(retval) { if (PyErr_Occurred()) return retval; }
#else
#define ERROR(retval, exitval, s,...) { myError(__FILE__, __LINE__, __func__, s, ##__VA_ARGS__); exit (exitval); }
#define __check_forward_exception(retval)
#endif

/** Print a warning message. Takes a printf-like formating string followed by an optional printf-like variable number of arguments. Example: if file foo.c contains:
 * \code
 * void bar(void)
 *   {
 *     ...
 *     A = 15;
 *     WARNING ("invalid value of parameter A: %d", A);
 *     ...
 *   }
 * \endcode
 * and the WARNING macro (at line 47 of foo.c) is executed, it will print the message:<br>
\verbatim
*** warning in file foo.c, line 47, function bar:
*** invalid value of parameter A: 15
\endverbatim
 * on the standard error. */
#define WARNING(...) warning(__FILE__, __LINE__, __func__, __VA_ARGS__)

/** Wrapper around the regular malloc memory allocator. Allocates \e n bytes and returns a pointer to the allocated memory. Errors are catched, no need to check result.
 * \return pointer to allocated memory area. */
#define XMALLOC(n) xmalloc(__FILE__, __LINE__, __func__, (n))

/** Wrapper around the regular calloc memory allocator. Allocates memory for an array of \e n elements of \e s bytes each and returns a pointer to the allocated memory. Errors are catched, no need to check result.
 * \return pointer to allocated memory area. */
#define XCALLOC(n,s) xcalloc(__FILE__, __LINE__, __func__, (n), (s))

/** Wrapper around the regular realloc memory allocator. Resize the memory area pointed to py \e p to \e s bytes and returns a pointer to the allocated memory. Errors are catched, no need to check result.
 * \return pointer to allocated memory area. */
#define XREALLOC(p,s) xrealloc(__FILE__, __LINE__, __func__, (p), (s))

/** Wrapper around the regular fopen. Opens file \e f in mode \e m. Errors are catched, no need to check result.
 * \return The FILE pointer to the opened file. */
#define XFOPEN(f,m) xfopen(__FILE__, __LINE__, __func__, (f), (m))

/** Returns the Hamming weight of a 64 bits word. Note: the input's width can be anything between 0 and 64, as long as the unused bits are all zeroes.
 * \return The Hamming weight of the input as a 64 bits uint64_t. */
int hamming_weight (uint64_t val /**< The 64 bits input. */ );

/** Returns the Hamming distance between two 64 bits words. Note: the width of the inputs can be anything between 0 and 64, as long as they are the same, aligned and that the unused bits are all zeroes.
 * \return The Hamming distance between the two inputs as a 64 bits uint64_t. */
int hamming_distance (uint64_t val1 /**< The first 64 bits input. */ ,
		      uint64_t val2
		  /**< The second 64 bits input. */
  );

/** Prints an error message and exit with -1 status. Takes a variable number of arguments, as printf. */
void myError (const char *file /**< File name. */ ,
	    const int line /**< Line number. */ ,
	    const char *fnct /**< Name of calling function. */ ,
	    const char *frm, /**< A printf-like formatting string */
	    ...	 /**< A variable number of arguments. */
  );

/** Prints a warning message. Takes a variable number of arguments, as printf. */
void warning (const char *file /**< File name. */ ,
	      const int line /**< Line number. */ ,
	      const char *fnct /**< Name of calling function. */ ,
	      const char *frm,
			     /**< A printf-like formatting string */
	      .../**< A variable number of arguments. */
  );

/** Wrapper around the regular malloc memory allocator. Errors are catched, no need to check result.
 * \return pointer to allocated memory area. */
void *xmalloc (const char *file /**< File name. */ ,
	       const int line /**< Line number. */ ,
	       const char *fnct /**< Name of calling function. */ ,
	       const size_t size
		/**< Size of an element. */
  );

/** Wrapper around the regular calloc memory allocator. Errors are catched, no need to check result.
 * \return pointer to allocated memory area. */
void *xcalloc (const char *file /**< File name. */ ,
	       const int line /**< Line number. */ ,
	       const char *fnct /**< Name of calling function. */ ,
	       const size_t nmemb /**< Number of elements to allocate. */ ,
	       const size_t size /**< Size of an element. */
  );

/** Wrapper around the regular realloc memory allocator. Errors are catched, no need to check result.
 * \return pointer to allocated memory area. */
void *xrealloc (const char *file /**< File name. */ ,
		const int line /**< Line number. */ ,
		const char *fnct /**< Name of calling function. */ ,
		void *ptr /**< Source pointer. */ ,
		const size_t size /**< Size of new memory area. */
  );

/** Wrapper around the regular fopen. Errors are catched, no need to check result.
 * \return The FILE pointer to the opened file. */
FILE *xfopen (const char *file /**< File name. */ ,
	      const int line /**< Line number. */ ,
	      const char *fnct /**< Name of calling function. */ ,
	      const char *name /**< File name */ ,
	      const char *mode
		     /**< Mode */
  );

#endif /** not UTILS_H */
