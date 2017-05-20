/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
*/

/** \file pcc.h
 *  The \b pcc library, a software library dedicated to the computation of  Pearson Correlation Coefficients (PCC).
 *  \author Renaud Pacalet, renaud.pacalet@telecom-paristech.fr
 *  \date 2009-07-08
 *
 * Defines a data structure and a set of functions used to compute and manage Pearson Correlation Coefficients (PCC) between scalar, floating point (double) random variables. These coefficients are a statistical tool to evaluate the correlation between two random variables. The formula of a PCC between random variables X and Y is:<br>
 *   PCC(X, Y) = [E(X*Y) - E(X)*E(Y)] / [s(X) * s(Y)]<br>
 * where E(Z) is the expectation (average value) of random variable Z and s(Z) is its unbiased standard deviation. The value of the PCC is in range -1 to +1. Values close to 0 indicate no or a weak correlation. Values close to -1 or +1 indicate strong correlations.<br>
 * The \b pcc library can be used to compute a set of PCCs between one random variable (denoted X), common to all PCCs, and a set of random variables (denoted Y0, Y1, ..., Yn-1). To compute such a set of PCCs one must first initialize a PCC context (pcc_init()), indicating the number ny of Y random variables. Then, realizations of the random variables must be accumulated into the context: first a realization of the X variable (pcc_insert_x()), followed by realizations of each of the ny Y variables (pcc_insert_y()). Once a sufficient number of realizations are accumulated in the PCC context, a call to pcc_consolidate() computes the ny PCCs. Calls to pcc_get_pcc() return their values. Note: more realizations can be accumulated after a call to pcc_consolidate(), another call to pcc_consolidate() will take them into account altogether with the previous ones and compute the new PCC values. A call to pcc_free() deallocates the PCC context and makes it reusable for the computation of a new set of PCCs. Example of use with ny=4 Y variables, in which get_next_x and get_next_y are two functions returning realizations of the random variables:
 * \code
 * pcc_context *ctx;
 * double x, y, pcc;
 * int i, j, nexp;
 * ...
 * ctx = pcc_init(4);           // Initialize context for four Y random variables
 * for(i = 0; i < nexp; i++)    // For nexp experiments
 *   {
 *     x = get_next_x();          // Realization of random variable X
 *     pcc_insert_x(ctx, x);      // Insert realization of X into context
 *     for(j = 0; j < 4; j++)     // For the four Y random variables
 *       {
 *         y = get_next_y(j);       // Realization of random variable Yj
 *         pcc_insert_y(ctx, j, y); // Insert realization of Yj into context
 *       } // End for the four Y random variables
 *   } // End for nexp experiments
 * pcc_consolidate(ctx); // Finalize computation of the four PCCs
 * for(j = 0; j < 4; j++) // For the four Y random variables
 *   {
 *     pcc = pcc_get_pcc(ctx, j);             // Get PCC(X,Yj)
 *     printf("PCC(X, Y%d) = %lf\n", j, pcc); // Print PCC(X,Yj)
 *   } // End for the four Y random variables
 * pcc_free(ctx);      // Deallocate context
 * ctx = pcc_init(12); // Initialize context for twelve other Y random variables
 * ...
 * pcc_free(ctx);      // Deallocate context
 * \endcode
 * \attention 
 * It is an error to break the realization insertion scheme: if you initialized your PCC context for ny Y variables, first insert a realization of X, followed by one and only one realization of each of the ny Y variables. Then, insert a new realization of X and ny new realizations of the ny Y variables, and so on.  Consolidate only after inserting the realization of the last Y variable. Do not consolidate when in an intermediate state.
 * */

#ifndef PCC_H
#define PCC_H

/** The data structure used to compute and manage a set of Pearson correlation coefficients. */
struct pcc_context_s
{
  int ny;      /**< The number of Y random variables. */
  int nr;      /**< The current number of realizations of the random variables. */
  double rx;   /**< The last inserted realization of X. */
  double x;    /**< The sum of the realizations of X. */
  double x2;   /**< The sum of the squares of the realizations of X. */
  double *y;   /**< The array of the sums of the realizations of the Ys. */
  double *y2;  /**< The array of the sums of the squares of the realizations of the Ys. */
  double *xy;  /**< The array of the sums of the products between realizations of X and Ys. */
  double *pcc; /**< The array of the PCCs. */
  char state;  /**< Tracker for insertion of the realizations. */
  int cnt;     /**< Tracker for insertion of the realizations. */
  char *flags; /**< Tracker for insertion of the realizations. */
};

typedef struct pcc_context_s pcc_context;

/** Initializes a PCC context.
 * \return An initialized PCC context. */
pcc_context *pcc_init (int ny
	   /**< The number of Y random variables to manage. */
  );

/** Inserts a new X realization in a PCC context. */
void pcc_insert_x (pcc_context *ctx,
		     /**< The PCC context. */
		   double x
		     /**< The realization of the X random variable. */
  );

/** Inserts a new Y realization in a PCC context. */
void pcc_insert_y (pcc_context *ctx,
		     /**< The PCC context. */
		   int ny,
		     /**< The index of the Y random variable (0 to ctx->ny - 1). */
		   double y
		     /**< The realization of the Y random variable. */
  );

/** Consolidates a set of PCCs (computes all the PCCs from the already inserted realizations). */
void pcc_consolidate (pcc_context *ctx
		    /**< The PCC context. */
  );

/** Gets the PCCs from a consolidated PCC context. */
double pcc_get_pcc (pcc_context *ctx,
		     /**< The PCC context. */
		    int ny
		     /**< The index of the PCC to get (0 to ctx->ny - 1). */
  );

/** Deallocates a PCC context. */
void pcc_free (pcc_context *ctx
		    /**< The PCC context. */
  );

#endif /** not PCC_H */
