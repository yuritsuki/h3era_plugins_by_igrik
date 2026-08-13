/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// заполнение строк диалога WoG Опций из JSON файла ///////////////////////////

#include <cerrno>
#include <cctype>
#include <climits>
#include <cstring>
#include <cstdlib>

#define o_WogOptions ((_DlgSetup_*)0x2918390)
#define o_ChooseFile ((_ChooseFile_*)0x7B3614)

char strPage[1024];
char strList[1024];

char* textPage = "wog_options.page%d.%s";
char* list = "group%d.%s";

char* name = "name";
char* hint = "hint";
char* popUp = "popup";

char* optionsName = "wog_options.main.name";
char* optionsHint = "wog_options.main.hint";
char* optionsPopUp = "wog_options.main.popup";
char* optionsIntro = "wog_options.main.intro";

/////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////// WoG option lock configuration and enforcement /////////////////////

#define o_WogOptionNumbers ((int*)0x2919030)
#define o_WogOptionValues  ((int*)0x2771920)

namespace WogOptionLocks
{
    const int OPTION_LIMIT = 1000;
    const int PRESET_BYTES = OPTION_LIMIT * sizeof(int);
    const int PAGE_COUNT = 8;
    const int GROUP_COUNT = 4;
    const int ITEM_LIMIT = 20;
    const int PRESET_PATH_LIMIT = 1024;

    struct LockEntry
    {
        bool locked;
        bool radio;
        int forcedValue;
        int radioCount;
        std::string reason;

        LockEntry() : locked(false), radio(false), forcedValue(0), radioCount(0) {}
    };

    LockEntry entries[OPTION_LIMIT];
    bool dirty = true;
    bool ready = false;
    int shadowValues[OPTION_LIMIT] = {};
    bool shadowValid[OPTION_LIMIT] = {};
    int preparedSaveValues[OPTION_LIMIT] = {};
    char preparedSavePath[PRESET_PATH_LIMIT] = {};
    bool preparedSaveReady = false;

    std::string TrimAndLowerAscii(const char* value)
    {
        std::string result = value ? value : "";
        size_t first = 0;
        while (first < result.size() && isspace((unsigned char)result[first]))
            ++first;
        size_t last = result.size();
        while (last > first && isspace((unsigned char)result[last - 1]))
            --last;
        result = result.substr(first, last - first);
        for (size_t i = 0; i < result.size(); ++i)
            result[i] = (char)tolower((unsigned char)result[i]);
        return result;
    }

    bool TryReadJson(const char* key, std::string& value)
    {
        const char* raw = GetEraJSON(key);
        if (!raw || strcmp(raw, key) == 0)
            return false;
        value = raw;
        return true;
    }

    bool TryParseBool(const std::string& text, bool& value)
    {
        const std::string normalized = TrimAndLowerAscii(text.c_str());
        if (normalized == "1" || normalized == "true" || normalized == "on" || normalized == "yes")
        {
            value = true;
            return true;
        }
        if (normalized == "0" || normalized == "false" || normalized == "off" || normalized == "no")
        {
            value = false;
            return true;
        }
        return false;
    }

    bool TryParseInt(const std::string& text, int& value)
    {
        const char* begin = text.c_str();
        char* end = NULL;
        errno = 0;
        long parsed = strtol(begin, &end, 10);
        if (begin == end || errno == ERANGE)
            return false;
        while (*end && isspace((unsigned char)*end))
            ++end;
        if (*end || parsed < INT_MIN || parsed > INT_MAX)
            return false;
        value = (int)parsed;
        return true;
    }

    std::string DefaultReason()
    {
        std::string value;
        if (TryReadJson("wnd.dlg_wog_options.search.locked_hint", value) && !value.empty())
            return value;
        return std::string();
    }

