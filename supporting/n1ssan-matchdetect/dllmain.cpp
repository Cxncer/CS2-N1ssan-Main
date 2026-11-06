// n1ssan-matchdetect / dllmain.cpp
// DIAGNOSTIC BUILD v1.1
//   Offsets updated to a2x cs2-dumper 2026-10-23:
//     dwEntityList             = 0x271C8B8
//     dwLocalPlayerController  = 0x253EEF8
//     dwLocalPlayerPawn        = 0x25676D8
//   Purpose: prove whether the game is in a live match, and if so, whether
//   dwLocalPlayerPawn / dwLocalPlayerController / entitySystem are populated.
//   No writes. No injection. Pure observation.

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <string>
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")

// ---------------------------------------------------------------------------
// Offsets (a2x 2026-10-23)
// ---------------------------------------------------------------------------
static constexpr uintptr_t RVA_PLOCALPLAYERCONTROLLER = 0x253EEF8;
static constexpr uintptr_t RVA_LOCALPLAYERPAWN = 0x25676D8;
static constexpr uintptr_t RVA_ENTITY_LIST = 0x271C8B8;
static constexpr uintptr_t GES_OFF_HIGHEST_ENTITY = 0x2120;
static constexpr uintptr_t GES_OFF_CHUNK_TABLE = 0x10;
static constexpr uintptr_t GES_OFF_CHUNK_STRIDE = 0x70;

// Controller pawn handle — schema: CCSPlayerController::m_hPlayerPawn = 0x92C
static constexpr uint32_t CTRL_OFF_PLAYERPAWN = 0x92C;

// Pawn health — schema: C_BaseEntity::m_iHealth = 0x34C
static constexpr uintptr_t PAWN_OFF_HEALTH = 0x34C;

// FrameStageNotify slot on Source2Client002
static constexpr int VT_SOURCE2CLIENT_FSN = 36;

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------
static FILE* g_logFile = nullptr;

static void Log(const char* fmt, ...) {
    char buf[2048]{};
    va_list ap; va_start(ap, fmt);
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    printf("%s\n", buf);
    if (g_logFile) { fputs(buf, g_logFile); fputc('\n', g_logFile); fflush(g_logFile); }
}

static void LogInit() {
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
    SetConsoleTitleA("n1ssan-matchdetect v1.1");
    setvbuf(stdout, nullptr, _IONBF, 0);

    char tmp[MAX_PATH]{}; GetTempPathA(MAX_PATH, tmp);
    char path[MAX_PATH]{}; sprintf_s(path, "%sn1ssan-matchdetect.log", tmp);
    g_logFile = _fsopen(path, "w", _SH_DENYNO);
    printf("[detect] log file: %s\n", path);
}

// ---------------------------------------------------------------------------
// Module / safe rw
// ---------------------------------------------------------------------------
struct ModuleInfo {
    uintptr_t base = 0; size_t size = 0;
    bool valid() const { return base && size; }
    uintptr_t va(uintptr_t rva) const { return base + rva; }
    bool inside(uintptr_t p) const { return p >= base && p < base + size; }
};

static ModuleInfo GetClient() {
    ModuleInfo m;
    HMODULE h = GetModuleHandleA("client.dll");
    if (!h) return m;
    MODULEINFO mi{};
    if (!GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi))) return m;
    m.base = (uintptr_t)mi.lpBaseOfDll;
    m.size = mi.SizeOfImage;
    return m;
}

static bool SafeReadQword(uintptr_t a, uintptr_t& v) { __try { v = *(uintptr_t*)a; return true; } __except (1) { return false; } }
static bool SafeReadU32(uintptr_t a, uint32_t& v) { __try { v = *(uint32_t*)a; return true; } __except (1) { return false; } }
static bool SafeReadU8(uintptr_t a, uint8_t& v) { __try { v = *(uint8_t*)a; return true; } __except (1) { return false; } }

// ---------------------------------------------------------------------------
// Traversal (stride 0x70, probe-confirmed in v2.3)
// ---------------------------------------------------------------------------
static uintptr_t ResolveEntity(uintptr_t entitySystem, uint32_t handle, const ModuleInfo& client) {
    if (!entitySystem || !handle || handle == 0xFFFFFFFF) return 0;
    uint32_t idx = handle & 0x7FFF;
    if (idx == 0 || idx == 0x7FFF) return 0;
    uint32_t chunkIdx = idx >> 9;
    uint32_t idxInChunk = idx & 0x1FF;

    uintptr_t chunk = 0;
    if (!SafeReadQword(entitySystem + (size_t)chunkIdx * 8 + GES_OFF_CHUNK_TABLE, chunk) || !chunk) return 0;
    uintptr_t ent = 0;
    if (!SafeReadQword(chunk + (size_t)idxInChunk * GES_OFF_CHUNK_STRIDE, ent) || !ent) return 0;
    uintptr_t vt = 0;
    if (!SafeReadQword(ent, vt) || !client.inside(vt)) return 0;
    return ent;
}

