#include "pch.h"
#include <climits>
#include <cstring>
#include <vector>

namespace
{
    const int MAP_S = 36;
    const int MAP_XL = 144;
    const int MAP_H = 180;
    const int MAP_XH = 216;
    const int MAP_G = 252;
    const int ITEM_H = 3003;
    const int ITEM_XH = 3004;
    const int ITEM_G = 3005;

    const int ITEM_S = 281;
    const int ITEM_M = 282;
    const int ITEM_L = 283;
    const int ITEM_XL = 284;
    const int ITEM_UNDERGROUND = 285;

    const int kMapDimensions[] = { 36, 72, 108, 144, 180, 216, 252 };
    const int kMapSizeItems[] =
        { ITEM_S, ITEM_M, ITEM_L, ITEM_XL, ITEM_H, ITEM_XH, ITEM_G };
    const int kTemplateAreas[] =
        { 1, 2, 4, 8, 9, 16, 18, 25, 32, 36, 49, 50, 72, 98 };

    bool g_minimapInstalled = false;
    bool g_hdCompatibilityInstalled = false;
    bool g_hdTemplateUpdaterInstalled = false;
    bool g_hdTemplateFormatterInstalled = false;
    bool g_afterWogRegistered = false;
    bool g_refreshingTemplateSizes = false;
    int g_hdTemplateUpdateDepth = 0;
    _ptr_ g_hdMapperAddress = 0;
    H3LoadedPcx16* g_minimap = nullptr;
    char** g_lastFilteredTemplateData = nullptr;
    int g_lastFilteredTemplateCount = -1;

    bool IsSpace(char c)
    {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }

    bool ParsePositiveInt(LPCSTR text, int& value)
    {
        if (!text)
            return false;
        while (IsSpace(*text))
            ++text;
        if (*text == '+')
            ++text;
        if (*text < '0' || *text > '9')
            return false;

        int parsed = 0;
        do
        {
            const int digit = *text - '0';
            if (parsed > (INT_MAX - digit) / 10)
                return false;
            parsed = parsed * 10 + digit;
            ++text;
        } while (*text >= '0' && *text <= '9');

        while (IsSpace(*text))
            ++text;
        if (*text || parsed <= 0)
            return false;
        value = parsed;
        return true;
    }

    bool ParseNonNegativeInt(LPCSTR text, int& value, bool emptyIsZero = false)
    {
        if (!text)
            return false;
        while (IsSpace(*text))
            ++text;
        if (!*text && emptyIsZero)
        {
            value = 0;
            return true;
        }
        if (*text == '+')
            ++text;
        if (*text < '0' || *text > '9')
            return false;

        int parsed = 0;
        do
        {
            const int digit = *text - '0';
            if (parsed > (INT_MAX - digit) / 10)
                return false;
            parsed = parsed * 10 + digit;
            ++text;
        } while (*text >= '0' && *text <= '9');

        while (IsSpace(*text))
            ++text;
        if (*text)
            return false;
        value = parsed;
        return true;
    }

    bool IsMarked(LPCSTR text)
    {
        if (!text)
            return false;
        while (IsSpace(*text))
            ++text;
        return *text != 0;
    }

    struct RmgZoneRule
    {
        bool humanStart;
        bool computerStart;
        int minimumHumans;
        int maximumHumans;
        int minimumTotal;
        int maximumTotal;
    };

    enum TemplatePlayability
    {
        TEMPLATE_UNKNOWN,
        TEMPLATE_PLAYABLE,
        TEMPLATE_UNPLAYABLE
    };

