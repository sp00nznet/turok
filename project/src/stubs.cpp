// Turok (Xbox 360) - Kernel stubs
// Game-specific Xbox 360 API overrides not yet in ReXGlue SDK

#include "turok_config.h"
#include <rex/runtime/guest/context.h>
#include <rex/runtime/guest/memory.h>
#include <rex/kernel/kernel_state.h>
#include <cstring>

using namespace rex::runtime::guest;

#define TUROK_STUB_RETURN(name, val) \
    extern "C" PPC_FUNC(name) { (void)base; ctx.r3.u64 = (val); }

#define TUROK_STUB(name) TUROK_STUB_RETURN(name, 0)

// XAM UI stubs (game shows achievement/marketplace dialogs)
TUROK_STUB(__imp__XamShowGamerCardUIForXUID)
TUROK_STUB(__imp__XamShowAchievementsUI)
TUROK_STUB(__imp__XamShowMarketplaceUI)
TUROK_STUB_RETURN(__imp__XamUserCreateStatsEnumerator, 1)  // fail = no stats
TUROK_STUB(__imp__XamVoiceSubmitPacket)

// Content license bypass - return full license mask
// Override the license check wrapper to always grant full content
extern "C" PPC_FUNC(__imp__XamContentGetLicenseMask) {
    uint32_t mask_ptr = ctx.r3.u32;
    if (mask_ptr)
        PPC_STORE_U32(mask_ptr, 0xFFFFFFFF);
    ctx.r3.u32 = 0;  // ERROR_SUCCESS
}

// Kernel memory allocation
TUROK_STUB(__imp__ExAllocatePoolWithTag)

// ObReferenceObject
TUROK_STUB(__imp__ObReferenceObject)

// XNet networking stubs (Turok has Xbox Live multiplayer)
TUROK_STUB(__imp__XNetStartup)
TUROK_STUB(__imp__XNetCleanup)
TUROK_STUB(__imp__XNetGetTitleXnAddr)
TUROK_STUB(__imp__XNetServerToInAddr)
TUROK_STUB(__imp__XNetUnregisterInAddr)
TUROK_STUB(__imp__XNetConnect)
TUROK_STUB(__imp__XNetGetConnectStatus)
TUROK_STUB(__imp__XNetQosLookup)
TUROK_STUB(__imp__XNetQosRelease)
TUROK_STUB(__imp__XNetQosServiceLookup)
TUROK_STUB(__imp__XNetCreateKey)
TUROK_STUB(__imp__XNetRegisterKey)
TUROK_STUB(__imp__XNetUnregisterKey)
TUROK_STUB(__imp__XNetXnAddrToInAddr)
TUROK_STUB(__imp__XNetInAddrToXnAddr)
TUROK_STUB(__imp__XNetGetEthernetLinkStatus)

// Winsock stubs
TUROK_STUB(__imp__WSAStartup)
TUROK_STUB(__imp__WSACleanup)
TUROK_STUB(__imp__WSAGetLastError)
TUROK_STUB_RETURN(__imp__socket, 0xFFFFFFFF)  // INVALID_SOCKET
TUROK_STUB(__imp__closesocket)
TUROK_STUB(__imp__bind)
TUROK_STUB(__imp__listen)
TUROK_STUB_RETURN(__imp__accept, 0xFFFFFFFF)
TUROK_STUB_RETURN(__imp__connect, 0xFFFFFFFF)
TUROK_STUB_RETURN(__imp__send, 0xFFFFFFFF)
TUROK_STUB_RETURN(__imp__recv, 0xFFFFFFFF)
TUROK_STUB_RETURN(__imp__sendto, 0xFFFFFFFF)
TUROK_STUB_RETURN(__imp__recvfrom, 0xFFFFFFFF)
TUROK_STUB(__imp__setsockopt)
TUROK_STUB(__imp__ioctlsocket)
TUROK_STUB(__imp__select)
TUROK_STUB(__imp__WSAGetOverlappedResult)

