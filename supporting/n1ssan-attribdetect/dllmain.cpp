// n1ssan-attribdetect / dllmain.cpp
// v1.4 — skins.json cross-reference for paintkit name lookup.
// READ-ONLY diagnostic DLL. No injection, no SOCache creation, no writes.

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cctype>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <Psapi.h>

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "user32.lib")

static std::string g_dllPath;

static uintptr_t g_rvaCCSInventoryManagerGet = 0;
static uintptr_t g_rvaGetItemInLoadout = 0;

static constexpr uintptr_t RVA_PLOCALPLAYERCONTROLLER = 0x253EEF8;
static constexpr uintptr_t RVA_ENTITY_LIST = 0x271C8B8;

static constexpr uintptr_t GES_OFF_CHUNK_TABLE = 0x10;
static constexpr uintptr_t GES_OFF_CHUNK_STRIDE = 0x70;
static constexpr uint32_t ENT_HANDLE_MASK = 0x7FFF;
static constexpr uint32_t ENT_CHUNK_SHIFT = 9;
static constexpr uint32_t ENT_INDEX_MASK = 0x1FF;

static constexpr uint32_t CTRL_OFF_PLAYERPAWN = 0x92C;
static constexpr uint32_t CTRL_OFF_INVSERVICES = 0x820;

static constexpr uintptr_t OFF_MGR_LOCALINVENTORY = 0x528D0;
static constexpr uintptr_t OFF_INV_OWNER = 0x10;

static constexpr uintptr_t PAWN_OFF_WEAPON_SERVICES = 0x12F0;
static constexpr uintptr_t WS_OFF_MY_WEAPONS = 0x48;
static constexpr uintptr_t WS_OFF_ACTIVE_WEAPON = 0x60;

static constexpr uintptr_t ECON_ENTITY_ATTR_MGR = 0x1290;
static constexpr uintptr_t ATTR_CONTAINER_ITEM = 0x50;

static constexpr uintptr_t EIV_OFF_ITEM_DEFINDEX = 0x1BA;
static constexpr uintptr_t EIV_OFF_ITEM_ID_HIGH = 0x1D0;
static constexpr uintptr_t EIV_OFF_ITEM_ID_LOW = 0x1D4;
static constexpr uintptr_t EIV_OFF_ACCOUNT_ID = 0x1D8;
static constexpr uintptr_t EIV_OFF_INVENTORY_POS = 0x1DC;
static constexpr uintptr_t EIV_OFF_INITIALIZED = 0x1E8;
static constexpr uintptr_t EIV_OFF_ATTRLIST = 0x208;
static constexpr uintptr_t EIV_OFF_NETATTRLIST = 0x280;
static constexpr uintptr_t EIV_OFF_CUSTOMNAME = 0x2F8;

static constexpr uintptr_t ALIST_OFF_SIZE = 0x08;
static constexpr uintptr_t ALIST_OFF_DATA = 0x10;
static constexpr uintptr_t ALIST_OFF_CAP = 0x18;

static constexpr uintptr_t VEC_OFF_SIZE = 0x00;
static constexpr uintptr_t VEC_OFF_DATA = 0x08;
static constexpr uintptr_t VEC_OFF_CAP = 0x10;

static constexpr uintptr_t CEATTR_OFF_DEFINDEX = 0x30;
static constexpr uintptr_t CEATTR_OFF_VALUE = 0x34;
static constexpr uintptr_t CEATTR_OFF_INITVALUE = 0x38;
static constexpr uintptr_t CEATTR_OFF_REFUND = 0x3C;
static constexpr uintptr_t CEATTR_OFF_SETBONUS = 0x40;

static constexpr uintptr_t ECON_ENTITY_OFF_FALLBACK_PK = 0x18A8;
static constexpr uintptr_t ECON_ENTITY_OFF_FALLBACK_SD = 0x18AC;
static constexpr uintptr_t ECON_ENTITY_OFF_FALLBACK_WR = 0x18B0;
static constexpr uintptr_t ECON_ENTITY_OFF_FALLBACK_ST = 0x18B4;

static constexpr uintptr_t INV_SERVICES_OFF_NET_LOADOUT = 0x40;
static constexpr uintptr_t INV_SERVICES_OFF_SRV_WEAPON_SLOTS = 0x88;

static constexpr int ANCHOR_DEF_USPS = 61;
static constexpr int ANCHOR_DEF_GLOCK = 4;

// ---------------------------------------------------------------------------
// Logging
// ---------------------------------------------------------------------------
static FILE* g_logFile = nullptr;
static char g_logPath[MAX_PATH]{};

static void Log(const char* fmt, ...);

static void LogInit(const char* dllPath) {
    if (!AllocConsole()) {}
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stderr), "CONOUT$", "w", stderr);
    SetConsoleTitleA("n1ssan-attribdetect v1.4");
    setvbuf(stdout, nullptr, _IONBF, 0);
    printf("[n1ssan-attribdetect] console attached\n");
    char dir[MAX_PATH]{}; strcpy_s(dir, dllPath);
    char* slash = strrchr(dir, '\\'); if (slash) *(slash + 1) = 0;
    sprintf_s(g_logPath, "%sn1ssan-attribdetect.log", dir);
    g_logFile = _fsopen(g_logPath, "w", _SH_DENYNO);
    if (!g_logFile) {
        char tmp[MAX_PATH]{}; GetTempPathA(MAX_PATH, tmp);
        sprintf_s(g_logPath, "%sn1ssan-attribdetect.log", tmp);
        g_logFile = _fsopen(g_logPath, "w", _SH_DENYNO);
    }
    printf("[n1ssan-attribdetect] log: %s\n", g_logPath);
    Log("[build] %s v1.4 compiled %s %s", __FILE__, __DATE__, __TIME__);
    Log("[build] dll path: %s", dllPath);
}

