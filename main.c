//===-- main.c - TEK Injector source code ---------------------------------===//
//
// Copyright (c) 2024 Nuclearist
// Part of TEK Injector, under the MIT License
// See https://github.com/Nuclearistt/tek-injector/blob/main/LICENSE for
//    license information.
// SPDX-License-Identifier: MIT
//
//===----------------------------------------------------------------------===//
//
// Since PE image of this program is intended to be copied to another process
//    the goal is to place everything it needs within the image, in particular
//    have no DLL dependencies other than ntdll.dll and kernel32.dll which
//    reside at the same address in all process, and minimize the use of CRT as
//    most of its functions create or use context in heap outside of image.
//    There are dependencies on shell32.dll and user32.dll, but they're only
//    used on host side so don't cause any issues.
//
//===----------------------------------------------------------------------===//

#include <stdint.h>
// Exclude unnecessary APIs
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#pragma clang diagnostic push
// Clang really doesn't like Win32 API
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wignored-pragma-intrinsic"
#pragma clang diagnostic ignored "-Wmicrosoft-anon-tag"
#pragma clang diagnostic ignored "-Wnonportable-include-path"
#pragma clang diagnostic ignored "-Wpragma-pack"
#include <Windows.h>
#include <shellapi.h>
#pragma clang diagnostic pop

//===- TEK Injector error codes -------------------------------------------===//

enum error_code {
  EC_SUCCESS = 0,
  EC_MIN = -8,
  // Injector error codes
  IEC_VIRTUAL_PROTECT_FAILED,
  IEC_CREATE_PROCESS_FAILED,
  IEC_VIRTUAL_ALLOC_FAILED,
  IEC_WRITE_PROC_MEM_FAILED,
  IEC_SET_THREAD_CTX_FAILED,
  // Game error codes
  GEC_STEAM_API_INIT_FAILED,
  GEC_EOS_AUTH_LOGIN_FAILED
};
const wchar_t *const error_code_strings[] = {
    L"Failed to change PE section protection",
    L"Failed to create game process",
    L"Failed to allocate memory in game process",
    L"Failed to write image to game process",
    L"Failed to modify game thread context",
    L"Steam API initialization failed",
    L"Epic Games authentication failed"};

//===- Game-side functions (called inside game process) -------------------===//

//===--- vmn_stub methods (implemented in vm-stubs.s) ---------------------===//

extern void vm0_stub();
extern void vm1_stub();
extern void vm2_stub();
extern void vm3_stub();
extern void vm4_stub();
extern void vm5_stub();
extern void vm6_stub();
extern void vm7_stub();
extern void vm8_stub();
extern void vm9_stub();
extern void vm10_stub();
extern void vm11_stub();
extern void vm12_stub();
extern void vm13_stub();
extern void vm14_stub();
extern void vm15_stub();
extern void vm16_stub();
extern void vm17_stub();
extern void vm18_stub();
extern void vm19_stub();
extern void vm20_stub();
extern void vm21_stub();
extern void vm22_stub();
extern void vm23_stub();
extern void vm24_stub();
extern void vm25_stub();
extern void vm26_stub();
extern void vm27_stub();
extern void vm28_stub();
extern void vm29_stub();
extern void vm30_stub();
extern void vm31_stub();
extern void vm32_stub();
extern void vm33_stub();
extern void vm34_stub();
extern void vm35_stub();
extern void vm36_stub();
extern void vm37_stub();

//===--- Steam API method wrappers ----------------------------------------===//

struct cpp_interface_wrapper {
  void *const *vtable;      // Pointer to virtual method table
  void *original_interface; // Read by vmn_stub methods
};
uint64_t steam_id; // Steam ID of current user

//===----- ISteamApps -----------------------------------------------------===//

void *isteam_apps_vtable[29];
struct cpp_interface_wrapper isteam_apps_wrapper;

bool return_true() { return true; }
int asa_get_dlc_count() { return 2; }
bool asa_get_dlc_data_by_index(struct cpp_interface_wrapper *_, int iDLC,
                               uint32_t *pAppID, bool *pbAvailable,
                               char *pchName) {
  *pbAvailable = true;
  switch (iDLC) {
  case 0:
    *pAppID = 2849450;
    memcpy(pchName, "ARK: Scorched Earth Ascended",
           sizeof("ARK: Scorched Earth Ascended"));
    return true;
  case 1:
    *pAppID = 2881150;
    memcpy(pchName, "ARK: Bob's Tall Tales", sizeof("ARK: Bob's Tall Tales"));
    return true;
  default:
    return false;
  }
}
uint64_t *get_app_owner(struct cpp_interface_wrapper *_, uint64_t *steamId) {
  *steamId = steam_id;
  return steamId;
}

//===----- ISteamMatchmakingServers ---------------------------------------===//

struct MatchMakingKeyValuePair_t {
  char m_szKey[256];
  char m_szValue[256];
};

struct server_rules_callback_wrapper {
  void *const *vtable; // Pointer to virtual method table
  void *original_callback;
  int query_handle; // Handle returned by ServerRules()
};

void *server_rules_callback_wrapper_vtable[3];
void *isteam_matchmaking_servers_vtable[17];
struct cpp_interface_wrapper isteam_matchmaking_servers_wrapper;

void rules_responded(struct server_rules_callback_wrapper *wrapper,
                     const char *pchRule, const char *pchValue) {
  // If server reports that it has BattlEye or doesn't have TEK Wrapper, cancel
  //    the query
  if ((!strcmp(pchRule, "SERVERUSESBATTLEYE_b") && pchValue[0] != 'f') ||
      (!strcmp(pchRule, "SEARCHKEYWORDS_s") &&
       strncmp(pchValue, "TEKWrapper", 10))) {
    // Call CancelServerQuery on the original ISteamMatchmakingServers
    ((void (*)(void *, int))((*(
        (void ***)isteam_matchmaking_servers_wrapper.original_interface))[16]))(
        isteam_matchmaking_servers_wrapper.original_interface,
        wrapper->query_handle);
    // Call RulesFailedToRespond on original callback
    ((void (*)(void *))((*((void ***)wrapper->original_callback))[1]))(
        wrapper->original_callback);
    // Detach original callback so game won't receive any calls anymore
    wrapper->original_callback = NULL;
  } else if (wrapper->original_callback)
    ((void (*)(void *, const char *, const char *))(
        (*((void ***)wrapper->original_callback))[0]))(
        wrapper->original_callback, pchRule, pchValue);
}
void rules_failed_to_respond(struct server_rules_callback_wrapper *wrapper) {
  // Call RulesFailedToRespond on original callback
  if (wrapper->original_callback)
    ((void (*)(void *))((*((void ***)wrapper->original_callback))[1]))(
        wrapper->original_callback);
  // Free the memory used for wrapper object
  HeapFree(GetProcessHeap(), 0, wrapper);
}
void rules_refresh_complete(struct server_rules_callback_wrapper *wrapper) {
  // Call RulesRefreshComplete on original callback
  if (wrapper->original_callback)
    ((void (*)(void *))((*((void ***)wrapper->original_callback))[2]))(
        wrapper->original_callback);
  // Free the memory used for wrapper object
  HeapFree(GetProcessHeap(), 0, wrapper);
}