// Xbox Live session stubs
TUROK_STUB(__imp__XSessionCreate)
TUROK_STUB(__imp__XSessionDelete)
TUROK_STUB(__imp__XSessionStart)
TUROK_STUB(__imp__XSessionEnd)
TUROK_STUB(__imp__XSessionModify)
TUROK_STUB(__imp__XSessionJoinRemote)
TUROK_STUB(__imp__XSessionLeaveRemote)
TUROK_STUB(__imp__XSessionGetDetails)
TUROK_STUB(__imp__XSessionSearchEx)
TUROK_STUB(__imp__XSessionArbitrationRegister)
TUROK_STUB(__imp__XSessionWriteStats)
TUROK_STUB(__imp__XSessionFlushStats)

// User sign-in (single player - always signed in as player 1)
extern "C" PPC_FUNC(__imp__XamUserGetSigninState) {
    uint32_t user_index = ctx.r3.u32;
    ctx.r3.u64 = (user_index == 0) ? 1 : 0;
}

extern "C" PPC_FUNC(__imp__XamUserGetSigninInfo) {
    uint32_t user_index = ctx.r3.u32;
    uint32_t info_ptr = ctx.r5.u32;

    if (!info_ptr) {
        ctx.r3.u64 = 0x80070057;
        return;
    }

    std::memset(base + info_ptr, 0, 40);

    if (user_index != 0) {
        ctx.r3.u64 = 0x80070490;
        return;
    }

    PPC_STORE_U64(info_ptr + 0, 0xB13EBABEBABE0001ULL);
    PPC_STORE_U32(info_ptr + 12, 1);
    std::memcpy(base + info_ptr + 24, "Player 1", 9);
    ctx.r3.u64 = 0;
}

extern "C" PPC_FUNC(__imp__XamUserGetXUID) {
    uint32_t user_index = ctx.r3.u32;
    uint32_t xuid_ptr = ctx.r5.u32;

    if (!xuid_ptr) { ctx.r3.u64 = 0x80070057; return; }
    if (user_index != 0) {
        PPC_STORE_U64(xuid_ptr, 0);
        ctx.r3.u64 = 0x80070490;
        return;
    }
    PPC_STORE_U64(xuid_ptr, 0xB13EBABEBABE0001ULL);
    ctx.r3.u64 = 0;
}

extern "C" PPC_FUNC(__imp__XamUserGetName) {
    uint32_t user_index = ctx.r3.u32;
    uint32_t buffer_ptr = ctx.r4.u32;
    uint32_t buffer_len = ctx.r5.u32;

    if (user_index != 0) { ctx.r3.u64 = 0x80070490; return; }
    if (buffer_ptr && buffer_len > 0) {
        const char* name = "Player 1";
        uint32_t copy_len = std::min(buffer_len, (uint32_t)9);
        std::memcpy(base + buffer_ptr, name, copy_len);
        *(base + buffer_ptr + copy_len - 1) = 0;
    }
    ctx.r3.u64 = 0;
}

extern "C" PPC_FUNC(__imp__XamUserCheckPrivilege) {
    uint32_t out_ptr = ctx.r5.u32;
    if (out_ptr) PPC_STORE_U32(out_ptr, 0);
    ctx.r3.u64 = 0;
}

extern "C" PPC_FUNC(__imp__XamUserGetMembershipTier) {
    ctx.r3.u64 = 6;  // Gold
}

extern "C" PPC_FUNC(__imp__XamShowSigninUI) {
    (void)base;
    auto* ks = rex::kernel::kernel_state();
    if (ks) {
        ks->BroadcastNotification(0x0000000A, 1);  // XN_SYS_SIGNINCHANGED
        ks->BroadcastNotification(0x00000009, 0);   // XN_SYS_UI off
    }
    ctx.r3.u64 = 0;
}

extern "C" PPC_FUNC(__imp__XamUserIsOnlineEnabled) {
    (void)base;
    ctx.r3.u64 = 1;
}

