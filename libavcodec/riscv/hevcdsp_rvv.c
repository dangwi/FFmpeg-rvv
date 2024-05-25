/*
 * Copyright (c) 2024 Sophgo, Inc. All rights reserved.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavcodec/riscv/hevcdsp_rvv.h"
#if HAVE_RVV
#include <riscv_vector.h>
#include "libavutil/common.h"

static const int8_t transform[32][32] = {
    { 64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,
      64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64,  64 },
    { 90,  90,  88,  85,  82,  78,  73,  67,  61,  54,  46,  38,  31,  22,  13,   4,
      -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90 },
    { 90,  87,  80,  70,  57,  43,  25,   9,  -9, -25, -43, -57, -70, -80, -87, -90,
     -90, -87, -80, -70, -57, -43, -25,  -9,   9,  25,  43,  57,  70,  80,  87,  90 },
    { 90,  82,  67,  46,  22,  -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13,
      13,  38,  61,  78,  88,  90,  85,  73,  54,  31,   4, -22, -46, -67, -82, -90 },
    { 89,  75,  50,  18, -18, -50, -75, -89, -89, -75, -50, -18,  18,  50,  75,  89,
      89,  75,  50,  18, -18, -50, -75, -89, -89, -75, -50, -18,  18,  50,  75,  89 },
    { 88,  67,  31, -13, -54, -82, -90, -78, -46, -4,   38,  73,  90,  85,  61,  22,
     -22, -61, -85, -90, -73, -38,   4,  46,  78,  90,  82,  54,  13, -31, -67, -88 },
    { 87,  57,   9, -43, -80, -90, -70, -25,  25,  70,  90,  80,  43,  -9, -57, -87,
     -87, -57,  -9,  43,  80,  90,  70,  25, -25, -70, -90, -80, -43,   9,  57,  87 },
    { 85,  46, -13, -67, -90, -73, -22,  38,  82,  88,  54,  -4, -61, -90, -78, -31,
      31,  78,  90,  61,   4, -54, -88, -82, -38,  22,  73,  90,  67,  13, -46, -85 },
    { 83,  36, -36, -83, -83, -36,  36,  83,  83,  36, -36, -83, -83, -36,  36,  83,
      83,  36, -36, -83, -83, -36,  36,  83,  83,  36, -36, -83, -83, -36,  36,  83 },
    { 82,  22, -54, -90, -61,  13,  78,  85,  31, -46, -90, -67,   4,  73,  88,  38,
     -38, -88, -73,  -4,  67,  90,  46, -31, -85, -78, -13,  61,  90,  54, -22, -82 },
    { 80,   9, -70, -87, -25,  57,  90,  43, -43, -90, -57,  25,  87,  70,  -9, -80,
     -80,  -9,  70,  87,  25, -57, -90, -43,  43,  90,  57, -25, -87, -70,   9,  80 },
    { 78,  -4, -82, -73,  13,  85,  67, -22, -88, -61,  31,  90,  54, -38, -90, -46,
      46,  90,  38, -54, -90, -31,  61,  88,  22, -67, -85, -13,  73,  82,   4, -78 },
    { 75, -18, -89, -50,  50,  89,  18, -75, -75,  18,  89,  50, -50, -89, -18,  75,
      75, -18, -89, -50,  50,  89,  18, -75, -75,  18,  89,  50, -50, -89, -18,  75 },
    { 73, -31, -90, -22,  78,  67, -38, -90, -13,  82,  61, -46, -88,  -4,  85,  54,
     -54, -85,   4,  88,  46, -61, -82,  13,  90,  38, -67, -78,  22,  90,  31, -73 },
    { 70, -43, -87,   9,  90,  25, -80, -57,  57,  80, -25, -90,  -9,  87,  43, -70,
     -70,  43,  87,  -9, -90, -25,  80,  57, -57, -80,  25,  90,   9, -87, -43,  70 },
    { 67, -54, -78,  38,  85, -22, -90,   4,  90,  13, -88, -31,  82,  46, -73, -61,
      61,  73, -46, -82,  31,  88, -13, -90,  -4,  90,  22, -85, -38,  78,  54, -67 },
    { 64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64,
      64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64,  64, -64, -64,  64 },
    { 61, -73, -46,  82,  31, -88, -13,  90,  -4, -90,  22,  85, -38, -78,  54,  67,
     -67, -54,  78,  38, -85, -22,  90,   4, -90,  13,  88, -31, -82,  46,  73, -61 },
    { 57, -80, -25,  90,  -9, -87,  43,  70, -70, -43,  87,   9, -90,  25,  80, -57,
     -57,  80,  25, -90,   9,  87, -43, -70,  70,  43, -87,  -9,  90, -25, -80,  57 },
    { 54, -85,  -4,  88, -46, -61,  82,  13, -90,  38,  67, -78, -22,  90, -31, -73,
      73,  31, -90,  22,  78, -67, -38,  90, -13, -82,  61,  46, -88,   4,  85, -54 },
    { 50, -89,  18,  75, -75, -18,  89, -50, -50,  89, -18, -75,  75,  18, -89,  50,
      50, -89,  18,  75, -75, -18,  89, -50, -50,  89, -18, -75,  75,  18, -89,  50 },
    { 46, -90,  38,  54, -90,  31,  61, -88,  22,  67, -85,  13,  73, -82,   4,  78,
     -78,  -4,  82, -73, -13,  85, -67, -22,  88, -61, -31,  90, -54, -38,  90, -46 },
    { 43, -90,  57,  25, -87,  70,   9, -80,  80,  -9, -70,  87, -25, -57,  90, -43,
     -43,  90, -57, -25,  87, -70,  -9,  80, -80,   9,  70, -87,  25,  57, -90,  43 },
    { 38, -88,  73,  -4, -67,  90, -46, -31,  85, -78,  13,  61, -90,  54,  22, -82,
      82, -22, -54,  90, -61, -13,  78, -85,  31,  46, -90,  67,   4, -73,  88, -38 },
    { 36, -83,  83, -36, -36,  83, -83,  36,  36, -83,  83, -36, -36,  83, -83,  36,
      36, -83,  83, -36, -36,  83, -83,  36,  36, -83,  83, -36, -36,  83, -83,  36 },
    { 31, -78,  90, -61,   4,  54, -88,  82, -38, -22,  73, -90,  67, -13, -46,  85,
     -85,  46,  13, -67,  90, -73,  22,  38, -82,  88, -54,  -4,  61, -90,  78, -31 },
    { 25, -70,  90, -80,  43,   9, -57,  87, -87,  57,  -9, -43,  80, -90,  70, -25,
     -25,  70, -90,  80, -43,  -9,  57, -87,  87, -57,   9,  43, -80,  90, -70,  25 },
    { 22, -61,  85, -90,  73, -38,  -4,  46, -78,  90, -82,  54, -13, -31,  67, -88,
      88, -67,  31,  13, -54,  82, -90,  78, -46,   4,  38, -73,  90, -85,  61, -22 },
    { 18, -50,  75, -89,  89, -75,  50, -18, -18,  50, -75,  89, -89,  75, -50,  18,
      18, -50,  75, -89,  89, -75,  50, -18, -18,  50, -75,  89, -89,  75, -50,  18 },
    { 13, -38,  61, -78,  88, -90,  85, -73,  54, -31,   4,  22, -46,  67, -82,  90,
     -90,  82, -67,  46, -22,  -4,  31, -54,  73, -85,  90, -88,  78, -61,  38, -13 },
    {  9, -25,  43, -57,  70, -80,  87, -90,  90, -87,  80, -70,  57, -43,  25, -9,
      -9,  25, -43,  57, -70,  80, -87,  90, -90,  87, -80,  70, -57,  43, -25,   9 },
    {  4, -13,  22, -31,  38, -46,  54, -61,  67, -73,  78, -82,  85, -88,  90, -90,
      90, -90,  88, -85,  82, -78,  73, -67,  61, -54,  46, -38,  31, -22,  13,  -4 },
};

#define SET(dst, x)   (dst) = (x)
#define SCALE(dst, x) (dst) = av_clip_int16(((x) + add) >> shift)

#define TR_4(dst, src, dstep, sstep, assign, end)                 \
    do {                                                          \
        const int e0 = 64 * src[0 * sstep] + 64 * src[2 * sstep]; \
        const int e1 = 64 * src[0 * sstep] - 64 * src[2 * sstep]; \
        const int o0 = 83 * src[1 * sstep] + 36 * src[3 * sstep]; \
        const int o1 = 36 * src[1 * sstep] - 83 * src[3 * sstep]; \
                                                                  \
        assign(dst[0 * dstep], e0 + o0);                          \
        assign(dst[1 * dstep], e1 + o1);                          \
        assign(dst[2 * dstep], e1 - o1);                          \
        assign(dst[3 * dstep], e0 - o0);                          \
    } while (0)

#define TR_8(dst, src, dstep, sstep, assign, end)                 \
    do {                                                          \
        int i, j;                                                 \
        int e_8[4];                                               \
        int o_8[4] = { 0 };                                       \
        for (i = 0; i < 4; i++)                                   \
            for (j = 1; j < end; j += 2)                          \
                o_8[i] += transform[4 * j][i] * src[j * sstep];   \
        TR_4(e_8, src, 1, 2 * sstep, SET, 4);                     \
                                                                  \
        for (i = 0; i < 4; i++) {                                 \
            assign(dst[i * dstep], e_8[i] + o_8[i]);              \
            assign(dst[(7 - i) * dstep], e_8[i] - o_8[i]);        \
        }                                                         \
    } while (0)

#define TR_16(dst, src, dstep, sstep, assign, end)                \
    do {                                                          \
        int i, j;                                                 \
        int e_16[8];                                              \
        int o_16[8] = { 0 };                                      \
        for (i = 0; i < 8; i++)                                   \
            for (j = 1; j < end; j += 2)                          \
                o_16[i] += transform[2 * j][i] * src[j * sstep];  \
        TR_8(e_16, src, 1, 2 * sstep, SET, 8);                    \
                                                                  \
        for (i = 0; i < 8; i++) {                                 \
            assign(dst[i * dstep], e_16[i] + o_16[i]);            \
            assign(dst[(15 - i) * dstep], e_16[i] - o_16[i]);     \
        }                                                         \
    } while (0)

#define TR_32(dst, src, dstep, sstep, assign, end)                \
    do {                                                          \
        int i, j;                                                 \
        int e_32[16];                                             \
        int o_32[16] = { 0 };                                     \
        for (i = 0; i < 16; i++)                                  \
            for (j = 1; j < end; j += 2)                          \
                o_32[i] += transform[j][i] * src[j * sstep];      \
        TR_16(e_32, src, 1, 2 * sstep, SET, end / 2);             \
                                                                  \
        for (i = 0; i < 16; i++) {                                \
            assign(dst[i * dstep], e_32[i] + o_32[i]);            \
            assign(dst[(31 - i) * dstep], e_32[i] - o_32[i]);     \
        }                                                         \
    } while (0)

#define IDCT_VAR4(H)                                              \
    int limit2 = FFMIN(col_limit + 4, H)
#define IDCT_VAR8(H)                                              \
    int limit  = FFMIN(col_limit, H);                             \
    int limit2 = FFMIN(col_limit + 4, H)
#define IDCT_VAR16(H)   IDCT_VAR8(H)
#define IDCT_VAR32(H)   IDCT_VAR8(H)

#define IDCT(H)                                                   \
void ff_hevc_idct_ ## H ## x ## H ## _rvv(int16_t *coeffs,        \
                                          int col_limit)          \
{                                                                 \
    int i;                                                        \
    int      shift = 7;                                           \
    int      add   = 1 << (shift - 1);                            \
    int16_t *src   = coeffs;                                      \
    IDCT_VAR ## H(H);                                             \
                                                                  \
    for (i = 0; i < H; i++) {                                     \
        TR_ ## H(src, src, H, H, SCALE, limit2);                  \
        if (limit2 < H && i%4 == 0 && !!i)                        \
            limit2 -= 4;                                          \
        src++;                                                    \
    }                                                             \
                                                                  \
    shift = 12; /* 20 - BIT_DEPTH */                              \
    add   = 1 << (shift - 1);                                     \
    for (i = 0; i < H; i++) {                                     \
        TR_ ## H(coeffs, coeffs, 1, 1, SCALE, limit);             \
        coeffs += H;                                              \
    }                                                             \
}

IDCT( 4)
IDCT( 8)
IDCT(16)
IDCT(32)

#endif