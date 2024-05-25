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

#ifndef AVCODEC_RISCV_HEVCDSP_RVV_H
#define AVCODEC_RISCV_HEVCDSP_RVV_H

#include "libavcodec/hevcdsp.h"

void ff_hevc_idct_4x4_rvv(int16_t *coeffs, int col_limit);
void ff_hevc_idct_8x8_rvv(int16_t *coeffs, int col_limit);
void ff_hevc_idct_16x16_rvv(int16_t *coeffs, int col_limit);
void ff_hevc_idct_32x32_rvv(int16_t *coeffs, int col_limit);

#endif  // #ifndef AVCODEC_RISCV_HEVCDSP_RVV_H