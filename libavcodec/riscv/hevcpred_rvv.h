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

#ifndef AVCODEC_RISCV_HEVCPRED_RVV_H
#define AVCODEC_RISCV_HEVCPRED_RVV_H

#include "libavcodec/hevcdec.h"

void intra_pred_2_rvv(HEVCContext *s, int x0, int y0, int c_idx);
void intra_pred_3_rvv(HEVCContext *s, int x0, int y0, int c_idx);
void intra_pred_4_rvv(HEVCContext *s, int x0, int y0, int c_idx);
void intra_pred_5_rvv(HEVCContext *s, int x0, int y0, int c_idx);


#endif  // #ifndef AVCODEC_RISCV_HEVCPRED_RVV_H