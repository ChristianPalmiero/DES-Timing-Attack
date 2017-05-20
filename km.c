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

#include <utils.h>
#include <des.h>
#include <km.h>

des_key_manager
*des_km_init (void)
{
  des_key_manager *km;

  km = XCALLOC (1, sizeof (struct des_key_manager_s));
  km->mask = UINT64_C (0x0);
  km->key = UINT64_C (0x0);
  return km;
}

int
des_km_known (des_key_manager *km)
{
  return hamming_weight(km->mask);
}

int
des_km_set_sk (des_key_manager *km, int rk, int sk, int force, uint64_t mask,
	       uint64_t val)
{
  if ((sk < 1) || (sk > 8))
    {
      ERROR (0, -1, "Invalid subkey number: %d", sk);
    }
  if (mask >> 6)
    {
      ERROR (0, -1, "Invalid mask value: 0x%016" PRIx64, mask);
    }
  if (val >> 6)
    {
      ERROR (0, -1, "Invalid subkey value: 0x%016" PRIx64, val);
    }
  mask <<= 6 * (8 - sk);
  val <<= 6 * (8 - sk);
  return des_km_set_rk (km, rk, force, mask, val);
}

int
des_km_set_rk (des_key_manager *km, int rk, int force, uint64_t mask,
	       uint64_t val)
{
  int i;

  if ((rk < 1) || (rk > 16))
    {
      ERROR (0, -1, "Invalid round key number: %d", rk);
    }
  if (mask >> 48)
    {
      ERROR (0, -1, "Invalid mask value: 0x%016" PRIx64, mask);
    }
  if (val >> 48)
    {
      ERROR (0, -1, "Invalid subkey value: 0x%016" PRIx64, val);
    }
  mask = des_n_pc2 (mask); __check_forward_exception (0)
  val = des_n_pc2 (val); __check_forward_exception (0)
  for (i = rk; i < 16; i++)
    {
      mask = des_ls (mask); __check_forward_exception (0)
      val = des_ls (val); __check_forward_exception (0)
      if (left_shifts[i] == 1)
	{
	  mask = des_ls (mask); __check_forward_exception (0)
	  val = des_ls (val); __check_forward_exception (0)
	}
    }
  return des_km_set_c0d0 (km, force, mask, val);
}

uint64_t
des_km_set_c0d0 (des_key_manager *km, int force, uint64_t mask, uint64_t val)
{
  uint64_t conflict;

  if (mask >> 56)
    {
      ERROR (0, -1, "Invalid mask value: 0x%016" PRIx64, mask);
    }
  if (val >> 56)
    {
      ERROR (0, -1, "Invalid subkey value: 0x%016" PRIx64, val);
    }
  val &= mask;
  conflict = (km->key ^ val) & km->mask & mask;
  if ((conflict == UINT64_C (0x0)) || force)
    {
      km->mask |= mask;
      km->key = (km->key & (~mask)) | val;
    }
  return (conflict == UINT64_C (0x0)) ? UINT64_C (0x1) : UINT64_C (0x0);
}

int
des_km_set_key (des_key_manager *km, int force, uint64_t mask, uint64_t val)
{
  mask = des_pc1 (mask); __check_forward_exception (0)
  val = des_pc1 (val); __check_forward_exception (0)
  return des_km_set_c0d0 (km, force, mask, val);
}

uint64_t
des_km_get_sk (des_key_manager *km, int rk, int sk, uint64_t * mask)
{
  uint64_t val;

  if ((sk < 1) || (sk > 8))
    {
      ERROR (0, -1, "Invalid subkey number: %d", sk);
    }
  val = des_km_get_rk (km, rk, mask); __check_forward_exception (0)
  *mask = ((*mask) >> (6 * (8 - sk))) & UINT64_C (0x3f);
  val = (val >> (6 * (8 - sk))) & UINT64_C (0x3f);
  return val;
}

uint64_t
des_km_get_rk (des_key_manager *km, int rk, uint64_t * mask)
{
  uint64_t val;
  int i;

  if ((rk < 1) || (rk > 16))
    {
      ERROR (0, -1, "Invalid round key number: %d", rk);
    }
  val = des_km_get_c0d0 (km, mask); __check_forward_exception (0)
  for (i = 0; i < rk; i++)
    {
      *mask = des_ls (*mask); __check_forward_exception (0)
      val = des_ls (val); __check_forward_exception (0)
      if (left_shifts[i] == 1)
	{
	  *mask = des_ls (*mask); __check_forward_exception (0)
	  val = des_ls (val); __check_forward_exception (0)
	}
    }
  *mask = des_pc2 (*mask); __check_forward_exception (0)
  val = des_pc2 (val); __check_forward_exception (0)
  return val;
}

uint64_t
des_km_get_c0d0 (des_key_manager *km, uint64_t * mask)
{
  *mask = km->mask;
  return km->key;
}

uint64_t
des_km_get_key (des_key_manager *km, uint64_t * mask)
{
  uint64_t val, tmp, mask2;
  int i;

  val = des_km_get_c0d0 (km, mask); __check_forward_exception (0)
  *mask = des_n_pc1 (*mask); __check_forward_exception (0)
  val = des_n_pc1 (val); __check_forward_exception (0)
  tmp = *mask >> 1;
  mask2 = UINT64_C (0x1);
  for (i = 0; i < 8; i++)
    {
      if ((tmp & UINT64_C (0x7f)) != UINT64_C (0x7f))
	{
	  *mask &= (~mask2);
	}
      mask2 <<= 8;
      tmp >>= 8;
    }
  return val;
}

void
des_km_init_for_unknown (des_key_manager *km)
{
  km->key &= km->mask;
}

uint64_t
des_km_for_unknown (des_key_manager *km)
{
  uint64_t key;

  key = km->key & (~(km->mask));
  key += km->mask + UINT64_C (0x1);
  key &= ~(km->mask);
  key &= UINT64_C (0xffffffffffffff);
  km->key = (km->key & km->mask) | key;
  return key;
}

void
des_km_free (des_key_manager *km)
{
  free (km);
}
