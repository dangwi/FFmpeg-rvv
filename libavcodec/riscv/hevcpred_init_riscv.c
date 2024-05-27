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
#include "libavutil/mem_internal.h"
#include "libavcodec/riscv/hevcpred_rvv.h"
#include "libavcodec/riscv/hevcdsp_rvv.h"

void ff_hevc_pred_init_riscv(HEVCPredContext *hpc, int bit_depth)
{
#if HAVE_RVV
    int cpu_flags = av_get_cpu_flags();

    if (cpu_flags & AV_CPU_FLAG_RVI) {
        if (bit_depth == 8) {
            hpc->intra_pred[0]   = intra_pred_2_rvv;
            hpc->intra_pred[1]   = intra_pred_3_rvv;
            hpc->intra_pred[2]   = intra_pred_4_rvv;
            hpc->intra_pred[3]   = intra_pred_5_rvv;
        } 
    }

#endif
}