static void Log(const char* fmt, ...) {
    char buf[2048]{};
    va_list ap; va_start(ap, fmt);
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    printf("%s\n", buf);
    if (g_logFile) { fputs(buf, g_logFile); fputc('\n', g_logFile); fflush(g_logFile); }
}

// ---------------------------------------------------------------------------
// Module info / safe read
// ---------------------------------------------------------------------------
struct ModuleInfo {
    uintptr_t base = 0; size_t size = 0;
    bool valid() const { return base && size; }
    uintptr_t va(uintptr_t rva) const { return base + rva; }
    bool inside(uintptr_t p) const { return p >= base && p < base + size; }
};

static ModuleInfo GetClientModule() {
    ModuleInfo m;
    HMODULE h = GetModuleHandleA("client.dll");
    if (!h) return m;
    MODULEINFO mi{};
    if (!GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi))) return m;
    m.base = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
    m.size = mi.SizeOfImage;
    return m;
}

static bool SafeReadQword(uintptr_t a, uintptr_t& v) { __try { v = *reinterpret_cast<uintptr_t*>(a); return true; } __except (1) { return false; } }
static bool SafeReadU32(uintptr_t a, uint32_t& v) { __try { v = *reinterpret_cast<uint32_t*>(a); return true; } __except (1) { return false; } }
static bool SafeReadI32(uintptr_t a, int32_t& v) { __try { v = *reinterpret_cast<int32_t*>(a); return true; } __except (1) { return false; } }
static bool SafeReadU16(uintptr_t a, uint16_t& v) { __try { v = *reinterpret_cast<uint16_t*>(a); return true; } __except (1) { return false; } }
static bool SafeReadU8(uintptr_t a, uint8_t& v) { __try { v = *reinterpret_cast<uint8_t*>(a); return true; } __except (1) { return false; } }

static bool IsInText(uintptr_t p, const ModuleInfo& mod) {
    if (!mod.inside(p)) return false;
    __try {
        const IMAGE_DOS_HEADER* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(mod.base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
        const IMAGE_NT_HEADERS* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(mod.base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE) return false;
        const IMAGE_SECTION_HEADER* sec = IMAGE_FIRST_SECTION(nt);
        for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++sec) {
            if (memcmp(sec->Name, ".text", 5) == 0) {
                uintptr_t start = mod.base + sec->VirtualAddress;
                uintptr_t end = start + sec->Misc.VirtualSize;
                return p >= start && p < end;
            }
        }
    }
    __except (1) { return false; }
    return false;
}

static const char* ClassifyPtr(uintptr_t v, const ModuleInfo& mod) {
    if (!v) return "null";
    if (v < 0x10000) return "small";
    if (mod.inside(v)) return IsInText(v, mod) ? "client.text" : "client.data";
    if (v >= 0x10000000000ULL) return "heap";
    return "other";
}

// ---------------------------------------------------------------------------
// MinHook
// ---------------------------------------------------------------------------
typedef int (WINAPI* MH_Initialize_t)();
typedef int (WINAPI* MH_CreateHook_t)(LPVOID, LPVOID, LPVOID*);
typedef int (WINAPI* MH_EnableHook_t)(LPVOID);
typedef int (WINAPI* MH_DisableHook_t)(LPVOID);

static MH_Initialize_t MH_Initialize_p = nullptr;
static MH_CreateHook_t MH_CreateHook_p = nullptr;
static MH_EnableHook_t MH_EnableHook_p = nullptr;
static MH_DisableHook_t MH_DisableHook_p = nullptr;
static HMODULE g_hMinHook = nullptr;

static bool LoadMinHook() {
    static const char* names[] = { "MinHook.x64.dll", "minhook.dll", "MinHook.dll" };
    char dir[MAX_PATH]{}; strcpy_s(dir, g_dllPath.c_str());
    char* slash = strrchr(dir, '\\'); if (slash) *(slash + 1) = 0;
    for (const char* n : names) {
        g_hMinHook = LoadLibraryA(n); if (g_hMinHook) break;
        char full[MAX_PATH]{}; sprintf_s(full, "%s%s", dir, n);
        g_hMinHook = LoadLibraryA(full); if (g_hMinHook) break;
    }
    if (!g_hMinHook) { Log("[minhook] load failed (err=%lu)", GetLastError()); return false; }
    MH_Initialize_p = (MH_Initialize_t)GetProcAddress(g_hMinHook, "MH_Initialize");
    MH_CreateHook_p = (MH_CreateHook_t)GetProcAddress(g_hMinHook, "MH_CreateHook");
    MH_EnableHook_p = (MH_EnableHook_t)GetProcAddress(g_hMinHook, "MH_EnableHook");
    MH_DisableHook_p = (MH_DisableHook_t)GetProcAddress(g_hMinHook, "MH_DisableHook");
    if (!MH_Initialize_p || !MH_CreateHook_p || !MH_EnableHook_p) { Log("[minhook] missing exports"); return false; }
    if (MH_Initialize_p() != 0) { Log("[minhook] init failed"); return false; }
    Log("[minhook] loaded");
    return true;
}

// ---------------------------------------------------------------------------
// Pattern scanner
// ---------------------------------------------------------------------------
static constexpr size_t kMaxPatternBytes = 64;

struct ParsedPattern {
    uint8_t bytes[kMaxPatternBytes];
    bool    wildcards[kMaxPatternBytes];
    size_t  len;
};

static bool ParsePatternFixed(const char* pattern, ParsedPattern& out) {
    out.len = 0;
    const char* p = pattern;
    while (*p) {
        while (*p == ' ') ++p;
        if (!*p) break;
        if (out.len >= kMaxPatternBytes) return false;
        if (*p == '?') {
            out.bytes[out.len] = 0;
            out.wildcards[out.len] = true;
            ++out.len;
            while (*p == '?') ++p;
        }
        else {
            char hex[3] = { p[0], p[1], 0 };
            out.bytes[out.len] = (uint8_t)strtoul(hex, nullptr, 16);
            out.wildcards[out.len] = false;
            ++out.len;
            p += 2;
        }
    }
    return out.len > 0;
}