    bool IsRmgBlockPlayable(int minimumArea, int maximumArea,
        const std::vector<RmgZoneRule>& zones)
    {
        if (minimumArea <= 0 || maximumArea < minimumArea || zones.empty())
            return false;

        for (int size = 0; size < 7; ++size)
        {
            const int scale = kMapDimensions[size] / MAP_S;
            for (int levels = 1; levels <= 2; ++levels)
            {
                const int fullArea = scale * scale * levels;
                for (int islands = 0; islands <= 1; ++islands)
                {
                    const int area = islands ?
                        (fullArea / 2 > 0 ? fullArea / 2 : 1) : fullArea;
                    if (area < minimumArea || area > maximumArea)
                        continue;

                    for (int requestedHumans = 1; requestedHumans <= 8;
                        ++requestedHumans)
                    {
                        for (int requestedComputers = 0;
                            requestedHumans + requestedComputers <= 8;
                            ++requestedComputers)
                        {
                            int humans = requestedHumans;
                            int computers = requestedComputers;
                            // Stock RMG never attempts a one-participant map.
                            if (humans + computers < 2)
                            {
                                humans = 1;
                                computers = 1;
                            }
                            const int total = humans + computers;
                            int humanStarts = 0;
                            int allStarts = 0;
                            for (size_t zoneIndex = 0; zoneIndex < zones.size();
                                ++zoneIndex)
                            {
                                const RmgZoneRule& zone = zones[zoneIndex];
                                if (humans < zone.minimumHumans ||
                                    humans > zone.maximumHumans ||
                                    total < zone.minimumTotal ||
                                    total > zone.maximumTotal)
                                    continue;
                                if (zone.humanStart)
                                {
                                    ++humanStarts;
                                    ++allStarts;
                                }
                                else if (zone.computerStart)
                                    ++allStarts;
                            }
                            if (humanStarts >= humans && allStarts >= total)
                                return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool ResolveTemplateFilePath(LPCSTR templatesDirectory,
        LPCSTR templateName, char (&path)[MAX_PATH * 2])
    {
        if (!templatesDirectory || !templateName)
            return false;
        const size_t rootLength = std::strlen(templatesDirectory);
        const char separator = rootLength &&
            (templatesDirectory[rootLength - 1] == '\\' ||
                templatesDirectory[rootLength - 1] == '/') ? 0 : '\\';

        // HD.Dir.Templates currently names the _HD3_Data root.  Retain the
        // second form for builds that publish the Templates directory itself.
        const char* formats[] =
        {
            separator ? "%s%cTemplates\\%s\\rmg.txt" : "%sTemplates\\%s\\rmg.txt",
            separator ? "%s%c%s\\rmg.txt" : "%s%s\\rmg.txt"
        };
        for (int form = 0; form < 2; ++form)
        {
            int result = -1;
            if (separator)
                result = _snprintf_s(path, sizeof(path), _TRUNCATE,
                    formats[form], templatesDirectory, separator, templateName);
            else
                result = _snprintf_s(path, sizeof(path), _TRUNCATE,
                    formats[form], templatesDirectory, templateName);
            if (result < 0)
                continue;
            const DWORD attributes = ::GetFileAttributesA(path);
            if (attributes != INVALID_FILE_ATTRIBUTES &&
                !(attributes & FILE_ATTRIBUTE_DIRECTORY))
                return true;
        }
        return false;
    }

    bool ReadSmallFile(LPCSTR path, std::vector<char>& data)
    {
        HANDLE file = ::CreateFileA(path, GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE)
            return false;

        DWORD highSize = 0;
        const DWORD lowSize = ::GetFileSize(file, &highSize);
        if ((lowSize == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR) ||
            highSize != 0 || lowSize == 0 || lowSize > 16 * 1024 * 1024)
        {
            ::CloseHandle(file);
            return false;
        }

        data.resize(static_cast<size_t>(lowSize) + 1);
        DWORD totalRead = 0;
        while (totalRead < lowSize)
        {
            DWORD bytesRead = 0;
            if (!::ReadFile(file, &data[totalRead], lowSize - totalRead,
                &bytesRead, nullptr) || bytesRead == 0)
            {
                ::CloseHandle(file);
                data.clear();
                return false;
            }
            totalRead += bytesRead;
        }
        ::CloseHandle(file);
        data[lowSize] = 0;
        return true;
    }

    TemplatePlayability ReadTemplatePlayability(LPCSTR templateName)
    {
        if (!globalPatcher || !templateName || !*templateName)
            return TEMPLATE_UNKNOWN;
        LPCSTR templatesDirectory = globalPatcher->VarGetValue<LPCSTR>(
            "HD.Dir.Templates", nullptr);
        if (!templatesDirectory ||
            ::IsBadStringPtrA(templatesDirectory, MAX_PATH) || !*templatesDirectory)
            return TEMPLATE_UNKNOWN;

        char path[MAX_PATH * 2];
        if (!ResolveTemplateFilePath(templatesDirectory, templateName, path))
            return TEMPLATE_UNKNOWN;
        std::vector<char> fileData;
        if (!ReadSmallFile(path, fileData))
            return TEMPLATE_UNKNOWN;

        bool foundBlock = false;
        bool malformed = false;
        bool haveCurrentBlock = false;
        int minimumArea = 0;
        int maximumArea = 0;
        std::vector<RmgZoneRule> zones;
        char* cursor = &fileData[0];
        char* end = cursor + fileData.size() - 1;
        while (cursor < end)
        {
            char* columns[13] = {};
            int columnCount = 1;
            columns[0] = cursor;
            while (cursor < end && *cursor != '\r' && *cursor != '\n')
            {
                if (*cursor == '\t' && columnCount < 13)
                {
                    *cursor = 0;
                    columns[columnCount++] = cursor + 1;
                }
                else if (*cursor == '\t' && columnCount == 13)
                {
                    // Only columns 0..12 are needed; terminate the last one
                    // at its real boundary and ignore the remaining fields.
                    *cursor = 0;
                    ++columnCount;
                }
                ++cursor;
            }
            while (cursor < end && (*cursor == '\r' || *cursor == '\n'))
                *cursor++ = 0;

            int newMinimum = 0;
            int newMaximum = 0;
            if (columnCount >= 3 && columns[0] && *columns[0] &&
                ParsePositiveInt(columns[1], newMinimum) &&
                ParsePositiveInt(columns[2], newMaximum))
            {
                if (haveCurrentBlock &&
                    IsRmgBlockPlayable(minimumArea, maximumArea, zones))
                    return TEMPLATE_PLAYABLE;
                if (haveCurrentBlock && zones.empty())
                    malformed = true;
                foundBlock = true;
                haveCurrentBlock = true;
                minimumArea = newMinimum;
                maximumArea = newMaximum;
                zones.clear();
                if (maximumArea < minimumArea)
                    malformed = true;
                continue;
            }

            int zoneId = 0;
            if (!haveCurrentBlock || columnCount < 13 ||
                !ParsePositiveInt(columns[3], zoneId))
                continue;

            RmgZoneRule zone = {};
            zone.humanStart = IsMarked(columns[4]);
            zone.computerStart = IsMarked(columns[5]);
            if (!ParseNonNegativeInt(columns[9], zone.minimumHumans, true) ||
                !ParseNonNegativeInt(columns[10], zone.maximumHumans) ||
                !ParseNonNegativeInt(columns[11], zone.minimumTotal) ||
                !ParseNonNegativeInt(columns[12], zone.maximumTotal) ||
                zone.maximumHumans < zone.minimumHumans ||
                zone.maximumTotal < zone.minimumTotal)
            {
                malformed = true;
                continue;
            }
            zones.push_back(zone);
        }

        const bool playable = haveCurrentBlock &&
            IsRmgBlockPlayable(minimumArea, maximumArea, zones);
        if (haveCurrentBlock && zones.empty())
            malformed = true;
        if (playable)
            return TEMPLATE_PLAYABLE;
        return foundBlock && !malformed ? TEMPLATE_UNPLAYABLE : TEMPLATE_UNKNOWN;
    }

    bool AnySizeEnabled(const bool (&enabled)[7])
    {
        for (int i = 0; i < 7; ++i)
            if (enabled[i])
                return true;
        return false;
    }

    void CopySizeMask(bool (&destination)[7], const bool (&source)[7])
    {
        for (int i = 0; i < 7; ++i)
            destination[i] = source[i];
    }

    bool LoadTemplateSizeMasks(H3SelectScenarioDialog* dialog,
        bool (&surface)[7], bool (&underground)[7])
    {
        if (!dialog)
            return false;
        std::memset(surface, 0, sizeof(surface));
        std::memset(underground, 0, sizeof(underground));

        H3TextTable* table = H3TextTable::Load("rmg.txt");
        if (!table)
            return false;

        bool foundRange = false;
        int rawMinimum = 0;
        int rawMaximum = 0;
        const UINT32 rows = table->CountRows();
        for (UINT32 row = 0; row < rows; ++row)
        {
            H3Vector<LPCSTR>& columns = (*table)[row];
            if (columns.Count() >= 3 &&
                ParsePositiveInt(columns[1], rawMinimum) &&
                ParsePositiveInt(columns[2], rawMaximum) &&
                rawMaximum >= rawMinimum)
            {
                foundRange = true;
                for (int i = 0; i < 7; ++i)
                {
                    const int scale = kMapDimensions[i] / MAP_S;
                    const int surfaceArea = scale * scale;
                    const int undergroundArea = surfaceArea * 2;
                    const int effectiveSurfaceArea =
                        dialog->waterContentSelected == 2 ?
                        (surfaceArea / 2 > 0 ? surfaceArea / 2 : 1) : surfaceArea;
                    const int effectiveUndergroundArea =
                        dialog->waterContentSelected == 2 ?
                        (undergroundArea / 2 > 0 ? undergroundArea / 2 : 1) :
                        undergroundArea;
                    surface[i] = surface[i] ||
                        (effectiveSurfaceArea >= rawMinimum &&
                            effectiveSurfaceArea <= rawMaximum);
                    underground[i] = underground[i] ||
                        (effectiveUndergroundArea >= rawMinimum &&
                            effectiveUndergroundArea <= rawMaximum);
                }
            }
        }
        table->UnLoad();
        return foundRange &&
            (AnySizeEnabled(surface) || AnySizeEnabled(underground));
    }

    bool CurrentTemplateSelectionSupported(H3SelectScenarioDialog* dialog)
    {
        if (!dialog)
            return false;
        bool surface[7] = {};
        bool underground[7] = {};
        if (!LoadTemplateSizeMasks(dialog, surface, underground))
            return false;

        for (int i = 0; i < 7; ++i)
        {
            if (kMapDimensions[i] == dialog->mapDimension)
            {
                if (dialog->numberLevels == 1)
                    return surface[i];
                if (dialog->numberLevels == 2)
                    return underground[i];
                return false;
            }
        }
        return false;
    }

    void SetSizeButtons(H3SelectScenarioDialog* dialog,
        const bool (&enabled)[7], bool redraw)
    {
        if (!dialog)
            return;
        for (int i = 0; i < 7; ++i)
        {
            H3DlgItem* item = dialog->GetH3DlgItem(kMapSizeItems[i]);
            if (!item)
                continue;

            const bool changed =
                (item->IsEnabled() != FALSE) != enabled[i];
            if (changed)
                item->EnableItem(enabled[i]);
            if (redraw && changed && item->IsVisible())
            {
                item->Draw();
                item->Refresh();
            }
        }
    }

    void SetAllSizeButtonsEnabled(H3SelectScenarioDialog* dialog,
        bool enabled, bool redraw = false)
    {
        bool states[7];
        for (int i = 0; i < 7; ++i)
            states[i] = enabled;
        SetSizeButtons(dialog, states, redraw);
    }

    void SetUndergroundButtonEnabled(H3SelectScenarioDialog* dialog,
        bool enabled, bool redraw)
    {
        if (!dialog)
            return;
        H3DlgItem* item = dialog->GetH3DlgItem(ITEM_UNDERGROUND);
        if (!item)
            return;
        const bool changed = (item->IsEnabled() != FALSE) != enabled;
        if (changed)
            item->EnableItem(enabled);
        if (redraw && changed && item->IsVisible())
        {
            item->Draw();
            item->Refresh();
        }
    }

    struct TemplateButtonEnabledState
    {
        bool sizes[7];
        bool underground;
    };

    void CaptureTemplateButtonEnabledState(H3SelectScenarioDialog* dialog,
        TemplateButtonEnabledState& state)
    {
        for (int i = 0; i < 7; ++i)
        {
            H3DlgItem* item = dialog ?
                dialog->GetH3DlgItem(kMapSizeItems[i]) : nullptr;
            state.sizes[i] = !item || item->IsEnabled() != FALSE;
        }
        H3DlgItem* underground = dialog ?
            dialog->GetH3DlgItem(ITEM_UNDERGROUND) : nullptr;
        state.underground = !underground ||
            underground->IsEnabled() != FALSE;
    }

    void RedrawFinalTemplateButtonStates(H3SelectScenarioDialog* dialog,
        const TemplateButtonEnabledState& previous,
        bool controlsWereUnlocked)
    {
        if (!dialog)
            return;
        for (int i = 0; i < 7; ++i)
        {
            H3DlgItem* item = dialog->GetH3DlgItem(kMapSizeItems[i]);
            const bool enabled = item && item->IsEnabled() != FALSE;
            if (item && item->IsVisible() &&
                (enabled != previous.sizes[i] ||
                    (controlsWereUnlocked && !enabled)))
            {
                item->Draw();
                item->Refresh();
            }
        }
        H3DlgItem* underground = dialog->GetH3DlgItem(ITEM_UNDERGROUND);
        const bool undergroundEnabled = underground &&
            underground->IsEnabled() != FALSE;
        if (underground && underground->IsVisible() &&
            (undergroundEnabled != previous.underground ||
                (controlsWereUnlocked && !undergroundEnabled)))
        {
            underground->Draw();
            underground->Refresh();
        }
    }

    int FindDimensionIndex(int dimension)
    {
        for (int i = 0; i < 7; ++i)
            if (kMapDimensions[i] == dimension)
                return i;
        return -1;
    }

    int FindSizeItemIndex(int itemId)
    {
        for (int i = 0; i < 7; ++i)
            if (kMapSizeItems[i] == itemId)
                return i;
        return -1;
    }

    int FindNearestEnabledSize(int dimension, const bool (&enabled)[7])
    {
        int best = -1;
        int bestDistance = INT_MAX;
        for (int i = 0; i < 7; ++i)
        {
            if (!enabled[i])
                continue;
            const int distance = kMapDimensions[i] > dimension ?
                kMapDimensions[i] - dimension : dimension - kMapDimensions[i];
            if (distance < bestDistance)
            {
                best = i;
                bestDistance = distance;
            }
        }
        return best;
    }

    void SendNativeScenarioClick(H3SelectScenarioDialog* dialog, int itemId)
    {
        H3Msg message;
        std::memset(&message, 0, sizeof(message));
        message.command = eMsgCommand::MOUSE_BUTTON;
        message.subtype = eMsgSubtype::LBUTTON_CLICK;
        message.itemId = itemId;
        THISCALL_2(int, 0x587FD0, dialog, &message);
    }

    bool SwitchNumberLevels(H3SelectScenarioDialog* dialog)
    {
        if (!dialog)
            return false;
        const int previousLevels = dialog->numberLevels;
        g_refreshingTemplateSizes = true;
        SetUndergroundButtonEnabled(dialog, true, false);
        SendNativeScenarioClick(dialog, ITEM_UNDERGROUND);
        g_refreshingTemplateSizes = false;
        return dialog->numberLevels != previousLevels;
    }

    bool ChooseNativeSize(H3SelectScenarioDialog* dialog, int sizeIndex)
    {
        if (!dialog || sizeIndex < 0 || sizeIndex >= 7)
            return false;
        g_refreshingTemplateSizes = true;
        H3DlgItem* item = dialog->GetH3DlgItem(kMapSizeItems[sizeIndex]);
        if (item)
            item->EnableItem(true);
        SendNativeScenarioClick(dialog, kMapSizeItems[sizeIndex]);
        g_refreshingTemplateSizes = false;
        return dialog->mapDimension == kMapDimensions[sizeIndex];
    }

    void RefreshTemplateSizes(H3SelectScenarioDialog* dialog,
        bool correctInvalid, bool redraw)
    {
        if (!dialog || g_refreshingTemplateSizes ||
            !dialog->GetH3DlgItem(ITEM_S) || !dialog->GetH3DlgItem(ITEM_G))
            return;

        HMODULE hdWog = ::GetModuleHandleA("HD_WOG.dll");
        if (hdWog && !g_hdTemplateUpdaterInstalled)
        {
            // An unknown HD build may perform multi-message correction.  Do
            // not leave a partial mask that could obstruct that sequence.
            SetAllSizeButtonsEnabled(dialog, true, redraw);
            SetUndergroundButtonEnabled(dialog, true, redraw);
            return;
        }

        bool surface[7] = {};
        bool underground[7] = {};
        if (!LoadTemplateSizeMasks(dialog, surface, underground))
        {
            SetAllSizeButtonsEnabled(dialog, true, redraw);
            SetUndergroundButtonEnabled(dialog, true, redraw);
            return;
        }
        const bool supportsUnderground = AnySizeEnabled(underground);
        bool available[7] = {};
        for (int i = 0; i < 7; ++i)
            available[i] = surface[i] || underground[i];

        if (correctInvalid)
        {
            int selected = FindDimensionIndex(dialog->mapDimension);
            if (selected >= 0 && available[selected])
            {
                const bool currentLevelSupported = dialog->numberLevels == 1 ?
                    surface[selected] : dialog->numberLevels == 2 ?
                    underground[selected] : false;
                if (!currentLevelSupported)
                {
                    const int requiredLevels = surface[selected] ? 1 :
                        underground[selected] ? 2 : 0;
                    if (requiredLevels && dialog->numberLevels != requiredLevels)
                        SwitchNumberLevels(dialog);
                }
            }
            else
            {
                bool compatible[7] = {};
                if (dialog->numberLevels == 2)
                    CopySizeMask(compatible, underground);
                else
                    CopySizeMask(compatible, surface);
                if (!AnySizeEnabled(compatible))
                {
                    const bool alternateAvailable = dialog->numberLevels == 2 ?
                        AnySizeEnabled(surface) : AnySizeEnabled(underground);
                    if (alternateAvailable && SwitchNumberLevels(dialog))
                    {
                        if (dialog->numberLevels == 2)
                            CopySizeMask(compatible, underground);
                        else
                            CopySizeMask(compatible, surface);
                    }
                }
                const int replacement = FindNearestEnabledSize(
                    dialog->mapDimension, compatible);
                if (replacement >= 0)
                    ChooseNativeSize(dialog, replacement);
            }
        }

        // HotA-style availability: a size remains clickable when it is valid
        // on at least one level count. Invalid level transitions are blocked
        // separately, so the selected size has priority over Underground.
        SetSizeButtons(dialog, available, redraw);
        SetUndergroundButtonEnabled(dialog, supportsUnderground, redraw);
    }

    void PrepareScenarioDialogMessage(H3SelectScenarioDialog* dialog, H3Msg* message)
    {
        if (!dialog || !message || g_refreshingTemplateSizes ||
            g_hdTemplateUpdateDepth > 0 ||
            message->command != eMsgCommand::MOUSE_BUTTON ||
            message->subtype != eMsgSubtype::LBUTTON_CLICK)
            return;

        const int sizeIndex = FindSizeItemIndex(message->itemId);
        if (sizeIndex < 0 && message->itemId != ITEM_UNDERGROUND)
            return;

        bool surface[7] = {};
        bool underground[7] = {};
        if (!LoadTemplateSizeMasks(dialog, surface, underground))
            return;

        if (message->itemId == ITEM_UNDERGROUND)
        {
            const int selected = FindDimensionIndex(dialog->mapDimension);
            if (selected < 0)
                return;
            const bool turningUndergroundOn = dialog->numberLevels != 2;
            const bool targetSupported = turningUndergroundOn ?
                underground[selected] : surface[selected];
            if (!targetSupported)
                message->itemId = 0;
            return;
        }

        if (!surface[sizeIndex] && !underground[sizeIndex])
        {
            message->itemId = 0;
            return;
        }

        int requiredLevels = 0;
        if (dialog->numberLevels == 2 && !underground[sizeIndex] && surface[sizeIndex])
            requiredLevels = 1;
        else if (dialog->numberLevels != 2 && !surface[sizeIndex] && underground[sizeIndex])
            requiredLevels = 2;

        if (requiredLevels && dialog->numberLevels != requiredLevels &&
            !SwitchNumberLevels(dialog))
            message->itemId = 0;
    }

    int MapWidth() { return IntAt(0x6783C8); }
    int MapHeight() { return IntAt(0x6783CC); }

    bool BytesEqual(_ptr_ address, const _byte_* expected, size_t size)
    {
        return address && expected &&
            std::memcmp(reinterpret_cast<const void*>(address), expected, size) == 0;
    }

    bool ApplyPatchSet(Patch** patches, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            if (!patches[i])
            {
                for (int j = 0; j < count; ++j)
                    if (patches[j])
                        patches[j]->Destroy();
                return false;
            }
        }

        int applied = 0;
        for (; applied < count; ++applied)
        {
            // Apply() returns the zero-based chain position; zero is success.
            const int position = patches[applied]->Apply();
            if (position < 0 || !patches[applied]->IsApplied())
                break;
        }
        if (applied == count)
            return true;

        for (int i = count - 1; i >= 0; --i)
            if (patches[i]->IsApplied())
                patches[i]->Undo();
        for (int i = 0; i < count; ++i)
            patches[i]->Destroy();
        return false;
    }

    bool ScalePcx16(H3LoadedPcx16* destination, H3LoadedPcx16* source,
        int sourceWidth, int sourceHeight, int destinationX, int destinationY,
        int destinationWidth, int destinationHeight)
    {
        if (!destination || !source || !destination->buffer || !source->buffer ||
            sourceWidth <= 0 || sourceHeight <= 0 || destinationWidth <= 0 || destinationHeight <= 0)
            return false;

        destinationWidth = std::min<int>(destinationWidth, destination->width - destinationX);
        destinationHeight = std::min<int>(destinationHeight, destination->height - destinationY);
        if (destinationX < 0 || destinationY < 0 || destinationWidth <= 0 || destinationHeight <= 0)
            return false;

        _word_* src = reinterpret_cast<_word_*>(source->buffer);
        _word_* dst = reinterpret_cast<_word_*>(destination->buffer);
        const int srcPitch = source->scanlineSize >> 1;
        const int dstPitch = destination->scanlineSize >> 1;
        for (int y = 0; y < destinationHeight; ++y)
        {
            const int sourceY = y * sourceHeight / destinationHeight;
            for (int x = 0; x < destinationWidth; ++x)
                dst[destinationX + x + (destinationY + y) * dstPitch] =
                    src[x * sourceWidth / destinationWidth + sourceY * srcPitch];
        }
        return true;
    }

    bool IsObjectAccent(UINT type)
    {
        switch (type)
        {
        case 17: case 20: case 33: case 34: case 42: case 53: case 87: case 98:
        case 116: case 118: case 119: case 121: case 123: case 126: case 128:
        case 131: case 133: case 134: case 135: case 137: case 148: case 149:
        case 152: case 153: case 154: case 155: case 158: case 159: case 160:
            return true;
        default:
            return false;
        }
    }

    int MapSizeIndex()
    {
        const int width = MapWidth();
        const int height = MapHeight();
        if (width != height)
            return 0;
        switch (height)
        {
        case MAP_S:  return 1;
        case 72:     return 2;
        case 108:    return 3;
        case MAP_XL: return 4;
        case MAP_H:  return 5;
        case MAP_XH: return 6;
        case MAP_G:  return 7;
        default:     return 0;
        }
    }

    bool HasExtendedRadarFrames(H3LoadedDef* radar);

    bool DrawExtendedMinimap(H3AdventureManager* manager, UINT packedPosition,
        INT8 showMines, INT8 showHeroes, INT8 showTowns)
    {
        const int width = MapWidth();
        const int height = MapHeight();
        if (!manager || !manager->dlg || !P_WindowManager || !P_WindowManager->screenPcx16 ||
            MapSizeIndex() < 5 || !HasExtendedRadarFrames(manager->radarDef))
            return false;

        H3DlgItem* item = manager->dlg->minimapTransparentOverlay;
        if (!item)
            return false;
        const int mmX = item->GetX();
        const int mmY = item->GetY();
        const int mmW = item->GetWidth();
        const int mmH = item->GetHeight();
        if (mmW <= 0 || mmH <= 0 || mmW > 288 || mmH > 288)
            return false;

        if (!g_minimap)
            g_minimap = H3LoadedPcx16::Create("", MAP_G, MAP_G);
        if (!g_minimap || !g_minimap->buffer)
            return false;

        const int z = (packedPosition >> 26) & 1;
        _word_* pixels = reinterpret_cast<_word_*>(g_minimap->buffer);
        const int pitch = g_minimap->scanlineSize >> 1;
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                H3MapItem* mapItem = manager->GetMapItem(x, y, z);
                _word_ color = 0;
                if (mapItem && mapItem->land >= 0 && mapItem->land < 10 &&
                    manager->terrainDef[mapItem->land] && manager->terrainDef[mapItem->land]->palette565)
                {
                    bool visible = false;
                    if (IntAt(0x699588) == 0)
                    {
                        const _word_ visibility = FASTCALL_3(_word_, 0x4F8040, x, y, z);
                        visible = (visibility & static_cast<_word_>(ByteAt(0x69CD08))) != 0;
                    }
                    if ((showMines && mapItem->objectType == 53) ||
                        (showHeroes && mapItem->objectType == 34) ||
                        (showTowns && mapItem->objectType == 98))
                        visible = true;

                    if (visible)
                    {
                        const bool accent = IsObjectAccent(mapItem->GetRealType());
                        color = manager->terrainDef[mapItem->land]->palette565->color[accent ? 9 : 8];
                    }
                }
                pixels[x + y * pitch] = color;
            }
        }

        if (!ScalePcx16(P_WindowManager->screenPcx16, g_minimap,
            width, height, mmX, mmY, mmW, mmH))
            return false;

        // The stock code that follows draws the viewport with the live
        // RADAR.DEF resource and performs the requested screen update.
        return true;
    }

    _LHF_(ExtendedMinimapRenderer)
    {
        if (MapSizeIndex() < 5)
            return EXEC_DEFAULT;

        H3AdventureManager* manager =
            *reinterpret_cast<H3AdventureManager**>(c->ebp - 8);
        const UINT packedPosition = *reinterpret_cast<UINT*>(c->ebp + 8);
        const INT8 showMines = *reinterpret_cast<INT8*>(c->ebp + 0x14);
        const INT8 showHeroes = *reinterpret_cast<INT8*>(c->ebp + 0x18);
        const INT8 showTowns = *reinterpret_cast<INT8*>(c->ebp + 0x1C);
        if (!DrawExtendedMinimap(manager, packedPosition, showMines, showHeroes, showTowns))
            return EXEC_DEFAULT;

        // All stock guards and radar locals have run at this seam. Skip only
        // the unsupported H/XH/G terrain loop, retaining native RADAR.DEF
        // clipping, viewport drawing and redraw behavior.
        c->return_address = 0x413207;
        return NO_EXEC_DEFAULT;
    }

    bool HasExtendedRadarFrames(H3LoadedDef* radar)
    {
        if (!radar || radar->groupsCount < 1 || !radar->groups ||
            !radar->groups[0] || radar->groups[0]->count < 29 ||
            !radar->groups[0]->frames)
            return false;

        static const int expectedWidth[] = { 0, 76, 38, 25, 19, 15, 13, 11 };
        static const int expectedHeight[] = { 0, 68, 34, 23, 17, 13, 11, 9 };
        H3DefFrame** frames = radar->groups[0]->frames;
        if (radar->widthDEF != 144 || radar->heightDEF != 144)
            return false;
        for (int i = 1; i <= 7; ++i)
            if (!frames[i] || frames[i]->frameWidth != expectedWidth[i] ||
                frames[i]->frameHeight != expectedHeight[i])
                return false;
        return true;
    }

    _LHF_(ExtendedRadarFrameSelection)
    {
        const int sizeIndex = MapSizeIndex();
        if (!sizeIndex)
            return EXEC_DEFAULT;

        H3AdventureManager* manager =
            *reinterpret_cast<H3AdventureManager**>(c->ebp - 8);
        H3LoadedDef* radar = manager ? manager->radarDef : nullptr;
        if (!HasExtendedRadarFrames(radar))
            return EXEC_DEFAULT;

        int frame = sizeIndex;
        if (DwordAt(0x6AACA4))
        {
            switch (DwordAt(0x68C708))
            {
            case 0x41800000: frame += 7;  break; // 16.00 pixels per cell
            case 0x413D70A4: frame += 14; break; // 11.84 pixels per cell
            case 0x40F5C28F: frame += 21; break; //  7.68 pixels per cell
            default: return EXEC_DEFAULT;
            }
        }

        H3LoadedDef::DefGroup* group = radar->groups[0];
        if (frame < 0 || frame >= group->count || !group->frames[frame])
            return EXEC_DEFAULT;

        H3DefFrame* selected = group->frames[frame];
        *reinterpret_cast<int*>(c->ebp + 0x14) = frame;
        *reinterpret_cast<float*>(c->ebp + 0x18) = 144.0f / MapHeight();
        *reinterpret_cast<int*>(c->ebp + 0x1C) =
            selected->frameWidth <= 0 || selected->frameHeight <= 0;
        return EXEC_DEFAULT;
    }

    _LHF_(ExtendedRadarSelectionScale)
    {
        if (MapSizeIndex() < 5)
            return EXEC_DEFAULT;
        *reinterpret_cast<float*>(c->ebp - 8) = 144.0f / MapHeight();
        c->return_address = 0x40A0AD;
        return NO_EXEC_DEFAULT;
    }

    _LHF_(ExtendedWorldRadarScale)
    {
        if (MapSizeIndex() < 5)
            return EXEC_DEFAULT;
        *reinterpret_cast<float*>(c->ebp + 8) = 144.0f / MapHeight();
        c->return_address = 0x5FD187;
        return NO_EXEC_DEFAULT;
    }

    _LHF_(ExtendedWorldRadarX)
    {
        if (MapSizeIndex() < 5)
            return EXEC_DEFAULT;
        c->eax = 144;
        return NO_EXEC_DEFAULT;
    }

    _LHF_(ExtendedWorldRadarY)
    {
        if (MapSizeIndex() < 5)
            return EXEC_DEFAULT;
        c->edi = 144;
        return NO_EXEC_DEFAULT;
    }

    bool InstallMinimapFallback()
    {
        if (g_minimapInstalled)
            return true;
        if (::GetModuleHandleA("_HD3_.dll") || ::GetModuleHandleA("HD_WOG.dll") ||
            globalPatcher->VarGetValue<INT32>("HD.BPP", 16) != 16)
            return false;

        // Hook after stock initializes its radar state but before its
        // size-limited terrain loop. Entry and +6 stay untouched so chaining
        // minimap overlays can validate and install independently.
        const _ptr_ sites[] =
            { 0x412D6B, 0x41336E, 0x40A077, 0x5FD156, 0x5FCCD6, 0x5FCCEE };
        for (int i = 0; i < 6; ++i)
            if (globalPatcher->GetFirstPatchAt(sites[i]) != nullptr)
                return false;

        static const _byte_ renderer[] =
            { 0xC7, 0x45, 0xF0, 0x00, 0x00, 0x00, 0x00,
              0x85, 0xF6, 0x89, 0x5D, 0xBC };
        static const _byte_ radarFrame[] =
            { 0x8B, 0x45, 0x08, 0xC1, 0xE0, 0x06, 0x66, 0x3B, 0xC6, 0x89, 0x45, 0xC4 };
        static const _byte_ selection[] =
            { 0xA1, 0xCC, 0x83, 0x67, 0x00, 0x83, 0xF8, 0x24, 0x74, 0x25 };
        static const _byte_ worldScale[] =
            { 0x83, 0xF8, 0x24, 0x74, 0x25, 0x83, 0xF8, 0x48, 0x74, 0x17 };
        static const _byte_ worldX[] =
            { 0xA1, 0xC8, 0x83, 0x67, 0x00, 0x8D, 0x44, 0x41, 0xFF };
        static const _byte_ worldY[] =
            { 0x8B, 0x3D, 0xCC, 0x83, 0x67, 0x00, 0x8D, 0x7C, 0x7A, 0xFF };
        if (!BytesEqual(sites[0], renderer, sizeof(renderer)) ||
            !BytesEqual(sites[1], radarFrame, sizeof(radarFrame)) ||
            !BytesEqual(sites[2], selection, sizeof(selection)) ||
            !BytesEqual(sites[3], worldScale, sizeof(worldScale)) ||
            !BytesEqual(sites[4], worldX, sizeof(worldX)) ||
            !BytesEqual(sites[5], worldY, sizeof(worldY)))
            return false;

        Patch* patches[6] =
        {
            _PI->CreateLoHook(sites[0], ExtendedMinimapRenderer),
            _PI->CreateLoHook(sites[1], ExtendedRadarFrameSelection),
            _PI->CreateLoHook(sites[2], ExtendedRadarSelectionScale),
            _PI->CreateLoHook(sites[3], ExtendedWorldRadarScale),
            _PI->CreateLoHook(sites[4], ExtendedWorldRadarX),
            _PI->CreateLoHook(sites[5], ExtendedWorldRadarY)
        };
        g_minimapInstalled = ApplyPatchSet(patches, 6);
        return g_minimapInstalled;
    }

    bool GetImageBounds(HMODULE module, _ptr_& imageStart, _ptr_& imageEnd,
        IMAGE_NT_HEADERS32*& nt)
    {
        if (!module)
            return false;
        _byte_* base = reinterpret_cast<_byte_*>(module);
        IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0)
            return false;
        nt = reinterpret_cast<IMAGE_NT_HEADERS32*>(base + dos->e_lfanew);
        if (nt->Signature != IMAGE_NT_SIGNATURE ||
            nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 ||
            nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC ||
            nt->OptionalHeader.SizeOfImage == 0)
            return false;
        imageStart = reinterpret_cast<_ptr_>(base);
        imageEnd = imageStart + nt->OptionalHeader.SizeOfImage;
        return imageEnd > imageStart;
    }

    _ptr_ FindUniqueExecutablePattern(HMODULE module, const _byte_* pattern,
        const char* mask, size_t patternSize)
    {
        if (!pattern || patternSize == 0)
            return 0;
        _ptr_ imageStart = 0;
        _ptr_ imageEnd = 0;
        IMAGE_NT_HEADERS32* nt = nullptr;
        if (!GetImageBounds(module, imageStart, imageEnd, nt))
            return 0;

        IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
        _ptr_ match = 0;
        int matches = 0;
        for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
        {
            if (!(section->Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;
            size_t sectionSize = section->Misc.VirtualSize ?
                section->Misc.VirtualSize : section->SizeOfRawData;
            const size_t sectionOffset = section->VirtualAddress;
            if (sectionOffset >= nt->OptionalHeader.SizeOfImage ||
                sectionSize > nt->OptionalHeader.SizeOfImage - sectionOffset ||
                sectionSize < patternSize)
                continue;
            const _byte_* bytes = reinterpret_cast<const _byte_*>(imageStart + sectionOffset);
            for (size_t offset = 0; offset <= sectionSize - patternSize; ++offset)
            {
                bool equal = true;
                for (size_t j = 0; j < patternSize; ++j)
                {
                    if ((!mask || mask[j] == 'x') && bytes[offset + j] != pattern[j])
                    {
                        equal = false;
                        break;
                    }
                }
                if (equal)
                {
                    match = reinterpret_cast<_ptr_>(bytes + offset);
                    if (++matches > 1)
                        return 0;
                }
            }
        }
        return matches == 1 ? match : 0;
    }

    struct HdPointerVector
    {
        char** data;
        int count;
        int capacityBytes;
    };

    bool IsWritableImageRange(HMODULE module, _ptr_ address, size_t size)
    {
        _ptr_ imageStart = 0;
        _ptr_ imageEnd = 0;
        IMAGE_NT_HEADERS32* nt = nullptr;
        if (!size || !GetImageBounds(module, imageStart, imageEnd, nt) ||
            address < imageStart || address > imageEnd - size)
            return false;

        IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
        for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
        {
            if (!(section->Characteristics & IMAGE_SCN_MEM_WRITE))
                continue;
            const size_t sectionSize = section->Misc.VirtualSize ?
                section->Misc.VirtualSize : section->SizeOfRawData;
            const _ptr_ sectionStart = imageStart + section->VirtualAddress;
            const _ptr_ sectionEnd = sectionStart + sectionSize;
            if (sectionSize >= size && sectionEnd >= sectionStart &&
                address >= sectionStart &&
                address <= sectionEnd - size)
                return true;
        }
        return false;
    }

    bool HasAppliedPatchOwnerAt(_ptr_ address, LPCSTR expectedOwner)
    {
        if (!globalPatcher || !expectedOwner)
            return false;
        for (Patch* patch = globalPatcher->GetFirstPatchAt(address);
            patch; patch = patch->GetAppliedAfter())
        {
            LPCSTR owner = patch->GetOwner();
            if (patch->IsApplied() && patch->GetAddress() == address && owner &&
                std::strcmp(owner, expectedOwner) == 0)
                return true;
        }
        return false;
    }

    bool HasVerifiedMirrorProvider()
    {
        // HW_SOD publishes this instance/version and installs all three hooks
        // as part of its indivisible mirror-generation pipeline.  An empty
        // owner with the same name is not sufficient evidence of support.
        return globalPatcher && globalPatcher->GetInstance("wzx_HW") &&
            globalPatcher->VarGetValue<int>("mod.HWrules.ver", 0) >= 1 &&
            HasAppliedPatchOwnerAt(0x4C02C2, "wzx_HW") &&
            HasAppliedPatchOwnerAt(0x538294, "wzx_HW") &&
            HasAppliedPatchOwnerAt(0x4EE010, "wzx_HW");
    }

    bool LocateHdTemplateList(HMODULE module, HdPointerVector*& templates,
        int*& selectedIndex)
    {
        // HD_WOG rebuilds the visible template captions from this raw folder
        // vector whenever a scenario dialog is created.  Locate that builder
        // semantically instead of relying on module-version RVAs.
        static const _byte_ builderPattern[] =
        {
            0x55,0x8B,0xEC,0x83,0xEC,0x4C,0xC7,0x45,0xF0,0,0,0,0,
            0xC7,0x45,0xF4,0x01,0,0,0,0xEB,0x09,0x8B,0x45,0xF4,
            0x83,0xC0,0x01,0x89,0x45,0xF4
        };
        const _ptr_ builder = FindUniqueExecutablePattern(module,
            builderPattern, nullptr, sizeof(builderPattern));

        _ptr_ imageStart = 0;
        _ptr_ imageEnd = 0;
        IMAGE_NT_HEADERS32* nt = nullptr;
        static const _byte_ rawCountOpcode[] = { 0x83, 0x3D };
        static const _byte_ selectedOpcode[] = { 0x8B, 0x0D };
        static const _byte_ epilogue[] = { 0xC2, 0x04, 0x00 };
        if (!builder || !GetImageBounds(module, imageStart, imageEnd, nt) ||
            builder > imageEnd - 0x277 ||
            !BytesEqual(builder + 0x73, rawCountOpcode, sizeof(rawCountOpcode)) ||
            ByteAt(builder + 0x82) != 0xB9 ||
            ByteAt(builder + 0x91) != 0xA1 ||
            !BytesEqual(builder + 0x261, selectedOpcode, sizeof(selectedOpcode)) ||
            !BytesEqual(builder + 0x274, epilogue, sizeof(epilogue)))
            return false;

        const _ptr_ rawCountAddress = DwordAt(builder + 0x75);
        const _ptr_ vectorAddress = rawCountAddress - sizeof(int);
        const _ptr_ selectedAddress = DwordAt(builder + 0x263);
        if (DwordAt(builder + 0x83) != vectorAddress ||
            DwordAt(builder + 0x92) != vectorAddress ||
            !IsWritableImageRange(module, vectorAddress, sizeof(HdPointerVector)) ||
            !IsWritableImageRange(module, selectedAddress, sizeof(int)))
            return false;

        templates = reinterpret_cast<HdPointerVector*>(vectorAddress);
        selectedIndex = reinterpret_cast<int*>(selectedAddress);
        return true;
    }

    bool FilterUnsupportedHdTemplates(HMODULE module)
    {
        if (!module || HasVerifiedMirrorProvider())
            return true;

        HdPointerVector* templates = nullptr;
        int* selectedIndex = nullptr;
        if (!LocateHdTemplateList(module, templates, selectedIndex) ||
            !templates || !selectedIndex || templates->count <= 0 ||
            templates->count > 512 || templates->capacityBytes < templates->count * 4 ||
            templates->capacityBytes > 1024 * 1024 || !templates->data ||
            ::IsBadWritePtr(templates->data, templates->count * sizeof(char*)))
            return false;

        if (templates->data == g_lastFilteredTemplateData &&
            templates->count == g_lastFilteredTemplateCount)
            return true;

        const int oldCount = templates->count;
        for (int i = 0; i < oldCount; ++i)
        {
            LPCSTR name = templates->data[i];
            if (!name || ::IsBadStringPtrA(name, MAX_PATH) || !*name)
                return false;
        }

        const int oldSelection = *selectedIndex;
        char* selectedName = oldSelection >= 0 && oldSelection < oldCount ?
            templates->data[oldSelection] : nullptr;
        int newCount = 0;
        int newSelection = -1;
        for (int read = 0; read < oldCount; ++read)
        {
            char* name = templates->data[read];
            // Entry zero is HD's built-in Default choice and is always kept.
            if (read > 0 && ReadTemplatePlayability(name) == TEMPLATE_UNPLAYABLE)
                continue;
            templates->data[newCount] = name;
            if (name == selectedName)
                newSelection = newCount;
            ++newCount;
        }

        for (int i = newCount; i < oldCount; ++i)
            templates->data[i] = nullptr;
        templates->count = newCount;
        *selectedIndex = newSelection >= 0 ? newSelection : 0;
        g_lastFilteredTemplateData = templates->data;
        g_lastFilteredTemplateCount = templates->count;
        return true;
    }

    _ptr_ FindSoleDirectCaller(HMODULE module, _ptr_ target)
    {
        _ptr_ imageStart = 0;
        _ptr_ imageEnd = 0;
        IMAGE_NT_HEADERS32* nt = nullptr;
        if (!target || !GetImageBounds(module, imageStart, imageEnd, nt))
            return 0;

        _ptr_ match = 0;
        int matches = 0;
        IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(nt);
        for (unsigned i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
        {
            if (!(section->Characteristics & IMAGE_SCN_MEM_EXECUTE))
                continue;
            size_t sectionSize = section->Misc.VirtualSize ?
                section->Misc.VirtualSize : section->SizeOfRawData;
            const size_t sectionOffset = section->VirtualAddress;
            if (sectionOffset >= nt->OptionalHeader.SizeOfImage ||
                sectionSize > nt->OptionalHeader.SizeOfImage - sectionOffset || sectionSize < 5)
                continue;
            const _byte_* bytes = reinterpret_cast<const _byte_*>(imageStart + sectionOffset);
            for (size_t offset = 0; offset <= sectionSize - 5; ++offset)
            {
                if (bytes[offset] != 0xE8)
                    continue;
                const _ptr_ call = reinterpret_cast<_ptr_>(bytes + offset);
                const INT32 displacement = *reinterpret_cast<const INT32*>(call + 1);
                if (call + 5 + displacement == target)
                {
                    match = call;
                    if (++matches > 1)
                        return 0;
                }
            }
        }
        return matches == 1 ? match : 0;
    }

    void __stdcall HdNormalizeTemplateSizes(HiHook*, int islands,
        int* minimum, int* maximum)
    {
        if (!minimum || !maximum)
            return;
        if (islands)
        {
            *minimum *= 2;
            *maximum *= 2;
        }
        for (int i = 0; i < static_cast<int>(sizeof(kTemplateAreas) / sizeof(kTemplateAreas[0])); ++i)
            if (*minimum <= kTemplateAreas[i]) { *minimum = kTemplateAreas[i]; break; }
        for (int i = static_cast<int>(sizeof(kTemplateAreas) / sizeof(kTemplateAreas[0])) - 1; i >= 0; --i)
            if (*maximum >= kTemplateAreas[i]) { *maximum = kTemplateAreas[i]; break; }
    }

    int __stdcall HdSizeToItem(HiHook*, int dimension)
    {
        switch (dimension)
        {
        case MAP_H: return ITEM_H;
        case MAP_XH: return ITEM_XH;
        case MAP_G: return ITEM_G;
        default: return dimension / MAP_S + 280;
        }
    }

    bool ValidateNormalizer(HMODULE module, _ptr_ normalize)
    {
        static const _byte_ epilogue[] = { 0x8B, 0xE5, 0x5D, 0xC2, 0x0C, 0x00 };
        static const _byte_ originalTable[] = { 1, 2, 4, 8, 9, 16, 18, 32 };
        static const _byte_ callerPrefix[] =
            { 0x8D, 0x45, 0xF4, 0x50, 0x8D, 0x4D, 0xF8, 0x51,
              0x0F, 0xB6, 0x55, 0x0C, 0x52 };
        static const _byte_ callerSuffix[] = { 0x83, 0x7D, 0x08, 0x00 };

        _ptr_ imageStart = 0;
        _ptr_ imageEnd = 0;
        IMAGE_NT_HEADERS32* nt = nullptr;
        if (!GetImageBounds(module, imageStart, imageEnd, nt) ||
            normalize < imageStart || normalize > imageEnd - 0xA4 ||
            !BytesEqual(normalize + 0x9E, epilogue, sizeof(epilogue)))
            return false;

        const _ptr_ table1 = DwordAt(normalize + 0x44);
        const _ptr_ table2 = DwordAt(normalize + 0x55);
        const _ptr_ table3 = DwordAt(normalize + 0x80);
        const _ptr_ table4 = DwordAt(normalize + 0x91);
        if (table1 != table2 || table1 != table3 || table1 != table4 ||
            table1 < imageStart || table1 > imageEnd - sizeof(originalTable) ||
            !BytesEqual(table1, originalTable, sizeof(originalTable)))
            return false;

        const _ptr_ caller = FindSoleDirectCaller(module, normalize);
        return caller >= imageStart + sizeof(callerPrefix) &&
            caller + 5 + sizeof(callerSuffix) <= imageEnd &&
            BytesEqual(caller - sizeof(callerPrefix), callerPrefix, sizeof(callerPrefix)) &&
            BytesEqual(caller + 5, callerSuffix, sizeof(callerSuffix));
    }

    bool IsDirectCallTo(_ptr_ callAddress, _ptr_ target)
    {
        return ByteAt(callAddress) == 0xE8 &&
            callAddress + 5 + *reinterpret_cast<const INT32*>(callAddress + 1) == target;
    }

    LPCSTR __stdcall HdFormatTemplateSize(HiHook* hook, int areaToken)
    {
        // Preserve HD's internal alternating-buffer state even when replacing
        // the returned caption for an extended size.
        LPCSTR original = STDCALL_1(LPCSTR, hook->GetDefaultFunc(), areaToken);
        switch (areaToken)
        {
        case 25: return "H";
        case 50: return "H+U";
        case 36: return "XH";
        case 72: return "XH+U";
        case 49: return "G";
        case 98: return "G+U";
        default: return original;
        }
    }

    bool TryInstallHdTemplateFormatter(HMODULE module)
    {
        if (g_hdTemplateFormatterInstalled)
            return true;
        if (!module)
            return false;

        static const _byte_ formatterMiddle[] =
        {
            0x8B,0x4D,0x08,0x89,0x4D,0xF0,0x8B,0x55,0xF0,0x83,
            0xEA,0x01,0x89,0x55,0xF0,0x83,0x7D,0xF0,0x1F,0x77
        };
        static const _byte_ prologue[] =
            { 0x55,0x8B,0xEC,0x83,0xEC,0x10 };
        static const _byte_ epilogue[] =
            { 0x8B,0xE5,0x5D,0xC2,0x04,0x00 };
        const _ptr_ middle = FindUniqueExecutablePattern(module,
            formatterMiddle, nullptr, sizeof(formatterMiddle));

        _ptr_ imageStart = 0;
        _ptr_ imageEnd = 0;
        IMAGE_NT_HEADERS32* nt = nullptr;
        if (!middle || !GetImageBounds(module, imageStart, imageEnd, nt) ||
            middle < imageStart + 0x20)
            return false;
        const _ptr_ formatter = middle - 0x20;
        if (formatter > imageEnd - 0x1B2 ||
            !BytesEqual(formatter, prologue, sizeof(prologue)) ||
            !BytesEqual(formatter + 0x103, epilogue, sizeof(epilogue)) ||
            !IsDirectCallTo(formatter + 0x185, formatter) ||
            !IsDirectCallTo(formatter + 0x1A3, formatter) ||
            !IsDirectCallTo(formatter + 0x1AD, formatter) ||
            globalPatcher->GetFirstPatchAt(formatter))
            return false;

        Patch* patch = _PI->CreateHiHook(formatter, SPLICE_, EXTENDED_, STDCALL_,
            reinterpret_cast<void*>(HdFormatTemplateSize));
        Patch* patches[] = { patch };
        g_hdTemplateFormatterInstalled = ApplyPatchSet(patches, 1);
        return g_hdTemplateFormatterInstalled;
    }

    bool ValidateHdTemplateUpdater(HMODULE module, _ptr_ updater, _ptr_ mapper)
    {
        static const _byte_ prologue[] = { 0x55,0x8B,0xEC,0x83,0xEC,0x60 };
        static const _byte_ epilogue[] = { 0x8B,0xE5,0x5D,0xC2,0x08,0x00 };
        static const _byte_ firstCallerPrefix[] =
            { 0x8B,0x55,0x08,0x8B,0x82,0xA0,0x18,0x00,0x00,0x3B,0x45,0xD0,
              0x7D,0x51,0x8B,0x4D,0xD0,0x51 };
        static const _byte_ secondCallerPrefix[] =
            { 0x8B,0x55,0x08,0x8B,0x82,0xA0,0x18,0x00,0x00,0x3B,0x45,0xCC,
              0x7E,0x51,0x8B,0x4D,0xCC,0x51 };

        _ptr_ imageStart = 0;
        _ptr_ imageEnd = 0;
        IMAGE_NT_HEADERS32* nt = nullptr;
        if (!GetImageBounds(module, imageStart, imageEnd, nt) ||
            updater < imageStart || updater > imageEnd - 0x410 ||
            mapper + 0x20 != updater ||
            !BytesEqual(updater, prologue, sizeof(prologue)) ||
            !BytesEqual(updater + 0x40A, epilogue, sizeof(epilogue)) ||
            !BytesEqual(updater + 0x1C9 - sizeof(firstCallerPrefix),
                firstCallerPrefix, sizeof(firstCallerPrefix)) ||
            !BytesEqual(updater + 0x28B - sizeof(secondCallerPrefix),
                secondCallerPrefix, sizeof(secondCallerPrefix)) ||
            !IsDirectCallTo(updater + 0x1C9, mapper) ||
            !IsDirectCallTo(updater + 0x28B, mapper))
            return false;

        // The first four absolute operands are the updater's cookie and
        // template minimum/maximum globals.  Relocations may change their
        // values, but all must still point inside HD_WOG's image.
        const _ptr_ globals[] =
        {
            DwordAt(updater + 7), DwordAt(updater + 17),
            DwordAt(updater + 26), DwordAt(updater + 35)
        };
        for (int i = 0; i < 4; ++i)
            if (globals[i] < imageStart || globals[i] >= imageEnd)
                return false;
        return true;
    }

    int __stdcall HdTemplateUpdater(HiHook* hook,
        H3SelectScenarioDialog* dialog, int showErrors)
    {
        const bool nestedCorrection = g_refreshingTemplateSizes;
        const bool unlockedForCorrection = !nestedCorrection &&
            !CurrentTemplateSelectionSupported(dialog);
        TemplateButtonEnabledState previous;
        CaptureTemplateButtonEnabledState(dialog, previous);

        // Unlock controls only when HD must correct an invalid combination.
        // Ordinary size, Underground and player-setting clicks already have a
        // valid selection and must not churn every DEF's enabled state.
        if (unlockedForCorrection)
        {
            SetAllSizeButtonsEnabled(dialog, true);
            SetUndergroundButtonEnabled(dialog, true, false);
        }

        ++g_hdTemplateUpdateDepth;
        const int result = STDCALL_2(int, hook->GetDefaultFunc(), dialog, showErrors);
        --g_hdTemplateUpdateDepth;

        if (!nestedCorrection)
        {
            // Apply the final union mask without repainting the whole row.
            // Native code owns selected/highlight frames; XXL repaints only
            // controls whose final state changed. If HD needed temporary
            // unlocking, final-disabled controls are repainted as well so a
            // native synthetic click cannot leave an enabled-looking frame.
            RefreshTemplateSizes(dialog, true, false);
            RedrawFinalTemplateButtonStates(
                dialog, previous, unlockedForCorrection);
        }
        return result;
    }

    bool TryInstallHdTemplateUpdater(HMODULE module, _ptr_ mapper)
    {
        if (g_hdTemplateUpdaterInstalled)
            return true;
        if (!module || !mapper)
            return false;

        const _ptr_ updater = mapper + 0x20;
        if (!ValidateHdTemplateUpdater(module, updater, mapper) ||
            globalPatcher->GetFirstPatchAt(updater))
            return false;

        Patch* patch = _PI->CreateHiHook(updater, SPLICE_, EXTENDED_, STDCALL_,
            reinterpret_cast<void*>(HdTemplateUpdater));
        Patch* patches[] = { patch };
        g_hdTemplateUpdaterInstalled = ApplyPatchSet(patches, 1);
        return g_hdTemplateUpdaterInstalled;
    }

    bool InstallHdCompatibility()
    {
        HMODULE hdWog = ::GetModuleHandleA("HD_WOG.dll");
        if (!hdWog)
            return false;

        // This is intentionally retried for every scenario-dialog creation.
        // HD normally enumerates folders only once, so subsequent calls are
        // idempotent; retrying also catches a list rebuilt by a future build.
        FilterUnsupportedHdTemplates(hdWog);
        TryInstallHdTemplateFormatter(hdWog);

        if (g_hdCompatibilityInstalled)
        {
            if (!g_hdTemplateUpdaterInstalled)
                TryInstallHdTemplateUpdater(hdWog, g_hdMapperAddress);
            return true;
        }

        static const _byte_ clampPattern[] =
        {
            0x81,0xB9,0xA0,0x18,0,0,0,0,0,0, 0x7E,0x09,0xC7,0x45,0xF4,0,0,0,0,
            0xEB,0x0C,0x8B,0x55,0xFC,0x8B,0x82,0xA0,0x18,0,0
        };
        static const char clampMask[] = "xxxxxx????xxxxx????xxxxxxxxxxx";
        static const _byte_ normalizePattern[] =
        {
            0x55,0x8B,0xEC,0x83,0xEC,0x08,0x0F,0xB6,0x45,0x08,0x85,0xC0,0x74,0x18,
            0x8B,0x4D,0x0C,0x8B,0x11,0xD1,0xE2,0x8B,0x45,0x0C,0x89,0x10,0x8B,0x4D,
            0x10,0x8B,0x11,0xD1,0xE2,0x8B,0x45,0x10,0x89,0x10,0xC7,0x45,0xFC,0,0,0,0
        };
        static const _byte_ mapperPattern[] =
        {
            0x55,0x8B,0xEC,0x8B,0x45,0x08,0x99,0xB9,0x24,0,0,0,0xF7,0xF9,
            0x05,0x18,0x01,0,0,0x5D,0xC2,0x04,0
        };
        const _ptr_ clamp = FindUniqueExecutablePattern(
            hdWog, clampPattern, clampMask, sizeof(clampPattern));
        const _ptr_ normalize = FindUniqueExecutablePattern(
            hdWog, normalizePattern, nullptr, sizeof(normalizePattern));
        const _ptr_ mapper = FindUniqueExecutablePattern(
            hdWog, mapperPattern, nullptr, sizeof(mapperPattern));
        if (!clamp || !normalize || !mapper || !ValidateNormalizer(hdWog, normalize) ||
            globalPatcher->GetFirstPatchAt(normalize) ||
            globalPatcher->GetFirstPatchAt(mapper))
            return false;

        const int cap = *reinterpret_cast<int*>(clamp + 6);
        const int replacement = *reinterpret_cast<int*>(clamp + 15);
        if (cap != replacement || (cap != MAP_XL && cap != MAP_G))
            return false;

        Patch* patches[4];
        int count = 0;
        if (cap == MAP_XL)
        {
            if (globalPatcher->GetFirstPatchAt(clamp + 6) ||
                globalPatcher->GetFirstPatchAt(clamp + 15))
                return false;
            patches[count++] = _PI->CreateDwordPatch(clamp + 6, MAP_G);
            patches[count++] = _PI->CreateDwordPatch(clamp + 15, MAP_G);
        }
        patches[count++] = _PI->CreateHiHook(normalize, SPLICE_, EXTENDED_, STDCALL_,
            reinterpret_cast<void*>(HdNormalizeTemplateSizes));
        patches[count++] = _PI->CreateHiHook(mapper, SPLICE_, EXTENDED_, STDCALL_,
            reinterpret_cast<void*>(HdSizeToItem));
        g_hdCompatibilityInstalled = ApplyPatchSet(patches, count);
        if (g_hdCompatibilityInstalled)
        {
            g_hdMapperAddress = mapper;
            TryInstallHdTemplateUpdater(hdWog, mapper);
        }
        return g_hdCompatibilityInstalled;
    }

    void __stdcall OnAfterWoG(Era::TEvent*)
    {
        // Regular DLL plugins are now loaded. The no-HD path can safely claim
        // its six stock seams here.
        InstallMinimapFallback();
    }

}

void XXLRuntimeFix_Init()
{
    if (!g_afterWogRegistered)
    {
        g_afterWogRegistered = true;
        Era::RegisterHandler(OnAfterWoG, "OnAfterWoG");
    }
}

bool XXLRuntimeFix_EnsureHdCompatibility()
{
    return InstallHdCompatibility();
}

void XXLRuntimeFix_RefreshTemplateSizes(H3SelectScenarioDialog* dialog, bool correctInvalid)
{
    // Called while the random-map controls are still being constructed; the
    // parent dialog will paint the resulting states on its normal first draw.
    RefreshTemplateSizes(dialog, correctInvalid, false);
}

void XXLRuntimeFix_BeforeScenarioDialogMessage(H3SelectScenarioDialog* dialog,
    H3Msg* message)
{
    PrepareScenarioDialogMessage(dialog, message);
}

void XXLRuntimeFix_AfterScenarioDialogMessage(H3SelectScenarioDialog* dialog)
{
    if (!dialog || g_refreshingTemplateSizes || g_hdTemplateUpdateDepth > 0)
        return;

    HMODULE hdWog = ::GetModuleHandleA("HD_WOG.dll");
    if (hdWog)
    {
        if (!g_hdTemplateUpdaterInstalled)
            SetAllSizeButtonsEnabled(dialog, true, true);
        return;
    }
    RefreshTemplateSizes(dialog, true, true);
}