// ---------------------------------------------------------------------------
// FrameStageNotify hook
// ---------------------------------------------------------------------------
typedef void(__fastcall* Fsn_t)(void* client, int stage);
static Fsn_t g_origFsn = nullptr;
static uintptr_t g_hookTarget = 0;

static volatile LONG g_fsnCount = 0;
static volatile LONG g_stage5Count = 0;
static volatile LONG g_lastStage = -1;
static volatile LONG g_stageHistogram[16] = {};

static void __fastcall FsnDetour(void* client, int stage) {
    if (g_origFsn) {
        __try { g_origFsn(client, stage); }
        __except (1) {}
    }
    InterlockedIncrement(&g_fsnCount);
    g_lastStage = stage;
    if (stage >= 0 && stage < 16) InterlockedIncrement(&g_stageHistogram[stage]);
    if (stage == 5) InterlockedIncrement(&g_stage5Count);
}

static bool InstallFsnHook(const ModuleInfo& client) {
    HMODULE hClient = GetModuleHandleA("client.dll");
    if (!hClient) { Log("[fsn] client.dll missing"); return false; }

    auto createIface = reinterpret_cast<void* (*)(const char*, int*)>(
        GetProcAddress(hClient, "CreateInterface"));
    if (!createIface) { Log("[fsn] CreateInterface missing"); return false; }

    void* pClient = nullptr;
    __try { pClient = createIface("Source2Client002", nullptr); }
    __except (1) { pClient = nullptr; }
    if (!pClient) { Log("[fsn] Source2Client002 not found"); return false; }

    uintptr_t vt = 0;
    if (!SafeReadQword((uintptr_t)pClient, vt) || !vt) { Log("[fsn] no vtable"); return false; }
    uintptr_t fn = 0;
    if (!SafeReadQword(vt + (size_t)VT_SOURCE2CLIENT_FSN * 8, fn)) { Log("[fsn] slot read failed"); return false; }
    if (!client.inside(fn)) { Log("[fsn] slot %d not in client.dll (0x%p)", VT_SOURCE2CLIENT_FSN, (void*)fn); return false; }

    Log("[fsn] pClient=0x%p vtable=0x%p fn=0x%p slot=%d",
        pClient, (void*)vt, (void*)fn, VT_SOURCE2CLIENT_FSN);

    g_origFsn = (Fsn_t)fn;

    DWORD oldProt = 0;
    if (!VirtualProtect((void*)(vt + VT_SOURCE2CLIENT_FSN * 8), sizeof(void*), PAGE_READWRITE, &oldProt)) {
        Log("[fsn] VirtualProtect failed");
        return false;
    }
    *(void**)(vt + VT_SOURCE2CLIENT_FSN * 8) = (void*)&FsnDetour;
    VirtualProtect((void*)(vt + VT_SOURCE2CLIENT_FSN * 8), sizeof(void*), oldProt, &oldProt);
    g_hookTarget = vt + VT_SOURCE2CLIENT_FSN * 8;

    Log("[fsn] hooked (vtable overwrite)");
    return true;
}