static uintptr_t ScanPatternRaw(const uint8_t* base, size_t sz,
    const uint8_t* bytes, const bool* wildcards,
    size_t patLen, int& hitCount)
{
    hitCount = 0;
    uintptr_t firstHit = 0;
    __try {
        for (size_t i = 0; i + patLen <= sz; ++i) {
            bool ok = true;
            for (size_t j = 0; j < patLen; ++j) {
                if (!wildcards[j] && base[i + j] != bytes[j]) { ok = false; break; }
            }
            if (ok) {
                if (hitCount == 0) firstHit = (uintptr_t)(base + i);
                ++hitCount;
                if (hitCount > 1) break;
            }
        }
    }
    __except (1) { hitCount = -1; return 0; }
    return firstHit;
}

static uintptr_t ScanPattern(const ModuleInfo& mod, const char* name, const char* pattern) {
    if (!mod.valid()) return 0;
    ParsedPattern pp{};
    if (!ParsePatternFixed(pattern, pp)) return 0;

    int hits = 0;
    uintptr_t hitAddr = ScanPatternRaw(reinterpret_cast<const uint8_t*>(mod.base),
        mod.size, pp.bytes, pp.wildcards, pp.len, hits);
    if (hits != 1) { Log("[scan] %s: hits=%d", name, hits); return 0; }
    return hitAddr;
}

static uintptr_t ResolveCallTarget(uintptr_t cs) {
    uint32_t raw = 0;
    __try { raw = *reinterpret_cast<uint32_t*>(cs + 1); }
    __except (1) { return 0; }
    return cs + 5 + (int32_t)raw;
}

static bool ResolveRVAs(const ModuleInfo& mod) {
    {
        uintptr_t cs = ScanPattern(mod, "CCSInvMgrGet", "E8 ? ? ? ? 48 8B D8 E8 ? ? ? ? 8B 70");
        if (cs) {
            uintptr_t t = ResolveCallTarget(cs);
            if (t && mod.inside(t)) {
                g_rvaCCSInventoryManagerGet = t - mod.base;
                Log("[rva] MgrGet +0x%zX", g_rvaCCSInventoryManagerGet);
            }
        }
    }
    {
        uintptr_t h = ScanPattern(mod, "GetItemInLoadout", "40 55 48 83 EC ? 49 63 E8");
        if (h) {
            g_rvaGetItemInLoadout = h - mod.base;
            Log("[rva] GetItemInLoadout +0x%zX", g_rvaGetItemInLoadout);
        }
    }
    return g_rvaCCSInventoryManagerGet;
}

// ---------------------------------------------------------------------------
// Entity resolution
// ---------------------------------------------------------------------------
static uintptr_t ResolveEntityByHandle(uintptr_t entitySystem, uint32_t handle, const ModuleInfo& client) {
    if (!entitySystem || !handle || handle == 0xFFFFFFFF) return 0;
    uint32_t idx = handle & ENT_HANDLE_MASK;
    if (idx == 0 || idx == ENT_HANDLE_MASK) return 0;
    uint32_t chunkIdx = idx >> ENT_CHUNK_SHIFT;
    uint32_t idxInChunk = idx & ENT_INDEX_MASK;
    uintptr_t chunkPtr = 0;
    if (!SafeReadQword(entitySystem + (size_t)chunkIdx * 8 + GES_OFF_CHUNK_TABLE, chunkPtr) || !chunkPtr) return 0;
    uintptr_t entPtr = 0;
    if (!SafeReadQword(chunkPtr + (size_t)idxInChunk * GES_OFF_CHUNK_STRIDE, entPtr) || !entPtr) return 0;
    uintptr_t vt = 0;
    if (!SafeReadQword(entPtr, vt) || !client.inside(vt)) return 0;
    return entPtr;
}

// ---------------------------------------------------------------------------
// Weapon name table (for skins.json weapon → defIndex)
// ---------------------------------------------------------------------------
struct WeaponEntry { const char* name; uint16_t defIndex; bool isKnife; };
static const WeaponEntry kWeaponTable[] = {
    {"glock-18",4,false},{"usp-s",61,false},{"p2000",32,false},{"p250",36,false},
    {"five-seven",3,false},{"tec-9",30,false},{"cz75-auto",63,false},
    {"desert eagle",1,false},{"dual berettas",2,false},{"r8 revolver",64,false},
    {"mac-10",17,false},{"mp9",34,false},{"mp7",33,false},{"mp5-sd",23,false},
    {"ump-45",24,false},{"p90",19,false},{"pp-bizon",26,false},
    {"galil ar",13,false},{"famas",10,false},{"ak-47",7,false},
    {"m4a4",16,false},{"m4a1-s",60,false},{"ssg 08",40,false},
    {"sg 553",39,false},{"aug",8,false},{"awp",9,false},{"g3sg1",11,false},
    {"scar-20",38,false},{"nova",35,false},{"xm1014",25,false},
    {"mag-7",27,false},{"sawed-off",29,false},{"negev",28,false},{"m249",14,false},
    {"bayonet",500,true},{"classic knife",503,true},{"flip knife",505,true},
    {"gut knife",506,true},{"karambit",507,true},{"m9 bayonet",508,true},
    {"huntsman knife",509,true},{"falchion knife",512,true},{"bowie knife",514,true},
    {"butterfly knife",515,true},{"shadow daggers",516,true},{"paracord knife",517,true},
    {"survival knife",518,true},{"ursus knife",519,true},{"navaja knife",520,true},
    {"nomad knife",521,true},{"stiletto knife",522,true},{"talon knife",523,true},
    {"skeleton knife",525,true},{"kukri knife",526,true},
};

