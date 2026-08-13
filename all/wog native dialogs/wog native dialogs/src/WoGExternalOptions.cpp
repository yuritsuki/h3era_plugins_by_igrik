#include "WoGExternalOptionApi.h"

#include <cctype>
#include <cstring>
#include <string>

void InvalidateWoGOptionLocks();

namespace WoGExternalOptions
{
    const int PAGE_COUNT = 8;
    const int GROUP_COUNT = 4;
    const int ITEM_LIMIT = 20;
    const int REGISTRY_LIMIT = 32;
    const int KEY_LIMIT = 96;
    const int REASON_LIMIT = 1024;

    enum LockState
    {
        LOCK_NONE,
        LOCK_APPLIED,
        LOCK_FAILED
    };

    struct Provider
    {
        bool used;
        char key[KEY_LIMIT];
        WogExternalCheckboxV2 descriptor;
        LockState lockState;
        bool forcedChecked;
        char lockReason[REASON_LIMIT];
    };

    struct SessionRow
    {
        Provider* provider;
        uint32_t state;
    };

    struct SessionGroup
    {
        int count;
        SessionRow rows[ITEM_LIMIT];
    };

    Provider providers[REGISTRY_LIMIT] = {};
    SessionGroup session[PAGE_COUNT][GROUP_COUNT] = {};
    Provider* begunProviders[REGISTRY_LIMIT] = {};
    int begunProviderCount = 0;
    bool sessionActive = false;
    bool publishedV2 = false;

    bool IsExecutableAddress(const void* address)
    {
        if (!address)
            return false;

        MEMORY_BASIC_INFORMATION memory = {};
        if (!VirtualQuery(address, &memory, sizeof(memory)) || memory.State != MEM_COMMIT)
            return false;

        const DWORD protection = memory.Protect & 0xFF;
        return protection == PAGE_EXECUTE || protection == PAGE_EXECUTE_READ ||
            protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
    }

