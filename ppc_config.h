#pragma once
#ifndef PPC_CONFIG_H_INCLUDED
#define PPC_CONFIG_H_INCLUDED

#include <cstdio>
#include <cstdlib>
#include <cstdint>

// Turok (Xbox 360) - PE image layout
// .text section: 0x823F0000 - 0x833FAA00 (16.7 MB of code)
// BINK section:  0x833FAA00 - 0x8340D8F8
#define PPC_IMAGE_BASE 0x82000000ull
#define PPC_IMAGE_SIZE 0x1550000ull
#define PPC_CODE_BASE  0x823F0000ull
#define PPC_CODE_SIZE  0x101D8F8ull

// Override __builtin_trap() for switch case defaults
#ifdef __builtin_trap
#undef __builtin_trap
#endif
#define __builtin_trap() do { \
    static int _tc = 0; \
    if (++_tc <= 10) \
        fprintf(stderr, "[WARN] Switch case out of range (LR=0x%08X)\n", \
                (uint32_t)ctx.lr); \
} while(0)

// Safe indirect call with NULL check, range validation, and import thunk resolution
#define PPC_CALL_INDIRECT_FUNC(x) do { \
    uint32_t _target = (x); \
    if (_target == 0) { \
        static int _nc = 0; \
        if (++_nc <= 5) \
            fprintf(stderr, "[WARN] Indirect call to NULL\n"); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    if (_target < PPC_CODE_BASE || _target >= PPC_CODE_BASE + PPC_CODE_SIZE) { \
        if (_target >= PPC_IMAGE_BASE && _target < PPC_CODE_BASE) { \
            uint32_t insn0 = PPC_LOAD_U32(_target); \
            uint32_t insn1 = PPC_LOAD_U32(_target + 4); \
            uint16_t hi = insn0 & 0xFFFF; \
            int16_t lo = (int16_t)(insn1 & 0xFFFF); \
            uint32_t iat_addr = ((uint32_t)hi << 16) + lo; \
            uint32_t resolved = PPC_LOAD_U32(iat_addr); \
            if (resolved >= PPC_CODE_BASE && \
                resolved < PPC_CODE_BASE + PPC_CODE_SIZE) { \
                PPCFunc* _fn = PPC_LOOKUP_FUNC(base, resolved); \
                if (_fn) { _fn(ctx, base); break; } \
            } \
            ctx.r3.u32 = 0; \
            break; \
        } \
        static int _oor = 0; \
        if (++_oor <= 20) \
            fprintf(stderr, "[WARN] Indirect call to 0x%08X outside code range\n", _target); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    PPCFunc* _fn = PPC_LOOKUP_FUNC(base, _target); \
    if (!_fn) { \
        static int _nf = 0; \
        if (++_nf <= 50) \
            fprintf(stderr, "[WARN] Indirect call to 0x%08X: no recompiled function\n", _target); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    _fn(ctx, base); \
} while(0)

#ifdef PPC_INCLUDE_DETAIL
#include "ppc_detail.h"
#endif

#endif