static std::string ToLower(const std::string& s) {
    std::string r = s;
    for (auto& c : r) c = (char)tolower((unsigned char)c);
    return r;
}
static std::string Trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
static const WeaponEntry* FindWeapon(const std::string& name) {
    std::string key = ToLower(Trim(name));
    for (const auto& e : kWeaponTable) if (key == e.name) return &e;
    return nullptr;
}

// ---------------------------------------------------------------------------
// skins.json load + (defIndex, paintKit) → name lookup
// ---------------------------------------------------------------------------
struct SkinEntry {
    std::string weaponName;   // "Glock-18"
    std::string skinName;     // "Block-18"
    uint16_t    defIndex;     // resolved from kWeaponTable
    int         paintKit;
    bool        defResolved;  // false if weapon name didn't match kWeaponTable
};

static std::vector<SkinEntry> g_skins;

// minimal JSON scanner — extracted from earlier versions
class JsonScanner {
public:
    const std::string& s; size_t i = 0;
    JsonScanner(const std::string& src) : s(src) {}
    void SkipWs() { while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) ++i; }
    bool Match(char c) { SkipWs(); if (i < s.size() && s[i] == c) { ++i; return true; } return false; }
    char Peek() { SkipWs(); return i < s.size() ? s[i] : '\0'; }
    std::string ParseString() {
        SkipWs(); std::string out;
        if (i >= s.size() || s[i] != '"') return out;
        ++i;
        while (i < s.size() && s[i] != '"') {
            char c = s[i];
            if (c == '\\' && i + 1 < s.size()) {
                char e = s[i + 1];
                switch (e) {
                case 'n': out.push_back('\n'); i += 2; break;
                case 't': out.push_back('\t'); i += 2; break;
                case 'r': out.push_back('\r'); i += 2; break;
                case '"': out.push_back('"'); i += 2; break;
                case '\\': out.push_back('\\'); i += 2; break;
                case '/': out.push_back('/'); i += 2; break;
                case 'u': i += 6; break;
                default: out.push_back(e); i += 2; break;
                }
            }
            else { out.push_back(c); ++i; }
        }
        if (i < s.size() && s[i] == '"') ++i;
        return out;
    }
    bool ParseInt(long long& out) {
        SkipWs(); out = 0;
        bool neg = false;
        if (i < s.size() && s[i] == '-') { neg = true; ++i; }
        if (i >= s.size() || !isdigit((unsigned char)s[i])) return false;
        while (i < s.size() && isdigit((unsigned char)s[i])) {
            out = out * 10 + (s[i] - '0'); ++i;
        }
        if (neg) out = -out;
        return true;
    }
    void SkipValue() {
        SkipWs(); if (i >= s.size()) return;
        char c = s[i];
        if (c == '"') { ParseString(); return; }
        if (c == '{') { SkipObject(); return; }
        if (c == '[') { SkipArray(); return; }
        while (i < s.size() && (isalnum((unsigned char)s[i]) || s[i] == '-' || s[i] == '+' || s[i] == '.')) ++i;
    }
    void SkipObject() {
        if (!Match('{')) return;
        while (i < s.size()) {
            SkipWs(); if (Match('}')) return;
            if (Peek() == '"') ParseString();
            if (!Match(':')) return;
            SkipValue();
            if (Match(',')) continue;
        }
    }
    void SkipArray() {
        if (!Match('[')) return;
        while (i < s.size()) {
            SkipWs(); if (Match(']')) return;
            SkipValue();
            if (Match(',')) continue;
        }
    }
};

// Parse a top-level array of skin objects.
static bool LoadSkinsJson(const char* path, std::vector<SkinEntry>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return false;
    std::stringstream ss; ss << f.rdbuf();
    std::string data = ss.str();
    if (data.empty()) return false;

    JsonScanner j(data);
    j.SkipWs();
    if (!j.Match('[')) return false;

    while (j.i < j.s.size()) {
        j.SkipWs();
        if (j.Match(']')) break;
        if (!j.Match('{')) break;

        SkinEntry e{};
        e.defIndex = 0;
        e.paintKit = 0;
        e.defResolved = false;

        while (j.i < j.s.size()) {
            j.SkipWs();
            if (j.Match('}')) break;
            std::string k = j.ParseString();
            if (!j.Match(':')) break;
            j.SkipWs();
            if (k == "weapon")          e.weaponName = j.ParseString();
            else if (k == "name")       e.skinName = j.ParseString();
            else if (k == "paintKit") {
                long long v = 0;
                if (j.ParseInt(v)) e.paintKit = (int)v;
                else j.SkipValue();
            }
            else j.SkipValue();
            j.Match(',');
        }

        if (!e.weaponName.empty()) {
            const WeaponEntry* wi = FindWeapon(e.weaponName);
            if (wi) { e.defIndex = wi->defIndex; e.defResolved = true; }
            out.push_back(e);
        }
        j.Match(',');
    }
    return true;
}

static std::string g_skinsLoadedFrom;

static void LoadSkinsForLookup() {
    std::vector<std::string> candidates;
    candidates.push_back("E:\\Vibe Coding\\CS2 Lore\\CS2 Lore\\n1ssan\\src\\skins.json");

    char dir[MAX_PATH]{}; strcpy_s(dir, g_dllPath.c_str());
    char* slash = strrchr(dir, '\\'); if (slash) *(slash + 1) = 0;
    {
        char p[MAX_PATH]{}; sprintf_s(p, "%sskins.json", dir); candidates.push_back(p);
    }
    {
        char p[MAX_PATH]{}; sprintf_s(p, "%s..\\..\\..\\n1ssan\\src\\skins.json", dir); candidates.push_back(p);
    }

    for (const auto& c : candidates) {
        std::vector<SkinEntry> tmp;
        if (LoadSkinsJson(c.c_str(), tmp) && !tmp.empty()) {
            g_skins = std::move(tmp);
            g_skinsLoadedFrom = c;
            break;
        }
    }

    if (g_skins.empty()) {
        Log("[skins] failed to load skins.json from any candidate path");
        return;
    }

    int resolved = 0;
    int unresolved = 0;
    std::set<std::string> unresolvedNames;
    for (const auto& e : g_skins) {
        if (e.defResolved) ++resolved;
        else { ++unresolved; unresolvedNames.insert(e.weaponName); }
    }
    Log("[skins] loaded %zu entries from %s", g_skins.size(), g_skinsLoadedFrom.c_str());
    Log("[skins]   %d entries with resolved defIndex, %d unresolved", resolved, unresolved);
    for (const auto& n : unresolvedNames) {
        Log("[skins]   UNRESOLVED weapon name: \"%s\" (no kWeaponTable match)", n.c_str());
    }
}