void *
request_internet_server_list(struct cpp_interface_wrapper *wrapper,
                             uint32_t iApp,
                             struct MatchMakingKeyValuePair_t **ppchFilters,
                             uint32_t nFilters, void *pRequestServersResponse) {
  char *cur = (*ppchFilters)[nFilters - 1].m_szValue;
  // Make cur point to the terminating null
  while (*cur)
    ++cur;
  memcpy(cur, ",SERVERUSESBATTLEYE_b:false,TEKWrapper:1",
         sizeof(",SERVERUSESBATTLEYE_b:false,TEKWrapper:1"));
  return ((void *(*)(void *, uint32_t, struct MatchMakingKeyValuePair_t **,
                     uint32_t,
                     void *))((*((void ***)wrapper->original_interface))[0]))(
      wrapper->original_interface, iApp, ppchFilters, nFilters,
      pRequestServersResponse);
}
int server_rules(struct cpp_interface_wrapper *wrapper, uint32_t unIP,
                 uint16_t usPort, void *pRequestServersResponse) {
  // Create a callback wrapper for this query
  struct server_rules_callback_wrapper *const callbackWrapper = HeapAlloc(
      GetProcessHeap(), 0, sizeof(struct server_rules_callback_wrapper));
  callbackWrapper->vtable = server_rules_callback_wrapper_vtable;
  callbackWrapper->original_callback = pRequestServersResponse;
  int query = ((int (*)(void *, uint32_t, uint16_t, void *))(
      (*((void ***)wrapper->original_interface))[15]))(
      wrapper->original_interface, unIP, usPort, callbackWrapper);
  callbackWrapper->query_handle = query;
  return query;
}

//===----- ISteamUGC ------------------------------------------------------===//

void *isteam_ugc_vtable[31];
struct cpp_interface_wrapper isteam_ugc_wrapper;

uint32_t get_num_subscribed_items() {
  // Search the Mods directory in game root
  WIN32_FIND_DATAW findData;
  const HANDLE find = FindFirstFileExW(
      L"..\\..\\..\\Mods\\*", FindExInfoBasic, &findData,
      FindExSearchLimitToDirectories, NULL, FIND_FIRST_EX_LARGE_FETCH);
  if (find == INVALID_HANDLE_VALUE)
    return 0;
  DWORD numInstalledMods = 0;
  FindNextFileW(find, &findData); // Skip .. entry
  while (FindNextFileW(find, &findData)) {
    // Only cound directories with IDs as names
    bool validNumber = true;
    for (const WCHAR *i = findData.cFileName; *i; ++i)
      if (*i < L'0' || *i > L'9') {
        validNumber = false;
        break;
      }
    if (validNumber)
      numInstalledMods++;
  }
  FindClose(find);
  return numInstalledMods;
}
uint32_t get_subscribed_items(struct cpp_interface_wrapper *_,
                              uint64_t *pvecPublishedFileID,
                              uint32_t cMaxEntries) {
  // Search the Mods directory in game root
  WIN32_FIND_DATAW findData;
  const HANDLE find = FindFirstFileExW(
      L"..\\..\\..\\Mods\\*", FindExInfoBasic, &findData,
      FindExSearchLimitToDirectories, NULL, FIND_FIRST_EX_LARGE_FETCH);
  if (find == INVALID_HANDLE_VALUE)
    return 0;
  FindNextFileW(find, &findData); // Skip .. entry
  while (FindNextFileW(find, &findData)) {
    uint64_t id = 0;
    for (const WCHAR *i = findData.cFileName; *i; ++i) {
      const uint64_t digit = (uint64_t)*i - L'0';
      if (digit > 9) {
        id = 0;
        break;
      }
      id = id * 10 + digit;
    }
    if (id)
      *pvecPublishedFileID++ = id;
  }
  FindClose(find);
  return cMaxEntries;
}
bool get_item_install_info(struct cpp_interface_wrapper *_,
                           uint64_t nPublishedFileID, uint64_t *punSizeOnDisk,
                           char *pchFolder, uint32_t cchFolderSize,
                           bool *pbLegacyItem) {
  *punSizeOnDisk = 0; // This is not used by the game so not worth computing
  *pbLegacyItem = false;
  WCHAR pathBuffer[MAX_PATH] = L"..\\..\\..\\Mods\\";
  WCHAR numberBuffer[20];
  numberBuffer[19] = L'\0';
  WCHAR *const numberBufferEnd = numberBuffer + 20;
  WCHAR *i = numberBufferEnd - 1;
  while (nPublishedFileID) {
    *--i = L'0' + (nPublishedFileID % 10);
    nPublishedFileID /= 10;
  }
  memcpy(pathBuffer + 14, i, (numberBufferEnd - i) * sizeof(WCHAR));
  if (GetFileAttributesW(pathBuffer) == INVALID_FILE_ATTRIBUTES) {
    // Mod directory doesn't exist
    *pchFolder = '\0';
    return false;
  } else {
    // Why not just use GetFileAttributesA? It only supports ASCII and
    //    non-latin characters in path break it.
    WideCharToMultiByte(CP_UTF8, 0, pathBuffer, -1, pchFolder, cchFolderSize,
                        NULL, NULL);
    return true;
  }
}

//===----- ISteamUtils ----------------------------------------------------===//

void *isteam_utils_vtable[38];
struct cpp_interface_wrapper isteam_utils_wrapper;

uint32_t asa_get_app_id() { return 2399830; }
uint32_t ase_get_app_id() { return 346110; }

//===--- steam_api64.dll function wrappers --------------------------------===//

// Sets up ISteamApps wrappers in ASE
void *(*SteamApps)();
void *SteamApps_wrapper() {
  if (!isteam_apps_wrapper.original_interface) {
    isteam_apps_vtable[0] = return_true;    // BIsSubscribed
    isteam_apps_vtable[1] = vm1_stub;       // BIsLowViolence
    isteam_apps_vtable[2] = vm2_stub;       // BIsCybercafe
    isteam_apps_vtable[3] = vm3_stub;       // BIsVACBanned
    isteam_apps_vtable[4] = vm4_stub;       // GetCurrentGameLanguage
    isteam_apps_vtable[5] = vm5_stub;       // GetAvailableGameLanguages
    isteam_apps_vtable[6] = return_true;    // BIsSubscribedApp
    isteam_apps_vtable[7] = return_true;    // BIsDlcInstalled
    isteam_apps_vtable[8] = vm8_stub;       // GetEarliestPurchaseUnixTime
    isteam_apps_vtable[9] = vm9_stub;       // BIsSubscribedFromFreeWeekend
    isteam_apps_vtable[10] = vm10_stub;     // GetDLCCount
    isteam_apps_vtable[11] = vm11_stub;     // BGetDLCDataByIndex
    isteam_apps_vtable[12] = vm12_stub;     // InstallDLC
    isteam_apps_vtable[13] = vm13_stub;     // UninstallDLC
    isteam_apps_vtable[14] = vm14_stub;     // RequestAppProofOfPurchaseKey
    isteam_apps_vtable[15] = vm15_stub;     // GetCurrentBetaName
    isteam_apps_vtable[16] = vm16_stub;     // MarkContentCorrupt
    isteam_apps_vtable[17] = vm17_stub;     // GetInstalledDepots
    isteam_apps_vtable[18] = vm18_stub;     // GetAppInstallDir
    isteam_apps_vtable[19] = vm19_stub;     // BIsAppInstalled
    isteam_apps_vtable[20] = get_app_owner; // GetAppOwner
    isteam_apps_vtable[21] = vm21_stub;     // GetLaunchQueryParam
    isteam_apps_vtable[22] = vm22_stub;     // GetDlcDownloadProgress
    isteam_apps_vtable[23] = vm23_stub;     // GetAppBuildId
    isteam_apps_wrapper.vtable = isteam_apps_vtable;
    isteam_apps_wrapper.original_interface = SteamApps();
  }
  return &isteam_apps_wrapper;
}