    bool Reload()
    {
        bool loaded[OPTION_LIMIT] = {};
        bool radio[OPTION_LIMIT] = {};
        int radioCount[OPTION_LIMIT] = {};
        bool hasRuntimeOptions = false;

        for (int optionId = 0; optionId < OPTION_LIMIT; ++optionId)
            entries[optionId] = LockEntry();

        _DlgSetup_* ds = o_WogOptions;
        if (!ds)
        {
            ready = false;
            return false;
        }

        for (int page = 0; page < PAGE_COUNT; ++page)
        {
            _DlgSetup_Page_* pageData = ds->Pages[page];
            if (!pageData)
                continue;
            for (int group = 0; group < GROUP_COUNT; ++group)
            {
                _DlgSetup_ItemList_* listData = pageData->ItemList[group];
                if (!listData)
                    continue;
                const int itemCount = min(listData->ItemCount, ITEM_LIMIT);
                for (int item = 0; item < itemCount; ++item)
                {
                    const int optionId = o_WogOptionNumbers[page * 80 + group * 20 + item];
                    if (optionId < 0 || optionId >= OPTION_LIMIT)
                        continue;
                    hasRuntimeOptions = true;
                    loaded[optionId] = true;
                    if (listData->Type == 2)
                    {
                        radio[optionId] = true;
                        radioCount[optionId] = max(radioCount[optionId], itemCount);
                    }
                }
            }
        }

        if (!hasRuntimeOptions)
        {
            ready = false;
            return false;
        }

        const std::string defaultReason = DefaultReason();
        for (int optionId = 0; optionId < OPTION_LIMIT; ++optionId)
        {
            char key[160];
            std::string raw;
            sprintf_s(key, sizeof(key), "wnd.dlg_wog_options.locks.%d.value", optionId);
            if (!TryReadJson(key, raw))
                continue;
            if (!loaded[optionId])
                continue;

            LockEntry entry;
            entry.locked = true;
            entry.radio = radio[optionId];
            entry.radioCount = radioCount[optionId];
            if (entry.radio)
            {
                if (!TryParseInt(raw, entry.forcedValue) || entry.forcedValue < 0 ||
                    entry.forcedValue >= entry.radioCount)
                {
                    continue;
                }
            }
            else
            {
                bool checked = false;
                if (!TryParseBool(raw, checked))
                    continue;
                entry.forcedValue = checked ? 1 : 0;
            }

            sprintf_s(key, sizeof(key), "wnd.dlg_wog_options.locks.%d.reason", optionId);
            if (!TryReadJson(key, entry.reason) || entry.reason.empty())
                entry.reason = defaultReason;
            entries[optionId] = entry;
        }

        ready = true;
        dirty = false;
        return true;
    }

    void EnsureLoaded()
    {
        if (dirty || !ready)
            Reload();
    }

    const char* JsonText(const char* key)
    {
        const char* value = GetEraJSON(key);
        return value && strcmp(value, key) != 0 ? value : o_NullString;
    }

    bool HasActiveLocks()
    {
        EnsureLoaded();
        if (!ready)
            return false;
        for (int optionId = 0; optionId < OPTION_LIMIT; ++optionId)
        {
            if (entries[optionId].locked)
                return true;
        }
        return false;
    }

    void CaptureLockedShadows(const int* values, bool onlyMissing)
    {
        EnsureLoaded();
        if (!ready || !values)
            return;
        for (int optionId = 0; optionId < OPTION_LIMIT; ++optionId)
        {
            if (entries[optionId].locked && (!onlyMissing || !shadowValid[optionId]))
            {
                shadowValues[optionId] = values[optionId];
                shadowValid[optionId] = true;
            }
        }
    }

    void ClearPreparedSave()
    {
        preparedSaveReady = false;
        preparedSavePath[0] = 0;
    }

    bool TryReadFullPreset(const char* path, int* values)
    {
        if (!path || !*path || !values)
            return false;

        HANDLE file = CreateFileA(path, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file == INVALID_HANDLE_VALUE)
            return false;

        DWORD bytesRead = 0;
        const BOOL read = ReadFile(file, values, PRESET_BYTES, &bytesRead, NULL);
        CloseHandle(file);
        return read && bytesRead == PRESET_BYTES;
    }

    bool IsPreparedFor(const char* path)
    {
        return preparedSaveReady && path &&
            _stricmp(preparedSavePath, path) == 0;
    }