// Look up skin name from (defIndex, paintKit).
// Rules:
//   exact = (defIndex match AND paintKit match)  → "exact match"
//   paint-only = paintKit match but defIndex differs → flag weapon-mismatch
static const SkinEntry* LookupSkin(uint16_t defIndex, int paintKit,
    const SkinEntry** outPaintOnlyMatch) {
    if (outPaintOnlyMatch) *outPaintOnlyMatch = nullptr;
    const SkinEntry* exact = nullptr;
    const SkinEntry* paintOnly = nullptr;
    for (const auto& e : g_skins) {
        if (e.paintKit != paintKit) continue;
        if (e.defResolved && e.defIndex == defIndex) { exact = &e; break; }
        if (!paintOnly) paintOnly = &e;
    }
    if (outPaintOnlyMatch) *outPaintOnlyMatch = paintOnly;
    return exact;
}

// ---------------------------------------------------------------------------
// Entry plausibility + stride scoring
// ---------------------------------------------------------------------------
struct EntryProbe {
    uint16_t defIdx;
    float    val;
    float    init;
    bool     isClean;
    bool     isReal;
};

static EntryProbe ProbeEntry(uintptr_t base, uintptr_t stride, uint64_t i, const ModuleInfo& mod) {
    EntryProbe p{};
    uintptr_t e = base + i * stride;
    uint32_t vraw = 0, iraw = 0;
    SafeReadU16(e + CEATTR_OFF_DEFINDEX, p.defIdx);
    SafeReadU32(e + CEATTR_OFF_VALUE, vraw);
    SafeReadU32(e + CEATTR_OFF_INITVALUE, iraw);
    p.val = *reinterpret_cast<float*>(&vraw);
    p.init = *reinterpret_cast<float*>(&iraw);
    (void)mod;

    bool defReal = (p.defIdx >= 1 && p.defIdx <= 1000);
    bool valFinite = (p.val == p.val) && (p.val > -1e9f && p.val < 1e9f);
    bool initFinite = (p.init == p.init);

    p.isReal = defReal && valFinite;
    p.isClean = p.isReal
        || (p.defIdx == 0 && p.val == 0.0f && p.init == 0.0f && initFinite);
    return p;
}

static uintptr_t ChooseStride(uintptr_t data, uint64_t size, const ModuleInfo& mod) {
    const uintptr_t candidates[] = { 0x38, 0x40, 0x48, 0x50 };
    uintptr_t bestStride = 0;
    int       bestScore = -1;
    int       bestRealCount = 0;

    for (uintptr_t stride : candidates) {
        int cleanCount = 0;
        int realCount = 0;
        uint64_t checkN = size < 8 ? size : 8;
        for (uint64_t i = 0; i < checkN; ++i) {
            EntryProbe p = ProbeEntry(data, stride, i, mod);
            if (p.isClean) ++cleanCount;
            if (p.isReal)  ++realCount;
        }
        Log("[stride-score] 0x%02zX: clean=%d/%llu real=%d/%llu",
            stride, cleanCount, (unsigned long long)checkN, realCount, (unsigned long long)checkN);
        if (cleanCount > bestScore || (cleanCount == bestScore && realCount > bestRealCount)) {
            bestScore = cleanCount;
            bestRealCount = realCount;
            bestStride = stride;
        }
    }
    return bestStride;
}