// Sets up ISteamMatchmakingServers wrappers in ASE
void *(*SteamMatchmakingServers)();
void *SteamMatchmakingServers_wrapper() {
  if (!isteam_matchmaking_servers_wrapper.original_interface) {
    server_rules_callback_wrapper_vtable[0] = rules_responded;
    server_rules_callback_wrapper_vtable[1] = rules_failed_to_respond;
    server_rules_callback_wrapper_vtable[2] = rules_refresh_complete;
    isteam_matchmaking_servers_vtable[0] =
        request_internet_server_list; // RequestInternetServerList
    isteam_matchmaking_servers_vtable[1] = vm1_stub; // RequestLANServerList
    isteam_matchmaking_servers_vtable[2] = vm2_stub; // RequestFriendsServerList
    isteam_matchmaking_servers_vtable[3] =
        vm3_stub; // RequestFavoritesServerList
    isteam_matchmaking_servers_vtable[4] = vm4_stub; // RequestHistoryServerList
    isteam_matchmaking_servers_vtable[5] =
        vm5_stub; // RequestSpectatorServerList
    isteam_matchmaking_servers_vtable[6] = vm6_stub;      // ReleaseRequest
    isteam_matchmaking_servers_vtable[7] = vm7_stub;      // GetServerDetails
    isteam_matchmaking_servers_vtable[8] = vm8_stub;      // CancelQuery
    isteam_matchmaking_servers_vtable[9] = vm9_stub;      // RefreshQuery
    isteam_matchmaking_servers_vtable[10] = vm10_stub;    // IsRefreshing
    isteam_matchmaking_servers_vtable[11] = vm11_stub;    // GetServerCount
    isteam_matchmaking_servers_vtable[12] = vm12_stub;    // RefreshServer
    isteam_matchmaking_servers_vtable[13] = vm13_stub;    // PingServer
    isteam_matchmaking_servers_vtable[14] = vm14_stub;    // PlayerDetails
    isteam_matchmaking_servers_vtable[15] = server_rules; // ServerRules
    isteam_matchmaking_servers_vtable[16] = vm16_stub;    // CancelServerQuery
    isteam_matchmaking_servers_wrapper.vtable =
        isteam_matchmaking_servers_vtable;
    isteam_matchmaking_servers_wrapper.original_interface =
        SteamMatchmakingServers();
  }
  return &isteam_matchmaking_servers_wrapper;
}

// Sets up ISteamUGC wrappers in ASE
void *(*SteamUGC)();
void *SteamUGC_wrapper() {
  if (!isteam_ugc_wrapper.original_interface) {
    isteam_ugc_vtable[0] = vm0_stub;   // CreateQueryUserUGCRequest
    isteam_ugc_vtable[1] = vm1_stub;   // CreateQueryAllUGCRequest
    isteam_ugc_vtable[2] = vm2_stub;   // SendQueryUGCRequest
    isteam_ugc_vtable[3] = vm3_stub;   // GetQueryUGCResult
    isteam_ugc_vtable[4] = vm4_stub;   // ReleaseQueryUGCRequest
    isteam_ugc_vtable[5] = vm5_stub;   // AddRequiredTag
    isteam_ugc_vtable[6] = vm6_stub;   // AddExcludedTag
    isteam_ugc_vtable[7] = vm7_stub;   // SetReturnLongDescription
    isteam_ugc_vtable[8] = vm8_stub;   // SetReturnTotalOnly
    isteam_ugc_vtable[9] = vm9_stub;   // SetAllowCachedResponse
    isteam_ugc_vtable[10] = vm10_stub; // SetCloudFileNameFilter
    isteam_ugc_vtable[11] = vm11_stub; // SetMatchAnyTag
    isteam_ugc_vtable[12] = vm12_stub; // SetSearchText
    isteam_ugc_vtable[13] = vm13_stub; // SetRankedByTrendDays
    isteam_ugc_vtable[14] = vm14_stub; // RequestUGCDetails
    isteam_ugc_vtable[15] = vm15_stub; // CreateItem
    isteam_ugc_vtable[16] = vm16_stub; // StartItemUpdate
    isteam_ugc_vtable[17] = vm17_stub; // SetItemTitle
    isteam_ugc_vtable[18] = vm18_stub; // SetItemDescription
    isteam_ugc_vtable[19] = vm19_stub; // SetItemVisibility
    isteam_ugc_vtable[20] = vm20_stub; // SetItemTags
    isteam_ugc_vtable[21] = vm21_stub; // SetItemContent
    isteam_ugc_vtable[22] = vm22_stub; // SetItemPreview
    isteam_ugc_vtable[23] = vm23_stub; // SubmitItemUpdate
    isteam_ugc_vtable[24] = vm24_stub; // GetItemUpdateProgress
    isteam_ugc_vtable[25] = vm25_stub; // SubscribeItem
    isteam_ugc_vtable[26] = vm26_stub; // UnsubscribeItem
    isteam_ugc_vtable[27] = get_num_subscribed_items; // GetNumSubscribedItems
    isteam_ugc_vtable[28] = get_subscribed_items;     // GetSubscribedItems
    isteam_ugc_vtable[29] = get_item_install_info;    // GetItemInstallInfo
    isteam_ugc_vtable[30] = vm30_stub;                // GetItemUpdateInfo
    isteam_ugc_wrapper.vtable = isteam_ugc_vtable;
    isteam_ugc_wrapper.original_interface = SteamUGC();
  }
  return &isteam_ugc_wrapper;
}

// Sets up ISteamUtils wrappers in ASE
void *(*SteamUtils)();
void *SteamUtils_wrapper() {
  if (!isteam_utils_wrapper.original_interface) {
    isteam_utils_vtable[0] = vm0_stub;       // GetSecondsSinceAppActive
    isteam_utils_vtable[1] = vm1_stub;       // GetSecondsSinceComputerActive
    isteam_utils_vtable[2] = vm2_stub;       // GetConnectedUniverse
    isteam_utils_vtable[3] = vm3_stub;       // GetServerRealTime
    isteam_utils_vtable[4] = vm4_stub;       // GetIPCountry
    isteam_utils_vtable[5] = vm5_stub;       // GetImageSize
    isteam_utils_vtable[6] = vm6_stub;       // GetImageRGBA
    isteam_utils_vtable[7] = vm7_stub;       // GetCSERIPPort
    isteam_utils_vtable[8] = vm8_stub;       // GetCurrentBatteryPower
    isteam_utils_vtable[9] = ase_get_app_id; // GetAppID
    isteam_utils_vtable[10] = vm10_stub;     // SetOverlayNotificationPosition
    isteam_utils_vtable[11] = vm11_stub;     // IsAPICallCompleted
    isteam_utils_vtable[12] = vm12_stub;     // GetAPICallFailureReason
    isteam_utils_vtable[13] = vm13_stub;     // GetAPICallResult
    isteam_utils_vtable[14] = vm14_stub;     // RunFrame
    isteam_utils_vtable[15] = vm15_stub;     // GetIPCCallCount
    isteam_utils_vtable[16] = vm16_stub;     // SetWarningMessageHook
    isteam_utils_vtable[17] = vm17_stub;     // IsOverlayEnabled
    isteam_utils_vtable[18] = vm18_stub;     // BOverlayNeedsPresent
    isteam_utils_vtable[19] = vm19_stub;     // CheckFileSignature
    isteam_utils_vtable[20] = vm20_stub;     // ShowGamepadTextInput
    isteam_utils_vtable[21] = vm21_stub;     // GetEnteredGamepadTextLength
    isteam_utils_vtable[22] = vm22_stub;     // GetEnteredGamepadTextInput
    isteam_utils_vtable[23] = vm23_stub;     // GetSteamUILanguage
    isteam_utils_vtable[24] = vm24_stub;     // IsSteamRunningInVR
    isteam_utils_wrapper.vtable = isteam_utils_vtable;
    isteam_utils_wrapper.original_interface = SteamUtils();
  }
  return &isteam_utils_wrapper;
}

