/*
 * Copyright (C) Telecom ParisTech
 * 
 * This file must be used under the terms of the CeCILL. This source
 * file is licensed as described in the file COPYING, which you should
 * have received as part of this distribution. The terms are also
 * available at:
 * http://www.cecill.info/licences/Licence_CeCILL_V1.1-US.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

#include <utils.h>
#include <des.h>
#include <rdtsc_timer.h>

uint64_t
rand_uint64_t(void);

extern uint64_t
des_p_ta(uint64_t val);

uint64_t
des_f_ta(uint64_t rk, uint64_t val);

uint64_t
des_enc_ta(uint64_t * ks, uint64_t val);

extern int
des_check_f(uint64_t (*f_enc)(uint64_t *, uint64_t), uint64_t (*f_dec)(uint64_t *, uint64_t));

int
des_check_ta(void);

int
measure(uint64_t * ks, uint64_t pt, double th, int average, double *time, uint64_t * ct);

#define TH 1.1
#define AVG 10

int
main(int argc, char **argv) {
  int n, i, j, k, l;
  uint64_t ks[16], pt, ct, key;
  double t;
  struct tms dummy;
  FILE *dat, *txt;

  if(!des_check_ta()) {
    ERROR(-1, -1, "%s: DES functional test failed", argv[0]);
  }
  if(argc != 2 && argc != 3) {
    ERROR(-1, -1, "usage: %s <n> [<key>]", argv[0]);
  }
  n = atoi(argv[1]);
  if(n < 1) {
    ERROR(-1, -1, "%s: number of experiments (<n>) shall be greater than 1 (%d)", argv[0], n);
  }
  txt = XFOPEN("ta.key", "w");
  dat = XFOPEN("ta.dat", "w");
  srand(times(&dummy));
  if(argc == 2) {
    key = rand_uint64_t();
  } else {
    key = strtoull(argv[2], NULL, 0);
  }
  des_ks(ks, key);
  fprintf(txt, "# 64-bits key (with parity bits):    0x%016" PRIx64 "\n", key);
  fprintf(txt, "# 56-bits key (without parity bits):   0x%014" PRIx64 "\n", des_pc1(key));
  for(i = 0; i < 16; i++) {
    fprintf(txt, "# 48-bits round key %2d - 6-bits subkeys: 0x%012" PRIx64 " -", i + 1, ks[i]);
    for(j = 7; j >= 0; j--) {
      fprintf(txt, " 0x%02" PRIx64, (ks[i] >> (j * 6)) & 0x3f);
    }
    fprintf(txt, "\n");
  }
  fprintf(txt, "k16=0x%012" PRIx64 "\n", ks[15]);
  j = n / 100;
  k = 0;
  l = 0;
  for(i = 0; i < n; i++) {
    pt = rand_uint64_t();
    measure(ks, pt, TH, AVG, &t, &ct);
    if(ct != des_enc_ta(ks, pt)) {
      ERROR(-1, -1, "data dependent DES functionally incorrect");
    }
    fprintf(dat, "0x%016" PRIx64 " %f\n", ct, t);
    k += 1;
    if(k == j) {
      l += 1;
      fprintf(stderr, "%3d%%[4D", l);
      k = 0;
    }
  }
  fprintf(stderr, "\n");
  fclose(txt);
  fclose(dat);
  fprintf(stderr, "Acquisitions stored in: ta.dat\n");
  fprintf(stderr, "Secret key stored in:  ta.key\n");
  fprintf(stderr, "Last round key (hex): 0x%012" PRIx64 "\n", ks[15]);
  printf("0x%012" PRIx64 "\n", ks[15]);
  return 0;
}

uint64_t
rand_uint64_t(void) {
  uint64_t res;
  int i;

  res = UINT64_C(0x0);
  for(i = 0; i < 4; i++) {
    res <<= 16;
    res |= (uint64_t)(rand() & 0xffff);
  }
  return res;
}

uint64_t
des_f_ta(uint64_t rk, uint64_t val) {
  if(val >> 32) {
    ERROR(0, -1, "Invalid R input value for F function: 0x%016" PRIx64, val);
  }
  if(rk >> 48) {
    ERROR(0, -1, "Invalid RK input value for F function: 0x%016" PRIx64, rk);
  }
  return des_p_ta(des_sboxes(des_e(val) ^ rk));
}

uint64_t
des_enc_ta(uint64_t * ks, uint64_t val) {
  uint64_t lr, r, l, tmp;
  int i;

  lr = des_ip(val);
  r = des_right_half(lr);
  l = des_left_half(lr);
  for(i = 0; i < 16; i++) {
    tmp = r;
    r = l ^ des_f_ta(ks[i], r);
    l = tmp;
  }
  return des_fp((r << 32) | l);
}

int
des_check_ta(void) {
  return des_check_f(des_enc_ta, des_dec);
}

int
measure(uint64_t * ks, uint64_t pt, double th, int average, double *time, uint64_t * ct) {
  uint64_t a, b, t, min, *m;
  int i, n, cnt;

  if(average < 1) {
    ERROR(0, -1, "Invalid average value: %d", average);
  }
  if(th < 1.0) {
    ERROR(0, -1, "Invalid threshold value: %f", th);
  }
  m = XCALLOC(average, sizeof(uint64_t));
  min = UINT64_C(0);
  for(i = 0; i < average; i++) {
    a = get_rdtsc_timer();
    *ct = des_enc_ta(ks, pt);
    b = get_rdtsc_timer();
    t = b - a;
    m[i] = t;
    if(i == 0 || t < min) {
      min = t;
    }
  }
  n = 0;
  i = 0;
  cnt = average;
  while(n < average) {
    if(m[i] <= th * min) {
      n += 1;
      i =(i + 1) % average;
    } else {
      do {
        a = get_rdtsc_timer();
        *ct = des_enc_ta(ks, pt);
        b = get_rdtsc_timer();
        cnt += 1;
        t = b - a;
      }
      while(t > min * th);
      if(t < min) {
        n = 0;
        min = t;
      }
      m[i] = t;
      n += 1;
      i =(i + 1) % average;
    }
  }
  t = 0;
  for(i = 0; i < average; i++) {
    t += m[i];
  }
  *time = (double)(t) / (double)(average);
  return cnt;
}