// ---------------------------------------------------------------------------
// Attribute-list dump
// ---------------------------------------------------------------------------
static void DumpAttrList(const char* tag, uintptr_t listBase, const ModuleInfo& mod,
    uint16_t weaponDefIndex) {
    uintptr_t size = 0, data = 0, cap = 0;
    SafeReadQword(listBase + ALIST_OFF_SIZE, size);
    SafeReadQword(listBase + ALIST_OFF_DATA, data);
    SafeReadQword(listBase + ALIST_OFF_CAP, cap);
    Log("[%s] @0x%p size=%llu data=0x%p cap=%llu",
        tag, (void*)listBase, (unsigned long long)size, (void*)data, (unsigned long long)cap);

    if (size == 0 || size > 128) return;
    if (data < 0x10000 || mod.inside(data)) {
        Log("[%s] >> data ptr not plausible (class=%s), cannot walk",
            tag, ClassifyPtr(data, mod));
        return;
    }

    uintptr_t chosenStride = ChooseStride(data, size, mod);
    if (!chosenStride) {
        Log("[%s] >> no stride produced clean entries. Dumping raw 0x60 bytes at data:", tag);
        for (size_t off = 0; off < 0x60; off += 0x10) {
            uintptr_t q0 = 0, q1 = 0;
            SafeReadQword(data + off, q0);
            SafeReadQword(data + off + 0x08, q1);
            Log("[%s]   raw +0x%02zX: %016llX %016llX | %s / %s",
                tag, off, (unsigned long long)q0, (unsigned long long)q1,
                ClassifyPtr(q0, mod), ClassifyPtr(q1, mod));
        }
        return;
    }

    Log("[%s] >> using stride 0x%02zX, walking %llu entries",
        tag, chosenStride, (unsigned long long)size);

    for (uint64_t i = 0; i < size && i < 32; ++i) {
        uintptr_t e = data + i * chosenStride;
        uint16_t defIdx = 0;
        uint32_t valRaw = 0, initRaw = 0, refund = 0;
        uint8_t setBonus = 0;
        SafeReadU16(e + CEATTR_OFF_DEFINDEX, defIdx);
        SafeReadU32(e + CEATTR_OFF_VALUE, valRaw);
        SafeReadU32(e + CEATTR_OFF_INITVALUE, initRaw);
        SafeReadU32(e + CEATTR_OFF_REFUND, refund);
        SafeReadU8(e + CEATTR_OFF_SETBONUS, setBonus);
        float val = *reinterpret_cast<float*>(&valRaw);
        float init = *reinterpret_cast<float*>(&initRaw);

        // Base hint
        std::string hint;
        if (defIdx == 6) hint = " [PAINTKIT]";
        else if (defIdx == 7) hint = " [def 7 candidate — seed?]";
        else if (defIdx == 8) hint = " [def 8 candidate — wear?]";
        else if (defIdx == 80 || defIdx == 81 || defIdx == 82) hint = " [def 80/81/82 candidate]";
        else if (defIdx == 0 && val == 0.0f && init == 0.0f) hint = " [padding/empty]";

        // If paintkit, cross-reference skins.json
        if (defIdx == 6) {
            int pkInt = (int)(val + 0.5f);
            const SkinEntry* paintOnly = nullptr;
            const SkinEntry* exact = LookupSkin(weaponDefIndex, pkInt, &paintOnly);
            if (exact) {
                hint += " -> \"";
                hint += exact->skinName;
                hint += "\" (";
                hint += exact->weaponName;
                hint += ")";
            }
            else if (paintOnly) {
                hint += " -> paintKit ";
                hint += std::to_string(pkInt);
                hint += " matches \"";
                hint += paintOnly->skinName;
                hint += "\" (";
                hint += paintOnly->weaponName;
                hint += ") but defIndex mismatch!";
            }
            else {
                hint += " -> paintKit ";
                hint += std::to_string(pkInt);
                hint += " not in skins.json";
            }
        }

        Log("[%s]   entry[%llu] @0x%p def=%u val=%.6f init=%.6f refund=%u setBonus=%u%s",
            tag, (unsigned long long)i, (void*)e, defIdx, val, init,
            refund, setBonus, hint.c_str());
    }
}