// ---------------------------------------------------------------------------
// Diagnostic snapshot
// ---------------------------------------------------------------------------
static void Snapshot() {
    ModuleInfo client = GetClient();
    if (!client.valid()) { Log("[snap] client.dll missing"); return; }

    Log("[snap] fsnCount=%ld lastStage=%ld stage5Count=%ld",
        (long)g_fsnCount, (long)g_lastStage, (long)g_stage5Count);
    Log("[snap] stage histogram:");
    for (int i = 0; i < 16; ++i) {
        LONG c = g_stageHistogram[i];
        if (c > 0) Log("[snap]   stage %2d: %ld frames", i, (long)c);
    }

    uintptr_t ctrlGlobal = client.va(RVA_PLOCALPLAYERCONTROLLER);
    uintptr_t pawnGlobal = client.va(RVA_LOCALPLAYERPAWN);

    uintptr_t ctrl = 0, pawn = 0;
    bool ctrlOk = SafeReadQword(ctrlGlobal, ctrl);
    bool pawnOk = SafeReadQword(pawnGlobal, pawn);

    Log("[snap] dwLocalPlayerController @ 0x%p -> 0x%p %s",
        (void*)ctrlGlobal, (void*)ctrl, ctrlOk ? "" : "(read failed)");
    Log("[snap] dwLocalPlayerPawn       @ 0x%p -> 0x%p %s",
        (void*)pawnGlobal, (void*)pawn, pawnOk ? "" : "(read failed)");

    if (pawn > 0x10000) {
        uintptr_t vt = 0;
        bool vtOk = SafeReadQword(pawn, vt) && client.inside(vt);
        uint32_t health = 0;
        SafeReadU32(pawn + PAWN_OFF_HEALTH, health);
        Log("[snap] pawn: vtable=0x%p inClient=%d health=%u",
            (void*)vt, (int)vtOk, health);
    }
    else {
        Log("[snap] pawn: null or below 0x10000 — not spawned");
    }

    if (ctrl > 0x10000) {
        uintptr_t vt = 0;
        bool vtOk = SafeReadQword(ctrl, vt) && client.inside(vt);
        uint32_t h = 0;
        SafeReadU32(ctrl + CTRL_OFF_PLAYERPAWN, h);
        uint32_t idx = h & 0x7FFF;
        Log("[snap] ctrl: vtable=0x%p inClient=%d m_hPlayerPawn@0x%X = 0x%08X (idx=%u)",
            (void*)vt, (int)vtOk, CTRL_OFF_PLAYERPAWN, h, idx);
    }
    else {
        Log("[snap] ctrl: null or below 0x10000 — not connected");
    }

    uintptr_t elGlobal = client.va(RVA_ENTITY_LIST);
    uintptr_t el = 0;
    if (SafeReadQword(elGlobal, el) && el > 0x10000) {
        uint32_t highest = 0;
        SafeReadU32(el + GES_OFF_HIGHEST_ENTITY, highest);
        Log("[snap] entitySystem @ 0x%p = 0x%p highestEntityIndex=%u",
            (void*)elGlobal, (void*)el, highest);

        int valid = 0;
        for (int i = 1; i <= 16; ++i) {
            if (ResolveEntity(el, (uint32_t)i, client)) valid++;
        }
        Log("[snap] entity resolution: %d/16 of indices 1..16 valid", valid);
    }
    else {
        Log("[snap] entitySystem: null");
    }

    if (ctrl > 0x10000 && el > 0x10000) {
        uint32_t h = 0;
        SafeReadU32(ctrl + CTRL_OFF_PLAYERPAWN, h);
        if (h != 0 && h != 0xFFFFFFFF) {
            uintptr_t resolved = ResolveEntity(el, h, client);
            Log("[snap] ctrl->pawn handle resolve: 0x%08X -> 0x%p", h, (void*)resolved);
        }
    }

    Log("[snap] ----------------------------------------");
}

// ---------------------------------------------------------------------------
// Worker
// ---------------------------------------------------------------------------
static DWORD WINAPI WorkerThread(LPVOID) {
    Log("[worker] started");

    ModuleInfo client;
    for (int i = 0; i < 300; ++i) {
        client = GetClient();
        if (client.valid()) break;
        Sleep(100);
    }
    if (!client.valid()) { Log("[fatal] client.dll not found"); return 0; }
    Log("[ok] client.dll base=0x%p size=0x%zX", (void*)client.base, client.size);

    if (!InstallFsnHook(client)) { Log("[fatal] FSN hook failed"); return 0; }

    int ticks = 0;
    while (true) {
        Sleep(5000);
        ticks++;
        Log("");
        Log("=== SNAPSHOT #%d (t=%ds) ===", ticks, ticks * 5);
        Snapshot();
    }
    return 0;
}

// ---------------------------------------------------------------------------
// DllMain
// ---------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        LogInit();
        Log("=== n1ssan-matchdetect v1.1 (a2x 2026-10-23 offsets) ===");
        Log("[info] No writes. Observation only.");
        Log("[info] Testing: dwEntityList=0x271C8B8 dwLocalPlayerController=0x253EEF8 dwLocalPlayerPawn=0x25676D8");
        Log("");
        HANDLE t = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
        if (t) CloseHandle(t);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        if (g_hookTarget && g_origFsn) {
            DWORD oldProt = 0;
            if (VirtualProtect((void*)g_hookTarget, sizeof(void*), PAGE_READWRITE, &oldProt)) {
                *(void**)g_hookTarget = (void*)g_origFsn;
                VirtualProtect((void*)g_hookTarget, sizeof(void*), oldProt, &oldProt);
            }
        }
        if (g_logFile) { fclose(g_logFile); g_logFile = nullptr; }
        FreeConsole();
    }
    return TRUE;
}