    const char* FindRadioChoiceName(int optionId, int choice)
    {
        _DlgSetup_* ds = o_WogOptions;
        if (!ds)
            return NULL;

        for (int page = 0; page < PAGE_COUNT; ++page)
        {
            _DlgSetup_Page_* pageData = ds->Pages[page];
            if (!pageData)
                continue;
            for (int group = 0; group < GROUP_COUNT; ++group)
            {
                _DlgSetup_ItemList_* listData = pageData->ItemList[group];
                if (!listData || listData->Type != 2)
                    continue;
                const int itemCount = min(listData->ItemCount, ITEM_LIMIT);
                int groupOptionId = -1;
                for (int item = 0; item < itemCount && groupOptionId < 0; ++item)
                    groupOptionId = o_WogOptionNumbers[page * 80 + group * 20 + item];
                if (groupOptionId == optionId && choice >= 0 && choice < itemCount &&
                    listData->ItemName && listData->ItemName[choice] && *listData->ItemName[choice])
                    return listData->ItemName[choice];
            }
        }
        return NULL;
    }
}

enum WoGPresetOperation
{
    WOG_PRESET_NONE = 0,
    WOG_PRESET_SAVE = 1,
    WOG_PRESET_LOAD = 2
};

int wogPresetOperation = WOG_PRESET_NONE;
bool wogPresetHooksAvailable = false;

void InvalidateWoGOptionLocks()
{
    WogOptionLocks::dirty = true;
    WogOptionLocks::ready = false;
}

void RefreshWoGOptionLocks()
{
    WogOptionLocks::dirty = true;
    WogOptionLocks::Reload();
}

bool IsWoGOptionLocked(int optionId)
{
    WogOptionLocks::EnsureLoaded();
    return optionId >= 0 && optionId < WogOptionLocks::OPTION_LIMIT &&
        WogOptionLocks::entries[optionId].locked;
}

const char* GetWoGOptionLockReason(int optionId)
{
    if (!IsWoGOptionLocked(optionId))
        return o_NullString;
    return WogOptionLocks::entries[optionId].reason.c_str();
}

bool FormatWoGOptionLockLabel(int optionId, char* output, size_t outputSize)
{
    if (!output || !outputSize)
        return false;
    output[0] = 0;
    if (!IsWoGOptionLocked(optionId))
        return false;

    const WogOptionLocks::LockEntry& entry = WogOptionLocks::entries[optionId];
    const char* value = NULL;
    if (entry.radio)
        value = WogOptionLocks::FindRadioChoiceName(optionId, entry.forcedValue);
    else
        value = WogOptionLocks::JsonText(
            entry.forcedValue ? "wnd.dlg_wog_options.lock_explanation.locked_on" :
                                "wnd.dlg_wog_options.lock_explanation.locked_off");

    if (entry.radio)
    {
        char numericValue[32] = {};
        if (!value || !*value)
        {
            _snprintf_s(numericValue, sizeof(numericValue), _TRUNCATE, "%d",
                entry.forcedValue);
            value = numericValue;
        }
        _snprintf_s(output, outputSize, _TRUNCATE,
            WogOptionLocks::JsonText("wnd.dlg_wog_options.lock_explanation.locked_choice_format"),
            value);
    }
    else if (value && *value)
        strncpy_s(output, outputSize, value, _TRUNCATE);
    else
        _snprintf_s(output, outputSize, _TRUNCATE, "%d", entry.forcedValue);
    return true;
}

void ForceWoGOptionLockValues()
{
    WogOptionLocks::EnsureLoaded();
    if (!WogOptionLocks::ready)
        return;
    WogOptionLocks::CaptureLockedShadows(o_WogOptionValues, true);
    for (int optionId = 0; optionId < WogOptionLocks::OPTION_LIMIT; ++optionId)
    {
        if (WogOptionLocks::entries[optionId].locked)
            o_WogOptionValues[optionId] = WogOptionLocks::entries[optionId].forcedValue;
    }
}

