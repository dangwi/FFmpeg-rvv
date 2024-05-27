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
#include "libavcodec/bit_depth_template.c"
#include "libavcodec/hevcdec.h"

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

static const int16_t trans4x4[16] = {
    64,  64,  64,  64,
    83,  36, -36, -83,
    64, -64, -64,  64,
    36, -83,  83, -36
};

static const int16_t trans4x4odd[16] = {
    89,  75,  50,  18,
    75, -18, -89, -50,
    50, -89,  18,  75,
    18, -50,  75, -89
};

/*
#define TR_4(dst, src, dstep, sstep, assign, end)                 \
    do {                                                          \
        int vl;                                                   \
        vint16m1_t vcoe16;                                        \
        vint32m2_t vo32;                                          \
        vl = vsetvl_e16m1(4);                                     \
        vo32 = vmv_v_x_i32m2(0, vl);                              \
        vcoe16 = vle16_v_i16m1(&trans4x4[0], vl);                 \
        vo32 = vwmacc_vx_i32m2(vo32, src[0 * sstep], vcoe16, vl);         \
        vcoe16 = vle16_v_i16m1(&trans4x4[4], vl);                 \
        vo32 = vwmacc_vx_i32m2(vo32, src[1 * sstep], vcoe16, vl);         \
        vcoe16 = vle16_v_i16m1(&trans4x4[8], vl);                 \
        vo32 = vwmacc_vx_i32m2(vo32, src[2 * sstep], vcoe16, vl);         \
        vcoe16 = vle16_v_i16m1(&trans4x4[12], vl);                \
        vo32 = vwmacc_vx_i32m2(vo32, src[3 * sstep], vcoe16, vl);         \
        vse32_v_i32m2(&dst[0], vo32, vl);                         \
    } while (0)
*/

#define TR_8_bak(dst, src, dstep, sstep, assign, end)                 \
    do {                                                          \
        int i, j;                                                 \
        int e_8[4];                                               \
        int o_8[4];                                               \
        int vl;                                                   \
        vint8m1_t  vtrans8;                                       \
        vint16m2_t vtrans16;                                      \
        vint32m4_t vo32;                                          \
        vl = vsetvl_e32m4(4);                                     \
        vo32 = vmv_v_x_i32m4(0, vl);                              \
        for (j = 1; j < end; j += 2) {                            \
            vtrans8  = vle8_v_i8m1(&transform[4 * j][0], vl);     \
            vtrans16 = vwadd_vx_i16m2(vtrans8, 0, vl);            \
            vo32 = vwmacc_vx_i32m4(vo32, src[j * sstep], vtrans16, vl); \
        }                                                         \
        vse32_v_i32m4(&o_8[0], vo32, vl);                         \
                                                                  \
        vint16m1_t vcoe16;                                        \
        vint32m2_t vo32m2;                                        \
        vl = vsetvl_e16m1(4);                                     \
        vo32m2 = vmv_v_x_i32m2(0, vl);                              \
        vcoe16 = vle16_v_i16m1(&trans4x4[0], vl);                   \
        vo32m2 = vwmacc_vx_i32m2(vo32m2, src[0 * 2 * sstep], vcoe16, vl);         \
        vcoe16 = vle16_v_i16m1(&trans4x4[4], vl);           \
        vo32m2 = vwmacc_vx_i32m2(vo32m2, src[1 * 2 * sstep], vcoe16, vl);         \
        vcoe16 = vle16_v_i16m1(&trans4x4[8], vl);           \
        vo32m2 = vwmacc_vx_i32m2(vo32m2, src[2 * 2 * sstep], vcoe16, vl);         \
        vcoe16 = vle16_v_i16m1(&trans4x4[12], vl);           \
        vo32m2 = vwmacc_vx_i32m2(vo32m2, src[3 * 2 * sstep], vcoe16, vl);         \
        vse32_v_i32m2(e_8, vo32m2, vl);                         \
                                                                  \
        for (i = 0; i < 4; i++) {                                 \
            assign(dst[i * dstep], e_8[i] + o_8[i]);              \
            assign(dst[(7 - i) * dstep], e_8[i] - o_8[i]);        \
        }                                                         \
    } while (0)

