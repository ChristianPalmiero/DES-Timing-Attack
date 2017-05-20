#!/usr/bin/env python2

#
# Copyright (C) Telecom ParisTech
# 
# This file must be used under the terms of the CeCILL. This source
# file is licensed as described in the file COPYING, which you should
# have received as part of this distribution. The terms are also
# available at:
# http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
#

"""The pcc library, a software library dedicated to the computation of
Pearson Correlation Coefficients (PCC).

:Date: 2009-07-08
:Authors:
    - Renaud Pacalet, renaud.pacalet@telecom-paristech.fr

Defines a data structure and a set of functions used to compute and manage
Pearson Correlation Coefficients (PCC) between scalar, floating point
(double) random variables. These coefficients are a statistical tool to
evaluate the correlation between two random variables. The formula of a PCC
between random variables X and Y is::

PCC(X, Y) = [E(X*Y) - E(X)*E(Y)] / [s(X) * s(Y)]

where E(Z) is the expectation (average value) of random variable Z and s(Z)
is its unbiased standard deviation. The value of the PCC is in range -1 to
+1. Values close to 0 indicate no or a weak correlation. Values close to -1
or +1 indicate strong correlations.

The pcc library can be used to compute a set of PCCs between one random variable
(denoted X), common to all PCCs, and a set of random variables (denoted Y0, Y1,
..., Yn-1). To compute such a set of PCCs one must first initialize a PCC
context (`pccContext`), indicating the number ny of Y random variables. Then,
realizations of the random variables must be accumulated into the context: first
a realization of the X variable (`pccContext.insert_x()`), followed by
realizations of each of the ny Y variables (`pccContext.insert_y()`). Once a
sufficient number of realizations are accumulated in the PCC context, a call to
`pccContext.consolidate()` computes the ny PCCs. Calls to `pccContext.get_pcc()`
return their values. Note: more realizations can be accumulated after a call to
`pccContext.consolidate()`, another call to `pccContext.consolidate()` will take
them into account altogether with the previous ones and compute the new PCC
values.  Example of use with ny=4 Y variables, in which `get_next_x` and
`get_next_y` are two functions returning realizations of the random variables::

import pcc
...
ctx = pcc.pccContext (4);       # Initialize context for four Y random variables
for i in range (nexp):          # For nexp experiments
    x = get_next_x ()           # Realization of random variable X
    ctx.insert_x (x)            # Insert realization of X into context
    for j in range(4):          # For the four Y random variables
        y = get_next_y (j)      # Realization of random variable Yj
        ctx.insert_y (j, y)     # Insert realization of Yj into context

ctx.consolidate ()              # Finalize computation of the four PCCs

for j in range (4):             # For the four Y random variables
    pcc = ctx.get_pcc (j)       # Get PCC(X,Yj)
    print ("PCC(X, Y%d) = %lf" % (j, pcc))    # Print PCC(X,Yj)

ctx = pcc.pccContext (12)       # Initialize context for twelve other Y random variables
...

Attention 
=========

It is an error to break the realization insertion scheme: if you initialized
your PCC context for ny Y variables, first insert a realization of X,
followed by one and only one realization of each of the ny Y variables. Then,
insert a new realization of X and ny new realizations of the ny Y variables,
and so on. Consolidate only after inserting the realization of the last Y
variable. Do not consolidate when in an intermediate state.

"""

import math

class pccContext:
    """The data structure used to compute and manage a set of Pearson correlation
    coefficients.

    Attributes:
        ny (int): Number of Y random variables
        nr (int): Current number of realizations of the random variables
        pcc (List[float]): List of the PCCs

    """

    def __init__ (self, ny):
        """Initializes a PCC context.

        Args:
            ny (int): Number of Y random variables to manage

        """
        
        if not isinstance (ny, int):
            raise TypeError ('parameter ny should be a number')
            
        if ny < 1:
            raise ValueError ("Invalid number of Y random variables: %d" % ny);

        self.ny = ny
        self.nr = 0
        self.__x = 0.0
        self.__x2 = 0.0
        self.__y = [0.0] * ny
        self.__y2 = [0.0] * ny
        self.__xy = [0.0] * ny
        self.pcc = [0.0] * ny
        self.__state = 0
        self.__flags = [0] * ny
        self.__rx = -1

    def insert_x (self, x):
        """Inserts a new X realization in a PCC context.

        Args:
            x (float): The realization of the X random variable.

        """

        if any ([d != self.__state for d in self.__flags]):
            raise ValueError ('missing realizations %s' % ', '.join(['Y%d' % d for d in range(self.ny) if self.__flags[d] != self.__state]))

        self.__rx = x
        self.__x += x
        self.__x2 += x*x
        self.__state = 1 - self.__state
        self.nr += 1

    def insert_y (self, ny, y):
        """Insert a new Y realization in a PCC context

        Args:
            ny (int): Index of the Y random variable (0 to self.ny - 1)
            y (float): Realization of the Y random variable

        """

        
        if not isinstance (ny, int) or not (isinstance (y, float) or isinstance (y, int) or isinstance (y, long)):
            raise TypeError('parameters ny and y should be numbers')

        if ny < 0 or ny >= self.ny:
            raise ValueError ('Invalid Y index: %d' % ny)
        if self.__rx < 0:
            raise ValueError ('a realization of X should be first inserted')
        if self.__flags[ny] == self.__state:
            raise ValueError ('Y realization #%d inserted twice' % ny)

        self.__y[ny] += y
        self.__y2[ny] += y*y
        self.__xy[ny] += self.__rx*y
        self.__flags[ny] = self.__state

    def consolidate (self):
        """Consolidates a set of PCCs (computes all the PCCs from the already
        inserted realizations)

        """

        if any ([d != self.__state for d in self.__flags]):
            raise ValueError ('missing realizations %s' % ', '.join(['Y%d' % d for d in range(ny) if self.__flags[d] != self.__state]))

        if self.nr < 2:
            raise ValueError ('not enough realizations (%d, min 2)' % self.nr)

        tmp = math.sqrt (self.nr * self.__x2 - self.__x*self.__x)
        self.pcc = [(self.nr * xy - self.__x*y) / tmp / math.sqrt (self.nr*y2 - y*y) for xy,y,y2 in zip (self.__xy, self.__y, self.__y2)]

    def get_pcc (self, ny):
        """Gets the PCCs from a consolidated PCC context.

        Args:
            ny (int): Index of the PCC to get (0 to self.ny - 1)

        Returns:
            the PCC corresponding to number ny

        """

        if not isinstance (ny, int):
            raise TypeError ('parameter ny should be a number')
        if ny < 0 or ny >= self.ny:
            raise ValueError ('Invalid Y index: %d' % ny)

        return self.pcc[ny]
