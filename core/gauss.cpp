//  Author: Troy Sandblom

#include "gauss.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>


/*
 *  function:	gauss(double **a, double *b, double *x, int n)
 *
 *  Solves linear equations.  Finds vector x such that ax = b.
 *
 *	a - nXn matrix
 *	b - vector size n
 *	x - vector size n
 *	n - number of variables (size of vectors) (must be > 1)
 *
 *	this function will alter a and b, and put the solution in x.
 *	returns 1 if the solution was found, 0 otherwise.
 *
 *
 */
/*--------------------------------------------------------------------------*/
int gauss(double **a, double *b, double *x, int n)
{
  int i,j,k;
  int ip, kk, jj;
  double temp, *temp_p;
  double pivot;
  double q;

#ifndef UNDER_CE
  assert(n > 1);
#endif


  /*
   *	transform matrix to echelon form.
   */
  for (i = 0; i < n-1; i++) {

    /*
     *	Find the pivot.
     */
    pivot = 0.0;
    for (j = i; j < n; j++) {
      temp = fabs(a[j][i]);
      if (temp > pivot) {
        pivot = temp;
        ip = j;
      }
    }

    if (pivot < 1.E-14)
      /*
       *   Error - singular matrix.
       */
      return 0;


    /*
     *	Move the pivot row to the ith position
     */
    if (ip != i) {
      temp_p = a[i];
      a[i] = a[ip];
      a[ip] = temp_p;
      temp = b[i];
      b[i] = b[ip];
      b[ip] = temp;
    }

    /*
     *	Zero entries below the diagonal.
     */
    for (k = i + 1; k < n; k++) {

      q = -a[k][i] / a[i][i];

      a[k][i] = 0.0;

      for (j = i + 1; j < n; j++)
        a[k][j] = q * a[i][j] + a[k][j];
      b[k] = q * b[i] + b[k];
    }

  }

  if (fabs(a[n-1][n-1]) < 1.E-14) {
    return 0;
  }


  /*
   *	Backsolve to obtain solution vector x.
   */
  kk = n - 1;
  x[kk] = b[kk] / a[kk][kk];
  for (k = 0; k < n - 1; k++) {
    kk = n - k - 2;
    q = 0.0;

    for (j = 0; j <= k; j++) {
      jj = n - j - 1;
      q = q + a[kk][jj] * x[jj];
    }
    x[kk] = (b[kk] - q) / a[kk][kk];
  }

  return 1;
}