#define TR_8(dst, src, dstep, sstep, assign, end)                 \
    do {                                                          \
        int i, j;                                                 \
        int e_8[4];                                               \
        int o_8[4];                                               \
        int vl;                                                   \
        vint16m1_t vtrans16, vout16;                              \
        vint32m2_t ve32, vo32;                                    \
        vl = vsetvl_e32m2(4);                                     \
        vo32 = vmv_v_x_i32m2(0, vl);                              \
        vtrans16 = vle16_v_i16m1(&trans4x4odd[0], vl);            \
        vo32 = vwmacc_vx_i32m2(vo32, src[1 * sstep], vtrans16, vl); \
        vtrans16 = vle16_v_i16m1(&trans4x4odd[4], vl);            \
        vo32 = vwmacc_vx_i32m2(vo32, src[3 * sstep], vtrans16, vl); \
        vtrans16 = vle16_v_i16m1(&trans4x4odd[8], vl);            \
        vo32 = vwmacc_vx_i32m2(vo32, src[5 * sstep], vtrans16, vl); \
        vtrans16 = vle16_v_i16m1(&trans4x4odd[12], vl);           \
        vo32 = vwmacc_vx_i32m2(vo32, src[7 * sstep], vtrans16, vl); \
                                                                  \
        ve32 = vmv_v_x_i32m2(0, vl);                              \
        vtrans16 = vle16_v_i16m1(&trans4x4[0], vl);               \
        ve32 = vwmacc_vx_i32m2(ve32, src[0 * sstep], vtrans16, vl); \
        vtrans16 = vle16_v_i16m1(&trans4x4[4], vl);               \
        ve32 = vwmacc_vx_i32m2(ve32, src[2 * sstep], vtrans16, vl); \
        vtrans16 = vle16_v_i16m1(&trans4x4[8], vl);               \
        ve32 = vwmacc_vx_i32m2(ve32, src[4 * sstep], vtrans16, vl); \
        vtrans16 = vle16_v_i16m1(&trans4x4[12], vl);              \
        ve32 = vwmacc_vx_i32m2(ve32, src[6 * sstep], vtrans16, vl); \
                                                                  \
        vse32_v_i32m2(o_8, vo32, vl);                             \
        vse32_v_i32m2(e_8, ve32, vl);                             \
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
        int o_16[8];                                              \
        int vl;                                                   \
        vint8m1_t  vtrans8;                                       \
        vint16m2_t vtrans16;                                      \
        vint32m4_t vo32;                                          \
        vl = vsetvl_e32m4(8);                                     \
        vo32 = vmv_v_x_i32m4(0, vl);                              \
        for (j = 1; j < end; j += 2) {                            \
            vtrans8  = vle8_v_i8m1(&transform[2 * j][0], vl);     \
            vtrans16 = vwadd_vx_i16m2(vtrans8, 0, vl);            \
            vo32 = vwmacc_vx_i32m4(vo32, src[j * sstep], vtrans16, vl); \
        }                                                         \
        vse32_v_i32m4(o_16, vo32, vl);                            \
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
        int o_32[16];                                             \
        int vl;                                                   \
        vint8m1_t  vtrans8;                                       \
        vint16m2_t vtrans16;                                      \
        vint32m4_t vo32;                                          \
        vl = vsetvl_e32m4(16);                                    \
        vo32 = vmv_v_x_i32m4(0, vl);                              \
        for (j = 1; j < end; j += 2) {                            \
            vtrans8  = vle8_v_i8m1(&transform[j][0], vl);         \
            vtrans16 = vwadd_vx_i16m2(vtrans8, 0, vl);            \
            vo32 = vwmacc_vx_i32m4(vo32, src[j * sstep], vtrans16, vl); \
        }                                                         \
        vse32_v_i32m4(o_32, vo32, vl);                            \
        TR_16(e_32, src, 1, 2 * sstep, SET, end / 2);             \
                                                                  \
        for (i = 0; i < 16; i++) {                                \
            assign(dst[i * dstep], e_32[i] + o_32[i]);            \
            assign(dst[(31 - i) * dstep], e_32[i] - o_32[i]);     \
        }                                                         \
    } while (0)

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

