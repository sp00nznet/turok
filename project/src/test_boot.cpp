// Turok - Console test harness for isolating crashes

#include "turok_config.h"
#include "turok_init.h"

#include <rex/runtime.h>
#include <rex/runtime/guest/memory.h>
#include <rex/logging.h>
#include <rex/cvar.h>

#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>

static uint64_t g_guest_base = 0;

static LONG CALLBACK NullPageHandler(EXCEPTION_POINTERS* ep) {
    auto* ctx = ep->ContextRecord;
    auto* rec = ep->ExceptionRecord;
    if (rec->ExceptionCode != STATUS_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;
    if (rec->ExceptionInformation[0] != 0) return EXCEPTION_CONTINUE_SEARCH;
    uint64_t addr = rec->ExceptionInformation[1];
    // Use cached base (RSI may be clobbered by memcpy/rep movsb etc.)
    uint64_t base = g_guest_base;
    if (base == 0) return EXCEPTION_CONTINUE_SEARCH;
    if (addr >= base && addr < base + 0x10000) {
        uint8_t* rip = (uint8_t*)ctx->Rip;
        int rex = 0, oplen = 0;
        uint8_t op = rip[0];
        if ((op & 0xF0) == 0x40) { rex = op; op = rip[1]; oplen = 1; }
        if (op == 0x8B) {
            uint8_t modrm = rip[oplen + 1];
            int reg = (modrm >> 3) & 7;
            if (rex & 0x04) reg += 8;
            int mod = (modrm >> 6) & 3;
            int rm = modrm & 7;
            int insn_len = oplen + 2;
            if (rm == 4 && mod != 3) insn_len++;
            if (mod == 0 && rm == 5) insn_len += 4;
            else if (mod == 1) insn_len += 1;
            else if (mod == 2) insn_len += 4;
            uint64_t* regs[] = { &ctx->Rax, &ctx->Rcx, &ctx->Rdx, &ctx->Rbx,
                                  &ctx->Rsp, &ctx->Rbp, &ctx->Rsi, &ctx->Rdi,
                                  &ctx->R8, &ctx->R9, &ctx->R10, &ctx->R11,
                                  &ctx->R12, &ctx->R13, &ctx->R14, &ctx->R15 };
            if (reg < 16) *regs[reg] = 0;
            ctx->Rip += insn_len;
            static int _nullpage_count = 0;
            if (++_nullpage_count <= 20) {
                fprintf(stderr, "[NULLPAGE] Read from guest addr 0x%04llX, zeroed reg %d\n",
                        addr - base, reg);
                fflush(stderr);
            }
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG CALLBACK GuestPageCommitHandler(EXCEPTION_POINTERS* ep) {
    auto* rec = ep->ExceptionRecord;
    if (rec->ExceptionCode != STATUS_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;
    uint64_t addr = rec->ExceptionInformation[1];
    uint64_t guest_base = g_guest_base;
    if (guest_base == 0) return EXCEPTION_CONTINUE_SEARCH;
    if (addr < guest_base || addr >= guest_base + 0x100000000ULL) return EXCEPTION_CONTINUE_SEARCH;
    void* page = (void*)(addr & ~0xFFFULL);
    void* result = VirtualAlloc(page, 0x1000, MEM_COMMIT, PAGE_READWRITE);
    if (result) return EXCEPTION_CONTINUE_EXECUTION;
    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
    auto* ctx = ep->ContextRecord;
    auto* rec = ep->ExceptionRecord;
    fprintf(stderr, "\n========== CRASH ==========\n");
    fprintf(stderr, "Exception: 0x%08lX at RIP=0x%016llX\n",
            rec->ExceptionCode, (unsigned long long)ctx->Rip);
    if (rec->ExceptionCode == STATUS_ACCESS_VIOLATION) {
        fprintf(stderr, "Access address: 0x%016llX (%s)\n",
                (unsigned long long)rec->ExceptionInformation[1],
                rec->ExceptionInformation[0] == 0 ? "READ" : "WRITE");
    }
    fprintf(stderr, "RAX=0x%016llX RBX=0x%016llX RCX=0x%016llX RDX=0x%016llX\n",
            ctx->Rax, ctx->Rbx, ctx->Rcx, ctx->Rdx);
    fprintf(stderr, "RSI=0x%016llX RDI=0x%016llX RSP=0x%016llX RBP=0x%016llX\n",
            ctx->Rsi, ctx->Rdi, ctx->Rsp, ctx->Rbp);
    fprintf(stderr, "R8 =0x%016llX R9 =0x%016llX R10=0x%016llX R11=0x%016llX\n",
            ctx->R8, ctx->R9, ctx->R10, ctx->R11);
    fprintf(stderr, "R12=0x%016llX R13=0x%016llX R14=0x%016llX R15=0x%016llX\n",
            ctx->R12, ctx->R13, ctx->R14, ctx->R15);
    uint64_t* sp = (uint64_t*)ctx->Rsp;
    fprintf(stderr, "\nStack (RSP):\n");
    for (int i = 0; i < 16; i++) {
        __try {
            fprintf(stderr, "  [RSP+%02X] = 0x%016llX\n", i*8, sp[i]);
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            fprintf(stderr, "  [RSP+%02X] = <unreadable>\n", i*8);
        }
    }
    fprintf(stderr, "===========================\n");
    fflush(stderr);
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    AddVectoredExceptionHandler(1, GuestPageCommitHandler);
    AddVectoredExceptionHandler(1, NullPageHandler);
    SetUnhandledExceptionFilter(CrashHandler);
#endif
    fprintf(stderr, "[test] Starting Turok boot test...\n");
    fflush(stderr);

    rex::cvar::Init(argc, argv);

    auto log_config = rex::BuildLogConfig(nullptr, "trace", {});
    rex::InitLogging(log_config);

    std::filesystem::path game_dir;
    if (argc > 1) {
        game_dir = argv[1];
    } else {
        game_dir = "game_files";
    }

    fprintf(stderr, "[test] Game dir: %s\n", game_dir.string().c_str());
    fflush(stderr);

    auto runtime = std::make_unique<rex::Runtime>(game_dir);

    auto status = runtime->Setup(
        static_cast<uint32_t>(PPC_CODE_BASE),
        static_cast<uint32_t>(PPC_CODE_SIZE),
        static_cast<uint32_t>(PPC_IMAGE_BASE),
        static_cast<uint32_t>(PPC_IMAGE_SIZE),
        PPCFuncMappings);

    fprintf(stderr, "[test] Setup returned: 0x%08X\n", status);
    fflush(stderr);

    if (status != 0) {
        fprintf(stderr, "[test] Setup FAILED\n");
        return 1;
    }

    // Cache guest base for exception handlers (RSI may not always hold base)
    g_guest_base = (uint64_t)runtime->memory()->virtual_membase();

    status = runtime->LoadXexImage("game:\\default.xex");
    fprintf(stderr, "[test] LoadXexImage returned: 0x%08X\n", status);
    fflush(stderr);

    if (status != 0) {
        fprintf(stderr, "[test] LoadXexImage FAILED\n");
        return 1;
    }

    fprintf(stderr, "[test] Boot test PASSED - XEX loaded successfully!\n");
    fflush(stderr);

    auto thread = runtime->LaunchModule();
    if (thread) {
        fprintf(stderr, "[test] Module launched, waiting...\n");
        fflush(stderr);
        thread->Wait(0, 0, 0, nullptr);
    }

    fprintf(stderr, "[test] Done.\n");
    return 0;
}
