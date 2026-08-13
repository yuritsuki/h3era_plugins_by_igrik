#pragma once

#include <stdint.h>

// A process-local, PE32 C ABI shared through a Patcher_x86 variable.  The
// structures contain only fixed-width fields and borrowed pointers; neither
// side allocates or frees memory owned by the other plugin.
#define WOG_EXTERNAL_OPTIONS_API_V2_VARIABLE "WoG.NativeDialogs.ExternalOptions.Api.2"
#define WOG_EXTERNAL_OPTIONS_API_V2_MAGIC    0x32584557u // "WEX2" on little-endian x86
#define WOG_EXTERNAL_OPTIONS_ABI_V2           0x00020000u

enum WogExternalOptionText
{
    WOG_EXTERNAL_TEXT_NAME = 1,
    WOG_EXTERNAL_TEXT_HINT = 2,
    WOG_EXTERNAL_TEXT_POPUP = 3,
    WOG_EXTERNAL_TEXT_SOURCE_BADGE = 4,
    WOG_EXTERNAL_TEXT_SEARCH_ALIASES = 5
};

enum WogExternalOptionState
{
    WOG_EXTERNAL_STATE_VISIBLE = 1u << 0,
    WOG_EXTERNAL_STATE_ENABLED = 1u << 1,
    WOG_EXTERNAL_STATE_CHECKED = 1u << 2
};

enum WogExternalOptionChangeCause
{
    WOG_EXTERNAL_CHANGE_DIRECT_CLICK = 1,
    WOG_EXTERNAL_CHANGE_SELECT_ALL = 2,
    WOG_EXTERNAL_CHANGE_CLEAR_ALL = 3
};

enum WogExternalOptionRegisterResult
{
    WOG_EXTERNAL_REGISTERED = 1,
    WOG_EXTERNAL_ALREADY_REGISTERED = 2,
    WOG_EXTERNAL_REGISTER_INVALID = -1,
    WOG_EXTERNAL_REGISTER_CONFLICT = -2,
    WOG_EXTERNAL_REGISTER_FULL = -3
};

#pragma pack(push, 4)

// Every externally owned row has a virtual numeric ID without entering WoG's
// native 1000-value option array or .dat serialization. IDs 1000..9999 are
// reserved for this external namespace.
struct WogExternalCheckboxV2
{
    uint32_t structSize;
    uint32_t abiVersion;
    const char* stableKey;
    int32_t optionId;
    uint32_t pageIndex;
    uint32_t groupIndex;
    void* context;

    int32_t (__stdcall *BeginMenuSession)(void* context);
    void (__stdcall *EndMenuSession)(void* context);
    uint32_t (__stdcall *QueryState)(void* context);
    const char* (__stdcall *GetText)(void* context, uint32_t textKind);
    int32_t (__stdcall *SetChecked)(void* context, int32_t checked, uint32_t cause);
    int32_t (__stdcall *ApplyDefault)(void* context);

    // Applies or clears a host-owned locked state. It must never mutate the
    // provider's persisted preference. The host passes (enforced=1,
    // checked=0|1) for a locked Off/On value; clearing passes enforced=0 and
    // checked is ignored. A nonzero return confirms the effective live policy.
    int32_t (__stdcall *SetEnforcedState)(void* context,
                                         int32_t enforced,
                                         int32_t checked);

    uint32_t reserved[2];
};

struct WogExternalOptionsApiV2
{
    uint32_t magic;
    uint32_t structSize;
    uint32_t abiVersion;
    int32_t (__stdcall *RegisterCheckbox)(const WogExternalCheckboxV2* descriptor);
    uint32_t reserved[4];
};

#pragma pack(pop)

#ifdef __cplusplus
static_assert(sizeof(void*) == 4, "The WoG external-option ABI is PE32/x86 only.");
static_assert(sizeof(WogExternalCheckboxV2) == 64,
    "Unexpected WogExternalCheckboxV2 packing.");
static_assert(sizeof(WogExternalOptionsApiV2) == 32,
    "Unexpected WogExternalOptionsApiV2 packing.");
#endif