bool PrepareWoGOptionPresetSave(const char* path)
{
    WogOptionLocks::ClearPreparedSave();
    if (!WogOptionLocks::HasActiveLocks())
        return true;
    if (!path || !*path)
        return false;

    memcpy(WogOptionLocks::preparedSaveValues, o_WogOptionValues,
        WogOptionLocks::PRESET_BYTES);

    int existingValues[WogOptionLocks::OPTION_LIMIT];
    const bool hasExistingPreset = WogOptionLocks::TryReadFullPreset(path, existingValues);
    for (int optionId = 0; optionId < WogOptionLocks::OPTION_LIMIT; ++optionId)
    {
        if (!WogOptionLocks::entries[optionId].locked)
            continue;

        if (hasExistingPreset)
            WogOptionLocks::preparedSaveValues[optionId] = existingValues[optionId];
        else if (WogOptionLocks::shadowValid[optionId])
            WogOptionLocks::preparedSaveValues[optionId] = WogOptionLocks::shadowValues[optionId];
        // If neither source exists, retain the current copied value. This is
        // the forced value and is preferable to cancelling the entire save.
    }

    strncpy_s(WogOptionLocks::preparedSavePath, path, _TRUNCATE);
    WogOptionLocks::preparedSaveReady = true;
    return true;
}

void ClearPreparedWoGOptionPresetSave()
{
    WogOptionLocks::ClearPreparedSave();
}

int __cdecl WOG_ReadPresetCaptureLocks(HiHook* hook, const char* path, int* values, int size)
{
    const int result = CALL_3(int, __cdecl, hook->GetDefaultFunc(), path, values, size);
    if (result >= WogOptionLocks::PRESET_BYTES && size >= WogOptionLocks::PRESET_BYTES)
        WogOptionLocks::CaptureLockedShadows(values, false);
    return result;
}

int __cdecl WOG_WritePresetPreservingLocks(HiHook* hook, const char* path, const int* values, int size)
{
    const int* valuesToWrite = values;
    if (size == WogOptionLocks::PRESET_BYTES && WogOptionLocks::HasActiveLocks())
    {
        if (!WogOptionLocks::IsPreparedFor(path) && !PrepareWoGOptionPresetSave(path))
        {
            return 1;
        }
        valuesToWrite = WogOptionLocks::preparedSaveValues;
    }

    const int result = CALL_3(int, __cdecl, hook->GetDefaultFunc(), path, valuesToWrite, size);
    WogOptionLocks::ClearPreparedSave();
    return result;
}

void ReapplyWoGOptionLocks(_DlgSetup_* ds)
{
    WogOptionLocks::EnsureLoaded();
    if (!WogOptionLocks::ready || !ds)
        return;

    for (int page = 0; page < WogOptionLocks::PAGE_COUNT; ++page)
    {
        _DlgSetup_Page_* pageData = ds->Pages[page];
        if (!pageData)
            continue;
        for (int group = 0; group < WogOptionLocks::GROUP_COUNT; ++group)
        {
            _DlgSetup_ItemList_* listData = pageData->ItemList[group];
            if (!listData)
                continue;
            const int itemCount = min(listData->ItemCount, WogOptionLocks::ITEM_LIMIT);
            if (listData->Type == 2)
            {
                int optionId = -1;
                for (int item = 0; item < itemCount && optionId < 0; ++item)
                    optionId = o_WogOptionNumbers[page * 80 + group * 20 + item];
                if (IsWoGOptionLocked(optionId))
                {
                    const int forced = WogOptionLocks::entries[optionId].forcedValue;
                    for (int item = 0; item < itemCount; ++item)
                        listData->ItemState[item] = item == forced ? 3 : 2;
                }
            }
            else
            {
                for (int item = 0; item < itemCount; ++item)
                {
                    const int optionId = o_WogOptionNumbers[page * 80 + group * 20 + item];
                    if (IsWoGOptionLocked(optionId))
                    {
                        bool visuallyChecked = WogOptionLocks::entries[optionId].forcedValue != 0;
                        if (optionId >= 1 && optionId <= 4)
                            visuallyChecked = !visuallyChecked;
                        listData->ItemState[item] = visuallyChecked ? 3 : 2;
                    }
                }
            }
        }
    }
}

void __stdcall WOG_OptionsArrayToDialog_Locks(HiHook* hook)
{
    ForceWoGOptionLockValues();
    CALL_0(void, __cdecl, hook->GetDefaultFunc());
    ReapplyWoGOptionLocks(o_WogOptions);
}

void __stdcall WOG_DialogToOptionsArray_Locks(HiHook* hook)
{
    CALL_0(void, __cdecl, hook->GetDefaultFunc());
    ForceWoGOptionLockValues();
}

