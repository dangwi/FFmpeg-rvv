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

void ff_hevc_dsp_init_riscv(HEVCDSPContext *c, const int bit_depth)
{
#if HAVE_RVV
    int cpu_flags = av_get_cpu_flags();

    if (cpu_flags & AV_CPU_FLAG_RVI) {
        if (bit_depth == 8) {
            c->idct[0] = ff_hevc_idct_4x4_rvv;
            c->idct[1] = ff_hevc_idct_8x8_rvv;
            c->idct[2] = ff_hevc_idct_16x16_rvv;
            c->idct[3] = ff_hevc_idct_32x32_rvv;
        }
    }
#endif
}