// Sets up interface wrappers in ASA, the only function that provides access
//    to Steam API interfaces in its newer versions
void *(*SteamInternal_FindOrCreateUserInterface)(int, const char *);
void *SteamInternal_FindOrCreateUserInterface_wrapper(int hSteamUser,
                                                      const char *pszVersion) {
  void *interface =
      SteamInternal_FindOrCreateUserInterface(hSteamUser, pszVersion);
  if (!strcmp(pszVersion, "STEAMAPPS_INTERFACE_VERSION008")) {
    isteam_apps_vtable[0] = return_true;        // BIsSubscribed
    isteam_apps_vtable[1] = vm1_stub;           // BIsLowViolence
    isteam_apps_vtable[2] = vm2_stub;           // BIsCybercafe
    isteam_apps_vtable[3] = vm3_stub;           // BIsVACBanned
    isteam_apps_vtable[4] = vm4_stub;           // GetCurrentGameLanguage
    isteam_apps_vtable[5] = vm5_stub;           // GetAvailableGameLanguages
    isteam_apps_vtable[6] = return_true;        // BIsSubscribedApp
    isteam_apps_vtable[7] = return_true;        // BIsDlcInstalled
    isteam_apps_vtable[8] = vm8_stub;           // GetEarliestPurchaseUnixTime
    isteam_apps_vtable[9] = vm9_stub;           // BIsSubscribedFromFreeWeekend
    isteam_apps_vtable[10] = asa_get_dlc_count; // GetDLCCount
    isteam_apps_vtable[11] = asa_get_dlc_data_by_index; // BGetDLCDataByIndex
    isteam_apps_vtable[12] = vm12_stub;                 // InstallDLC
    isteam_apps_vtable[13] = vm13_stub;                 // UninstallDLC
    isteam_apps_vtable[14] = vm14_stub;     // RequestAppProofOfPurchaseKey
    isteam_apps_vtable[15] = vm15_stub;     // GetCurrentBetaName
    isteam_apps_vtable[16] = vm16_stub;     // MarkContentCorrupt
    isteam_apps_vtable[17] = vm17_stub;     // GetInstalledDepots
    isteam_apps_vtable[18] = vm18_stub;     // GetAppInstallDir
    isteam_apps_vtable[19] = vm19_stub;     // BIsAppInstalled
    isteam_apps_vtable[20] = get_app_owner; // GetAppOwner
    isteam_apps_vtable[21] = vm21_stub;     // GetLaunchQueryParam
    isteam_apps_vtable[22] = vm22_stub;     // GetDlcDownloadProgress
    isteam_apps_vtable[23] = vm23_stub;     // GetAppBuildId
    isteam_apps_vtable[24] = vm24_stub;     // RequestAllProofOfPurchaseKeys
    isteam_apps_vtable[25] = vm25_stub;     // GetFileDetails
    isteam_apps_vtable[26] = vm26_stub;     // GetLaunchCommandLine
    isteam_apps_vtable[27] = vm27_stub;     // BIsSubscribedFromFamilySharing
    isteam_apps_vtable[28] = vm28_stub;     // BIsTimedTrial
    isteam_apps_wrapper.vtable = isteam_apps_vtable;
    isteam_apps_wrapper.original_interface = interface;
    return &isteam_apps_wrapper;
  } else if (!strcmp(pszVersion, "SteamUser021")) {
    // This interface is only needed to get user Steam ID
    ((uint64_t * (*)(void *, uint64_t *))((*((void ***)interface))[2]))(
        interface, &steam_id);
  } else if (!strcmp(pszVersion, "SteamUtils010")) {
    isteam_utils_vtable[0] = vm0_stub;       // GetSecondsSinceAppActive
    isteam_utils_vtable[1] = vm1_stub;       // GetSecondsSinceComputerActive
    isteam_utils_vtable[2] = vm2_stub;       // GetConnectedUniverse
    isteam_utils_vtable[3] = vm3_stub;       // GetServerRealTime
    isteam_utils_vtable[4] = vm4_stub;       // GetIPCountry
    isteam_utils_vtable[5] = vm5_stub;       // GetImageSize
    isteam_utils_vtable[6] = vm6_stub;       // GetImageRGBA
    isteam_utils_vtable[7] = vm7_stub;       // GetCSERIPPort
    isteam_utils_vtable[8] = vm8_stub;       // GetCurrentBatteryPower
    isteam_utils_vtable[9] = asa_get_app_id; // GetAppID
    isteam_utils_vtable[10] = vm10_stub;     // SetOverlayNotificationPosition
    isteam_utils_vtable[11] = vm11_stub;     // IsAPICallCompleted
    isteam_utils_vtable[12] = vm12_stub;     // GetAPICallFailureReason
    isteam_utils_vtable[13] = vm13_stub;     // GetAPICallResult
    isteam_utils_vtable[14] = vm14_stub;     // RunFrame
    isteam_utils_vtable[15] = vm15_stub;     // GetIPCCallCount
    isteam_utils_vtable[16] = vm16_stub;     // SetWarningMessageHook
    isteam_utils_vtable[17] = vm17_stub;     // IsOverlayEnabled
    isteam_utils_vtable[18] = vm18_stub;     // BOverlayNeedsPresent
    isteam_utils_vtable[19] = vm19_stub;     // CheckFileSignature
    isteam_utils_vtable[20] = vm20_stub;     // ShowGamepadTextInput
    isteam_utils_vtable[21] = vm21_stub;     // GetEnteredGamepadTextLength
    isteam_utils_vtable[22] = vm22_stub;     // GetEnteredGamepadTextInput
    isteam_utils_vtable[23] = vm23_stub;     // GetSteamUILanguage
    isteam_utils_vtable[24] = vm24_stub;     // IsSteamRunningInVR
    isteam_utils_vtable[25] = vm25_stub;     // SetOverlayNotificationInset
    isteam_utils_vtable[26] = vm26_stub;     // IsSteamInBigPictureMode
    isteam_utils_vtable[27] = vm27_stub;     // StartVRDashboard
    isteam_utils_vtable[28] = vm28_stub;     // IsVRHeadsetStreamingEnabled
    isteam_utils_vtable[29] = vm29_stub;     // SetVRHeadsetStreamingEnabled
    isteam_utils_vtable[30] = vm30_stub;     // IsSteamChinaLauncher
    isteam_utils_vtable[31] = vm31_stub;     // InitFilterText
    isteam_utils_vtable[32] = vm32_stub;     // FilterText
    isteam_utils_vtable[33] = vm33_stub;     // GetIPv6ConnectivityState
    isteam_utils_vtable[34] = vm34_stub;     // IsSteamRunningOnSteamDeck
    isteam_utils_vtable[35] = vm35_stub;     // ShowFloatingGamepadTextInput
    isteam_utils_vtable[36] = vm36_stub;     // SetGameLauncherMode
    isteam_utils_vtable[37] = vm37_stub;     // DismissFloatingGamepadTextInput
    isteam_utils_wrapper.vtable = isteam_utils_vtable;
    isteam_utils_wrapper.original_interface = interface;
    return &isteam_utils_wrapper;
  }
  return interface;
}