bool IsVerifiedWoGOptionSyncSite(_ptr_ address)
{
    static const unsigned char expected[] = { 0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14, 0x53, 0x56, 0x57 };

    Patch* patch = _P->GetFirstPatchAt(address);
    if (patch)
    {
        // Patcher_x86 HiHooks are explicitly chainable. Refuse a mixed/raw
        // intervention at the entry point, because its ABI cannot be proven.
        while (patch)
        {
            if (patch->GetType() != HIHOOK_)
                return false;
            patch = patch->GetAppliedAfter();
        }
        return true;
    }

    return memcmp((const void*)address, expected, sizeof(expected)) == 0;
}

bool IsVerifiedWoGPresetCallSite(_ptr_ address, const unsigned char* expected, size_t expectedSize)
{
    Patch* patch = _P->GetFirstPatchAt(address);
    if (patch)
    {
        while (patch)
        {
            if (patch->GetType() != HIHOOK_)
                return false;
            patch = patch->GetAppliedAfter();
        }
        return true;
    }

    return memcmp((const void*)address, expected, expectedSize) == 0;
}

bool InstallWoGOptionLockSyncHooks(PatcherInstance* patcher)
{
    const _ptr_ arrayToDialog = 0x778C58;
    const _ptr_ dialogToArray = 0x778ECC;
    if (!IsVerifiedWoGOptionSyncSite(arrayToDialog) ||
        !IsVerifiedWoGOptionSyncSite(dialogToArray))
    {
        return false;
    }

    patcher->WriteHiHook(arrayToDialog, SPLICE_, EXTENDED_, CDECL_, WOG_OptionsArrayToDialog_Locks);
    patcher->WriteHiHook(dialogToArray, SPLICE_, EXTENDED_, CDECL_, WOG_DialogToOptionsArray_Locks);
    return true;
}

