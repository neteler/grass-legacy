#include<stdio.h>
#include<stdlib.h>
#include<math.h>

double *cramer_von_mises (x, n)
  double *x;
  int n;
{
  int i, dcmp();
  static double y[2];
  double mean = 0.0, sdx = 0.0, fx, sqrt2, *xcopy, normp();

  sqrt2 = sqrt ((double) 2.0);
  y[1] = 0.0;
  if ((xcopy = (double *) malloc (n * sizeof (double))) == NULL)
    fprintf (stderr, "Memory error in cramer_von_mises\n"), exit (-1);

  for (i = 0; i < n; ++i)
  {
    xcopy[i] = x[i];
    mean += x[i];
    sdx += x[i] * x[i];
  }
  sdx = sqrt ((n * sdx - mean * mean) / (n * (n - 1.0)));
  mean /= n;
  qsort (xcopy, n, sizeof (double), dcmp);
  for (i = 0; i < n; ++i)
  {
    fx = 0.5 + normp ((xcopy[i] - mean) / sdx / sqrt2) / 2.0;
    if (fx <= 1e-5)
      fx = 1e-5;
    if (fx >= 0.99999)
      fx = 0.99999;
    fx -= (2.0 * i + 1.0) / (2.0 * n);
    y[1] += fx * fx;
  }
  y[1] += 1.0 / (double) (n * 12);
  y[0] = y[1] * (0.5 / n + 1.0);
#ifdef NOISY
  fprintf (stdout,"  TEST9  CVM(N) =%10.4f\n", y[0]);
#endif				/* NOISY */
  free(xcopy);
  return y;
}