//===--- EOS SDK types ----------------------------------------------------===//

enum EOS_EAuthScopeFlags {
  EOS_AS_NoFlags = 0x0,
  EOS_AS_BasicProfile = 0x1,
  EOS_AS_FriendsList = 0x2,
  EOS_AS_Presence = 0x4,
  EOS_AS_FriendsManagement = 0x8,
  EOS_AS_Email = 0x10,
  EOS_AS_Country = 0x20
};

enum EOS_EExternalAccountType {
  EOS_EAT_EPIC,
  EOS_EAT_STEAM,
  EOS_EAT_PSN,
  EOS_EAT_XBL,
  EOS_EAT_DISCORD,
  EOS_EAT_GOG,
  EOS_EAT_NINTENDO,
  EOS_EAT_UPLAY,
  EOS_EAT_OPENID,
  EOS_EAT_APPLE,
  EOS_EAT_GOOGLE,
  EOS_EAT_OCULUS,
  EOS_EAT_ITCHIO,
  EOS_EAT_AMAZON
};

enum EOS_EExternalCredentialType {
  EOS_ECT_EPIC,
  EOS_ECT_STEAM_APP_TICKET,
  EOS_ECT_PSN_ID_TOKEN,
  EOS_ECT_XBL_XSTS_TOKEN,
  EOS_ECT_DISCORD_ACCESS_TOKEN,
  EOS_ECT_GOG_SESSION_TICKET,
  EOS_ECT_NINTENDO_ID_TOKEN,
  EOS_ECT_NINTENDO_NSA_ID_TOKEN,
  EOS_ECT_UPLAY_ACCESS_TOKEN,
  EOS_ECT_OPENID_ACCESS_TOKEN,
  EOS_ECT_DEVICEID_ACCESS_TOKEN,
  EOS_ECT_APPLE_ID_TOKEN,
  EOS_ECT_GOOGLE_ID_TOKEN,
  EOS_ECT_OCULUS_USERID_NONCE,
  EOS_ECT_ITCHIO_JWT,
  EOS_ECT_ITCHIO_KEY,
  EOS_ECT_EPIC_ID_TOKEN,
  EOS_ECT_AMAZON_ACCESS_TOKEN,
  EOS_ECT_STEAM_SESSION_TICKET
};

enum EOS_ELoginCredentialType {
  EOS_LCT_Password,
  EOS_LCT_ExchangeCode,
  EOS_LCT_PersistentAuth,
  EOS_LCT_DeviceCode,
  EOS_LCT_Developer,
  EOS_LCT_RefreshToken,
  EOS_LCT_AccountPortal,
  EOS_LCT_ExternalAuth
};

struct EOS_Auth_CopyIdTokenOptions {
  int32_t ApiVersion;
  void *AccountId;
};

struct EOS_Auth_Credentials {
  int32_t ApiVersion;
  const char *Id;
  const char *Token;
  enum EOS_ELoginCredentialType Type;
  void *SystemAuthCredentialsOptions;
  enum EOS_EExternalCredentialType ExternalType;
};

struct EOS_Auth_IdToken {
  int32_t ApiVersion;
  void *AccountId;
  const char *JsonWebToken;
};

struct EOS_Auth_LoginCallbackInfo {
  int ResultCode;
  void *ClientData;
  void *LocalUserId;
  const void *PinGrantInfo;
  void *ContinuanceToken;
  const void *AccountFeatureRestrictedInfo;
  void *SelectedAccountId;
};

struct EOS_Auth_LoginOptions {
  int32_t ApiVersion;
  const struct EOS_Auth_Credentials *Credentials;
  enum EOS_EAuthScopeFlags ScopeFlags;
};

struct EOS_Connect_Credentials {
  int32_t ApiVersion;
  const char *Token;
  enum EOS_EExternalCredentialType Type;
};

struct EOS_Connect_ExternalAccountInfo {
  int32_t ApiVersion;
  void *ProductUserId;
  const char *DisplayName;
  const char *AccountId;
  enum EOS_EExternalAccountType AccountIdType;
  int64_t LastLoginTime;
};

struct EOS_Connect_LoginOptions {
  int32_t ApiVersion;
  const struct EOS_Connect_Credentials *Credentials;
  const void *UserLoginInfo;
};

//===--- EOSSDK-Win64-Shipping.dll function wrappers ----------------------===//

// Used by eos_auth_login_callback to track its state
bool eos_persistent_auth_failed = false;
// Used by eos_auth_login_callback
void *restrict auth_interface;
// Stores the token between EOS_Platform_Create and EOS_Connect_Login calls
struct EOS_Auth_IdToken *eos_auth_token;

int (*EOS_Auth_CopyIdToken)(void *Handle,
                            const struct EOS_Auth_CopyIdTokenOptions *Options,
                            struct EOS_Auth_IdToken **OutIdToken);
void (*EOS_Auth_Login)(void *Handle,
                       const struct EOS_Auth_LoginOptions *Options,
                       void *ClientData,
                       void (*const CompletionDelegate)(
                           const struct EOS_Auth_LoginCallbackInfo *));
void *(*EOS_Platform_GetAuthInterface)(void *Handle);
void *(*EOS_Platform_Tick)(void *Handle);

int (*EOS_Connect_CopyProductUserInfo)(
    void *Handle, const void *Options,
    struct EOS_Connect_ExternalAccountInfo **OutExternalAccountInfo);
int EOS_Connect_CopyProductUserInfo_wrapper(
    void *Handle, const void *Options,
    struct EOS_Connect_ExternalAccountInfo **OutExternalAccountInfo) {
  int result =
      EOS_Connect_CopyProductUserInfo(Handle, Options, OutExternalAccountInfo);
  if (!result) {
    // Set account type to Steam so stuff that relies on it doesn't break
    struct EOS_Connect_ExternalAccountInfo *const accountInfo =
        *OutExternalAccountInfo;
    accountInfo->AccountIdType = EOS_EAT_STEAM;
    accountInfo->AccountId = NULL;
  }
  return result;
}

void (*EOS_Connect_Login)(void *Handle,
                          const struct EOS_Connect_LoginOptions *Options,
                          void *ClientData, const void *CompletionDelegate);
void EOS_Connect_Login_wrapper(void *Handle,
                               const struct EOS_Connect_LoginOptions *Options,
                               void *ClientData,
                               const void *CompletionDelegate) {
  // Replace Steam session ticket with our Epic ID token
  struct EOS_Connect_Credentials *credentials =
      (struct EOS_Connect_Credentials *)Options->Credentials;
  credentials->Type = EOS_ECT_EPIC_ID_TOKEN;
  credentials->Token = eos_auth_token->JsonWebToken;
  EOS_Connect_Login(Handle, Options, ClientData, CompletionDelegate);
}

void eos_auth_login_callback(const struct EOS_Auth_LoginCallbackInfo *Data) {
  if (!Data->ResultCode)
    *((void **)Data->ClientData) = Data->LocalUserId; // Login succeeded
  else if (eos_persistent_auth_failed)
    ExitProcess(GEC_EOS_AUTH_LOGIN_FAILED); // Both auth types failed
  else {
    // Retry login with EOS_LCT_AccountPortal
    const struct EOS_Auth_Credentials credentials = {
        3, NULL, NULL, EOS_LCT_AccountPortal, NULL, 0};
    const struct EOS_Auth_LoginOptions options = {2, &credentials,
                                                  EOS_AS_NoFlags};
    EOS_Auth_Login(auth_interface, &options, Data->ClientData,
                   eos_auth_login_callback);
  }
}