void ff_hevc_idct_4x4_rvv(int16_t *coeffs, int col_limit)
{
    int vl;
    vint16m1_t vcoe16, vo16;
    vint32m2_t vo32v0, vo32v1, vo32v2, vo32v3;
    vint16m1_t vo16c0, vo16c1, vo16c2, vo16c3;

    vl = vsetvl_e16m1(4);
    vo32v0 = vmv_v_x_i32m2(0, vl);
    vo32v1 = vmv_v_x_i32m2(0, vl);
    vo32v2 = vmv_v_x_i32m2(0, vl);
    vo32v3 = vmv_v_x_i32m2(0, vl);

#define MACC1x4(line)                                                   \
    vcoe16 = vle16_v_i16m1(&trans4x4[line * 4], vl);                    \
    vo32v0 = vwmacc_vx_i32m2(vo32v0, coeffs[line * 4 + 0], vcoe16, vl); \
    vo32v1 = vwmacc_vx_i32m2(vo32v1, coeffs[line * 4 + 1], vcoe16, vl); \
    vo32v2 = vwmacc_vx_i32m2(vo32v2, coeffs[line * 4 + 2], vcoe16, vl); \
    vo32v3 = vwmacc_vx_i32m2(vo32v3, coeffs[line * 4 + 3], vcoe16, vl);

    MACC1x4(0)
    MACC1x4(1)
    MACC1x4(2)
    MACC1x4(3)

    vo16c0 = vnclip_wx_i16m1(vo32v0, 7, vl);
    vo16c1 = vnclip_wx_i16m1(vo32v1, 7, vl);
    vo16c2 = vnclip_wx_i16m1(vo32v2, 7, vl);
    vo16c3 = vnclip_wx_i16m1(vo32v3, 7, vl);

    vo32v0 = vmv_v_x_i32m2(0, vl);
    vo32v1 = vmv_v_x_i32m2(0, vl);
    vo32v2 = vmv_v_x_i32m2(0, vl);
    vo32v3 = vmv_v_x_i32m2(0, vl);

#define MACC4x1(col)                                                         \
    vo32v0 = vwmacc_vx_i32m2(vo32v0, trans4x4[col * 4 + 0], vo16c##col, vl); \
    vo32v1 = vwmacc_vx_i32m2(vo32v1, trans4x4[col * 4 + 1], vo16c##col, vl); \
    vo32v2 = vwmacc_vx_i32m2(vo32v2, trans4x4[col * 4 + 2], vo16c##col, vl); \
    vo32v3 = vwmacc_vx_i32m2(vo32v3, trans4x4[col * 4 + 3], vo16c##col, vl);

    MACC4x1(0)
    MACC4x1(1)
    MACC4x1(2)
    MACC4x1(3)

    vo16   = vnclip_wx_i16m1(vo32v0, 12, vl);
    vsse16_v_i16m1(&coeffs[0], 2 * 4, vo16, vl);
    vo16   = vnclip_wx_i16m1(vo32v1, 12, vl);
    vsse16_v_i16m1(&coeffs[1], 2 * 4, vo16, vl);
    vo16   = vnclip_wx_i16m1(vo32v2, 12, vl);
    vsse16_v_i16m1(&coeffs[2], 2 * 4, vo16, vl);
    vo16   = vnclip_wx_i16m1(vo32v3, 12, vl);
    vsse16_v_i16m1(&coeffs[3], 2 * 4, vo16, vl);

/*
    int vl;
    vint16m1_t vrow16;
    vint32m2_t vo32v0, vo32v1, vo32v2, vo32v3;
    vint16m1_t vo16;

    vl = vsetvl_e16m1(4);
    vo32v0 = vmv_v_x_i32m2(0, vl);
    vo32v1 = vmv_v_x_i32m2(0, vl);
    vo32v2 = vmv_v_x_i32m2(0, vl);
    vo32v3 = vmv_v_x_i32m2(0, vl);

#define MACC1x4(line)                                                \
    vrow16 = vle16_v_i16m1(&src[line * 4], vl);                   \
    vo32v0 = vwmacc_vx_i32m2(vo32v0, trans4x4[line * 4 + 0], vrow16, vl); \
    vo32v1 = vwmacc_vx_i32m2(vo32v1, trans4x4[line * 4 + 1], vrow16, vl); \
    vo32v2 = vwmacc_vx_i32m2(vo32v2, trans4x4[line * 4 + 2], vrow16, vl); \
    vo32v3 = vwmacc_vx_i32m2(vo32v3, trans4x4[line * 4 + 3], vrow16, vl);

    MACC1x4(0)
    MACC1x4(1)
    MACC1x4(2)
    MACC1x4(3)

    vo16   = vnclip_wx_i16m1(vo32v0, 7, vl);
    vse16_v_i16m1(&src[0], vo16, vl);

    vo16   = vnclip_wx_i16m1(vo32v1, 7, vl);
    vse16_v_i16m1(&src[4], vo16, vl);

    vo16   = vnclip_wx_i16m1(vo32v2, 7, vl);
    vse16_v_i16m1(&src[8], vo16, vl);

    vo16   = vnclip_wx_i16m1(vo32v3, 7, vl);
    vse16_v_i16m1(&src[12], vo16, vl);
*/
}

void ff_hevc_idct_8x8_rvv(int16_t *coeffs, int col_limit)
{
    int i;
    int      shift = 7;
    int      add   = 1 << (shift - 1);
    int16_t *src   = coeffs;
    int limit  = FFMIN(col_limit, 8);
    int limit2 = FFMIN(col_limit + 4, 8);

    for (i = 0; i < 8; i++) {
        TR_8_bak(src, src, 8, 8, SCALE, limit2);
        if (limit2 < 8 && i%4 == 0 && !!i)
            limit2 -= 4;
        src++;
    }

    shift = 12; /* 20 - BIT_DEPTH */
    add   = 1 << (shift - 1);
    for (i = 0; i < 8; i++) {
        TR_8_bak(coeffs, coeffs, 1, 1, SCALE, limit);
        coeffs += 8;
    }
}

/*
    int i;
    int      shift = 7;
    int      add   = 1 << (shift - 1);
    int16_t *src   = coeffs;
    IDCT_VAR##H(H);

    for (i = 0; i < H; i++) {
        TR_ ## H(src, src, H, H, SCALE, limit2);
        if (limit2 < H && i%4 == 0 && !!i)
            limit2 -= 4;
        src++;
    }

    shift = 12;
    add   = 1 << (shift - 1);
    for (i = 0; i < H; i++) {
        TR_ ## H(coeffs, coeffs, 1, 1, SCALE, limit);
        coeffs += H;
    }
*/

// IDCT( 8)
IDCT(16)
IDCT(32)

#include <stdio.h>
#include <time.h>

#define TIME_VAL                        \
    struct timespec start = {0, 0};     \
    struct timespec end   = {0, 0};     \
    static long total;                  \
    static int count;

#define TIME_START  clock_gettime(CLOCK_MONOTONIC, &start)
#define TIME_END    clock_gettime(CLOCK_MONOTONIC, &end)


static void *MEMCPY_RVV(void *restrict destination,
                    const void *restrict source, size_t n) {
    unsigned char *dst = destination;
    const unsigned char *src = source;
    // copy data byte by byte
    for (size_t vl; n > 0; n -= vl, src += vl, dst += vl) {
        vl = vsetvl_e8m8(n);
        vuint8m8_t vec_src = vle8_v_u8m8(src, vl);
        vse8_v_u8m8(dst, vec_src, vl);
    }
    return destination;
}


void put_hevc_pel_uni_pixels_rvv(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                                 int height, intptr_t mx, intptr_t my, int width)
{
    int y;
    pixel *src          = (pixel *)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);

    for (y = 0; y < height; y++) {
        MEMCPY_RVV(dst, src, width * sizeof(pixel));
        src += srcstride;
        dst += dststride;
    }
}

#define QPEL_FILTER(src, stride)                                               \
    (filter[0] * src[x - 3 * stride] +                                         \
     filter[1] * src[x - 2 * stride] +                                         \
     filter[2] * src[x -     stride] +                                         \
     filter[3] * src[x             ] +                                         \
     filter[4] * src[x +     stride] +                                         \
     filter[5] * src[x + 2 * stride] +                                         \
     filter[6] * src[x + 3 * stride] +                                         \
     filter[7] * src[x + 4 * stride])


void put_hevc_qpel_uni_hv_rvv(uint8_t *_dst,  ptrdiff_t _dststride,
                              uint8_t *_src, ptrdiff_t _srcstride,
                              int height, intptr_t mx, intptr_t my, int width)
{
    
// TIME_VAL;
// TIME_START;

    int x, y;
    const int8_t *filter;
    pixel *src = (pixel*)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    int16_t tmp_array[(MAX_PB_SIZE + QPEL_EXTRA) * MAX_PB_SIZE];
    int16_t *tmp = tmp_array;
    int shift =  6;
    int offset = 1 << (shift - 1);

    src   -= QPEL_EXTRA_BEFORE * srcstride;
    filter = ff_hevc_qpel_filters[mx - 1];

    int vl;
    vint8m1_t vfilter;
    vuint8m1_t vsrc;
    vint16m2_t vtmpm2;
    vint16m1_t vzero, vtmpm1;
    vl = vsetvl_e16m2(8);
    vfilter = vle8_v_i8m1(filter, vl);
    vzero = vmv_s_x_i16m1(vzero, 0, vl);

    for (y = 0; y < height + QPEL_EXTRA; y++) {
        for (x = 0; x < width; x++) {
            vsrc = vle8_v_u8m1(&src[x-3], vl);
            vtmpm2 = vwmulsu_vv_i16m2(vfilter, vsrc, vl);
            vtmpm1 = vredsum_vs_i16m2_i16m1(vtmpm1, vtmpm2, vzero, vl);
            tmp[x] = vmv_x_s_i16m1_i16(vtmpm1);
        }
        src += srcstride;
        tmp += MAX_PB_SIZE;
    }

    tmp    = tmp_array + QPEL_EXTRA_BEFORE * MAX_PB_SIZE;
    filter = ff_hevc_qpel_filters[my - 1];


    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            dst[x] = av_clip_pixel(((QPEL_FILTER(tmp, MAX_PB_SIZE) >> 6) + offset) >> shift);
        }
        tmp += MAX_PB_SIZE;
        dst += dststride;
    }

// TIME_END;
// total += (end.tv_sec - start.tv_sec) * 1000000000UL + end.tv_nsec - start.tv_nsec;
// count ++;
// if (count % 1000 == 0) {
//     printf("count: %5d, TIME: %ldus\n", count, total / 1000);
// }

}


#define EPEL_FILTER(src, stride)                                               \
    (filter[0] * src[x - stride] +                                             \
     filter[1] * src[x]          +                                             \
     filter[2] * src[x + stride] +                                             \
     filter[3] * src[x + 2 * stride])

void put_hevc_epel_uni_hv_rvv(uint8_t *_dst, ptrdiff_t _dststride, uint8_t *_src, ptrdiff_t _srcstride,
                              int height, intptr_t mx, intptr_t my, int width)
{
    int x, y;
    pixel *src = (pixel *)_src;
    ptrdiff_t srcstride = _srcstride / sizeof(pixel);
    pixel *dst          = (pixel *)_dst;
    ptrdiff_t dststride = _dststride / sizeof(pixel);
    const int8_t *filter = ff_hevc_epel_filters[mx - 1];
    int16_t tmp_array[(MAX_PB_SIZE + EPEL_EXTRA) * MAX_PB_SIZE];
    int16_t *tmp = tmp_array;
    int shift = 6;
    int offset = 1 << (shift - 1);

    src -= EPEL_EXTRA_BEFORE * srcstride;

    for (y = 0; y < height + EPEL_EXTRA; y++) {
        for (x = 0; x < width; x++)
            tmp[x] = EPEL_FILTER(src, 1);
        src += srcstride;
        tmp += MAX_PB_SIZE;
    }

    tmp      = tmp_array + EPEL_EXTRA_BEFORE * MAX_PB_SIZE;
    filter = ff_hevc_epel_filters[my - 1];

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++)
            dst[x] = av_clip_pixel(((EPEL_FILTER(tmp, MAX_PB_SIZE) >> 6) + offset) >> shift);
        tmp += MAX_PB_SIZE;
        dst += dststride;
    }
}


#endif