// ---------------------------------------------------------------------------
// Main dump function — one held weapon
// ---------------------------------------------------------------------------
static void DumpHeldWeapon(uintptr_t weaponEnt, uintptr_t ctrl, const ModuleInfo& mod,
    uintptr_t inventory, bool& isAnchor)
{
    isAnchor = false;

    uintptr_t view = weaponEnt + ECON_ENTITY_ATTR_MGR + ATTR_CONTAINER_ITEM;

    uint16_t defIdx = 0;
    uint32_t idHi = 0, idLo = 0, accountId = 0, invPos = 0;
    uint8_t init = 0;
    SafeReadU16(view + EIV_OFF_ITEM_DEFINDEX, defIdx);
    SafeReadU32(view + EIV_OFF_ITEM_ID_HIGH, idHi);
    SafeReadU32(view + EIV_OFF_ITEM_ID_LOW, idLo);
    SafeReadU32(view + EIV_OFF_ACCOUNT_ID, accountId);
    SafeReadU32(view + EIV_OFF_INVENTORY_POS, invPos);
    SafeReadU8(view + EIV_OFF_INITIALIZED, init);

    isAnchor = (defIdx == ANCHOR_DEF_USPS || defIdx == ANCHOR_DEF_GLOCK);
    const char* anchorName = (defIdx == ANCHOR_DEF_USPS) ? "USP-S" :
        (defIdx == ANCHOR_DEF_GLOCK) ? "Glock-18" : nullptr;

    Log("");
    Log("================================================================");
    if (isAnchor) {
        Log("[anchor] === HELD WEAPON: %s (def=%u) ===", anchorName, defIdx);
    }
    else {
        Log("[held] === HELD WEAPON (def=%u) ===", defIdx);
    }
    Log("================================================================");
    Log("[held] weaponEnt=0x%p view=0x%p", (void*)weaponEnt, (void*)view);

    Log("[held] def=%u idHi=0x%08X idLo=0x%08X (id=0x%08X%08X) account=0x%08X invPos=%u init=%u",
        defIdx, idHi, idLo, idHi, idLo, accountId, invPos, init);

    {
        char nameBuf[161] = {};
        __try {
            memcpy(nameBuf, (void*)(view + EIV_OFF_CUSTOMNAME), 160);
            nameBuf[160] = 0;
        }
        __except (1) {}
        if (nameBuf[0]) Log("[held] customName=\"%s\"", nameBuf);
    }

    int32_t pk = 0, sd = 0, st = 0;
    uint32_t wrRaw = 0;
    SafeReadI32(weaponEnt + ECON_ENTITY_OFF_FALLBACK_PK, pk);
    SafeReadI32(weaponEnt + ECON_ENTITY_OFF_FALLBACK_SD, sd);
    SafeReadU32(weaponEnt + ECON_ENTITY_OFF_FALLBACK_WR, wrRaw);
    SafeReadI32(weaponEnt + ECON_ENTITY_OFF_FALLBACK_ST, st);
    float wr = *reinterpret_cast<float*>(&wrRaw);
    Log("[fallback] pk=%d sd=%d wr=%.6f stattrak=%d", pk, sd, wr, st);

    Log("[held] --- C_EconItemView::m_AttributeList (+0x208) ---");
    DumpAttrList("attrlist", view + EIV_OFF_ATTRLIST, mod, defIdx);

    Log("[held] --- C_EconItemView::m_NetworkedDynamicAttributes (+0x280) ---");
    DumpAttrList("netattr", view + EIV_OFF_NETATTRLIST, mod, defIdx);

    if (ctrl) {
        uintptr_t invServices = 0;
        if (SafeReadQword(ctrl + CTRL_OFF_INVSERVICES, invServices) && invServices >= 0x10000) {
            Log("[held] --- CCSPlayerController_InventoryServices @0x%p ---", (void*)invServices);

            uintptr_t nlSize = 0, nlData = 0, nlCap = 0;
            SafeReadQword(invServices + INV_SERVICES_OFF_NET_LOADOUT + VEC_OFF_SIZE, nlSize);
            SafeReadQword(invServices + INV_SERVICES_OFF_NET_LOADOUT + VEC_OFF_DATA, nlData);
            SafeReadQword(invServices + INV_SERVICES_OFF_NET_LOADOUT + VEC_OFF_CAP, nlCap);
            Log("[loadout] m_vecNetworkableLoadout size=%llu data=0x%p cap=%llu",
                (unsigned long long)nlSize, (void*)nlData, (unsigned long long)nlCap);

            if (nlData >= 0x10000 && !mod.inside(nlData) && nlSize > 0 && nlSize < 256) {
                for (uint64_t i = 0; i < nlSize && i < 64; ++i) {
                    uintptr_t entry = nlData + i * 0x10;
                    uintptr_t pItem = 0;
                    uint16_t team = 0, slot = 0;
                    SafeReadQword(entry + 0x00, pItem);
                    SafeReadU16(entry + 0x08, team);
                    SafeReadU16(entry + 0x0A, slot);

                    if (pItem < 0x10000) continue;

                    uint16_t eDef = 0;
                    uint32_t eIdHi = 0, eIdLo = 0;
                    SafeReadU16(pItem + EIV_OFF_ITEM_DEFINDEX, eDef);
                    SafeReadU32(pItem + EIV_OFF_ITEM_ID_HIGH, eIdHi);
                    SafeReadU32(pItem + EIV_OFF_ITEM_ID_LOW, eIdLo);

                    const char* mark = (eDef == defIdx) ? " <<< MATCHES HELD DEF" : "";
                    Log("[loadout]   [%llu] pItem=0x%p team=%u slot=%u def=%u id=0x%08X%08X%s",
                        (unsigned long long)i, (void*)pItem, team, slot, eDef, eIdHi, eIdLo, mark);

                    if (eDef == defIdx && pItem != view) {
                        Log("[loadout]   ^^^ DIFFERENT view pointer for held def! dumping its attrlists:");
                        DumpAttrList("loadout-item.attrlist", pItem + EIV_OFF_ATTRLIST, mod, eDef);
                        DumpAttrList("loadout-item.netattr", pItem + EIV_OFF_NETATTRLIST, mod, eDef);
                    }
                }
            }
            else if (nlSize > 0 && (nlData < 0x10000 || mod.inside(nlData))) {
                Log("[loadout] >> data ptr implausible (class=%s), cannot walk",
                    ClassifyPtr(nlData, mod));
            }

            uintptr_t swSize = 0, swData = 0, swCap = 0;
            SafeReadQword(invServices + INV_SERVICES_OFF_SRV_WEAPON_SLOTS + VEC_OFF_SIZE, swSize);
            SafeReadQword(invServices + INV_SERVICES_OFF_SRV_WEAPON_SLOTS + VEC_OFF_DATA, swData);
            SafeReadQword(invServices + INV_SERVICES_OFF_SRV_WEAPON_SLOTS + VEC_OFF_CAP, swCap);
            Log("[srvslots] m_vecServerAuthoritativeWeaponSlots size=%llu data=0x%p cap=%llu",
                (unsigned long long)swSize, (void*)swData, (unsigned long long)swCap);

            if (swData >= 0x10000 && !mod.inside(swData) && swSize > 0 && swSize < 256) {
                const uintptr_t stride = 0x38;
                for (uint64_t i = 0; i < swSize && i < 64; ++i) {
                    uintptr_t entry = swData + i * stride;
                    uint16_t unClass = 0, unSlot = 0, unDef = 0;
                    SafeReadU16(entry + 0x30, unClass);
                    SafeReadU16(entry + 0x32, unSlot);
                    SafeReadU16(entry + 0x34, unDef);
                    const char* mark = (unDef == defIdx) ? " <<< MATCHES HELD DEF" : "";
                    Log("[srvslots]   [%llu] class=%u slot=%u def=%u%s",
                        (unsigned long long)i, unClass, unSlot, unDef, mark);
                }
            }
            else if (swSize > 0 && (swData < 0x10000 || mod.inside(swData))) {
                Log("[srvslots] >> data ptr implausible (class=%s), cannot walk",
                    ClassifyPtr(swData, mod));
            }
        }
    }

    Log("[held] === END DUMP ===");
    Log("");
}

// ---------------------------------------------------------------------------
// FSN hook + weapon-swap detection
// ---------------------------------------------------------------------------
typedef void* (*CreateInterface_t)(const char* name, int* ret);
typedef void(__fastcall* FrameStageNotify_t)(void* pClient, int stage);

static FrameStageNotify_t g_origFrameStageNotify = nullptr;
static void* g_pClient = nullptr;
static int g_frameStageNotifySlot = -1;
static constexpr int VT_SOURCE2CLIENT_FRAMESTAGENOTIFY = 36;

static uint32_t g_lastActiveHandle = 0xFFFFFFFF;
static uintptr_t g_lastWeaponEnt = 0;