// This is one of the first functions called in EOS API so it can be used for
//    early Epic Games authenticaiton
void *(*EOS_Platform_Create)(const void *Options);
void *EOS_Platform_Create_wrapper(const void *Options) {
  void *const platform = EOS_Platform_Create(Options);
  auth_interface = EOS_Platform_GetAuthInterface(platform);
  // Try to login with EOS_LCT_PersistentAuth first
  const struct EOS_Auth_Credentials credentials = {
      3, NULL, NULL, EOS_LCT_PersistentAuth, NULL, 0};
  const struct EOS_Auth_LoginOptions loginOptions = {2, &credentials,
                                                     EOS_AS_NoFlags};
  struct EOS_Auth_CopyIdTokenOptions copyTokenOptions = {1, NULL};
  EOS_Auth_Login(auth_interface, &loginOptions, &copyTokenOptions.AccountId,
                 eos_auth_login_callback);
  // Wait for login to complete
  while (!copyTokenOptions.AccountId) {
    EOS_Platform_Tick(platform);
    SwitchToThread(); // Yield execution to another thread to skip time without
                      //    excessive CPU usage
  }
  EOS_Auth_CopyIdToken(auth_interface, &copyTokenOptions, &eos_auth_token);
  return platform;
}

//===--- Process entry point wrappers -------------------------------------===//

// Substitutes entry point for ArkAscended.exe
void __declspec(noreturn)
asa_entry(void __declspec(noreturn) (*entryPoint)(DWORD64)) {
  // Step 1: load steam_api64.dll, find its IAT and call SteamAPI_Init
  // Get image headers
  const uint8_t *const moduleBase = (const uint8_t *)GetModuleHandleW(NULL);
  const IMAGE_NT_HEADERS64 *const ntHeaders =
      (const IMAGE_NT_HEADERS64
           *)(moduleBase + ((const IMAGE_DOS_HEADER *)moduleBase)->e_lfanew);
  const IMAGE_DELAYLOAD_DESCRIPTOR *const delayDescBase =
      (const IMAGE_DELAYLOAD_DESCRIPTOR
           *)(moduleBase +
              ntHeaders->OptionalHeader
                  .DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT]
                  .VirtualAddress);
  const IMAGE_DELAYLOAD_DESCRIPTOR *delayDesc = delayDescBase;
  // Find delay import descriptor for steam_api64.dll
  while (strcmp((const char *)(moduleBase + delayDesc->DllNameRVA),
                "steam_api64.dll"))
    delayDesc++;
  // Load the DLL explicitly, delay import loader is not aware of this relative
  //    path, then write the module handle to descriptor
  const HMODULE steamApiModule =
      LoadLibraryW(L"..\\..\\.."
                   L"\\Engine\\Binaries\\ThirdParty\\Steamworks\\Steamv153\\Win"
                   L"64\\steam_api64.dll");
  *((HMODULE *)(moduleBase + delayDesc->ModuleHandleRVA)) = steamApiModule;
  // Initialize Steam API
  if (!((bool (*const)())GetProcAddress(steamApiModule, "SteamAPI_Init"))())
    ExitProcess(GEC_STEAM_API_INIT_FAILED);
  // Step 2: setup SteamInternal_FindOrCreateUserInterface wrapper
  SteamInternal_FindOrCreateUserInterface =
      (void *(*)(int, const char *))GetProcAddress(
          steamApiModule, "SteamInternal_FindOrCreateUserInterface");
  // Search import name table for SteamInternal_FindOrCreateUserInterface
  const uint64_t *nameImportsBase =
      (const uint64_t *)(moduleBase + delayDesc->ImportNameTableRVA);
  uint32_t index = 0;
  while (strcmp(
      ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
          ->Name,
      "SteamInternal_FindOrCreateUserInterface"))
    index++;
  // Use the index to get function pointer from IAT and write wrapper address
  ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
      SteamInternal_FindOrCreateUserInterface_wrapper;
  // Step 3: setup EOS SDK wrappers for Epic Games authentication if necessary
  // GetEnvironmentVariableA returns 4 when app ID is 480 (3 characters + null)
  if (GetEnvironmentVariableA("GameAppId", NULL, 0) == 4) {
    // Find delay import descriptor for EOSSDK-Win64-Shipping.dll
    delayDesc = delayDescBase;
    while (strcmp((const char *)(moduleBase + delayDesc->DllNameRVA),
                  "EOSSDK-Win64-Shipping.dll"))
      delayDesc++;
    // Load the DLL explicitly, delay import loader is not aware of this
    //    relative path, then write the module handle to descriptor
    const HMODULE eosSdkModule =
        LoadLibraryW(L"RedpointEOS\\EOSSDK-Win64-Shipping.dll");
    *((HMODULE *)(moduleBase + delayDesc->ModuleHandleRVA)) = eosSdkModule;
    // Get all necessary function addresses
    EOS_Auth_CopyIdToken = (int (*)(
        void *, const struct EOS_Auth_CopyIdTokenOptions *,
        struct EOS_Auth_IdToken **))GetProcAddress(eosSdkModule,
                                                   "EOS_Auth_CopyIdToken");
    EOS_Auth_Login =
        (void (*)(void *, const struct EOS_Auth_LoginOptions *, void *,
                  void (*const)(const struct EOS_Auth_LoginCallbackInfo *)))
            GetProcAddress(eosSdkModule, "EOS_Auth_Login");
    EOS_Connect_CopyProductUserInfo = (int (*)(
        void *, const void *, struct EOS_Connect_ExternalAccountInfo **))
        GetProcAddress(eosSdkModule, "EOS_Connect_CopyProductUserInfo");
    EOS_Connect_Login =
        (void (*)(void *, const struct EOS_Connect_LoginOptions *, void *,
                  const void *))GetProcAddress(eosSdkModule,
                                               "EOS_Connect_Login");
    EOS_Platform_Create = (void *(*)(const void *))GetProcAddress(
        eosSdkModule, "EOS_Platform_Create");
    EOS_Platform_GetAuthInterface = (void *(*)(void *))GetProcAddress(
        eosSdkModule, "EOS_Platform_GetAuthInterface");
    EOS_Platform_Tick =
        (void *(*)(void *))GetProcAddress(eosSdkModule, "EOS_Platform_Tick");
    // Search import name table for EOS_Connect_CopyProductUserInfo
    nameImportsBase =
        (const uint64_t *)(moduleBase + delayDesc->ImportNameTableRVA);
    index = 0;
    while (strcmp(
        ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
            ->Name,
        "EOS_Connect_CopyProductUserInfo"))
      index++;
    // Use the index to get function pointer from IAT and write wrapper address
    ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
        EOS_Connect_CopyProductUserInfo_wrapper;
    // Search import name table for EOS_Connect_Login
    index = 0;
    while (strcmp(
        ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
            ->Name,
        "EOS_Connect_Login"))
      index++;
    // Use the index to get function pointer from IAT and write wrapper address
    ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
        EOS_Connect_Login_wrapper;
    // Search import name table for EOS_Platform_Create
    index = 0;
    while (strcmp(
        ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
            ->Name,
        "EOS_Platform_Create"))
      index++;
    // Use the index to get function pointer from IAT and write wrapper address
    ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
        EOS_Platform_Create_wrapper;
  }
  // Pass execution to actual entry point as if nothing happened
  entryPoint(__readgsqword(0x60)); // Set PEB as the argument
}

