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

#include "libavutil/cpu.h"
#include "libavcodec/riscv/hevcdsp_rvv.h"

#define RVV_TEST_IDCT 0
#if RVV_TEST_IDCT

#include <time.h>
#include <stdio.h>

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

#define BIT_DEPTH 8
#include "libavcodec/hevcdsp_template.c"
#undef BIT_DEPTH

#define TEST_FUNC(H)                                            \
void ff_hevc_idct_##H##x##H##_test(int16_t *coeffs,             \
                                   int col_limit);              \
void ff_hevc_idct_##H##x##H##_test(int16_t *coeffs,             \
                                   int col_limit)               \
{                                                               \
    struct timespec start = {0, 0};     \
    struct timespec end = {0, 0};       \
                                                                \
    int16_t coeffs_rvv[H * H];                                  \
    memcpy(coeffs_rvv, coeffs, sizeof(coeffs_rvv));             \
                                                                \
    idct_##H##x##H##_8(coeffs, col_limit);                      \
    if (1|H == 4) clock_gettime(CLOCK_MONOTONIC, &start);          \
    ff_hevc_idct_##H##x##H##_rvv(coeffs_rvv, col_limit);        \
    if (1|H == 4) clock_gettime(CLOCK_MONOTONIC, &end);            \
                                                                \
    if (1|H == 4) {                                               \
        static long total;                                    \
        static int count;                                       \
        total += (end.tv_sec - start.tv_sec) * 1000000000 + end.tv_nsec - start.tv_nsec;                   \
        count ++;                                               \
        if (count % 1000 == 0 || (H==32 && count % 300 == 0)) {                                \
            printf("IDCT_%d: %ldus\n", H, total / 1000);            \
        }                                                       \
    }       \
                                                                \
    if (memcmp(coeffs, coeffs_rvv, sizeof(coeffs_rvv))) {       \
        printf("idct_%dx%d failed\n", H, H);                    \
        for (int i=0; i<H; i++) {                               \
            for (int j=0; j<H; j++) {                           \
                printf("%3d:%3d\t", coeffs[i*H+j],              \
                                    coeffs_rvv[i*H+j]);         \
            }                                                   \
            printf("\n");                                       \
        }                                                       \
        exit(1);                                                \
    }                                                           \
}

TEST_FUNC(4)
TEST_FUNC(8)
TEST_FUNC(16)
TEST_FUNC(32)
#undef TEST_FUNC
#endif

#define RVV_TEST_PEL 0
#if RVV_TEST_PEL

#include <stdio.h>
#include <time.h>

#define TIME_VAL                        \
    struct timespec start = {0, 0};     \
    struct timespec end   = {0, 0};

#define TIME_START  clock_gettime(CLOCK_MONOTONIC, &start)
#define TIME_END    clock_gettime(CLOCK_MONOTONIC, &end)

static void put_hevc_qpel_uni_h_test(uint8_t *_dst,  ptrdiff_t _dststride,
                                     uint8_t *_src, ptrdiff_t _srcstride,
                                     int height, intptr_t mx, intptr_t my, int width)
{
    TIME_VAL;                                                   \
    int8_t _dst_rvv[_dststride * height];                       \
    memcpy(_dst_rvv, _dst, sizeof(_dst_rvv));                   \
                                                                \
    put_hevc_qpel_uni_h_8(_dst, _dststride, _src, _srcstride,   \
                          height, mx, my, width);               \
    TIME_START;                                                 \
    put_hevc_qpel_uni_h_rvv(_dst_rvv, _dststride, _src,         \
                            _srcstride, height, mx, my, width); \
    TIME_END;                                                   \
                                                                \
    if (1) {                                                    \
        static long total;                                      \
        static int count;                                       \
        total += (end.tv_sec - start.tv_sec) * 1000000000       \
                 + end.tv_nsec - start.tv_nsec;                 \
        count ++;                                               \
        if (count % 1000 == 0 || (H==32 && count % 300 == 0)) { \
            printf("IDCT_%d: %ldus\n", H, total / 1000);        \
        }                                                       \
    }                                                           \
                                                                \
    if (memcmp(_dst, _dst_rvv, sizeof(_dst_rvv))) {             \
        printf("put_hevc_qpel_uni_h_test failed\n");            \
        exit(1);                                                \
    }                                                           \
}

#endif

void ff_hevc_dsp_init_riscv(HEVCDSPContext *c, const int bit_depth)
{
#if HAVE_RVV
    int cpu_flags = av_get_cpu_flags();

    if (cpu_flags & AV_CPU_FLAG_RVI) {
        if (bit_depth == 8) {
#if RVV_TEST_IDCT
            c->idct[0] = ff_hevc_idct_4x4_test;
            c->idct[1] = ff_hevc_idct_8x8_test;
            c->idct[2] = ff_hevc_idct_16x16_test;
            c->idct[3] = ff_hevc_idct_32x32_test;
#else
            c->idct[0] = ff_hevc_idct_4x4_rvv;
            c->idct[1] = ff_hevc_idct_8x8_rvv;
            c->idct[2] = ff_hevc_idct_16x16_rvv;
            c->idct[3] = ff_hevc_idct_32x32_rvv;
#endif

#undef PEL_FUNC
#define PEL_FUNC(dst1, idx1, idx2, a)                            \
    for(int i = 0 ; i < 10 ; i++)                                    \
{                                                                \
    c->dst1[i][idx1][idx2] = a;                            \
}

#if RVV_TEST_PEL
            // PEL_FUNC(put_hevc_qpel_uni, 0, 0, put_hevc_pel_uni_pixels_test);
            PEL_FUNC(put_hevc_qpel_uni, 0, 1, put_hevc_qpel_uni_h_test);
            // PEL_FUNC(put_hevc_qpel_uni, 1, 0, put_hevc_qpel_uni_v);
            // PEL_FUNC(put_hevc_qpel_uni, 1, 1, put_hevc_qpel_uni_hv);
            // PEL_FUNC(put_hevc_qpel_uni_w, 0, 0, put_hevc_pel_uni_w_pixels);
            // PEL_FUNC(put_hevc_qpel_uni_w, 0, 1, put_hevc_qpel_uni_w_h);
            // PEL_FUNC(put_hevc_qpel_uni_w, 1, 0, put_hevc_qpel_uni_w_v);
            // PEL_FUNC(put_hevc_qpel_uni_w, 1, 1, put_hevc_qpel_uni_w_hv);
#else
            PEL_FUNC(put_hevc_qpel_uni, 0, 0, put_hevc_pel_uni_pixels_rvv);
            // PEL_FUNC(put_hevc_qpel_uni, 0, 1, put_hevc_qpel_uni_h_rvv);
            // PEL_FUNC(put_hevc_qpel_uni, 1, 0, put_hevc_qpel_uni_v);
            PEL_FUNC(put_hevc_qpel_uni, 1, 1, put_hevc_qpel_uni_hv_rvv);

            PEL_FUNC(put_hevc_epel_uni, 0, 0, put_hevc_pel_uni_pixels_rvv);
            // PEL_FUNC(put_hevc_epel_uni, 0, 1, put_hevc_epel_uni_h, depth);
            // PEL_FUNC(put_hevc_epel_uni, 1, 0, put_hevc_epel_uni_v, depth);
            // PEL_FUNC(put_hevc_epel_uni, 1, 1, put_hevc_epel_uni_hv, depth);
#endif


        }
    }
#endif
}