static void OnTickCheckHeldWeapon(const ModuleInfo& mod) {
    uintptr_t ctrl = 0;
    if (!SafeReadQword(mod.va(RVA_PLOCALPLAYERCONTROLLER), ctrl) || ctrl < 0x10000) return;

    uint32_t hPawn = 0;
    if (!SafeReadU32(ctrl + CTRL_OFF_PLAYERPAWN, hPawn) || !hPawn || hPawn == 0xFFFFFFFF) return;

    uintptr_t el = 0;
    if (!SafeReadQword(mod.va(RVA_ENTITY_LIST), el) || el < 0x10000) return;

    uintptr_t pawn = ResolveEntityByHandle(el, hPawn, mod);
    if (!pawn) return;

    uintptr_t ws = 0;
    if (!SafeReadQword(pawn + PAWN_OFF_WEAPON_SERVICES, ws) || ws < 0x10000 || ws == pawn) return;

    uint32_t activeHandle = 0;
    if (!SafeReadU32(ws + WS_OFF_ACTIVE_WEAPON, activeHandle)) return;
    if (activeHandle == 0xFFFFFFFF || activeHandle == 0) return;

    if (activeHandle == g_lastActiveHandle) return;

    uintptr_t weaponEnt = ResolveEntityByHandle(el, activeHandle, mod);
    if (!weaponEnt) return;

    if (weaponEnt == g_lastWeaponEnt && activeHandle == g_lastActiveHandle) return;

    g_lastActiveHandle = activeHandle;
    g_lastWeaponEnt = weaponEnt;

    uintptr_t inventory = 0;
    {
        using Fn_ManagerGet = void* (*)();
        auto fnManagerGet = reinterpret_cast<Fn_ManagerGet>(mod.va(g_rvaCCSInventoryManagerGet));
        __try {
            void* mgr = fnManagerGet();
            if (mgr) {
                SafeReadQword((uintptr_t)mgr + OFF_MGR_LOCALINVENTORY, inventory);
                if (inventory < 0x10000) inventory = 0;
            }
        }
        __except (1) { inventory = 0; }
    }

    Log("[swap] new active weapon handle=0x%08X ent=0x%p", activeHandle, (void*)weaponEnt);

    bool isAnchor = false;
    __try {
        DumpHeldWeapon(weaponEnt, ctrl, mod, inventory, isAnchor);
    }
    __except (1) {
        Log("[swap] dump threw");
    }
    if (isAnchor) {
        Log("[anchor] ^^^^ THIS IS AN ANCHOR WEAPON — compare with what you see in-game ^^^^");
    }
}

static void __fastcall OnFrameStage(void* pClient, int stage) {
    if (g_origFrameStageNotify) { __try { g_origFrameStageNotify(pClient, stage); } __except (1) {} }
    static bool firstFire = true;
    if (firstFire) { firstFire = false; Log("[fsn] first fire, stage=%d", stage); }
    if (stage != 6) return;

    ModuleInfo mod = GetClientModule();
    if (!mod.valid()) return;
    __try { OnTickCheckHeldWeapon(mod); }
    __except (1) {}
}

static bool InstallFrameStageNotifyHook(const ModuleInfo& client) {
    HMODULE hClient = GetModuleHandleA("client.dll");
    if (!hClient) return false;
    auto createIface = reinterpret_cast<CreateInterface_t>(GetProcAddress(hClient, "CreateInterface"));
    if (!createIface) return false;
    void* pClient = nullptr;
    __try { pClient = createIface("Source2Client002", nullptr); }
    __except (1) { pClient = nullptr; }
    if (!pClient) { Log("[fsn] Source2Client002 not found"); return false; }
    g_pClient = pClient;
    uintptr_t vt = 0;
    if (!SafeReadQword((uintptr_t)pClient, vt) || !vt) { Log("[fsn] no vtable"); return false; }
    uintptr_t fnAddr = 0;
    if (!SafeReadQword(vt + (size_t)VT_SOURCE2CLIENT_FRAMESTAGENOTIFY * 8, fnAddr)) { Log("[fsn] vtable read failed"); return false; }
    if (!client.inside(fnAddr)) { Log("[fsn] slot not inside client.dll"); return false; }
    if (MH_CreateHook_p((LPVOID)fnAddr, (LPVOID)&OnFrameStage, (LPVOID*)&g_origFrameStageNotify) != 0) {
        Log("[fsn] MH_CreateHook failed"); return false;
    }
    if (MH_EnableHook_p((LPVOID)fnAddr) != 0) {
        Log("[fsn] MH_EnableHook failed"); return false;
    }
    g_frameStageNotifySlot = VT_SOURCE2CLIENT_FRAMESTAGENOTIFY;
    Log("[fsn] hooked");
    return true;
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
static DWORD WINAPI InjectThread(LPVOID) {
    Log("=== n1ssan-attribdetect v1.4 ===");
    Log("Anchors: def=61 (USP-S), def=4 (Glock-18)");
    Log("Confirmed: def=6 = paintkit, CAttributeList layout = { ?, size@+0x08, data@+0x10, cap@+0x18 }");
    Log("Cross-references skins.json for paintkit name lookup.");

    LoadSkinsForLookup();

    if (!LoadMinHook()) { Log("[fatal] MinHook unavailable"); return 0; }

    ModuleInfo mod;
    for (int i = 0; i < 300; ++i) { mod = GetClientModule(); if (mod.valid()) break; Sleep(100); }
    if (!mod.valid()) { Log("[fatal] client.dll not found"); return 0; }
    Log("[ok] client.dll base=0x%p size=0x%zX", (void*)mod.base, mod.size);

    if (!InstallFrameStageNotifyHook(mod)) { Log("[fatal] FSN hook failed"); return 0; }
    if (!ResolveRVAs(mod)) { Log("[warn] some RVAs unresolved — continuing, loadout dumps may be limited"); }

    Log("[ready] waiting for weapon swaps...");
    while (true) { Sleep(10000); }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        char dllPath[MAX_PATH]{};
        GetModuleFileNameA(hModule, dllPath, MAX_PATH);
        g_dllPath = dllPath;
        LogInit(dllPath);
        HANDLE t = CreateThread(nullptr, 0, InjectThread, nullptr, 0, nullptr);
        if (t) CloseHandle(t);
    }
    else if (reason == DLL_PROCESS_DETACH) {
        if (g_hMinHook) { FreeLibrary(g_hMinHook); g_hMinHook = nullptr; }
        if (g_logFile) { fclose(g_logFile); g_logFile = nullptr; }
        FreeConsole();
    }
    return TRUE;
}