// Substitutes entry point for ShooterGame.exe
void __declspec(noreturn)
ase_entry(void __declspec(noreturn) (*entryPoint)(DWORD64)) {
  // Step 1: load steam_api64.dll, find its IAT and call SteamAPI_Init
  // Get image headers
  const uint8_t *const moduleBase = (const uint8_t *)GetModuleHandleW(NULL);
  const IMAGE_NT_HEADERS64 *const ntHeaders =
      (const IMAGE_NT_HEADERS64
           *)(moduleBase + ((const IMAGE_DOS_HEADER *)moduleBase)->e_lfanew);
  const IMAGE_DELAYLOAD_DESCRIPTOR *const delayDescBase =
      (const IMAGE_DELAYLOAD_DESCRIPTOR
           *)(moduleBase +
              ntHeaders->OptionalHeader
                  .DataDirectory[IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT]
                  .VirtualAddress);
  const IMAGE_DELAYLOAD_DESCRIPTOR *delayDesc = delayDescBase;
  // Find delay import descriptor for steam_api64.dll
  while (strcmp((const char *)(moduleBase + delayDesc->DllNameRVA),
                "steam_api64.dll"))
    delayDesc++;
  // Load the DLL explicitly, delay import loader is not aware of this relative
  //    path, then write the module handle to descriptor
  const HMODULE steamApiModule =
      LoadLibraryW(L"..\\..\\.."
                   L"\\Engine\\Binaries\\ThirdParty\\Steamworks\\Steamv132\\Win"
                   L"64\\steam_api64.dll");
  *((HMODULE *)(moduleBase + delayDesc->ModuleHandleRVA)) = steamApiModule;
  // Initialize Steam API
  if (!((bool (*const)())GetProcAddress(steamApiModule, "SteamAPI_Init"))())
    ExitProcess(GEC_STEAM_API_INIT_FAILED);
  // Step 2: setup Steam interface accessor wrappers
  // Get all necessary function addresses
  SteamApps = (void *(*)())GetProcAddress(steamApiModule, "SteamApps");
  SteamMatchmakingServers =
      (void *(*)())GetProcAddress(steamApiModule, "SteamMatchmakingServers");
  SteamUGC = (void *(*)())GetProcAddress(steamApiModule, "SteamUGC");
  SteamUtils = (void *(*)())GetProcAddress(steamApiModule, "SteamUtils");
  // Search import name table for SteamApps
  const uint64_t *nameImportsBase =
      (const uint64_t *)(moduleBase + delayDesc->ImportNameTableRVA);
  uint32_t index = 0;
  while (strcmp(
      ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
          ->Name,
      "SteamApps"))
    index++;
  // Use the index to get function pointer from IAT and write wrapper address
  ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
      SteamApps_wrapper;
  // Search import name table for SteamMatchmakingServers
  index = 0;
  while (strcmp(
      ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
          ->Name,
      "SteamMatchmakingServers"))
    index++;
  // Use the index to get function pointer from IAT and write wrapper address
  ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
      SteamMatchmakingServers_wrapper;
  // Search import name table for SteamUGC
  index = 0;
  while (strcmp(
      ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
          ->Name,
      "SteamUGC"))
    index++;
  // Use the index to get function pointer from IAT and write wrapper address
  ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
      SteamUGC_wrapper;
  // Search import name table for SteamUtils
  index = 0;
  while (strcmp(
      ((const IMAGE_IMPORT_BY_NAME *)(moduleBase + nameImportsBase[index]))
          ->Name,
      "SteamUtils"))
    index++;
  // Use the index to get function pointer from IAT and write wrapper address
  ((void **)(moduleBase + delayDesc->ImportAddressTableRVA))[index] =
      SteamUtils_wrapper;
  // Get user Steam ID
  void *const iSteamUser =
      ((void *(*)())GetProcAddress(steamApiModule, "SteamUser"))();
  ((uint64_t * (*)(void *, uint64_t *))((*((void ***)iSteamUser))[2]))(
      iSteamUser, &steam_id);
  // Pass execution to actual entry point as if nothing happened
  entryPoint(__readgsqword(0x60)); // Set PEB as the argument
}

//===- Host-side functions (called inside injector process) ---------------===//

enum error_code launch_game_and_inject(LPCWSTR exePath, int argc,
                                       const wchar_t **argv,
                                       const void *entryPoint,
                                       bool isEmbedded) {
  // Get image size from PE optional header
  uint8_t *const module = (uint8_t *)GetModuleHandleW(NULL);
  const IMAGE_NT_HEADERS64 *const ntHeaders =
      (const IMAGE_NT_HEADERS64 *)(module + ((const IMAGE_DOS_HEADER *)module)
                                                ->e_lfanew);
  const SIZE_T imageSize = ntHeaders->OptionalHeader.SizeOfImage;
  if (!isEmbedded) {
    // Make nasty sections with zeroed characteristics (yes you, .retplne !)
    //    readable
    const IMAGE_SECTION_HEADER *const sectionHeaders =
        (const IMAGE_SECTION_HEADER *)(ntHeaders + 1);
    for (DWORD i = 0; i < ntHeaders->FileHeader.NumberOfSections; i++)
      if (!(sectionHeaders[i].Characteristics & IMAGE_SCN_MEM_READ)) {
        DWORD oldProtect;
        if (!VirtualProtect(module + sectionHeaders[i].VirtualAddress,
                            sectionHeaders[i].Misc.VirtualSize, PAGE_READONLY,
                            &oldProtect))
          return IEC_VIRTUAL_PROTECT_FAILED;
      }
  }
  // Assemble command line for game process
  size_t commandLineLength = GetFullPathNameW(exePath, 0, NULL, NULL) + 2;
  for (int i = 0; i < argc; i++)
    commandLineLength += wcslen(argv[i]) + 1;
  const PWSTR commandLine =
      HeapAlloc(GetProcessHeap(), 0, commandLineLength * sizeof(WCHAR));
  commandLine[0] = L'"';
  size_t offset =
      1 + GetFullPathNameW(exePath, commandLineLength, commandLine + 1, NULL);
  commandLine[offset++] = L'\"';
  bool setHighPriority = false;
  for (int i = 0; i < argc; i++) {
    const size_t argLength = wcslen(argv[i]);
    if (argLength == 5 && !wcscmp(argv[i], L"-high"))
      setHighPriority = true; // Game doesn't use this argument now but we do
    commandLine[offset++] = L' ';
    memcpy(commandLine + offset, argv[i], argLength * sizeof(WCHAR));
    offset += argLength;
  }
  commandLine[offset] = L'\0';
  // Create suspended game process
  STARTUPINFOW startupInfo;
  startupInfo.cb = sizeof(STARTUPINFOW);
  PROCESS_INFORMATION procInfo;
  const BOOL processCreated =
      CreateProcessW(exePath, commandLine, NULL, NULL, FALSE,
                     setHighPriority ? CREATE_SUSPENDED | HIGH_PRIORITY_CLASS
                                     : CREATE_SUSPENDED,
                     NULL, NULL, &startupInfo, &procInfo);
  HeapFree(GetProcessHeap(), 0, commandLine);
  if (!processCreated)
    return IEC_CREATE_PROCESS_FAILED;
  // Allocate memory in game process for TEK Injector image
  enum error_code result = EC_SUCCESS;
  const LPVOID imageRegion =
      VirtualAllocEx(procInfo.hProcess, NULL, imageSize,
                     MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!imageRegion) {
    result = IEC_VIRTUAL_ALLOC_FAILED;
    goto Exit;
  }
  // Write TEK Injector image to the allocated region
  if (!WriteProcessMemory(procInfo.hProcess, imageRegion, module, imageSize,
                          NULL)) {
    result = IEC_WRITE_PROC_MEM_FAILED;
    goto Exit;
  }
  // The first function executed in a new Windows process is RtlUserThreadStart,
  //    which takes 2 arguments: address of function to run (PE loader sets it
  //    to the entry point specified in optional header), and argument to pass
  //    to that function (PE loader sets it to PEB address), we'll make it run
  //    our custom entry point instead and pass it address of real entry point
  //    so it can call or jump to it afterwards
  CONTEXT threadContext;
  threadContext.ContextFlags = CONTEXT_INTEGER;
  GetThreadContext(procInfo.hThread, &threadContext);
  // RCX contained address of entry point, it'll be the argument now
  threadContext.Rdx = threadContext.Rcx;
  // Set RCX to custom entry point address, adjusted for new address space
  threadContext.Rcx =
      (DWORD64)(entryPoint - (const void *)module + imageRegion);
  if (!SetThreadContext(procInfo.hThread, &threadContext)) {
    result = IEC_SET_THREAD_CTX_FAILED;
    goto Exit;
  }
  // Begin thread execution
  ResumeThread(procInfo.hThread);
  // Wait 10 seconds to see if game exits with any error code
  if (!WaitForSingleObject(procInfo.hProcess, 10000))
    GetExitCodeProcess(procInfo.hProcess, (LPDWORD)&result);
Exit:
  if (result > EC_MIN && result <= IEC_SET_THREAD_CTX_FAILED)
    TerminateThread(procInfo.hThread, result);
  CloseHandle(procInfo.hThread);
  CloseHandle(procInfo.hProcess);
  return result;
}