// Bink video stubs (if needed - Turok uses .bik movies)
TUROK_STUB(__imp__BinkOpen)
TUROK_STUB(__imp__BinkClose)
TUROK_STUB(__imp__BinkDoFrame)
TUROK_STUB(__imp__BinkNextFrame)
TUROK_STUB(__imp__BinkWait)
TUROK_STUB(__imp__BinkCopyToBuffer)
TUROK_STUB(__imp__BinkSetSoundSystem)
TUROK_STUB(__imp__BinkSetSoundOnOff)

// NetDll wrappers (rexglue uses NetDll_ prefix for winsock/xnet)
TUROK_STUB(__imp__NetDll_XNetCreateKey)
TUROK_STUB(__imp__NetDll_XNetRegisterKey)
TUROK_STUB(__imp__NetDll_XNetUnregisterKey)
TUROK_STUB(__imp__NetDll_XNetServerToInAddr)

// Additional XAM UI stubs
TUROK_STUB(__imp__XamShowFriendRequestUI)
TUROK_STUB(__imp__XamShowFriendsUI)
TUROK_STUB(__imp__XamShowGameInviteUI)
TUROK_STUB(__imp__XamShowMessageComposeUI)
TUROK_STUB(__imp__XamShowMessagesUI)
TUROK_STUB(__imp__XamShowPlayerReviewUI)
TUROK_STUB(__imp__XamShowPlayersUI)
TUROK_STUB(__imp__XamShowQuickChatUI)
TUROK_STUB(__imp__XamShowSigninUIp)
TUROK_STUB(__imp__XamShowVoiceMailUI)
TUROK_STUB(__imp__XamContentInstall)
TUROK_STUB(__imp__XamContentLaunchImage)

// GPU video stubs
TUROK_STUB(__imp__VdQuerySystemCommandBuffer)
TUROK_STUB(__imp__VdSetSystemCommandBuffer)

// More NetDll wrappers
TUROK_STUB(__imp__NetDll_WSACancelOverlappedIO)
TUROK_STUB(__imp__NetDll_WSAEventSelect)
TUROK_STUB(__imp__NetDll_WSAGetOverlappedResult)
TUROK_STUB(__imp__NetDll_WSARecv)
TUROK_STUB(__imp__NetDll_WSASend)
TUROK_STUB(__imp__NetDll_XNetConnect)
TUROK_STUB(__imp__NetDll_XNetGetBroadcastVersionStatus)
TUROK_STUB(__imp__NetDll_XNetGetConnectStatus)
TUROK_STUB(__imp__NetDll_XNetInAddrToServer)
TUROK_STUB(__imp__NetDll_XNetQosGetListenStats)
TUROK_STUB(__imp__NetDll_XNetQosLookup)
TUROK_STUB(__imp__NetDll_XNetSetOpt)
TUROK_STUB(__imp__NetDll_XNetTsAddrToInAddr)
TUROK_STUB(__imp__NetDll_XNetUnregisterInAddr)
TUROK_STUB(__imp__NetDll_getpeername)
TUROK_STUB(__imp__NetDll_getsockname)
TUROK_STUB(__imp__NetDll_getsockopt)

// XNet logon stubs
TUROK_STUB(__imp__XNetLogonGetMachineID)
TUROK_STUB_RETURN(__imp__XNetLogonGetTitleID, 0x545552B4)  // Turok title ID

// Additional XAM stubs
TUROK_STUB_RETURN(__imp__XamUserGetMembershipTierFromXUID, 6)  // Gold

// Audio query
TUROK_STUB(__imp__XAudioQueryDriverPerformance)

// XAM user stubs
TUROK_STUB(__imp__XamUserGetOnlineCountryFromXUID)

// XeCrypt stubs (DRM/signature verification - stub out)
TUROK_STUB(__imp__XeCryptBnQwBeSigVerify)
TUROK_STUB(__imp__XeCryptRotSumSha)
TUROK_STUB(__imp__XeKeysGetKey)

// Misc
TUROK_STUB(__imp__Refresh)
TUROK_STUB(__imp__XexUnloadImageAndExitThread)