    bool CopyAndValidateKey(const char* source, char* destination)
    {
        if (!source || !destination)
            return false;

        __try
        {
            size_t length = 0;
            while (length < KEY_LIMIT && source[length])
            {
                const unsigned char ch = (unsigned char)source[length];
                if (ch < 0x21 || ch > 0x7E)
                    return false;
                ++length;
            }
            if (!length || length >= KEY_LIMIT)
                return false;
            memcpy(destination, source, length + 1);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    bool SameDescriptor(const Provider& provider,
        const WogExternalCheckboxV2& descriptor)
    {
        const WogExternalCheckboxV2& current = provider.descriptor;
        return current.optionId == descriptor.optionId &&
            current.pageIndex == descriptor.pageIndex &&
            current.groupIndex == descriptor.groupIndex &&
            current.context == descriptor.context &&
            current.BeginMenuSession == descriptor.BeginMenuSession &&
            current.EndMenuSession == descriptor.EndMenuSession &&
            current.QueryState == descriptor.QueryState &&
            current.GetText == descriptor.GetText &&
            current.SetChecked == descriptor.SetChecked &&
            current.ApplyDefault == descriptor.ApplyDefault &&
            current.SetEnforcedState == descriptor.SetEnforcedState;
    }

    bool IsValidDescriptor(const WogExternalCheckboxV2& descriptor)
    {
        if (descriptor.structSize < sizeof(WogExternalCheckboxV2) ||
            descriptor.abiVersion != WOG_EXTERNAL_OPTIONS_ABI_V2 ||
            descriptor.pageIndex >= PAGE_COUNT || descriptor.groupIndex >= GROUP_COUNT ||
            descriptor.optionId < 1000 || descriptor.optionId > 9999)
            return false;

        for (int i = 0; i < 2; ++i)
        {
            if (descriptor.reserved[i])
                return false;
        }

        if (!IsExecutableAddress((const void*)descriptor.SetEnforcedState))
            return false;

        return IsExecutableAddress((const void*)descriptor.BeginMenuSession) &&
            (!descriptor.EndMenuSession || IsExecutableAddress((const void*)descriptor.EndMenuSession)) &&
            IsExecutableAddress((const void*)descriptor.QueryState) &&
            IsExecutableAddress((const void*)descriptor.GetText) &&
            IsExecutableAddress((const void*)descriptor.SetChecked) &&
            IsExecutableAddress((const void*)descriptor.ApplyDefault);
    }

    int FindFreeProvider()
    {
        for (int i = 0; i < REGISTRY_LIMIT; ++i)
        {
            if (!providers[i].used)
                return i;
        }
        return -1;
    }

    bool HasOptionIdConflict(int optionId)
    {
        for (int i = 0; i < REGISTRY_LIMIT; ++i)
        {
            if (providers[i].used && providers[i].descriptor.optionId == optionId)
                return true;
        }
        return false;
    }

    int32_t __stdcall RegisterCheckboxV2(const WogExternalCheckboxV2* source)
    {
        WogExternalCheckboxV2 descriptor = {};
        char key[KEY_LIMIT] = {};
        __try
        {
            if (!source || source->structSize < sizeof(WogExternalCheckboxV2))
                return WOG_EXTERNAL_REGISTER_INVALID;
            descriptor = *source;
            if (!CopyAndValidateKey(source->stableKey, key))
                return WOG_EXTERNAL_REGISTER_INVALID;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return WOG_EXTERNAL_REGISTER_INVALID;
        }

        if (!IsValidDescriptor(descriptor))
            return WOG_EXTERNAL_REGISTER_INVALID;

        for (int i = 0; i < REGISTRY_LIMIT; ++i)
        {
            if (!providers[i].used || strcmp(providers[i].key, key))
                continue;
            const bool identical = SameDescriptor(providers[i], descriptor);
            return identical ? WOG_EXTERNAL_ALREADY_REGISTERED :
                WOG_EXTERNAL_REGISTER_CONFLICT;
        }
        if (HasOptionIdConflict(descriptor.optionId))
            return WOG_EXTERNAL_REGISTER_CONFLICT;

        const int index = FindFreeProvider();
        if (index < 0)
            return WOG_EXTERNAL_REGISTER_FULL;

        Provider& provider = providers[index];
        provider.used = true;
        strcpy_s(provider.key, key);
        provider.descriptor = descriptor;
        provider.descriptor.stableKey = provider.key;

        // Registration commonly occurs from another plugin's DllMain. Mark
        // the JSON cache dirty, but defer every provider callback until WoG's
        // normal setup synchronization or menu-open path.
        InvalidateWoGOptionLocks();
        return WOG_EXTERNAL_REGISTERED;
    }

    WogExternalOptionsApiV2 apiV2 =
    {
        WOG_EXTERNAL_OPTIONS_API_V2_MAGIC,
        sizeof(WogExternalOptionsApiV2),
        WOG_EXTERNAL_OPTIONS_ABI_V2,
        RegisterCheckboxV2,
        { 0, 0, 0, 0 }
    };

    bool PublishApi(Patcher* patcher, const char* name, _dword_ value)
    {
        Variable* variable = patcher->VarFind((char*)name);
        if (!variable)
            variable = patcher->VarInit((char*)name, value);
        else if (!variable->GetValue())
            variable->SetValue(value);
        return variable && variable->GetValue() == value;
    }

    void Publish(Patcher* patcher)
    {
        if (!patcher)
            return;
        if (!publishedV2)
            publishedV2 = PublishApi(patcher, WOG_EXTERNAL_OPTIONS_API_V2_VARIABLE,
                (_dword_)&apiV2);
    }

    bool CallBegin(Provider& provider)
    {
        __try
        {
            return provider.descriptor.BeginMenuSession(provider.descriptor.context) != 0;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    void CallEnd(Provider& provider)
    {
        if (!provider.descriptor.EndMenuSession)
            return;
        __try
        {
            provider.descriptor.EndMenuSession(provider.descriptor.context);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    uint32_t Query(Provider& provider)
    {
        __try
        {
            return provider.descriptor.QueryState(provider.descriptor.context) &
                (WOG_EXTERNAL_STATE_VISIBLE | WOG_EXTERNAL_STATE_ENABLED |
                    WOG_EXTERNAL_STATE_CHECKED);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return 0;
        }
    }

    bool TryReadJson(const char* key, std::string& value)
    {
        const char* raw = GetEraJSON(key);
        if (!raw || !strcmp(raw, key))
            return false;
        value = raw;
        return true;
    }

    bool TryParseBool(const std::string& text, bool& value)
    {
        size_t first = 0;
        while (first < text.size() && isspace((unsigned char)text[first]))
            ++first;
        size_t last = text.size();
        while (last > first && isspace((unsigned char)text[last - 1]))
            --last;
        std::string normalized = text.substr(first, last - first);
        for (size_t i = 0; i < normalized.size(); ++i)
            normalized[i] = (char)tolower((unsigned char)normalized[i]);
        if (normalized == "1" || normalized == "true" || normalized == "on" ||
            normalized == "yes")
        {
            value = true;
            return true;
        }
        if (normalized == "0" || normalized == "false" || normalized == "off" ||
            normalized == "no")
        {
            value = false;
            return true;
        }
        return false;
    }

    bool CallSetEnforcedState(Provider& provider, bool enforced, bool checked)
    {
        if (!provider.descriptor.SetEnforcedState)
            return false;
        __try
        {
            return provider.descriptor.SetEnforcedState(provider.descriptor.context,
                enforced ? 1 : 0, checked ? 1 : 0) != 0;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    void LoadLockRequest(Provider& provider, bool& requested, bool& checked,
        char* reason, size_t reasonSize)
    {
        requested = false;
        checked = false;
        if (reason && reasonSize)
            reason[0] = 0;
        char key[160] = {};
        std::string raw;
        sprintf_s(key, sizeof(key), "wnd.dlg_wog_options.locks.%d.value",
            provider.descriptor.optionId);
        if (!TryReadJson(key, raw) || !TryParseBool(raw, checked))
        {
            requested = false;
            return;
        }
        requested = true;

        sprintf_s(key, sizeof(key), "wnd.dlg_wog_options.locks.%d.reason",
            provider.descriptor.optionId);
        if (!TryReadJson(key, raw) || raw.empty())
        {
            if (!TryReadJson("wnd.dlg_wog_options.search.locked_hint", raw))
                raw.clear();
        }
        if (reason && reasonSize)
            strncpy_s(reason, reasonSize, raw.c_str(), _TRUNCATE);
    }

    // This is intentionally callable before a menu session. Providers own
    // the live policy; neither WoG's option array nor preset data is touched.
    void RefreshLockPolicies()
    {
        for (int i = 0; i < REGISTRY_LIMIT; ++i)
        {
            Provider& provider = providers[i];
            if (!provider.used)
                continue;

            bool requested = false;
            bool checked = false;
            char reason[REASON_LIMIT] = {};
            LoadLockRequest(provider, requested, checked, reason, sizeof(reason));
            if (requested)
            {
                strncpy_s(provider.lockReason, reason, _TRUNCATE);
                provider.forcedChecked = checked;
                // Revalidate on every lifecycle/menu refresh. Another plugin
                // may have removed the provider's live seam after an earlier
                // successful application.
                provider.lockState = CallSetEnforcedState(provider, true, checked)
                    ? LOCK_APPLIED : LOCK_FAILED;
            }
            else if (provider.lockState != LOCK_NONE)
            {
                // A failed clear leaves the row safely disabled. Retain its
                // last known policy state and retry at the next lifecycle or
                // menu refresh instead of claiming that enforcement ended.
                if (CallSetEnforcedState(provider, false, false))
                {
                    provider.lockState = LOCK_NONE;
                    provider.lockReason[0] = 0;
                }
            }
        }
    }

    void __stdcall OnDeferredLockRefresh(Era::TEvent*)
    {
        RefreshLockPolicies();
    }

    uint32_t ApplyLockState(Provider& provider, uint32_t state)
    {
        if (provider.lockState == LOCK_APPLIED)
        {
            state &= ~(WOG_EXTERNAL_STATE_ENABLED | WOG_EXTERNAL_STATE_CHECKED);
            if (provider.forcedChecked)
                state |= WOG_EXTERNAL_STATE_CHECKED;
        }
        else if (provider.lockState == LOCK_FAILED)
        {
            // The provider could not verify the requested effective value.
            // Keep its saved checkmark visible, but do not permit mutation or
            // label the row as successfully locked.
            state &= ~WOG_EXTERNAL_STATE_ENABLED;
        }
        return state;
    }

    int OptionId(const SessionRow& row)
    {
        return row.provider ? row.provider->descriptor.optionId : -1;
    }

    bool IsLocked(const SessionRow& row)
    {
        return row.provider && row.provider->lockState == LOCK_APPLIED;
    }

    bool IsLockFailed(const SessionRow& row)
    {
        return row.provider && row.provider->lockState == LOCK_FAILED;
    }

    const char* LockReason(const SessionRow& row)
    {
        return IsLocked(row) ? row.provider->lockReason : o_NullString;
    }

    bool LockedChecked(const SessionRow& row)
    {
        return IsLocked(row) && row.provider->forcedChecked;
    }

    const char* GetText(Provider& provider, uint32_t kind)
    {
        __try
        {
            const char* text = provider.descriptor.GetText(provider.descriptor.context, kind);
            if (!text)
                return o_NullString;
            // Validate a bounded NUL terminator before callers copy the text.
            const size_t limit = kind == WOG_EXTERNAL_TEXT_POPUP ? 8192 :
                (kind == WOG_EXTERNAL_TEXT_HINT ? 2048 : 1024);
            for (size_t i = 0; i < limit; ++i)
            {
                if (!text[i])
                    return text;
            }
            return o_NullString;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return o_NullString;
        }
    }

    void EndSession()
    {
        for (int i = begunProviderCount - 1; i >= 0; --i)
        {
            if (begunProviders[i])
                CallEnd(*begunProviders[i]);
        }
        memset(session, 0, sizeof(session));
        memset(begunProviders, 0, sizeof(begunProviders));
        begunProviderCount = 0;
        sessionActive = false;
    }

    void BeginSession(_DlgSetup_* ds)
    {
        EndSession();
        if (!ds)
            return;

        RefreshLockPolicies();
        sessionActive = true;
        for (int i = 0; i < REGISTRY_LIMIT; ++i)
        {
            Provider& provider = providers[i];
            if (!provider.used || !CallBegin(provider))
                continue;
            begunProviders[begunProviderCount++] = &provider;

            const uint32_t state = ApplyLockState(provider, Query(provider));
            if (!(state & WOG_EXTERNAL_STATE_VISIBLE))
                continue;
            // An unavailable provider behaves as if it were not registered
            // for this dialog session. A successfully applied lock is the
            // only valid reason for admitting a disabled external row.
            if (provider.lockState != LOCK_APPLIED &&
                !(state & WOG_EXTERNAL_STATE_ENABLED))
                continue;

            const int page = (int)provider.descriptor.pageIndex;
            const int group = (int)provider.descriptor.groupIndex;
            if (!ds->Pages[page] || !ds->Pages[page]->Enabled ||
                !ds->Pages[page]->ItemList[group])
                continue;

            _DlgSetup_ItemList_* list = ds->Pages[page]->ItemList[group];
            SessionGroup& target = session[page][group];
            const int nativeCount = min(max(list->ItemCount, 0), ITEM_LIMIT);
            if (nativeCount + target.count >= ITEM_LIMIT)
                continue;

            // Required display text is validated once before geometry is
            // committed for the session. Aliases may legitimately be empty.
            if (!*GetText(provider, WOG_EXTERNAL_TEXT_NAME) ||
                !*GetText(provider, WOG_EXTERNAL_TEXT_HINT) ||
                !*GetText(provider, WOG_EXTERNAL_TEXT_POPUP) ||
                !*GetText(provider, WOG_EXTERNAL_TEXT_SOURCE_BADGE))
                continue;

            target.rows[target.count].provider = &provider;
            target.rows[target.count].state = state;
            ++target.count;
        }
    }

    int Count(int page, int group)
    {
        if (!sessionActive || page < 0 || page >= PAGE_COUNT ||
            group < 0 || group >= GROUP_COUNT)
            return 0;
        return session[page][group].count;
    }

    SessionRow* At(int page, int group, int externalIndex)
    {
        if (page < 0 || page >= PAGE_COUNT || group < 0 || group >= GROUP_COUNT ||
            externalIndex < 0 || externalIndex >= session[page][group].count)
            return NULL;
        return &session[page][group].rows[externalIndex];
    }

    uint32_t Refresh(SessionRow& row)
    {
        if (!row.provider)
            return 0;
        const uint32_t refreshed = ApplyLockState(*row.provider, Query(*row.provider));
        // Visibility is fixed for one dialog session so controls never change
        // geometry while the dialog is running.
        row.state = (row.state & WOG_EXTERNAL_STATE_VISIBLE) |
            (refreshed & (WOG_EXTERNAL_STATE_ENABLED | WOG_EXTERNAL_STATE_CHECKED));
        return row.state;
    }

    bool SetChecked(SessionRow& row, bool checked, uint32_t cause)
    {
        if (!row.provider || row.provider->lockState != LOCK_NONE ||
            !(Refresh(row) & WOG_EXTERNAL_STATE_ENABLED))
            return false;
        bool success = false;
        __try
        {
            success = row.provider->descriptor.SetChecked(
                row.provider->descriptor.context, checked ? 1 : 0, cause) != 0;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            success = false;
        }
        Refresh(row);
        return success;
    }

    bool ApplyDefault(SessionRow& row)
    {
        if (!row.provider || row.provider->lockState != LOCK_NONE ||
            !(Refresh(row) & WOG_EXTERNAL_STATE_ENABLED))
            return false;
        bool success = false;
        __try
        {
            success = row.provider->descriptor.ApplyDefault(
                row.provider->descriptor.context) != 0;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            success = false;
        }
        Refresh(row);
        return success;
    }

    void ApplyPageAction(int page, uint32_t action)
    {
        if (page < 0 || page >= PAGE_COUNT)
            return;
        for (int group = 0; group < GROUP_COUNT; ++group)
        {
            SessionGroup& target = session[page][group];
            for (int i = 0; i < target.count; ++i)
            {
                if (action == WOG_EXTERNAL_CHANGE_SELECT_ALL)
                    SetChecked(target.rows[i], true, action);
                else if (action == WOG_EXTERNAL_CHANGE_CLEAR_ALL)
                    SetChecked(target.rows[i], false, action);
                else if (action == 0)
                    ApplyDefault(target.rows[i]);
            }
        }
    }
}