void show_error_message(enum error_code code) {
  if (code < 0 && code > EC_MIN)
    MessageBoxW(NULL,
                error_code_strings[code + sizeof(error_code_strings) /
                                              sizeof(wchar_t *)],
                L"TEK Injector", MB_ICONERROR);
  else {
    LPWSTR message;
    if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL, code, 0, (LPWSTR)&message, 0, NULL)) {
      MessageBoxW(NULL, message, L"TEK Injector", MB_ICONERROR);
      LocalFree(message);
    } else {
      WCHAR messageBuffer[] = {L"Game process exited with code 0x00000000"};
      DWORD value = (code & 0xF0000000) >> 28;
      messageBuffer[32] = (value > 9 ? L'7' : L'0') + value;
      value = (code & 0xF0000000) >> 24;
      messageBuffer[33] = (value > 9 ? L'7' : L'0') + value;
      value = (code & 0xF0000000) >> 20;
      messageBuffer[34] = (value > 9 ? L'7' : L'0') + value;
      value = (code & 0xF0000000) >> 16;
      messageBuffer[35] = (value > 9 ? L'7' : L'0') + value;
      value = (code & 0xF0000000) >> 12;
      messageBuffer[36] = (value > 9 ? L'7' : L'0') + value;
      value = (code & 0xF0000000) >> 8;
      messageBuffer[37] = (value > 9 ? L'7' : L'0') + value;
      value = (code & 0xF0000000) >> 4;
      messageBuffer[38] = (value > 9 ? L'7' : L'0') + value;
      value = code & 0x0000000F;
      messageBuffer[39] = (value > 9 ? L'7' : L'0') + value;
      MessageBoxW(NULL, messageBuffer, L"TEK Injector", MB_ICONERROR);
    }
  }
}

// Exported entry points that can be used if TEK Injector is loaded into other
//    process' address space

__declspec(dllexport) void launch_asa(const wchar_t *exePath, int argc,
                                      const wchar_t **argv) {
  // Try using original app ID first
  SetEnvironmentVariableA("SteamAppId", "2399830");
  SetEnvironmentVariableA("GameAppId", "2399830");
  enum error_code result =
      launch_game_and_inject(exePath, argc, argv, asa_entry, true);
  if (result == GEC_STEAM_API_INIT_FAILED) {
    // Current Steam user doesn't own the game, fallback to Spacewar
    SetEnvironmentVariableA("SteamAppId", "480");
    SetEnvironmentVariableA("GameAppId", "480");
    result = launch_game_and_inject(exePath, argc, argv, asa_entry, true);
  }
  if (result)
    show_error_message(result);
}
__declspec(dllexport) void launch_ase(const wchar_t *exePath, int argc,
                                      const wchar_t **argv) {
  // Try using original app ID first
  SetEnvironmentVariableA("SteamAppId", "346110");
  SetEnvironmentVariableA("GameAppId", "346110");
  enum error_code result =
      launch_game_and_inject(exePath, argc, argv, ase_entry, true);
  if (result == GEC_STEAM_API_INIT_FAILED) {
    // Current Steam user doesn't own the game, fallback to Spacewar
    SetEnvironmentVariableA("SteamAppId", "480");
    SetEnvironmentVariableA("GameAppId", "480");
    result = launch_game_and_inject(exePath, argc, argv, ase_entry, true);
  }
  if (result)
    show_error_message(result);
}

// Main entry point used when running standalone tek-injector.exe
void __declspec(noreturn) entry() {
  LPCWSTR gameExe;
  const void *entryPoint;
  LPCSTR appId;
  // Check if ArkAscended.exe or ShooterGame.exe is present in current directory
  // For some odd reason using GetFileAttributesW breaks created game process,
  //    so have to use Ex variant
  WIN32_FILE_ATTRIBUTE_DATA attr;
  if (GetFileAttributesExW(L"ArkAscended.exe", GetFileExInfoStandard, &attr)) {
    gameExe = L"ArkAscended.exe";
    entryPoint = asa_entry;
    appId = "2399830";
  } else if (GetFileAttributesExW(L"ShooterGame.exe", GetFileExInfoStandard,
                                  &attr)) {
    gameExe = L"ShooterGame.exe";
    entryPoint = ase_entry;
    appId = "346110";
  } else {
    MessageBoxW(NULL,
                L"Neither ArkAscended.exe nor ShooterGame.exe was found in "
                L"current directory",
                L"TEK Injector", MB_ICONERROR);
    ExitProcess(0);
  }
  // Get command line
  int argc;
  LPCWSTR *const argv = (LPCWSTR *)CommandLineToArgvW(GetCommandLineW(), &argc);
  if (!argc)
    argc = 1; // Just a safety check so code below doesn't get broken
  // Launch the game
  // Try using original app ID first
  SetEnvironmentVariableA("SteamAppId", appId);
  SetEnvironmentVariableA("GameAppId", appId);
  enum error_code result =
      launch_game_and_inject(gameExe, argc - 1, argv + 1, entryPoint, false);
  if (result == GEC_STEAM_API_INIT_FAILED) {
    // Current Steam user doesn't own the game, fallback to Spacewar
    SetEnvironmentVariableA("SteamAppId", "480");
    SetEnvironmentVariableA("GameAppId", "480");
    result =
        launch_game_and_inject(gameExe, argc - 1, argv + 1, entryPoint, false);
  }
  LocalFree(argv);
  if (result)
    show_error_message(result);
  ExitProcess(result);
}
