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
#include <math.h>

#include <utils.h>
#include <pcc.h>

pcc_context
*pcc_init (int ny)
{
  pcc_context *ctx;
  int i;

  if (ny < 1)
    {
      ERROR (NULL, -1, "Invalid number of Y random variables: %d", ny);
    }
  ctx = XCALLOC (1, sizeof (struct pcc_context_s));
  ctx->ny = ny;
  ctx->nr = 0;
  ctx->x = 0.0;
  ctx->x2 = 0.0;
  ctx->y = XCALLOC (ny, sizeof (double));
  ctx->y2 = XCALLOC (ny, sizeof (double));
  ctx->xy = XCALLOC (ny, sizeof (double));
  ctx->pcc = XCALLOC (ny, sizeof (double));
  ctx->state = 0;
  ctx->cnt = ny;
  ctx->flags = XCALLOC (ny, sizeof (char));
  for (i = 0; i < ny; i++)
    {
      ctx->y[i] = 0.0;
      ctx->y2[i] = 0.0;
      ctx->xy[i] = 0.0;
      ctx->pcc[i] = 0.0;
      ctx->flags[i] = 0;
    }
  return ctx;
}

void
pcc_insert_x (pcc_context *ctx, double x)
{
  if (ctx->cnt != ctx->ny)
    {
      ERROR (, -1, "missing %d Y realizations", ctx->ny - ctx->cnt);
    }
  ctx->cnt = 0;
  ctx->state = 1 - ctx->state;
  ctx->rx = x;
  ctx->x += x;
  ctx->x2 += x * x;
  ctx->nr += 1;
}

void
pcc_insert_y (pcc_context *ctx, int ny, double y)
{
  if (ny < 0 || ny >= ctx->ny)
    {
      ERROR (, -1, "Invalid Y index: %d", ny);
    }
  if (ctx->flags[ny] == ctx->state)
    {
      ERROR (, -1, "Y realization #%d inserted twice", ny);
    }
  ctx->y[ny] += y;
  ctx->y2[ny] += y * y;
  ctx->xy[ny] += ctx->rx * y;
  ctx->cnt += 1;
  ctx->flags[ny] = ctx->state;
}

void
pcc_consolidate (pcc_context *ctx)
{
  double n;
  double tmp;
  int i;

  if (ctx->cnt != ctx->ny)
    {
      ERROR (, -1, "missing %d Y realizations", ctx->ny - ctx->cnt);
    }
  if (ctx->nr < 2)
    {
      ERROR (, -1, "not enough realizations (%d, min 2)", ctx->nr);
    }
  n = (double) (ctx->nr);
  tmp = sqrt (n * ctx->x2 - ctx->x * ctx->x);
  for (i = 0; i < ctx->ny; i++)
    {
      ctx->pcc[i] = (n * ctx->xy[i] - ctx->x * ctx->y[i]) / tmp /
	sqrt (n * ctx->y2[i] - ctx->y[i] * ctx->y[i]);
    }
}

double
pcc_get_pcc (pcc_context *ctx, int ny)
{
  if (ny < 0 || ny >= ctx->ny)
    {
      ERROR (0, -1, "Invalid Y index: %d", ny);
    }
  return ctx->pcc[ny];
}

void
pcc_free (pcc_context *ctx)
{
  free (ctx->y);
  free (ctx->y2);
  free (ctx->xy);
  free (ctx->pcc);
  free (ctx->flags);
  free (ctx);
}