bool InstallWoGOptionPresetHooks(PatcherInstance* patcher)
{
    const _ptr_ writePresetCall = 0x7779D2;
    const _ptr_ readPresetCall = 0x777B1C;
    static const unsigned char expectedWrite[] = { 0xE8, 0x4F, 0xBE, 0xFF, 0xFF };
    static const unsigned char expectedRead[] = { 0xE8, 0x46, 0xBD, 0xFF, 0xFF };

    if (!IsVerifiedWoGPresetCallSite(writePresetCall, expectedWrite, sizeof(expectedWrite)) ||
        !IsVerifiedWoGPresetCallSite(readPresetCall, expectedRead, sizeof(expectedRead)))
    {
        wogPresetHooksAvailable = false;
        return false;
    }

    patcher->WriteHiHook(writePresetCall, CALL_, EXTENDED_, CDECL_, WOG_WritePresetPreservingLocks);
    patcher->WriteHiHook(readPresetCall, CALL_, EXTENDED_, CDECL_, WOG_ReadPresetCaptureLocks);
    wogPresetHooksAvailable = true;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

_bool_ testUseJsonString = false;

// функция проверяет есть ли отладочный текст в wnd.json
// для удобства заполнения строк через json
void IsNeedUseJsonString()
{
    // узнаём есть ли строка ("test_json_strings": "test") в json файле
    string testJson = GetEraJSON("wnd.dlg_wog_options.test_json_strings");
    string test = "test";

    // переводим строку в нижний регистр
    transform(testJson.begin(), testJson.end(), testJson.begin(), tolower);

    if (testJson == test)
        testUseJsonString = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void SetupJsonText( char** target, const char* jsonName)
{
    // проверяем существует ли в JSON файле "заполненная строка"
    // если будет пустышка - не будем перезаписывать
    string s1 = jsonName;
    string s2 = GetEraJSON(jsonName);

    // перезаписываем строку текста
    if (testUseJsonString || s1 != s2)
        *target = GetEraJSON(jsonName);
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

// функия заполнения строк:
// имеет всего один вызов из 0x77941A
void __cdecl WOG_ProcessAll(HiHook* h)
{
    // не вырезаем оригинальную функцию (обратная совместимость)
    CALL_0(void, __cdecl, h->GetDefaultFunc() );

    // Item rows and the canonical option-number map are rebuilt immediately
    // after this callback. Defer JSON validation until the first sync hook,
    // when those runtime tables are complete.
    InvalidateWoGOptionLocks();

    // проверка на необходимость подмены строк из JSON файла
    // должна быть ("use_json_strings": "on")
    string useJson = GetEraJSON("wnd.dlg_wog_options.use_json_strings");
    string campareString = "on";

    // переводим строку useJson в нижний регистр
    transform(useJson.begin(), useJson.end(), useJson.begin(), tolower);

    // выходим, если не нужно загружать строки из JSON файла
    if (useJson != campareString)
        return;

    // для удобства заполнения строк через json (отладочный функционал)
    // при необходимости внести строку ("test_json_strings": "test")
    IsNeedUseJsonString();

    // получаем структуру диалога ВОГ Опций
    _DlgSetup_* ds = o_WogOptions;

    SetupJsonText(&ds->Name, optionsName);
    SetupJsonText(&ds->Hint, optionsHint);
    SetupJsonText(&ds->PopUp, optionsPopUp);
    SetupJsonText(&ds->Intro, optionsIntro);

    for (int i = 0; i < 8; i++) {
        sprintf(strPage, textPage, i, name);
        SetupJsonText(&ds->Pages[i]->Name, strPage);

        sprintf(strPage, textPage, i, hint);
        SetupJsonText(&ds->Pages[i]->Hint, strPage);

        sprintf(strPage, textPage, i, popUp);
        SetupJsonText(&ds->Pages[i]->PopUp, strPage);

        sprintf(strPage, textPage, i, list);
        for(int j = 0; j < 4; j++){
            sprintf(strList, strPage, j, name);
            SetupJsonText(&ds->Pages[i]->ItemList[j]->Name, strList);

            sprintf(strList, strPage, j, hint);
            SetupJsonText(&ds->Pages[i]->ItemList[j]->Hint, strList);

            sprintf(strList, strPage, j, popUp);
            SetupJsonText(&ds->Pages[i]->ItemList[j]->PopUp, strList);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

int __stdcall Dlg_SaveDatFile1(LoHook* h, HookContext* c)
{
    wogPresetOperation = WOG_PRESET_SAVE;
    _ChooseFile_* cf = o_ChooseFile;

    SetupJsonText(&cf->Caption, "dlg_datfile.captionsave");
    SetupJsonText(&cf->Description, "dlg_datfile.descrsave");
    SetupJsonText(&cf->Mask, "dlg_datfile.filemask");

    return EXEC_DEFAULT;
}

//int __stdcall Dlg_SaveDatFile2(LoHook* h, HookContext* c)
//{
//  c->eax = (int)GetEraJSON("dlg_datfile.cannotsave");
//
//  return EXEC_DEFAULT;
//}

int __stdcall Dlg_LoadDatFile1(LoHook* h, HookContext* c)
{
    wogPresetOperation = WOG_PRESET_LOAD;
    _ChooseFile_* cf = o_ChooseFile;

    SetupJsonText(&cf->Caption, "dlg_datfile.captionload");
    SetupJsonText(&cf->Description, "dlg_datfile.descrload");
    SetupJsonText(&cf->Mask, "dlg_datfile.filemask");

    return EXEC_DEFAULT;
}

//int __stdcall Dlg_LoadDatFile2(LoHook* h, HookContext* c)
//{
//  // c->eax = (int)GetEraJSON("dlg_datfile.cannotload");
//  SetupJsonText((char*)c->eax, "dlg_datfile.cannotload");
//
//  return EXEC_DEFAULT;
//}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////


void WoGOptionsStrings(PatcherInstance* _PI)
{
    if ( ERA_VERSION >= 3000 ) { // только ERA III
        _PI->WriteHiHook(0x778A9D, SPLICE_, EXTENDED_, CDECL_, WOG_ProcessAll);

        // Keep locked values synchronized through setup initialization,
        // preset load/default actions, dialog entry, and dialog exit.
        InstallWoGOptionLockSyncHooks(_PI);
        InstallWoGOptionPresetHooks(_PI);

        // save
        _PI->WriteLoHook(0x7779A9, Dlg_SaveDatFile1);

        // load
        _PI->WriteLoHook(0x777AF3, Dlg_LoadDatFile1);

    }

    return;
}


