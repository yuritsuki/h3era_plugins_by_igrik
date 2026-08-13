////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// Диалог WoG опций /////////////////////////////////////////////////////////////////////

#include <cwctype>
#include <string>
#include <vector>

struct _DlgSetup_;
#define o_DlgSetup ((_DlgSetup_*)0x2918390)
#define o_LastChoosenPage (*(_int_*)0x7B35FC)
#define o_WoGOptionIdMap ((int*)0x2919030)

// Implemented by the JSON lock support in WoGOptionsStrings.cpp.
bool IsWoGOptionLocked(int optionId);
const char* GetWoGOptionLockReason(int optionId);
bool FormatWoGOptionLockLabel(int optionId, char* output, size_t outputSize);
void RefreshWoGOptionLocks();
void ForceWoGOptionLockValues();
void ReapplyWoGOptionLocks(_DlgSetup_* ds);

_DlgStaticTextPcx8ed_* statbarWoGOptions = NULL;

namespace WoGOptionColors
{
    const int REGULAR = 1;
    const int HIGHLIGHT = 2;
    const int GOLD_TEXT = 3;
    const int WHITE = 4;
    const int LOCKED_TEXT = GOLD_TEXT;
}

namespace WoGOptionSkin
{
    const char* const BASE_PCX = "WOGOBASE.PCX";
    const char* const DARK_PCX = "WOGODARK.PCX";
    const char* const SELECT_PCX = "WOGOSEL.PCX";
    const char* const HEADER_PCX = "WOGOHDR.PCX";
    const char* const STATUS_PCX = "WOGOSTAT.PCX";
    const char* const BOTTOM_CORNER_LEFT_PCX = "WOGOBCL.PCX";
    const char* const BOTTOM_CORNER_RIGHT_PCX = "WOGOBCR.PCX";
    const char* const CORNER_TL_PCX = "WOGOTL.PCX";
    const char* const CORNER_TR_PCX = "WOGOTR.PCX";
    const char* const CORNER_BL_PCX = "WOGOBL.PCX";
    const char* const CORNER_BR_PCX = "WOGOBR.PCX";
    const char* const POPUP_TOP_PCX = "WOGOPT.PCX";
    const char* const POPUP_BOTTOM_PCX = "WOGOPB.PCX";
    const char* const POPUP_LEFT_PCX = "WOGOPL.PCX";
    const char* const POPUP_RIGHT_PCX = "WOGOPR.PCX";
    const char* const POPUP_TOP_CLASP_PCX = "WOGOCTA.PCX";
    const char* const BUTTON_DEF = "WOGOBTN.DEF";
    const char* const OPTION_CHECKBOX_DEF = "WOGOCHK.DEF";
    const char* const OPTION_RADIO_DEF = "WOGORAD.DEF";
    const char* const PRESET_BASE_PCX = "WOGOPRST.PCX";
    const char* const PRESET_LIST_PCX = "WOGOLST.PCX";
    const char* const PRESET_EDIT_PCX = "WOGOTXT.PCX";
    const char* const PRESET_HINT_PCX = "WOGOPHNT.PCX";
    const char* const PRESET_SCROLL_DEF = "WOGOSCR.DEF";
    const char* const PRESET_SCROLL_PCX = "WOGOSCV.PCX";
    const int BASE_SURFACE_ID = 18399;
    const int ACTIVE_PAGE_SURFACE_ID = 18404;
    const int BOTTOM_CORNER_LEFT_SURFACE_ID = 18413;
    const int BOTTOM_CORNER_RIGHT_SURFACE_ID = 18414;
    const int GROUP_HEADER_SURFACE_BASE = 19000;

    bool available = false;
    bool optionControlsAvailable = false;

    bool IsValidOptionControlDef(const char* defName)
    {
        _Def_* customDef = o_LoadDef((char*)defName);
        if (!customDef)
            return false;

        const bool valid = customDef->width == 16 && customDef->height == 16 &&
            customDef->groups_count >= 1 && customDef->groups && customDef->groups[0] &&
            customDef->groups[0]->frames_count >= 6;
        customDef->DerefOrDestruct();
        return valid;
    }

    bool HasOptionControlResources()
    {
        return IsValidOptionControlDef(OPTION_CHECKBOX_DEF) &&
            IsValidOptionControlDef(OPTION_RADIO_DEF);
    }

    bool HasResources()
    {
        return Era::PcxPngExists && Era::PcxPngExists(BASE_PCX) &&
            Era::PcxPngExists(DARK_PCX) && Era::PcxPngExists(SELECT_PCX) &&
            Era::PcxPngExists(HEADER_PCX) && Era::PcxPngExists(STATUS_PCX) &&
            Era::PcxPngExists(BOTTOM_CORNER_LEFT_PCX) &&
            Era::PcxPngExists(BOTTOM_CORNER_RIGHT_PCX);
    }

    bool HasPopupResources()
    {
        return available && Era::PcxPngExists(CORNER_TL_PCX) &&
            Era::PcxPngExists(CORNER_TR_PCX) && Era::PcxPngExists(CORNER_BL_PCX) &&
            Era::PcxPngExists(CORNER_BR_PCX) && Era::PcxPngExists(POPUP_TOP_PCX) &&
            Era::PcxPngExists(POPUP_BOTTOM_PCX) && Era::PcxPngExists(POPUP_LEFT_PCX) &&
            Era::PcxPngExists(POPUP_RIGHT_PCX) && Era::PcxPngExists(POPUP_TOP_CLASP_PCX);
    }

    bool HasPresetResources()
    {
        return HasResources() && Era::PcxPngExists(PRESET_BASE_PCX) &&
            Era::PcxPngExists(PRESET_LIST_PCX) &&
            Era::PcxPngExists(PRESET_EDIT_PCX) &&
            Era::PcxPngExists(PRESET_HINT_PCX) &&
            Era::PcxPngExists(PRESET_SCROLL_PCX);
    }

    void CreateBackground(_CustomDlg_* dlg)
    {
        available = dlg && HasResources();
        optionControlsAvailable = dlg && HasOptionControlResources();
        if (available)
        {
            // The complete fixed-size PNG owns the background and custom frame.
            // The stock dialgbox.def path remains only as a missing-resource fallback.
            dlg->AddItem(_DlgStaticPcx8_::Create(0, 0, dlg->width, dlg->height,
                BASE_SURFACE_ID, (char*)BASE_PCX));
        }
        else if (dlg)
            Set_DlgBackground_RK(dlg, 1, o_GameMgr->GetMeID());
    }

    void CreateBaseSurfaces(_CustomDlg_* dlg)
    {
        if (!available || !dlg)
            return;

        _DlgItem_* activePage = _DlgStaticPcx8_::Create(22, 49, 194, 49,
            ACTIVE_PAGE_SURFACE_ID, (char*)SELECT_PCX);
        dlg->AddItem(activePage);
        activePage->Hide_ButStayEnable();
    }

    void CreateFooterOrnaments(_CustomDlg_* dlg)
    {
        if (!available || !dlg)
            return;

        // The alpha corners occupy [0,64) and [736,800). The hint at [48,752)
        // extends beneath them so its full text area is owned and erased. Keep
        // the corners later in normal dialog z-order so they cover that overlap;
        // direct item-only reblits re-composite their translucent pixels and
        // visibly flash during mouse-over status updates.
        dlg->AddItem(_DlgStaticPcx8_::Create(0, 536, 64, 64,
            BOTTOM_CORNER_LEFT_SURFACE_ID, (char*)BOTTOM_CORNER_LEFT_PCX));
        dlg->AddItem(_DlgStaticPcx8_::Create(dlg->width - 64, 536, 64, 64,
            BOTTOM_CORNER_RIGHT_SURFACE_ID, (char*)BOTTOM_CORNER_RIGHT_PCX));
    }

    void UpdateStatusText(_CustomDlg_* dlg, const char* text,
        bool dialogRedrawPending = false)
    {
        if (!statbarWoGOptions)
            return;

        char* nextText = (char*)(text ? text : o_NullString);
        const char* currentText = statbarWoGOptions->text
            ? statbarWoGOptions->text : o_NullString;
        if (!strcmp(currentText, nextText))
            return;

        statbarWoGOptions->SetText(nextText);
        if (dialogRedrawPending)
            return;

        if (available && dlg)
        {
            // The custom status surface overlaps both foreground corner tiles.
            // Redraw the dialog once in item order so a hint update cannot
            // overwrite those later alpha layers.
            dlg->Redraw();
        }
        else
        {
            statbarWoGOptions->Draw();
            statbarWoGOptions->RedrawScreen();
        }
    }

    bool CreatePresetBackground(_CustomDlg_* dlg)
    {
        if (!dlg || !HasPresetResources())
            return false;

        dlg->AddItem(_DlgStaticPcx8_::Create(0, 0, dlg->width, dlg->height,
            19420, (char*)PRESET_BASE_PCX));
        return true;
    }

    bool ApplyPresetScrollbarSkin(_DlgScroll_* scroll)
    {
        if (!scroll || !HasPresetResources())
            return false;

        _Def_* customDef = o_LoadDef((char*)PRESET_SCROLL_DEF);
        if (!customDef)
            return false;

        const bool validDef = customDef->width == 16 && customDef->height == 16 &&
            customDef->groups_count >= 1 && customDef->groups && customDef->groups[0] &&
            customDef->groups[0]->frames_count >= 5;
        if (!validDef)
        {
            customDef->DerefOrDestruct();
            return false;
        }

        _Pcx8_* customTrack = o_LoadPcx8((char*)PRESET_SCROLL_PCX);
        if (!customTrack || customTrack->width < 16 || customTrack->height < 218)
        {
            if (customTrack)
                customTrack->DerefOrDestruct();
            customDef->DerefOrDestruct();
            return false;
        }

        // The native scrollbar owns one reference to each resource and releases
        // whatever pointers are stored when it is destroyed. Transfer the new
        // references only after both resources have passed their geometry checks.
        _Def_* stockDef = scroll->def;
        _Pcx8_* stockTrack = scroll->pcx;
        scroll->def = customDef;
        scroll->pcx = customTrack;
        if (stockDef)
            stockDef->DerefOrDestruct();
        if (stockTrack)
            stockTrack->DerefOrDestruct();
        return true;
    }

    void CreatePopupBackground(_CustomDlg_* dlg)
    {
        if (!dlg || !HasPopupResources())
            return;

        dlg->AddItem(_DlgStaticPcx8_::Create(0, 0, dlg->width, dlg->height,
            19400, (char*)DARK_PCX));

        // Use one artwork family for the full popup frame. Mixing the palette
        // frame helper with opaque PNG corners caused visible colour and
        // alignment seams at every corner.
        dlg->AddItem(_DlgStaticPcx8_::Create(0, 0, dlg->width, 8, 19401,
            (char*)POPUP_TOP_PCX));
        dlg->AddItem(_DlgStaticPcx8_::Create(0, dlg->height - 8, dlg->width, 8, 19402,
            (char*)POPUP_BOTTOM_PCX));
        dlg->AddItem(_DlgStaticPcx8_::Create(0, 0, 8, dlg->height, 19403,
            (char*)POPUP_LEFT_PCX));
        dlg->AddItem(_DlgStaticPcx8_::Create(dlg->width - 8, 0, 8, dlg->height, 19404,
            (char*)POPUP_RIGHT_PCX));

        const int cornerSize = 46;
        dlg->AddItem(_DlgStaticPcx8_::Create(0, 0, cornerSize, cornerSize, 19405,
            (char*)CORNER_TL_PCX));
        dlg->AddItem(_DlgStaticPcx8_::Create(dlg->width - cornerSize, 0,
            cornerSize, cornerSize, 19406,
            (char*)CORNER_TR_PCX));
        dlg->AddItem(_DlgStaticPcx8_::Create(0, dlg->height - cornerSize,
            cornerSize, cornerSize, 19407,
            (char*)CORNER_BL_PCX));
        dlg->AddItem(_DlgStaticPcx8_::Create(dlg->width - cornerSize,
            dlg->height - cornerSize, cornerSize, cornerSize, 19408,
            (char*)CORNER_BR_PCX));
        const int claspWidth = 48;
        dlg->AddItem(_DlgStaticPcx8_::Create((dlg->width - claspWidth) / 2, 0,
            claspWidth, 16, 19409, (char*)POPUP_TOP_CLASP_PCX));
    }

    void UpdateActivePage(_CustomDlg_* dlg, int page)
    {
        if (!available || !dlg)
            return;
        _DlgItem_* surface = dlg->GetItem(ACTIVE_PAGE_SURFACE_ID);
        if (!surface)
            return;
        if (page >= 0 && page < 8)
        {
            surface->SetRect(22, 49 + 60 * page, 194, 49);
            surface->Show_ButStayEnable();
        }
        else
            surface->Hide_ButStayEnable();
    }

}

namespace WoGOptionRows
{
    enum Kind
    {
        NATIVE,
        EXTERNAL
    };

    struct RowRef
    {
        Kind kind;
        int page;
        int group;
        int item;
        int optionId;
        _DlgSetup_ItemList_* list;
        WoGExternalOptions::SessionRow* external;

        RowRef() : kind(NATIVE), page(-1), group(-1), item(-1), optionId(-1),
            list(NULL), external(NULL) {}
    };

    int NativeCount(_DlgSetup_* ds, int page, int group)
    {
        if (!ds || page < 0 || page >= 8 || group < 0 || group >= 4 ||
            !ds->Pages[page] || !ds->Pages[page]->ItemList[group])
            return 0;
        return min(max(ds->Pages[page]->ItemList[group]->ItemCount, 0), 20);
    }

    int Count(_DlgSetup_* ds, int page, int group)
    {
        return min(20, NativeCount(ds, page, group) +
            WoGExternalOptions::Count(page, group));
    }

    int NativeOptionIdAt(_DlgSetup_* ds, int page, int group, int item)
    {
        const int nativeCount = NativeCount(ds, page, group);
        if (item < 0 || item >= nativeCount)
            return -1;

        int optionId = o_WoGOptionIdMap[page * 80 + group * 20 + item];
        _DlgSetup_ItemList_* list = ds->Pages[page]->ItemList[group];
        if (optionId < 0 && list->Type == 2)
        {
            for (int i = 0; i < nativeCount && optionId < 0; ++i)
                optionId = o_WoGOptionIdMap[page * 80 + group * 20 + i];
        }
        return optionId;
    }

    bool Resolve(_DlgSetup_* ds, int page, int group, int item, RowRef& row)
    {
        row = RowRef();
        if (!ds || page < 0 || page >= 8 || group < 0 || group >= 4 || item < 0 ||
            item >= Count(ds, page, group) || !ds->Pages[page] ||
            !ds->Pages[page]->ItemList[group])
            return false;

        row.page = page;
        row.group = group;
        row.item = item;
        row.list = ds->Pages[page]->ItemList[group];
        const int nativeCount = NativeCount(ds, page, group);
        if (item < nativeCount)
        {
            row.kind = NATIVE;
            row.optionId = NativeOptionIdAt(ds, page, group, item);
            // Rendering never depended on the canonical option-ID table.
            // Keep an unmapped native row visible even though it cannot be
            // indexed or decorated with ID-based search/lock information.
            return true;
        }

        row.kind = EXTERNAL;
        row.external = WoGExternalOptions::At(page, group, item - nativeCount);
        if (row.external)
            row.optionId = WoGExternalOptions::OptionId(*row.external);
        return row.external != NULL;
    }

    bool DecodeControl(_DlgSetup_* ds, int controlId, RowRef& row)
    {
        if (controlId < 1000 || controlId >= 9000)
            return false;
        const int page = controlId / 1000 - 1;
        const int local = controlId % 1000;
        const int group = local / 200;
        const int remainder = local % 200;
        const int item = remainder >= 100 ? remainder - 100 : remainder;
        if (!Resolve(ds, page, group, item, row))
            return false;
        return row.kind == EXTERNAL || row.optionId >= 0;
    }

    const char* Name(const RowRef& row)
    {
        if (row.kind == EXTERNAL && row.external && row.external->provider)
            return WoGExternalOptions::GetText(*row.external->provider,
                WOG_EXTERNAL_TEXT_NAME);
        return row.list && row.list->ItemName ? row.list->ItemName[row.item] : o_NullString;
    }

    const char* Hint(const RowRef& row)
    {
        if (row.kind == EXTERNAL && row.external && row.external->provider)
            return WoGExternalOptions::GetText(*row.external->provider,
                WOG_EXTERNAL_TEXT_HINT);
        return row.list && row.list->ItemHint ? row.list->ItemHint[row.item] : o_NullString;
    }

    const char* Popup(const RowRef& row)
    {
        if (row.kind == EXTERNAL && row.external && row.external->provider)
            return WoGExternalOptions::GetText(*row.external->provider,
                WOG_EXTERNAL_TEXT_POPUP);
        return row.list && row.list->ItemPopUp ? row.list->ItemPopUp[row.item] : o_NullString;
    }

    const char* Badge(const RowRef& row)
    {
        if (row.kind == EXTERNAL && row.external && row.external->provider)
            return WoGExternalOptions::GetText(*row.external->provider,
                WOG_EXTERNAL_TEXT_SOURCE_BADGE);
        return o_NullString;
    }

    const char* SearchAliases(const RowRef& row)
    {
        if (row.kind == EXTERNAL && row.external && row.external->provider)
            return WoGExternalOptions::GetText(*row.external->provider,
                WOG_EXTERNAL_TEXT_SEARCH_ALIASES);
        return o_NullString;
    }

    int State(RowRef& row, bool refreshExternal = true)
    {
        if (row.kind == EXTERNAL && row.external)
        {
            const uint32_t state = refreshExternal
                ? WoGExternalOptions::Refresh(*row.external) : row.external->state;
            const bool checked = (state & WOG_EXTERNAL_STATE_CHECKED) != 0;
            const bool enabled = (state & WOG_EXTERNAL_STATE_ENABLED) != 0;
            return enabled ? (checked ? 1 : 0) : (checked ? 3 : 2);
        }
        if (!row.list || !row.list->ItemState)
            return 2;
        return row.list->ItemState[row.item];
    }

    bool IsEnabled(RowRef& row)
    {
        if (row.kind == EXTERNAL && row.external)
            return (WoGExternalOptions::Refresh(*row.external) &
                WOG_EXTERNAL_STATE_ENABLED) != 0;
        const int state = State(row, false);
        return state >= 0 && state < 2;
    }

    bool IsLocked(const RowRef& row)
    {
        if (row.kind == EXTERNAL)
            return row.external && WoGExternalOptions::IsLocked(*row.external);
        return row.optionId >= 0 && IsWoGOptionLocked(row.optionId);
    }
}

void ShowWoGOptionPopup(char* text, char style)
{
    if (!WoGOptionSkin::HasPopupResources())
    {
        b_MsgBoxBig(text, style);
        return;
    }

    (void)style;
    int width = 400;
    int lines = medfont2->GetLinesCountInText(text, width - 40);
    int height = lines * 16;
    if (height < 80)
        height = 80;
    if (lines > 30)
    {
        width += 180;
        lines = medfont2->GetLinesCountInText(text, width - 40);
        height = lines * 16;
    }
    height += 40;
    if (height > 580)
    {
        height = 580;
        width += 200;
    }

    _CustomDlg_* dlg = _CustomDlg_::Create(-1, -1, width, height,
        DF_SCREENSHOT | DF_SHADOW, NULL);
    WoGOptionSkin::CreatePopupBackground(dlg);
    dlg->AddItem(_DlgStaticText_::Create(20, 20, width - 40, height - 40,
        text, "medfont2.fnt", WoGOptionColors::REGULAR, 2,
        ALIGN_H_CENTER | ALIGN_V_CENTER, 0));
    dlg->RMC_Show();
    dlg->Destroy(TRUE);
}

namespace WoGOptionSearch
{
    const int EDIT_ID = 18000;
    const int PLACEHOLDER_ID = 18001;
    const int BORDER_ID = 18002;
    const int RESULT_HIGHLIGHT_ID = 18003;
    const int RESULTS_HEADER_ID = 18004;
    const int NO_RESULTS_ID = 18005;
    const int RESULT_TEXT_BASE = 18100;
    const int TARGET_HIGHLIGHT_ID = 18300;
    const int VISIBLE_RESULTS = 18;
    const UINT TARGET_PULSE_INTERVAL_MS = 30;
    const DWORD ARROW_REPEAT_INTERVAL_MS = 100;

    struct PulseFrame
    {
        signed char xPadding;
        signed char yPadding;
    };

    // A full pulse, a short settle, then a smaller echo. Alternating X/Y
    // half-steps creates real intermediate rectangles while keeping the frame
    // centred on the option row.
    const PulseFrame TARGET_PULSE_FRAMES[] =
    {
        {1,1}, {2,1}, {2,2}, {3,2}, {3,3}, {4,3}, {4,4},
        {5,4}, {5,5}, {6,5}, {6,6}, {7,6}, {7,7}, {7,6},
        {6,6}, {6,5}, {5,5}, {5,4}, {4,4}, {4,3}, {3,3},
        {3,2}, {2,2}, {2,1}, {1,1},
        {1,1}, {1,1}, {1,1},
        {1,1}, {1,2}, {2,2}, {2,3}, {3,3}, {3,4}, {4,4},
        {4,5}, {5,5}, {4,5}, {4,4}, {3,4}, {3,3}, {2,3},
        {2,2}, {1,2}, {1,1},
        {1,1}, {1,1}, {1,1}
    };

    struct Entry
    {
        int optionId;
        bool external;
        WoGExternalOptions::SessionRow* externalRow;
        int page;
        int group;
        int item;
        int labelId;
        int buttonId;
        std::string name;
        std::string hint;
        std::string popup;
        std::string badge;
        std::string aliases;
        std::string pageName;
        std::string groupName;
        std::wstring normalizedName;
        std::wstring normalizedHint;
        std::wstring normalizedPopup;
        std::wstring normalizedBadge;
        std::wstring normalizedAliases;
        std::wstring normalizedAll;
    };

    struct Match
    {
        int entryIndex;
        int rank;
    };

    std::vector<Entry> index;
    std::vector<Match> matches;
    std::string queryText;
    bool active = false;
    int selected = -1;
    int firstVisible = 0;
    int highlightedEntry = -1;
    UINT_PTR targetPulseTimer = 0;
    _CustomDlg_* targetPulseDlg = NULL;
    DWORD targetPulseStartedAt = 0;
    int targetPulseRenderedStep = -1;
    int targetPulsePaddingX = -1;
    int targetPulsePaddingY = -1;
    DWORD lastArrowNavigationTick = 0;
    int lastArrowNavigationDirection = 0;
    char resultText[VISIBLE_RESULTS][1024];
    char resultHint[VISIBLE_RESULTS][2048];
    char resultPopup[VISIBLE_RESULTS][4096];
    char headerText[128];
    char statusText[4096];
    char popupText[8192];

    Entry* FindEntry(int page, int group, int item)
    {
        for (size_t i = 0; i < index.size(); ++i)
        {
            if (index[i].page == page && index[i].group == group && index[i].item == item)
                return &index[i];
        }
        return NULL;
    }

    const char* SafeText(const char* text)
    {
        return text ? text : o_NullString;
    }

    const char* JsonText(const char* key)
    {
        const char* value = GetEraJSON(key);
        return value && strcmp(value, key) != 0 ? value : o_NullString;
    }

    bool IsVirtualKeyDown(int key)
    {
        return ((GetKeyState(key) | GetAsyncKeyState(key)) & 0x8000) != 0;
    }

    std::wstring Normalize(const char* text)
    {
        if (!text || !*text)
            return std::wstring();

        UINT codePage = Era::GetCodePage ? Era::GetCodePage() : GetACP();
        if (!codePage)
            codePage = GetACP();
        int length = MultiByteToWideChar(codePage, 0, text, -1, NULL, 0);
        if (length <= 0 && codePage != GetACP())
        {
            codePage = GetACP();
            length = MultiByteToWideChar(codePage, 0, text, -1, NULL, 0);
        }
        if (length <= 0)
            return std::wstring();

        std::vector<wchar_t> buffer(length);
        MultiByteToWideChar(codePage, 0, text, -1, &buffer[0], length);
        if (length > 1)
            CharLowerBuffW(&buffer[0], length - 1);

        std::wstring result;
        result.reserve(length - 1);
        bool pendingSpace = false;
        for (int i = 0; i < length - 1; ++i)
        {
            const wchar_t ch = buffer[i];
            if (iswspace(ch))
            {
                pendingSpace = !result.empty();
            }
            else
            {
                if (pendingSpace)
                    result.push_back(L' ');
                result.push_back(ch);
                pendingSpace = false;
            }
        }
        return result;
    }

    std::vector<std::wstring> Tokens(const std::wstring& query)
    {
        std::vector<std::wstring> tokens;
        size_t start = 0;
        while (start < query.size())
        {
            size_t end = query.find(L' ', start);
            if (end == std::wstring::npos)
                end = query.size();
            if (end > start)
                tokens.push_back(query.substr(start, end - start));
            start = end + 1;
        }
        return tokens;
    }

    bool TryExactOptionNumber(const std::wstring& normalizedQuery, int& optionId)
    {
        std::wstring value = normalizedQuery;
        if (!value.empty() && value[0] == L'#')
            value.erase(0, 1);
        if (value.compare(0, 7, L"option ") == 0)
            value.erase(0, 7);
        if (value.empty())
            return false;
        int parsed = 0;
        for (size_t i = 0; i < value.size(); ++i)
        {
            if (value[i] < L'0' || value[i] > L'9')
                return false;
            parsed = parsed * 10 + (value[i] - L'0');
            if (parsed > 9999)
                return false;
        }
        optionId = parsed;
        return true;
    }

    int OptionIdAt(_DlgSetup_* ds, int page, int group, int item)
    {
        return WoGOptionRows::NativeOptionIdAt(ds, page, group, item);
    }

    bool DecodeOptionControl(_DlgSetup_* ds, int controlId, int& page, int& group, int& item, int& optionId)
    {
        WoGOptionRows::RowRef row;
        if (!WoGOptionRows::DecodeControl(ds, controlId, row) ||
            row.kind != WoGOptionRows::NATIVE)
            return false;
        page = row.page;
        group = row.group;
        item = row.item;
        optionId = row.optionId;
        return true;
    }

    void BuildIndex(_DlgSetup_* ds)
    {
        index.clear();
        matches.clear();
        if (!ds)
            return;

        for (int page = 0; page < 8; ++page)
        {
            _DlgSetup_Page_* pageData = ds->Pages[page];
            if (!pageData || !pageData->Enabled)
                continue;
            for (int group = 0; group < 4; ++group)
            {
                _DlgSetup_ItemList_* list = pageData->ItemList[group];
                if (!list)
                    continue;
                const int count = WoGOptionRows::Count(ds, page, group);
                for (int item = 0; item < count; ++item)
                {
                    WoGOptionRows::RowRef row;
                    if (!WoGOptionRows::Resolve(ds, page, group, item, row))
                        continue;
                    if (row.kind == WoGOptionRows::NATIVE && row.optionId < 0)
                        continue;
                    // Unavailable options are omitted from search. Locked
                    // rows remain searchable because their disabled control
                    // state represents an intentional, visible policy.
                    if (!WoGOptionRows::IsLocked(row) &&
                        WoGOptionRows::State(row, false) >= 2)
                        continue;
                    Entry entry;
                    entry.optionId = row.optionId;
                    entry.external = row.kind == WoGOptionRows::EXTERNAL;
                    entry.externalRow = row.external;
                    entry.page = page;
                    entry.group = group;
                    entry.item = item;
                    entry.labelId = 1000 * (page + 1) + 200 * group + item;
                    entry.buttonId = entry.labelId + 100;
                    entry.name = SafeText(WoGOptionRows::Name(row));
                    entry.hint = SafeText(WoGOptionRows::Hint(row));
                    entry.popup = SafeText(WoGOptionRows::Popup(row));
                    entry.badge = SafeText(WoGOptionRows::Badge(row));
                    entry.aliases = SafeText(WoGOptionRows::SearchAliases(row));
                    entry.pageName = SafeText(pageData->Name);
                    entry.groupName = SafeText(list->Name);
                    entry.normalizedName = Normalize(entry.name.c_str());
                    entry.normalizedHint = Normalize(entry.hint.c_str());
                    entry.normalizedPopup = Normalize(entry.popup.c_str());
                    entry.normalizedBadge = Normalize(entry.badge.c_str());
                    entry.normalizedAliases = Normalize(entry.aliases.c_str());
                    wchar_t number[32];
                    number[0] = 0;
                    if (entry.optionId >= 0)
                        _snwprintf_s(number, _countof(number), _TRUNCATE, L"%d", entry.optionId);
                    entry.normalizedAll = entry.normalizedName + L" " + entry.normalizedHint + L" " +
                        entry.normalizedPopup + L" " + entry.normalizedBadge + L" " +
                        entry.normalizedAliases + L" " + number;
                    index.push_back(entry);
                }
            }
        }
    }

    int Rank(const Entry& entry, const std::wstring& query, const std::vector<std::wstring>& tokens, int exactId)
    {
        if (exactId >= 0 && entry.optionId == exactId)
            return 0;
        for (size_t i = 0; i < tokens.size(); ++i)
        {
            if (entry.normalizedAll.find(tokens[i]) == std::wstring::npos)
                return -1;
        }
        if (entry.normalizedName == query)
            return 1;
        if (entry.normalizedName.compare(0, query.size(), query) == 0)
            return 2;
        if (entry.normalizedName.find(query) != std::wstring::npos)
            return 3;
        if (entry.normalizedHint.find(query) != std::wstring::npos)
            return 4;
        if (entry.normalizedPopup.find(query) != std::wstring::npos)
            return 5;
        return 6;
    }

    void FindMatches(const char* query)
    {
        queryText = query ? query : "";
        const std::wstring normalizedQuery = Normalize(queryText.c_str());
        matches.clear();
        selected = -1;
        firstVisible = 0;
        active = !normalizedQuery.empty();
        if (!active)
            return;

        int exactId = -1;
        if (!TryExactOptionNumber(normalizedQuery, exactId))
            exactId = -1;
        std::wstring matchQuery = normalizedQuery;
        if (exactId >= 0)
        {
            wchar_t number[32];
            _snwprintf_s(number, _countof(number), _TRUNCATE, L"%d", exactId);
            matchQuery = number;
        }
        const std::vector<std::wstring> tokens = Tokens(matchQuery);
        for (size_t i = 0; i < index.size(); ++i)
        {
            const int rank = Rank(index[i], matchQuery, tokens, exactId);
            if (rank >= 0)
            {
                Match match;
                match.entryIndex = (int)i;
                match.rank = rank;
                matches.push_back(match);
            }
        }
        std::stable_sort(matches.begin(), matches.end(), [](const Match& left, const Match& right)
        {
            return left.rank < right.rank;
        });
        if (!matches.empty())
            selected = 0;
    }

    void EnsureSelectionVisible()
    {
        if (selected < 0)
        {
            firstVisible = 0;
            return;
        }
        if (selected < firstVisible)
            firstVisible = selected;
        if (selected >= firstVisible + VISIBLE_RESULTS)
            firstVisible = selected - VISIBLE_RESULTS + 1;
        const int maxFirst = max(0, (int)matches.size() - VISIBLE_RESULTS);
        firstVisible = min(max(0, firstVisible), maxFirst);
    }
}

int Redraw_WoGDlgSetup_ElemOnPage(_CustomDlg_* dlg, _DlgSetup_* ds, int page) 
{
    int count = 0;  int id = 0; int state = 0;

    for (int j=0; j<4; j++) {
        count = WoGOptionRows::Count(ds, page, j);
        id = (1000*(page+1)) + (j*200);
        
        for (int i=0; i<count; i++) { 
            WoGOptionRows::RowRef row;
            if (!WoGOptionRows::Resolve(ds, page, j, i, row))
                continue;
            state = WoGOptionRows::State(row);
            const bool locked = WoGOptionRows::IsLocked(row);
            const bool unselectedLockedRadio = locked && row.list &&
                row.list->Type == 2 && state == 2;
            const int lockedTextColor = unselectedLockedRadio
                ? WoGOptionColors::REGULAR : WoGOptionColors::LOCKED_TEXT;
            if (state == 0) {
                ((_DlgStaticText_*)dlg->GetItem(id+i))->color = locked
                    ? lockedTextColor : WoGOptionColors::REGULAR;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->def_frame_index = 0;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->press_def_frame_index = 1;
            } else if (state == 1) {
                ((_DlgStaticText_*)dlg->GetItem(id+i))->color = locked
                    ? lockedTextColor : WoGOptionColors::REGULAR;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->def_frame_index = 2;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->press_def_frame_index = 3;
                // CALL_3(void, __thiscall, 0x5FF490, dlg, id+i+100, 16392); // on
            } else if (state == 2) {
                ((_DlgStaticText_*)dlg->GetItem(id+i))->color = locked
                    ? lockedTextColor : WoGOptionColors::REGULAR;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->def_frame_index = 4;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->press_def_frame_index = 4;
                // CALL_3(void, __thiscall, 0x5FF520, dlg, id+i+100, 16392); // off
            } else if (state == 3) {
                ((_DlgStaticText_*)dlg->GetItem(id+i))->color = locked
                    ? lockedTextColor : WoGOptionColors::REGULAR;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->def_frame_index = 5;
                ((_DlgButton_*)dlg->GetItem(id+i+100))->press_def_frame_index = 5;
            } else {
                sprintf(o_TextBuffer,
                    "Error (for %s)!!!\n\n Debug\nPage: %d (%d) \nCount: %d \n id: %d \n state: %d \n\n %s",
                    wndText::PLUGIN_AUTHOR, page, j, count, id+i+100, state,
                    WoGOptionRows::Name(row));
                b_MsgBox(o_TextBuffer, 1);
            }
        }
    }
    
    return 1;
} 


int ShowHide_WoGDlgSetup_ElemOnPage(_CustomDlg_* dlg, int page, _DlgSetup_* ds, int show) 
{   
    if (!ds->Pages[page]->Enabled) 
        return 1;
    // show: показать (1), спрятать (2)
    _DlgItem_* it;
    int id = 0;

    int countsElements = 0;
    for (int j=0; j<4; j++) {
        id = 1000*(page+1) +(j*200);

        const int displayCount = WoGOptionRows::Count(ds, page, j);
        if (displayCount > 0) {
            if (show) {
                it = dlg->GetItem(WoGOptionSkin::GROUP_HEADER_SURFACE_BASE + page*10+j); if(it) { it->Show(); }
                it = dlg->GetItem(800+page*10+j); if(it) { it->Show(); }
                it = dlg->GetItem(900+page*10+j); if(it) { it->Show(); }
                for (int i=0; i<displayCount; i++){
                    it = dlg->GetItem(id+i); if(it) { it->Show(); }
                    it = dlg->GetItem(id+i+100); if(it) { it->Show(); }
                }
            } else {
                it = dlg->GetItem(WoGOptionSkin::GROUP_HEADER_SURFACE_BASE + page*10+j); if(it) { it->Hide(); }
                it = dlg->GetItem(800+page*10+j); if(it) { it->Hide(); }
                it = dlg->GetItem(900+page*10+j); if(it) { it->Hide(); }
                for (int i=0; i<displayCount; i++){
                    it = dlg->GetItem(id+i); if(it) { it->Hide(); }
                    it = dlg->GetItem(id+i+100); if(it) { it->Hide(); }
                }
            }       
        }
    }
    return 1;
} 

int Create_WoGDlgSetup_ElemOnPage(_CustomDlg_* dlg, int page, _DlgSetup_* ds) 
{   
    int x = 225; int wx = 0; int y = 52; int dy = 19; int id = 0; int ry = 0;
    int itState = 0; int defState = 0;

    for (int j=0; j<4; j++) {
        const int displayCount = WoGOptionRows::Count(ds, page, j);
        if (displayCount == 0) continue;
        
        // смещение по x (для 3 и 4 групп)
        if (j == 0 && WoGOptionRows::Count(ds, page, 2) == 0 ) { wx = 275; }
        if (j == 2) { x = 500; y = 52; }
        if (j == 3 && x != 500 ) { x = 500; y = ry; }

        id = 1000*(page+1) +(j*200);

        if (WoGOptionSkin::available)
            dlg->AddItem(_DlgStaticPcx8_::Create(x, y, 267 +wx, 19,
                WoGOptionSkin::GROUP_HEADER_SURFACE_BASE + page*10+j,
                (char*)WoGOptionSkin::HEADER_PCX));

        // титульный текст группы
        dlg->AddItem(_DlgStaticText_::Create(x, y, 267 +wx, 19,
            ds->Pages[page]->ItemList[j]->Name, n_BigFont, WoGOptionColors::GOLD_TEXT,
            800+page*10+j, ALIGN_H_CENTER | ALIGN_V_CENTER, 0));

        // рамка
        int hy = displayCount * dy +3;
        b_YellowFrame_Create(dlg, x, y+19, 267 +wx, hy, 900+page*10+j, ON, o_Pal_Grey);

        // кнопки группы
        for (int i=0; i<displayCount; i++) {
            WoGOptionRows::RowRef row;
            if (!WoGOptionRows::Resolve(ds, page, j, i, row))
                continue;
            sprintf(o_TextBuffer, textProcS, WoGOptionRows::Name(row));
            const bool locked = WoGOptionRows::IsLocked(row);
            const int state = WoGOptionRows::State(row, false);
            const bool unselectedLockedRadio = locked && row.list &&
                row.list->Type == 2 && state == 2;
            const int textColor = locked && !unselectedLockedRadio
                ? WoGOptionColors::LOCKED_TEXT : WoGOptionColors::REGULAR;
            dlg->AddItem(_DlgStaticText_::Create(x+25, y+22 +dy*i, 238 +wx, 16, o_TextBuffer, n_SmallFont, 
                textColor, id +i, ALIGN_H_LEFT | ALIGN_V_CENTER, 0));

            itState = WoGOptionRows::State(row);
            if (itState < 0 || itState > 3) { itState = 1; }
            if (itState < 2) { defState = itState*2; }
            if (itState == 2) { defState = 4; }
            if (itState == 3) { defState = 5; }

            dlg->AddItem(_DlgButton_::Create(x+3, y+22 +dy*i, 262 +wx, 16, id+i+100,
                (char*)((row.kind == WoGOptionRows::NATIVE &&
                    ds->Pages[page]->ItemList[j]->Type == 2)
                    ? (WoGOptionSkin::optionControlsAvailable
                        ? WoGOptionSkin::OPTION_RADIO_DEF : radioBttnDef)
                    : (WoGOptionSkin::optionControlsAvailable
                        ? WoGOptionSkin::OPTION_CHECKBOX_DEF : checkboxDef)),
                defState, (itState < 2) ? (defState+1) : defState, 0, 0, 0)); 
             /* dlg->AddItem(_DlgTextButton_::Create(x+3, y+22 +dy*i, 262 +wx, 16, id+i+100, 
                 (ds->Pages[page]->ItemList[j]->Type == 2) ? "radiobttn.def" : "checkbox.def", 
                 ds->Pages[page]->ItemList[j]->ItemName[i], n_SmallFont, 
                 ds->Pages[page]->ItemList[j]->ItemState[i]*2, 
                 (ds->Pages[page]->ItemList[j]->ItemState[i] == 2) ? 4 : ds->Pages[page]->ItemList[j]->ItemState[i]*2+1, 0, 0, 0x4, 2)); */ 

        }
        wx = 0;
        if (j == 1) { ry = y;}
        y += hy +dy;
    }

    return 1;
}

void UpdateWoGPageLabelColors(_CustomDlg_* dlg, int selectedPage)
{
    if (!dlg)
        return;
    for (int page = 0; page < 8; ++page)
    {
        if (_DlgStaticText_* label = (_DlgStaticText_*)dlg->GetItem(41 + page))
            label->color = page == selectedPage
                ? WoGOptionColors::WHITE : WoGOptionColors::REGULAR;
    }
}

void setYellowFrames(_CustomDlg_* dlg, int page)
{
    UpdateWoGPageLabelColors(dlg, page);
    WoGOptionSkin::UpdateActivePage(dlg, page);
    if (WoGOptionSkin::available)
        return;

    if (page < 0 || page > 7) {
        b_YellowFrame_Create(dlg, 215, 46, 564, 3, 50, ON, o_Pal_Y);
        b_YellowFrame_Create(dlg, 216, 47, 562, 1, 51, ON, o_Pal_Y);
        b_YellowFrame_Create(dlg, 215, 518, 564, 3, 50, ON, o_Pal_Y);
        b_YellowFrame_Create(dlg, 216, 519, 562, 1, 51, ON, o_Pal_Y);
        b_YellowFrame_Create(dlg, 776, 49, 3, 469, 51, ON, o_Pal_Y); // h = 518
        b_YellowFrame_Create(dlg, 777, 50, 1, 467, 50, ON, o_Pal_Y);

        int dx=60; 
        for (int i=0; i<7; i++) {
            b_YellowFrame_Create(dlg, 215, 101+dx*i, 1, 5, 50, ON, o_Pal_Y);
            b_YellowFrame_Create(dlg, 217, 100+dx*i, 1, 7, 50, ON, o_Pal_Y);
            b_YellowFrame_Create(dlg, 216, 99+dx*i, 1, 9, 51, ON, o_Pal_Y);
        }

        // id = 60...140
        for (int i=0; i<8; i++) {
            b_YellowFrame_Create(dlg, 217, 46+dx*i, 1, 54, 60+10*i, ON, o_Pal_Y); // в.п.
            b_YellowFrame_Create(dlg, 215, 46+dx*i, 1, 55, 61+10*i, ON, o_Pal_Y);
            b_YellowFrame_Create(dlg, 216, 48+dx*i, 1, 52, 62+10*i, ON, o_Pal_Y);
            if (o_DlgSetup->Pages[i]->Enabled) {
                b_YellowFrame_Create(dlg, 20, 46+dx*i, 1, 54, 66+10*i, ON, o_Pal_Y); // в.л.
                b_YellowFrame_Create(dlg, 22, 48+dx*i, 1, 50, 66+10*i, ON, o_Pal_Y);
                                            b_YellowFrame_Create(dlg, 21, 47+dx*i, 1, 52, 63+10*i, OFF, o_Pal_Y);
                b_YellowFrame_Create(dlg, 20, 46+dx*i, 196, 1, 66+10*i, ON, o_Pal_Y); // г.в.
                b_YellowFrame_Create(dlg, 22, 48+dx*i, 194, 1, 66+10*i, ON, o_Pal_Y);
                                            b_YellowFrame_Create(dlg, 21, 47+dx*i, 196, 1, 64+10*i, OFF, o_Pal_Y);
                b_YellowFrame_Create(dlg, 20, 100+dx*i, 196, 1, 66+10*i, ON, o_Pal_Y); // г.н.
                b_YellowFrame_Create(dlg, 22, 98+dx*i, 194, 1, 66+10*i, ON, o_Pal_Y);
                                            b_YellowFrame_Create(dlg, 21, 99+dx*i, 196, 1, 65+10*i, OFF, o_Pal_Y);
            }
        }
    } else {
        int id = page*10 +60;
        int last_id = o_LastChoosenPage*10 +60; // элементы прошлой страницы

        if (o_LastChoosenPage != -1) {
            dlg->GetItem(last_id)->SendCommand(5, 4);
            dlg->GetItem(last_id+1)->SendCommand(5, 4);
            dlg->GetItem(last_id+2)->SendCommand(5, 4);
            dlg->GetItem(last_id+3)->SendCommand(6, 4);
            dlg->GetItem(last_id+4)->SendCommand(6, 4);
            dlg->GetItem(last_id+5)->SendCommand(6, 4);
        }

        dlg->GetItem(id)->SendCommand(6, 4);
        dlg->GetItem(id+1)->SendCommand(6, 4);
        dlg->GetItem(id+2)->SendCommand(6, 4);
        dlg->GetItem(id+3)->SendCommand(5, 4);
        dlg->GetItem(id+4)->SendCommand(5, 4);
        dlg->GetItem(id+5)->SendCommand(5, 4);
    }
}

namespace WoGOptionSearch
{
    const char* ExternalText(const Entry& entry, uint32_t kind, const char* fallback)
    {
        if (!entry.external || !entry.externalRow || !entry.externalRow->provider)
            return SafeText(fallback);
        const char* text = WoGExternalOptions::GetText(
            *entry.externalRow->provider, kind);
        return text && *text ? text : SafeText(fallback);
    }

    void FormatExternalLockLabel(const Entry& entry, char* output, size_t outputSize)
    {
        if (!output || !outputSize)
            return;
        output[0] = 0;
        const bool checked = entry.externalRow &&
            WoGExternalOptions::LockedChecked(*entry.externalRow);
        strncpy_s(output, outputSize,
            JsonText(checked ? "wnd.dlg_wog_options.lock_explanation.locked_on" :
                               "wnd.dlg_wog_options.lock_explanation.locked_off"),
            _TRUNCATE);
    }

    void FormatExternalStatus(const Entry& entry, char* output, size_t outputSize)
    {
        if (entry.externalRow && WoGExternalOptions::IsLocked(*entry.externalRow))
        {
            char lockLabel[1024] = {};
            FormatExternalLockLabel(entry, lockLabel, sizeof(lockLabel));
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.locked_status_format"),
                entry.optionId,
                lockLabel,
                SafeText(WoGExternalOptions::LockReason(*entry.externalRow)));
        }
        else
        {
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.external_id_status_format"),
                entry.optionId,
                ExternalText(entry, WOG_EXTERNAL_TEXT_SOURCE_BADGE, entry.badge.c_str()),
                ExternalText(entry, WOG_EXTERNAL_TEXT_HINT, entry.hint.c_str()));
        }
    }

    void FormatExternalPopup(const Entry& entry, char* output, size_t outputSize)
    {
        if (entry.externalRow && WoGExternalOptions::IsLocked(*entry.externalRow))
        {
            char lockLabel[1024] = {};
            FormatExternalLockLabel(entry, lockLabel, sizeof(lockLabel));
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.locked_popup_format"),
                ExternalText(entry, WOG_EXTERNAL_TEXT_NAME, entry.name.c_str()),
                ExternalText(entry, WOG_EXTERNAL_TEXT_POPUP, entry.popup.c_str()),
                lockLabel,
                SafeText(WoGExternalOptions::LockReason(*entry.externalRow)));
        }
        else
        {
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.popup_format"),
                ExternalText(entry, WOG_EXTERNAL_TEXT_NAME, entry.name.c_str()),
                ExternalText(entry, WOG_EXTERNAL_TEXT_POPUP, entry.popup.c_str()));
        }
    }

    void FormatStatus(int optionId, const char* hint, char* output, size_t outputSize)
    {
        if (IsWoGOptionLocked(optionId))
        {
            char lockLabel[1024] = {};
            FormatWoGOptionLockLabel(optionId, lockLabel, sizeof(lockLabel));
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.locked_status_format"), optionId,
                lockLabel, SafeText(GetWoGOptionLockReason(optionId)));
        }
        else
        {
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.status_format"), optionId, SafeText(hint));
        }
    }

    void FormatPopup(int optionId, const char* name, const char* popup, char* output, size_t outputSize)
    {
        if (IsWoGOptionLocked(optionId))
        {
            char lockLabel[1024] = {};
            FormatWoGOptionLockLabel(optionId, lockLabel, sizeof(lockLabel));
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.locked_popup_format"),
                SafeText(name),
                SafeText(popup),
                lockLabel,
                SafeText(GetWoGOptionLockReason(optionId)));
        }
        else
        {
            _snprintf_s(output, outputSize, _TRUNCATE,
                JsonText("wnd.dlg_wog_options.search.popup_format"),
                SafeText(name), SafeText(popup));
        }
    }

    void RedrawAllOptionStates(_CustomDlg_* dlg, _DlgSetup_* ds)
    {
        if (!dlg || !ds)
            return;
        for (int page = 0; page < 8; ++page)
        {
            if (ds->Pages[page] && ds->Pages[page]->Enabled)
                Redraw_WoGDlgSetup_ElemOnPage(dlg, ds, page);
        }
    }

    void HideAllPages(_CustomDlg_* dlg, _DlgSetup_* ds)
    {
        if (_DlgItem_* intro = dlg->GetItem(4))
            intro->Hide();
        for (int page = 0; page < 8; ++page)
        {
            if (ds->Pages[page] && ds->Pages[page]->Enabled)
                ShowHide_WoGDlgSetup_ElemOnPage(dlg, page, ds, 0);
        }
    }

    void HideResultControls(_CustomDlg_* dlg)
    {
        if (_DlgItem_* item = dlg->GetItem(RESULTS_HEADER_ID))
            item->Hide();
        if (_DlgItem_* item = dlg->GetItem(NO_RESULTS_ID))
            item->Hide();
        if (_DlgItem_* item = dlg->GetItem(RESULT_HIGHLIGHT_ID))
            item->Hide_ButStayEnable();
        for (int i = 0; i < VISIBLE_RESULTS; ++i)
        {
            if (_DlgItem_* item = dlg->GetItem(RESULT_TEXT_BASE + i))
                item->Hide();
        }
    }

    bool PositionTargetHighlight(_CustomDlg_* dlg, int entryIndex,
        int paddingX = 2, int paddingY = 2)
    {
        _DlgItem_* frame = dlg->GetItem(TARGET_HIGHLIGHT_ID);
        if (!frame || entryIndex < 0 || entryIndex >= (int)index.size())
        {
            if (frame)
                frame->Hide_ButStayEnable();
            return false;
        }
        _DlgItem_* target = dlg->GetItem(index[entryIndex].buttonId);
        if (!target)
        {
            frame->Hide_ButStayEnable();
            return false;
        }
        frame->SetRect(target->x - paddingX, target->y - paddingY,
            target->width + paddingX * 2, target->height + paddingY * 2);
        frame->Show_ButStayEnable();
        return true;
    }

    void StopTargetPulse(_CustomDlg_* dlg, bool redraw)
    {
        if (targetPulseTimer)
            KillTimer(NULL, targetPulseTimer);
        targetPulseTimer = 0;
        targetPulseDlg = NULL;
        targetPulseStartedAt = 0;
        targetPulseRenderedStep = -1;
        targetPulsePaddingX = -1;
        targetPulsePaddingY = -1;
        highlightedEntry = -1;

        if (dlg)
        {
            if (_DlgItem_* frame = dlg->GetItem(TARGET_HIGHLIGHT_ID))
                frame->Hide_ButStayEnable();
            if (redraw)
                dlg->Redraw();
        }
    }

    VOID CALLBACK TargetPulseTimerProc(HWND, UINT, UINT_PTR timerId, DWORD)
    {
        if (!targetPulseTimer || timerId != targetPulseTimer || !targetPulseDlg)
            return;

        const int stepCount = sizeof(TARGET_PULSE_FRAMES) / sizeof(TARGET_PULSE_FRAMES[0]);
        const DWORD elapsed = GetTickCount() - targetPulseStartedAt;
        const int step = (int)(elapsed / TARGET_PULSE_INTERVAL_MS);
        if (step >= stepCount)
        {
            _CustomDlg_* dlg = targetPulseDlg;
            StopTargetPulse(dlg, true);
            return;
        }

        if (step <= targetPulseRenderedStep)
            return;

        const PulseFrame& pulseFrame = TARGET_PULSE_FRAMES[step];
        const bool geometryChanged = pulseFrame.xPadding != targetPulsePaddingX ||
            pulseFrame.yPadding != targetPulsePaddingY;
        targetPulseRenderedStep = step;

        if (!PositionTargetHighlight(targetPulseDlg, highlightedEntry,
            pulseFrame.xPadding, pulseFrame.yPadding))
        {
            _CustomDlg_* dlg = targetPulseDlg;
            StopTargetPulse(dlg, true);
            return;
        }

        targetPulsePaddingX = pulseFrame.xPadding;
        targetPulsePaddingY = pulseFrame.yPadding;
        if (geometryChanged)
            targetPulseDlg->Redraw();
    }

    void StartTargetPulse(_CustomDlg_* dlg, int entryIndex)
    {
        StopTargetPulse(dlg, false);
        if (!dlg || entryIndex < 0 || entryIndex >= (int)index.size())
            return;

        highlightedEntry = entryIndex;
        targetPulseDlg = dlg;
        targetPulseStartedAt = GetTickCount();
        targetPulseRenderedStep = 0;
        targetPulsePaddingX = TARGET_PULSE_FRAMES[0].xPadding;
        targetPulsePaddingY = TARGET_PULSE_FRAMES[0].yPadding;
        targetPulseTimer = SetTimer(NULL, 0, TARGET_PULSE_INTERVAL_MS, TargetPulseTimerProc);
        if (!targetPulseTimer)
        {
            StopTargetPulse(dlg, false);
            return;
        }

        PositionTargetHighlight(dlg, highlightedEntry,
            targetPulsePaddingX, targetPulsePaddingY);
    }

    void RestoreNormalContent(_CustomDlg_* dlg, _DlgSetup_* ds)
    {
        StopTargetPulse(dlg, false);
        HideResultControls(dlg);
        if (o_LastChoosenPage >= 0 && o_LastChoosenPage < 8 && ds->Pages[o_LastChoosenPage] &&
            ds->Pages[o_LastChoosenPage]->Enabled)
        {
            ShowHide_WoGDlgSetup_ElemOnPage(dlg, o_LastChoosenPage, ds, 1);
        }
        else
        {
            if (_DlgItem_* intro = dlg->GetItem(4))
                intro->Show();
            if (_DlgItem_* frame = dlg->GetItem(TARGET_HIGHLIGHT_ID))
                frame->Hide_ButStayEnable();
        }
    }

    void RenderResults(_CustomDlg_* dlg, _DlgSetup_* ds)
    {
        StopTargetPulse(dlg, false);
        HideAllPages(dlg, ds);

        _DlgStaticText_* header = (_DlgStaticText_*)dlg->GetItem(RESULTS_HEADER_ID);
        const char* countFormat = JsonText("wnd.dlg_wog_options.search.result_count");
        const char* countMarker = strstr(countFormat, "%d");
        if (countMarker)
        {
            _snprintf_s(headerText, sizeof(headerText), _TRUNCATE, "%.*s%d%s",
                (int)(countMarker - countFormat), countFormat, (int)matches.size(), countMarker + 2);
        }
        else
        {
            headerText[0] = 0;
        }
        header->SetText(headerText);
        header->Show();

        _DlgItem_* noResults = dlg->GetItem(NO_RESULTS_ID);
        if (matches.empty())
            noResults->Show();
        else
            noResults->Hide();

        EnsureSelectionVisible();
        for (int slot = 0; slot < VISIBLE_RESULTS; ++slot)
        {
            _DlgStaticText_* row = (_DlgStaticText_*)dlg->GetItem(RESULT_TEXT_BASE + slot);
            const int matchIndex = firstVisible + slot;
            if (matchIndex < 0 || matchIndex >= (int)matches.size())
            {
                row->Hide();
                continue;
            }

            const Entry& entry = index[matches[matchIndex].entryIndex];
            const bool locked = entry.external
                ? (entry.externalRow && WoGExternalOptions::IsLocked(*entry.externalRow))
                : IsWoGOptionLocked(entry.optionId);
            char stateSuffix[1024] = {};
            if (locked)
            {
                char lockLabel[1024] = {};
                if (entry.external)
                    FormatExternalLockLabel(entry, lockLabel, sizeof(lockLabel));
                else
                    FormatWoGOptionLockLabel(entry.optionId, lockLabel, sizeof(lockLabel));
                _snprintf_s(stateSuffix, sizeof(stateSuffix), _TRUNCATE,
                    JsonText("wnd.dlg_wog_options.search.locked_suffix_format"), lockLabel);
            }
            if (entry.external)
            {
                _snprintf_s(resultText[slot], sizeof(resultText[slot]), _TRUNCATE,
                    JsonText("wnd.dlg_wog_options.search.external_id_result_format"),
                    entry.optionId,
                    ExternalText(entry, WOG_EXTERNAL_TEXT_SOURCE_BADGE, entry.badge.c_str()),
                    ExternalText(entry, WOG_EXTERNAL_TEXT_NAME, entry.name.c_str()),
                    entry.pageName.c_str(), entry.groupName.c_str(), stateSuffix);
                FormatExternalStatus(entry, resultHint[slot], sizeof(resultHint[slot]));
                FormatExternalPopup(entry, resultPopup[slot], sizeof(resultPopup[slot]));
            }
            else
            {
                _snprintf_s(resultText[slot], sizeof(resultText[slot]), _TRUNCATE,
                    JsonText("wnd.dlg_wog_options.search.result_format"), entry.optionId,
                    entry.name.c_str(), entry.pageName.c_str(), entry.groupName.c_str(), stateSuffix);
                FormatStatus(entry.optionId, entry.hint.c_str(), resultHint[slot], sizeof(resultHint[slot]));
                FormatPopup(entry.optionId, entry.name.c_str(), entry.popup.c_str(), resultPopup[slot], sizeof(resultPopup[slot]));
            }
            row->SetText(resultText[slot]);
            row->short_tip_text = resultHint[slot];
            // The enhanced handler below owns result descriptions. A full tip
            // here would make DefProc open a second, scrollable popup first.
            row->full_tip_text = o_NullString;
            row->color = locked ? WoGOptionColors::LOCKED_TEXT :
                (matchIndex == selected
                    ? WoGOptionColors::WHITE : WoGOptionColors::REGULAR);
            row->Show();
        }

        _DlgItem_* selectionFrame = dlg->GetItem(RESULT_HIGHLIGHT_ID);
        if (selected >= firstVisible && selected < firstVisible + VISIBLE_RESULTS)
        {
            _DlgItem_* row = dlg->GetItem(RESULT_TEXT_BASE + selected - firstVisible);
            selectionFrame->SetRect(row->x - 2, row->y - 1, row->width + 4, row->height + 2);
            selectionFrame->Show_ButStayEnable();
        }
        else
        {
            selectionFrame->Hide_ButStayEnable();
        }
        dlg->Redraw();
    }

    void UpdatePlaceholder(_CustomDlg_* dlg)
    {
        _DlgTextEdit_* edit = (_DlgTextEdit_*)dlg->GetItem(EDIT_ID);
        _DlgItem_* placeholder = dlg->GetItem(PLACEHOLDER_ID);
        if (!edit || !placeholder)
            return;
        if ((!edit->text || !*edit->text) && !edit->focused)
            placeholder->Show();
        else
            placeholder->Hide();
    }

    void UpdateFromEdit(_CustomDlg_* dlg, _DlgSetup_* ds, bool force)
    {
        _DlgTextEdit_* edit = (_DlgTextEdit_*)dlg->GetItem(EDIT_ID);
        const std::string current = edit && edit->text ? edit->text : "";
        if (!force && current == queryText)
        {
            UpdatePlaceholder(dlg);
            return;
        }

        FindMatches(current.c_str());
        if (active)
            RenderResults(dlg, ds);
        else
            RestoreNormalContent(dlg, ds);
        UpdatePlaceholder(dlg);
        dlg->Redraw();
    }

    void FocusEdit(_CustomDlg_* dlg, _DlgSetup_* ds)
    {
        dlg->SetFocuseToItem(EDIT_ID);
        UpdatePlaceholder(dlg);
        UpdateFromEdit(dlg, ds, false);
    }

    void MoveSelection(_CustomDlg_* dlg, _DlgSetup_* ds, int delta)
    {
        if (!active || matches.empty())
            return;
        selected = min(max(selected + delta, 0), (int)matches.size() - 1);
        EnsureSelectionVisible();
        RenderResults(dlg, ds);
    }

    bool AllowArrowNavigation(int direction)
    {
        const DWORD now = GetTickCount();
        if (direction != lastArrowNavigationDirection ||
            now - lastArrowNavigationTick >= ARROW_REPEAT_INTERVAL_MS)
        {
            lastArrowNavigationDirection = direction;
            lastArrowNavigationTick = now;
            return true;
        }
        return false;
    }

    void SwitchPage(_CustomDlg_* dlg, _DlgSetup_* ds, int page, bool clearTargetHighlight)
    {
        if (page < 0 || page >= 8 || !ds->Pages[page] || !ds->Pages[page]->Enabled)
            return;
        HideResultControls(dlg);
        HideAllPages(dlg, ds);
        ShowHide_WoGDlgSetup_ElemOnPage(dlg, page, ds, 1);
        setYellowFrames(dlg, page);
        ds->GetListener(2, page + 1, -1, -1);
        ForceWoGOptionLockValues();
        ReapplyWoGOptionLocks(ds);
        RedrawAllOptionStates(dlg, ds);
        if (clearTargetHighlight)
        {
            StopTargetPulse(dlg, false);
        }
        dlg->Redraw();
    }

    void ClearQuery(_CustomDlg_* dlg, _DlgSetup_* ds, bool restoreContent)
    {
        _DlgTextEdit_* edit = (_DlgTextEdit_*)dlg->GetItem(EDIT_ID);
        if (edit)
            edit->SetEditText(o_NullString);
        queryText.clear();
        matches.clear();
        selected = -1;
        firstVisible = 0;
        active = false;
        HideResultControls(dlg);
        if (restoreContent)
            RestoreNormalContent(dlg, ds);
        UpdatePlaceholder(dlg);
    }

    void SelectResult(_CustomDlg_* dlg, _DlgSetup_* ds)
    {
        if (selected < 0 || selected >= (int)matches.size())
            return;
        const int entryIndex = matches[selected].entryIndex;
        const Entry& entry = index[entryIndex];
        ClearQuery(dlg, ds, false);
        dlg->SetFocuseToItem(-1);
        SwitchPage(dlg, ds, entry.page, false);
        StartTargetPulse(dlg, entryIndex);
        if (entry.external)
            FormatExternalStatus(entry, statusText, sizeof(statusText));
        else
            FormatStatus(entry.optionId, entry.hint.c_str(), statusText, sizeof(statusText));
        WoGOptionSkin::UpdateStatusText(dlg, statusText, true);
        UpdatePlaceholder(dlg);
        dlg->Redraw();
    }

    void CreateControls(_CustomDlg_* dlg)
    {
        b_YellowFrame_Create(dlg, 0, 0, 1, 1, TARGET_HIGHLIGHT_ID, OFF, o_Pal_Y);
        b_YellowFrame_Create(dlg, 0, 0, 1, 1, RESULT_HIGHLIGHT_ID, OFF, o_Pal_Y);

        dlg->AddItem(_DlgStaticText_::Create(232, 54, 536, 22, o_NullString,
            n_BigFont, WoGOptionColors::GOLD_TEXT,
            RESULTS_HEADER_ID, ALIGN_H_CENTER | ALIGN_V_CENTER, 0));
        dlg->AddItem(_DlgStaticText_::Create(232, 230, 536, 40,
            (char*)JsonText("wnd.dlg_wog_options.search.no_results"),
            n_MedFont, WoGOptionColors::REGULAR, NO_RESULTS_ID,
            ALIGN_H_CENTER | ALIGN_V_CENTER, 0));
        for (int slot = 0; slot < VISIBLE_RESULTS; ++slot)
        {
            dlg->AddItem(_DlgStaticText_::Create(232, 80 + 24 * slot, 536, 22, o_NullString,
                n_SmallFont, WoGOptionColors::REGULAR, RESULT_TEXT_BASE + slot,
                ALIGN_H_LEFT | ALIGN_V_CENTER, 0));
        }
        HideResultControls(dlg);

        b_YellowFrame_Create(dlg, 19, 531, 277, 25, BORDER_ID, ON,
            WoGOptionSkin::available ? o_Pal_Grey : o_Pal_Y);
        dlg->AddItem(_DlgTextEdit_::Create(40, 534, 254, 19, 255, o_NullString,
            n_SmallFont, WoGOptionColors::WHITE,
            ALIGN_H_LEFT | ALIGN_V_CENTER,
            WoGOptionSkin::available ? (char*)WoGOptionSkin::DARK_PCX : "WoGTextEdit.pcx",
            EDIT_ID, 4, 0, 0));
        dlg->GetItem(EDIT_ID)->short_tip_text = (char*)JsonText("wnd.dlg_wog_options.search.placeholder");
        dlg->AddItem(_DlgStaticText_::Create(42, 535, 249, 17,
            (char*)JsonText("wnd.dlg_wog_options.search.placeholder"),
            n_SmallFont, WoGOptionColors::REGULAR, PLACEHOLDER_ID,
            ALIGN_H_LEFT | ALIGN_V_CENTER, 0));
    }

    void Reset()
    {
        index.clear();
        matches.clear();
        queryText.clear();
        active = false;
        selected = -1;
        firstVisible = 0;
        if (targetPulseTimer)
            KillTimer(NULL, targetPulseTimer);
        targetPulseTimer = 0;
        targetPulseDlg = NULL;
        targetPulseStartedAt = 0;
        targetPulseRenderedStep = -1;
        targetPulsePaddingX = -1;
        targetPulsePaddingY = -1;
        lastArrowNavigationTick = 0;
        lastArrowNavigationDirection = 0;
        highlightedEntry = -1;
    }
}

int apdFont = 0;

int __stdcall Dlg_WoG_Options_Proc(_CustomDlg_* dlg, _EventMsg_* msg)
{
    _DlgSetup_* ds = o_DlgSetup;

    // The native edit control rewrites accepted keyboard messages in place.
    // Preserve the original event so search input cannot fall through to the
    // dialog's B/N/R/L/S/M toolbar hotkeys.
    const int originalType = msg->type;
    const int originalSubtype = msg->subtype;
    const int originalItemId = msg->item_id;
    const int originalFlags = msg->flags;
    const int originalX = msg->x_abs;
    const int originalY = msg->y_abs;
    _DlgTextEdit_* searchEdit = (_DlgTextEdit_*)dlg->GetItem(WoGOptionSearch::EDIT_ID);
    const bool searchFocused = searchEdit && searchEdit->focused;

    enum SearchAction
    {
        SEARCH_ACTION_NONE,
        SEARCH_ACTION_FOCUS,
        SEARCH_ACTION_UP,
        SEARCH_ACTION_DOWN,
        SEARCH_ACTION_PAGE_UP,
        SEARCH_ACTION_PAGE_DOWN,
        SEARCH_ACTION_SELECT,
        SEARCH_ACTION_CLEAR,
        SEARCH_ACTION_CLOSE
    };
    SearchAction searchAction = SEARCH_ACTION_NONE;

    // Focused edits receive KEYDOWN before this callback and normally rewrite
    // it to a synthetic left-button-down event. Query the live virtual-key
    // state for that event; this also distinguishes arrows from NumPad digits
    // that share the same hardware scan codes.
    const bool syntheticEditKey = originalType == MT_MOUSEBUTTON &&
        originalSubtype == MST_LBUTTONDOWN && originalItemId == WoGOptionSearch::EDIT_ID &&
        !WoGOptionSearch::IsVirtualKeyDown(VK_LBUTTON);
    const bool searchKeyEvent = originalType == MT_KEYDOWN || syntheticEditKey;
    if (searchKeyEvent)
    {
        const bool controlDown = (originalFlags & MF_CTRL) ||
            WoGOptionSearch::IsVirtualKeyDown(VK_CONTROL);
        const bool ctrlF = controlDown &&
            ((originalType == MT_KEYDOWN && originalSubtype == HK_F) ||
                (syntheticEditKey && WoGOptionSearch::IsVirtualKeyDown('F')));
        if (ctrlF)
        {
            if (syntheticEditKey && searchEdit && searchEdit->text &&
                WoGOptionSearch::queryText != searchEdit->text)
                searchEdit->SetEditText((char*)WoGOptionSearch::queryText.c_str());
            searchAction = SEARCH_ACTION_FOCUS;
        }
        else if (WoGOptionSearch::active || searchFocused)
        {
            if (WoGOptionSearch::IsVirtualKeyDown(VK_UP))
                searchAction = SEARCH_ACTION_UP;
            else if (WoGOptionSearch::IsVirtualKeyDown(VK_DOWN))
                searchAction = SEARCH_ACTION_DOWN;
            else if (WoGOptionSearch::IsVirtualKeyDown(VK_PRIOR))
                searchAction = SEARCH_ACTION_PAGE_UP;
            else if (WoGOptionSearch::IsVirtualKeyDown(VK_NEXT))
                searchAction = SEARCH_ACTION_PAGE_DOWN;
            else if (WoGOptionSearch::IsVirtualKeyDown(VK_RETURN) ||
                (originalType == MT_KEYDOWN && originalSubtype == HK_ENTER))
                searchAction = WoGOptionSearch::active ? SEARCH_ACTION_SELECT : SEARCH_ACTION_CLOSE;
            else if (WoGOptionSearch::IsVirtualKeyDown(VK_ESCAPE) ||
                (originalType == MT_KEYDOWN && originalSubtype == HK_ESC))
                searchAction = WoGOptionSearch::active ? SEARCH_ACTION_CLEAR : SEARCH_ACTION_CLOSE;
        }
        else if (originalType == MT_KEYDOWN &&
            (originalSubtype == HK_ENTER || originalSubtype == HK_ESC))
            searchAction = SEARCH_ACTION_CLOSE;

        if (searchAction == SEARCH_ACTION_CLOSE)
        {
            msg->type = MT_EXIT;
            msg->subtype = MST_EXIT;
            msg->item_id = DIID_OK;
        }
        else if (searchAction != SEARCH_ACTION_NONE)
            msg->type = 0;
        else if (originalType == MT_KEYDOWN && (searchFocused || WoGOptionSearch::active))
            dlg->SetFocuseToItem(WoGOptionSearch::EDIT_ID);
    }

    // DefProc automatically opens full_tip_text on right-click. Search rows
    // instead use the enhanced full description below, so suppress default
    // handling for this one event before it can create the obsolete popup.
    const bool enhancedResultPopup = originalType == MT_MOUSEBUTTON &&
        originalSubtype == MST_RBUTTONDOWN &&
        originalItemId >= WoGOptionSearch::RESULT_TEXT_BASE &&
        originalItemId < WoGOptionSearch::RESULT_TEXT_BASE + WoGOptionSearch::VISIBLE_RESULTS;
    if (enhancedResultPopup)
        msg->type = 0;

    const int r = dlg->DefProc(msg);

    if (searchAction != SEARCH_ACTION_NONE)
    {
        if (searchAction == SEARCH_ACTION_FOCUS)
            WoGOptionSearch::FocusEdit(dlg, ds);
        else if (searchAction == SEARCH_ACTION_UP)
        {
            if (WoGOptionSearch::AllowArrowNavigation(-1))
                WoGOptionSearch::MoveSelection(dlg, ds, -1);
        }
        else if (searchAction == SEARCH_ACTION_DOWN)
        {
            if (WoGOptionSearch::AllowArrowNavigation(1))
                WoGOptionSearch::MoveSelection(dlg, ds, 1);
        }
        else if (searchAction == SEARCH_ACTION_PAGE_UP)
            WoGOptionSearch::MoveSelection(dlg, ds, -WoGOptionSearch::VISIBLE_RESULTS);
        else if (searchAction == SEARCH_ACTION_PAGE_DOWN)
            WoGOptionSearch::MoveSelection(dlg, ds, WoGOptionSearch::VISIBLE_RESULTS);
        else if (searchAction == SEARCH_ACTION_SELECT)
            WoGOptionSearch::SelectResult(dlg, ds);
        else if (searchAction == SEARCH_ACTION_CLEAR)
        {
            WoGOptionSearch::ClearQuery(dlg, ds, true);
            dlg->SetFocuseToItem(-1);
            WoGOptionSearch::UpdatePlaceholder(dlg);
        }
        else if (searchAction == SEARCH_ACTION_CLOSE)
            return 2;
        if (WoGOptionSearch::active)
            dlg->SetFocuseToItem(WoGOptionSearch::EDIT_ID);
        return 1;
    }

    if (originalType == MT_KEYDOWN && (searchFocused || WoGOptionSearch::active ||
        (searchEdit && searchEdit->focused)))
    {
        WoGOptionSearch::UpdateFromEdit(dlg, ds, false);
        return r;
    }

    if (originalType == WM_MOUSEWHEEL && WoGOptionSearch::active)
    {
        WoGOptionSearch::MoveSelection(dlg, ds, originalSubtype > 0 ? -1 : 1);
        dlg->SetFocuseToItem(WoGOptionSearch::EDIT_ID);
        return 1;
    }

    if (originalType == MT_MOUSEOVER)
    {
        _DlgItem_* it = dlg->FindItem(originalX, originalY);
        char* textBar = o_NullString;
        if (it)
        {
            int hitId = it->id;
            if (hitId == WoGOptionSearch::TARGET_HIGHLIGHT_ID &&
                WoGOptionSearch::highlightedEntry >= 0 &&
                WoGOptionSearch::highlightedEntry < (int)WoGOptionSearch::index.size())
                hitId = WoGOptionSearch::index[WoGOptionSearch::highlightedEntry].buttonId;

            if (hitId == 3 || hitId == 4)
                textBar = ds->Hint;
            if (hitId == 5)
                textBar = txtresWOG->GetString(86);
            if (hitId == 6)
                textBar = txtresWOG->GetString(87);
            if (hitId == 7)
                textBar = txtresWOG->GetString(83);
            if (hitId == 8)
                textBar = txtresWOG->GetString(88);
            if (hitId == 9)
                textBar = txtresWOG->GetString(82);
            if (hitId == 10)
                textBar = txtresWOG->GetString(84);
            if (hitId == DIID_OK)
                textBar = txtresWOG->GetString(85);
            if (hitId == WoGOptionSearch::EDIT_ID || hitId == WoGOptionSearch::PLACEHOLDER_ID)
                textBar = (char*)WoGOptionSearch::JsonText("wnd.dlg_wog_options.search.placeholder");

            if (hitId != apdFont && apdFont != 0)
            {
                UpdateWoGPageLabelColors(dlg, o_LastChoosenPage);
                dlg->Redraw();
            }

            if (hitId >= 41 && hitId <= 48)
            {
                const int page = hitId - 41;
                _DlgStaticText_* pageLabel = (_DlgStaticText_*)dlg->GetItem(hitId);
                pageLabel->color = page == o_LastChoosenPage
                    ? WoGOptionColors::WHITE : WoGOptionColors::HIGHLIGHT;
                textBar = ds->Pages[page]->Hint;
                apdFont = hitId;
                dlg->Redraw();
            }
            else
            {
                apdFont = 0;
            }

            if (hitId >= 800 && hitId < 900)
            {
                const int page = (hitId / 10) % 10;
                const int group = hitId % 10;
                if (page >= 0 && page < 8 && group >= 0 && group < 4 &&
                    ds->Pages[page] && ds->Pages[page]->ItemList[group])
                    textBar = ds->Pages[page]->ItemList[group]->Hint;
            }

            if (hitId >= WoGOptionSearch::RESULT_TEXT_BASE &&
                hitId < WoGOptionSearch::RESULT_TEXT_BASE + WoGOptionSearch::VISIBLE_RESULTS)
            {
                const int slot = hitId - WoGOptionSearch::RESULT_TEXT_BASE;
                const int matchIndex = WoGOptionSearch::firstVisible + slot;
                if (matchIndex >= 0 && matchIndex < (int)WoGOptionSearch::matches.size())
                    textBar = WoGOptionSearch::resultHint[slot];
            }
            else
            {
                WoGOptionRows::RowRef row;
                if (WoGOptionRows::DecodeControl(ds, hitId, row))
                {
                    if (row.kind == WoGOptionRows::EXTERNAL)
                    {
                        WoGOptionSearch::Entry* entry = WoGOptionSearch::FindEntry(
                            row.page, row.group, row.item);
                        if (entry)
                            WoGOptionSearch::FormatExternalStatus(*entry,
                                WoGOptionSearch::statusText,
                                sizeof(WoGOptionSearch::statusText));
                    }
                    else
                    {
                        WoGOptionSearch::FormatStatus(row.optionId,
                            WoGOptionRows::Hint(row), WoGOptionSearch::statusText,
                            sizeof(WoGOptionSearch::statusText));
                    }
                    textBar = WoGOptionSearch::statusText;
                }
            }

            WoGOptionSkin::UpdateStatusText(dlg, textBar);
        }
    }

    if (originalType == MT_MOUSEBUTTON)
    {
        if (originalSubtype == MST_LBUTTONDOWN)
        {
            if (originalItemId == WoGOptionSearch::EDIT_ID ||
                originalItemId == WoGOptionSearch::PLACEHOLDER_ID)
            {
                WoGOptionSearch::FocusEdit(dlg, ds);
                return r;
            }

            if (originalItemId >= WoGOptionSearch::RESULT_TEXT_BASE &&
                originalItemId < WoGOptionSearch::RESULT_TEXT_BASE + WoGOptionSearch::VISIBLE_RESULTS)
            {
                const int matchIndex = WoGOptionSearch::firstVisible +
                    originalItemId - WoGOptionSearch::RESULT_TEXT_BASE;
                if (matchIndex >= 0 && matchIndex < (int)WoGOptionSearch::matches.size())
                {
                    WoGOptionSearch::selected = matchIndex;
                    WoGOptionSearch::RenderResults(dlg, ds);
                }
                dlg->SetFocuseToItem(WoGOptionSearch::EDIT_ID);
                return 1;
            }

            if (originalItemId == 3 || originalItemId == 4)
                ShowWoGOptionPopup(ds->PopUp, MBX_OK);

            if (originalItemId >= 41 && originalItemId <= 48)
            {
                WoGOptionSearch::ClearQuery(dlg, ds, false);
                dlg->SetFocuseToItem(-1);
                WoGOptionSearch::SwitchPage(dlg, ds, originalItemId - 41, true);
                WoGOptionSearch::UpdatePlaceholder(dlg);
                return r;
            }
        }

        if (originalSubtype == MST_LBUTTONCLICK)
        {
            if (originalItemId >= WoGOptionSearch::RESULT_TEXT_BASE &&
                originalItemId < WoGOptionSearch::RESULT_TEXT_BASE + WoGOptionSearch::VISIBLE_RESULTS)
            {
                const int matchIndex = WoGOptionSearch::firstVisible +
                    originalItemId - WoGOptionSearch::RESULT_TEXT_BASE;
                if (matchIndex >= 0 && matchIndex < (int)WoGOptionSearch::matches.size())
                {
                    WoGOptionSearch::selected = matchIndex;
                    WoGOptionSearch::SelectResult(dlg, ds);
                }
                return r;
            }

            int callBack = 0;
            bool listenerInvoked = false;
            bool externalActionInvoked = false;
            int externalBulkAction = -1;

            if (originalItemId == 5 && o_LastChoosenPage != -1)
            {
                callBack = ds->GetListener(0, o_LastChoosenPage + 1, 0, 4);
                listenerInvoked = true;
                externalBulkAction = WOG_EXTERNAL_CHANGE_SELECT_ALL;
            }
            if (originalItemId == 6 && o_LastChoosenPage != -1)
            {
                callBack = ds->GetListener(0, o_LastChoosenPage + 1, 0, 5);
                listenerInvoked = true;
                externalBulkAction = WOG_EXTERNAL_CHANGE_CLEAR_ALL;
            }
            if (originalItemId == 7 && o_LastChoosenPage != -1)
            {
                callBack = ds->GetListener(0, o_LastChoosenPage + 1, 0, 2);
                listenerInvoked = true;
                externalBulkAction = 0;
            }
            if (originalItemId == 10 && o_LastChoosenPage != -1)
            {
                callBack = ds->GetListener(0, o_LastChoosenPage + 1, 0, 3);
                listenerInvoked = true;
            }
            if (originalItemId == 8 || originalItemId == 9)
            {
                if (_DlgItem_* intro = dlg->GetItem(4))
                    intro->Hide();
                if (o_LastChoosenPage == -1)
                {
                    WoGOptionSearch::ClearQuery(dlg, ds, false);
                    WoGOptionSearch::HideAllPages(dlg, ds);
                    ShowHide_WoGDlgSetup_ElemOnPage(dlg, 0, ds, 1);
                    setYellowFrames(dlg, 0);
                    o_LastChoosenPage = 0;
                    WoGOptionSearch::StopTargetPulse(dlg, false);
                }
                callBack = ds->GetListener(0, 1, 0, originalItemId == 8 ? 8 : 1);
                listenerInvoked = true;
            }

            int optionControlId = originalItemId;
            if (optionControlId == WoGOptionSearch::TARGET_HIGHLIGHT_ID &&
                WoGOptionSearch::highlightedEntry >= 0 &&
                WoGOptionSearch::highlightedEntry < (int)WoGOptionSearch::index.size())
                optionControlId = WoGOptionSearch::index[WoGOptionSearch::highlightedEntry].buttonId;

            WoGOptionRows::RowRef row;
            if (WoGOptionRows::DecodeControl(ds, optionControlId, row))
            {
                const int buttonId = 1000 * (row.page + 1) + 200 * row.group + 100 + row.item;
                if (optionControlId == buttonId && ds->ButtonsStates[8] == 1)
                {
                    if (row.kind == WoGOptionRows::EXTERNAL)
                    {
                        const uint32_t state = row.external
                            ? WoGExternalOptions::Refresh(*row.external) : 0;
                        if (row.external && WoGExternalOptions::IsLocked(*row.external))
                        {
                            if (WoGOptionSearch::Entry* entry = WoGOptionSearch::FindEntry(
                                row.page, row.group, row.item))
                            {
                                WoGOptionSearch::FormatExternalStatus(*entry,
                                    WoGOptionSearch::statusText,
                                    sizeof(WoGOptionSearch::statusText));
                                WoGOptionSkin::UpdateStatusText(dlg,
                                    WoGOptionSearch::statusText, true);
                            }
                        }
                        else if (state & WOG_EXTERNAL_STATE_ENABLED)
                        {
                            WoGExternalOptions::SetChecked(*row.external,
                                !(state & WOG_EXTERNAL_STATE_CHECKED),
                                WOG_EXTERNAL_CHANGE_DIRECT_CLICK);
                            externalActionInvoked = true;
                            if (WoGOptionSearch::Entry* entry = WoGOptionSearch::FindEntry(
                                row.page, row.group, row.item))
                            {
                                WoGOptionSearch::FormatExternalStatus(*entry,
                                    WoGOptionSearch::statusText,
                                    sizeof(WoGOptionSearch::statusText));
                                WoGOptionSkin::UpdateStatusText(dlg,
                                    WoGOptionSearch::statusText, true);
                            }
                        }
                    }
                    else
                    {
                        _DlgSetup_ItemList_* list = row.list;
                        if (IsWoGOptionLocked(row.optionId))
                        {
                            WoGOptionSearch::FormatStatus(row.optionId,
                                WoGOptionRows::Hint(row), WoGOptionSearch::statusText,
                                sizeof(WoGOptionSearch::statusText));
                            WoGOptionSkin::UpdateStatusText(dlg,
                                WoGOptionSearch::statusText, true);
                        }
                        else
                        {
                            int state = list->ItemState[row.item];
                            if (state < 2)
                            {
                                state = 1 - state;
                                if (list->Type == 1)
                                {
                                    list->ItemState[row.item] = state;
                                    ((_DlgButton_*)dlg->GetItem(buttonId))->def_frame_index = state * 2;
                                    ((_DlgButton_*)dlg->GetItem(buttonId))->press_def_frame_index = state * 2 + 1;
                                }
                                if (list->Type == 2)
                                {
                                    const int firstButtonId = 1000 * (row.page + 1) + 200 * row.group + 100;
                                    for (int k = 0; k < list->ItemCount; ++k)
                                    {
                                        ((_DlgButton_*)dlg->GetItem(firstButtonId + k))->def_frame_index = k == row.item ? 2 : 0;
                                        ((_DlgButton_*)dlg->GetItem(firstButtonId + k))->press_def_frame_index = k == row.item ? 3 : 1;
                                        list->ItemState[k] = k == row.item ? 1 : 0;
                                    }
                                }
                                callBack = ds->GetListener(1, row.page + 1, row.group, row.item);
                                listenerInvoked = true;
                            }
                        }
                    }
                }
            }

            if (externalBulkAction >= 0 && o_LastChoosenPage >= 0)
            {
                WoGExternalOptions::ApplyPageAction(o_LastChoosenPage,
                    (uint32_t)externalBulkAction);
                externalActionInvoked = true;
            }

            if (listenerInvoked || externalActionInvoked)
            {
                // Enforce even when the original listener reports no redraw:
                // presets and defaults may still change stored values.
                ForceWoGOptionLockValues();
                ReapplyWoGOptionLocks(ds);
                WoGOptionSearch::RedrawAllOptionStates(dlg, ds);
                if (WoGOptionSearch::active)
                    WoGOptionSearch::RenderResults(dlg, ds);
            }
            (void)callBack;
            dlg->Redraw();
        }

        if (originalSubtype == MST_RBUTTONDOWN)
        {
            if (originalItemId == 3 || originalItemId == 4)
                ShowWoGOptionPopup(ds->PopUp, MBX_RMC);

            if (originalItemId >= 41 && originalItemId <= 48)
                ShowWoGOptionPopup(ds->Pages[originalItemId - 41]->PopUp, MBX_RMC);

            if (originalItemId >= 800 && originalItemId < 900)
            {
                const int page = (originalItemId / 10) % 10;
                const int group = originalItemId % 10;
                if (page >= 0 && page < 8 && group >= 0 && group < 4 &&
                    ds->Pages[page] && ds->Pages[page]->ItemList[group])
                    ShowWoGOptionPopup(ds->Pages[page]->ItemList[group]->PopUp, MBX_RMC);
            }

            if (originalItemId >= WoGOptionSearch::RESULT_TEXT_BASE &&
                originalItemId < WoGOptionSearch::RESULT_TEXT_BASE + WoGOptionSearch::VISIBLE_RESULTS)
            {
                const int slot = originalItemId - WoGOptionSearch::RESULT_TEXT_BASE;
                const int matchIndex = WoGOptionSearch::firstVisible + slot;
                if (matchIndex >= 0 && matchIndex < (int)WoGOptionSearch::matches.size())
                    ShowWoGOptionPopup(WoGOptionSearch::resultPopup[slot], MBX_RMC);
                return 1;
            }
            else
            {
                int optionControlId = originalItemId;
                if (optionControlId == WoGOptionSearch::TARGET_HIGHLIGHT_ID &&
                    WoGOptionSearch::highlightedEntry >= 0 &&
                    WoGOptionSearch::highlightedEntry < (int)WoGOptionSearch::index.size())
                    optionControlId = WoGOptionSearch::index[WoGOptionSearch::highlightedEntry].buttonId;

                WoGOptionRows::RowRef row;
                if (WoGOptionRows::DecodeControl(ds, optionControlId, row))
                {
                    if (row.kind == WoGOptionRows::EXTERNAL)
                    {
                        if (WoGOptionSearch::Entry* entry = WoGOptionSearch::FindEntry(
                            row.page, row.group, row.item))
                            WoGOptionSearch::FormatExternalPopup(*entry,
                                WoGOptionSearch::popupText,
                                sizeof(WoGOptionSearch::popupText));
                    }
                    else
                    {
                        WoGOptionSearch::FormatPopup(row.optionId,
                            WoGOptionRows::Name(row), WoGOptionRows::Popup(row),
                            WoGOptionSearch::popupText,
                            sizeof(WoGOptionSearch::popupText));
                    }
                    ShowWoGOptionPopup(WoGOptionSearch::popupText, MBX_RMC);
                }
            }
        }
    }

    if (WoGOptionSearch::active)
        dlg->SetFocuseToItem(WoGOptionSearch::EDIT_ID);
    return r;
}

// Retained temporarily as a byte-for-byte behavioral reference while the
// enhanced procedure above replaces it at dialog construction.
#if 0
int __stdcall Dlg_WoG_Options_Proc_Legacy(_CustomDlg_* dlg, _EventMsg_* msg)
{
    int r = dlg->DefProc(msg);
    _DlgSetup_* ds = o_DlgSetup;     

    if (msg->type == MT_MOUSEOVER)  {
        _DlgItem_* it = dlg->FindItem(msg->x_abs, msg->y_abs);
        char* text_bar = o_NullString;
        if (it) {
            if (it->id == 3 || it->id == 4) { text_bar = o_DlgSetup->Hint; }
            if (it->id == 5) { text_bar = txtresWOG->GetString(86); }
            if (it->id == 6) { text_bar = txtresWOG->GetString(87); }
            if (it->id == 7) { text_bar = txtresWOG->GetString(83); }
            if (it->id == 8) { text_bar = txtresWOG->GetString(88); }
            if (it->id == 9) { text_bar = txtresWOG->GetString(82); }
            if (it->id == 10) { text_bar = txtresWOG->GetString(84); }
            if (it->id == DIID_OK) { text_bar = txtresWOG->GetString(85); } 

            if (it->id != apdFont && apdFont != 0) {
                ((_DlgStaticText_*)dlg->GetItem(apdFont))->SetFont(n_SmallFont);
                dlg->Redraw();
            } 

            if (it->id >=41 && it->id <=48) {
                int id = it->id -41;
                ((_DlgStaticText_*)dlg->GetItem(it->id))->SetFont(n_MedFont);
                text_bar = o_DlgSetup->Pages[id]->Hint;
                apdFont = it->id;   
                dlg->Redraw();
            } else { apdFont = 0; }

            if (it->id >= 800 && it->id < 900 ) {
                text_bar = ds->Pages[(it->id / 10) % 10]->ItemList[it->id % 10]->Hint;
            }

            if (it->id >= 1000 && it->id < 9000 ) { 
                text_bar = ds->Pages[(it->id / 1000) -1]->ItemList[( ( (it->id -100) / 100) % 10 ) / 2]->ItemHint[it->id % 100];
            }
            statbarWoGOptions->SetText(text_bar);
            statbarWoGOptions->Draw();
            statbarWoGOptions->RedrawScreen();
        }
    } // MT_MOUSEOVER

        if (msg->type == MT_MOUSEBUTTON) {
            if (msg->subtype == MST_LBUTTONDOWN){
                if (msg->item_id == 3 || msg->item_id == 4) 
                    ShowWoGOptionPopup( ds->PopUp, MBX_OK);
                
                if (msg->item_id >= 41 && msg->item_id <= 48) { 
                    ((_DlgStaticText_*)dlg->GetItem(4))->Hide();
                    for (int i=0; i<8; i++) {
                        ShowHide_WoGDlgSetup_ElemOnPage(dlg, i, ds, 0);
                    }                   
                    int id = msg->item_id -41;                  
                    ShowHide_WoGDlgSetup_ElemOnPage(dlg, id, ds, 1);
                    setYellowFrames(dlg, id);
                    ds->GetListener(2, id+1, -1, -1); // o_LastChoosenPage = id;
                    dlg->Redraw();                  
                }
                
            } // MST_LBUTTONDOWN

            if (msg->subtype == MST_LBUTTONCLICK) { // ЛКМ при отжатии
                int callBack = 0;
                if (msg->item_id == 5 ) { if (o_LastChoosenPage != -1) callBack = ds->GetListener(0, o_LastChoosenPage +1, 0, 4); }  // Выбрать все
                if (msg->item_id == 6 ) { if (o_LastChoosenPage != -1) callBack = ds->GetListener(0, o_LastChoosenPage +1, 0, 5); }  // Сбросить все
                if (msg->item_id == 7 ) { if (o_LastChoosenPage != -1) callBack = ds->GetListener(0, o_LastChoosenPage +1, 0, 2); }  // По умолчанию 
                if (msg->item_id == 10 ) { if (o_LastChoosenPage != -1) callBack = ds->GetListener(0, o_LastChoosenPage +1, 0, 3); } // Мультиплеер
                if (msg->item_id == 8 || msg->item_id == 9 ) {
                    ((_DlgStaticText_*)dlg->GetItem(4))->Hide();
                    if (o_LastChoosenPage == -1) {
                        ShowHide_WoGDlgSetup_ElemOnPage(dlg, 0, ds, 1);
                        setYellowFrames(dlg, 0);
                        o_LastChoosenPage = 0;
                    }
                    callBack = ds->GetListener(0, 1, 0, msg->item_id == 8 ? 8 : 1);
                }

                if (msg->item_id >= 1000 && msg->item_id <= 8999 ) { 
                    if (ds->ButtonsStates[8] == 1) {
                        int page = (msg->item_id / 1000) -1;
                        int itList = ( ( (msg->item_id -100) / 100) % 10 ) / 2;
                        int item = msg->item_id % 100;

                        int state = ds->Pages[page]->ItemList[itList]->ItemState[item];

                        if (state < 2) {
                            state *= -1; state += 1;
                            if (ds->Pages[page]->ItemList[itList]->Type == 1 ) { // чекбоксы
                                ds->Pages[page]->ItemList[itList]->ItemState[item] = state;                     
                                ((_DlgButton_*)dlg->GetItem(msg->item_id))->def_frame_index = state*2;
                                ((_DlgButton_*)dlg->GetItem(msg->item_id))->press_def_frame_index = (state*2)+1;
                            }
                            if (ds->Pages[page]->ItemList[itList]->Type == 2 ) { // радиобаттоны
                                int idoff = (1000*(page+1)) +(200*itList) +100;
                                for (int k=0; k<ds->Pages[page]->ItemList[itList]->ItemCount; k++) {
                                    if (k != item) {
                                        ((_DlgButton_*)dlg->GetItem(idoff+k))->def_frame_index = 0;
                                        ((_DlgButton_*)dlg->GetItem(idoff+k))->press_def_frame_index = 1;
                                        ds->Pages[page]->ItemList[itList]->ItemState[k] = 0;
                                    } else {
                                        ((_DlgButton_*)dlg->GetItem(idoff+k))->def_frame_index = 2;
                                        ((_DlgButton_*)dlg->GetItem(idoff+k))->press_def_frame_index = 3;
                                        ds->Pages[page]->ItemList[itList]->ItemState[item] = 1;
                                    }
                                }
                            }
                        callBack = ds->GetListener(1, page+1, itList, item);
                        }   
                    }
                }
                if ( callBack == 1) { 
                    for (int i=0; i<8; i++) {
                        if (ds->Pages[i]->Enabled) {
                            Redraw_WoGDlgSetup_ElemOnPage(dlg, ds, i);
                        }
                    }
                    callBack = 0;
                }
                dlg->Redraw();
            } // MST_LBUTTONCLICK

            if (msg->subtype == MST_RBUTTONDOWN){
                if (msg->item_id == 3 || msg->item_id == 4) 
                    ShowWoGOptionPopup( ds->PopUp, MBX_RMC );
                
                if (msg->item_id >= 41 && msg->item_id <= 48) { 
                    ShowWoGOptionPopup( ds->Pages[msg->item_id -41]->PopUp, MBX_RMC);
                }
                if (msg->item_id >= 800 && msg->item_id < 900) { 
                    ShowWoGOptionPopup( ds->Pages[(msg->item_id / 10) % 10]->ItemList[msg->item_id % 10]->PopUp, MBX_RMC);
                }
                if (msg->item_id >= 1000 && msg->item_id < 9000 ) { 
                    int page = (msg->item_id / 1000) -1;
                    int itList = ( ( (msg->item_id -100) / 100) % 10 ) / 2;
                    int item = msg->item_id % 100;
                    ShowWoGOptionPopup( ds->Pages[page]->ItemList[itList]->ItemPopUp[item], MBX_RMC);
                }
            } // MST_RBUTTONDOWN
        } // MT_MOUSEBUTTON

    return r;
}

#endif

/* void __fastcall Dlg_WoG_Options_Scroll(int step, _CustomDlg_* dlg)
{
    // Устанавливаем ползунок в ближайшее к точке клика положение
    _DlgSetup_* ds = o_DlgSetup;

    if (step >= 0 && step < 7)ShowHide_WoGDlgSetup_ElemOnPage(dlg, o_LastChoosenPage, ds, 0);
    ShowHide_WoGDlgSetup_ElemOnPage(dlg, step, ds, 1);
    setYellowFrames(dlg, step);
    o_LastChoosenPage = step; 

    //sprintf(o_TextBuffer, "%d", step);
    //b_MsgBox(o_TextBuffer, 1);

    _DlgMsg_ m;
    CALL_2(void, __thiscall, 0x5FF3A0, dlg, m.Set(512, 3, 11, 0, 0, 0, 0, 0) );
    dlg->Redraw(TRUE);

}*/


void __stdcall Dlg_WoG_Options_Show(HiHook* hook, int a1)
{
    int color = o_MeID; o_MeID = 1;
    _DlgSetup_* ds = o_DlgSetup;
    WoGOptionSearch::Reset();
    RefreshWoGOptionLocks();
    ForceWoGOptionLockValues();
    ReapplyWoGOptionLocks(ds);
    WoGExternalOptions::BeginSession(ds);
    WoGOptionSearch::BuildIndex(ds);
    _CustomDlg_* dlg = _CustomDlg_::Create(o_HD_X/2 -400, o_HD_Y/2 -300, 800, 600, DF_SCREENSHOT , Dlg_WoG_Options_Proc);
    WoGOptionSkin::CreateBackground(dlg);
    WoGOptionSkin::CreateBaseSurfaces(dlg);
    
    const int statusY = WoGOptionSkin::available ? dlg->height - 32 : dlg->height - 26;
    const int statusX = WoGOptionSkin::available ? 48 : 7;
    const int statusWidth = WoGOptionSkin::available ? 704 : dlg->width - 14;
    statbarWoGOptions = _DlgStaticTextPcx8ed_::Create(statusX, statusY, statusWidth, 18,
        o_NullString, n_SmallFont,
        WoGOptionSkin::available ? (char*)WoGOptionSkin::STATUS_PCX : "WoGOptions.pcx",
        WoGOptionColors::REGULAR, 2, ALIGN_H_CENTER | ALIGN_V_CENTER);
    dlg->AddItem(statbarWoGOptions); // подсказка в статус баре 
    WoGOptionSkin::CreateFooterOrnaments(dlg);

    dlg->AddItem(_DlgStaticText_::Create(214, 20, 370, 20, ds->Name,
        json_WoGOpt[0], WoGOptionColors::WHITE, 3,
        ALIGN_H_CENTER | ALIGN_V_CENTER, 0)); // id = 3
    dlg->AddItem(_DlgStaticText_::Create(230, 50, 538, 468, ds->Intro,
        json_WoGOpt[0], WoGOptionColors::REGULAR, 4,
        ALIGN_H_CENTER | ALIGN_V_CENTER, 0)); //id = 4

    // Create search/result/highlight controls before option rows so both gold
    // frames remain visually behind the clickable row controls.
    WoGOptionSearch::CreateControls(dlg);

    for (int i=0; i<8; i++) {
        if (ds->Pages[i]->Enabled) {
            dlg->AddItem(_DlgStaticText_::Create(23, 49 +60*i, 192, 49,
                ds->Pages[i]->Name, n_SmallFont, WoGOptionColors::REGULAR,
                41 +i, ALIGN_H_CENTER | ALIGN_V_CENTER, 0));    // id = 41...48
            
            Create_WoGDlgSetup_ElemOnPage(dlg, i, ds); // функция построения элементов диалога текущей в цикле страницы
            ShowHide_WoGDlgSetup_ElemOnPage(dlg, i, ds, 0); // скрываем все элементы на странице
        }
    }
    char* bttnName = WoGOptionSkin::available
        ? (char*)WoGOptionSkin::BUTTON_DEF : "WoGBttn.def";
    if (ds->ButtonsStates[5] == 1) {dlg->AddItem(_DlgButton_::Create(375, 528, 64, 30, 5, bttnName, 15, 16, 0, HK_B, 0)); } // id = 5 // выбрать всё
    if (ds->ButtonsStates[4] == 1) {dlg->AddItem(_DlgButton_::Create(440, 528, 64, 30, 6, bttnName, 18, 19, 0, HK_N, 0)); } // id = 6 // сбросить всё
    if (ds->ButtonsStates[2] == 1) {dlg->AddItem(_DlgButton_::Create(510, 528, 64, 30, 7, bttnName, 12, 13, 0, HK_R, 0)); } // id = 7 // по умолчанию
    if (ds->ButtonsStates[8] == 1) {dlg->AddItem(_DlgButton_::Create(580, 528, 64, 30, 8, bttnName, 6, 7, 0, HK_L, 0)); }   // id = 8 // загрузить
    if (ds->ButtonsStates[1] == 1) {dlg->AddItem(_DlgButton_::Create(645, 528, 64, 30, 9, bttnName, 9, 10, 0, HK_S, 0)); }  // id = 9 // сохранить
    if (ds->ButtonsStates[3] == 1) {dlg->AddItem(_DlgButton_::Create(305, 528, 64, 30, 10, bttnName, 21, 22, 0, HK_M, 0)); }  // id = 10 // мультиплеер
    if (ds->ButtonsStates[0] == 1) {dlg->AddItem(_DlgButton_::Create(715, 528, 64, 30, DIID_OK, bttnName, 0, 1, 1, 0, 2)); } // id = 30725; Enter/Esc handled by dialog proc
    //_DlgScroll_* wogOptScroll = _DlgScroll_::Create(26, 534, 334, 16, 11, 8, (_ptr_)Dlg_WoG_Options_Scroll, 0, 0, 0); // создать ползунок
    //dlg->AddItem(wogOptScroll);

    o_LastChoosenPage = -1;   // страница диалога
    setYellowFrames(dlg, o_LastChoosenPage); // создаем массово желтые рамки (dlg, номер страницы вог диалога)

    WoGOptionSearch::UpdatePlaceholder(dlg);
    dlg->Run();
    WoGOptionSearch::StopTargetPulse(dlg, false);
    ForceWoGOptionLockValues();
    ReapplyWoGOptionLocks(ds);
    dlg->Destroy(TRUE);
    WoGOptionSearch::Reset();
    WoGExternalOptions::EndSession();
    apdFont = 0;
    o_MeID = color;
}


// #############################################################################################
// ##################################  создание кнопки WoG Options #############################

int focusedItemID;

int __stdcall Y_NewScenarioDlg_Proc(HiHook* hook, _NewScenarioDlg_* this_, _EventMsg_* msg)
{
    if ( (msg->type == MT_MOUSEBUTTON) && (msg->subtype == MST_LBUTTONCLICK) && (msg->item_id == 4444) ){
        msg->item_id = 0;
        msg->x_abs = 640;
        msg->y_abs = 110; 
        // CALL_0(void, __cdecl, 0x7790E1);
    }

    if (msg->type == MT_MOUSEOVER) {
        _DlgItem_* it = this_->FindItem(msg->x_abs, msg->y_abs);
        if (it) {
            focusedItemID = it->id;
        }
    }

    // прокрутка списка героев и городов
    if (msg->type == WM_MOUSEWHEEL) {
        Y_NewScenarioDlg_SetScrolledShoose(this_, msg);
    }

    return CALL_2(int, __thiscall, hook->GetDefaultFunc(), this_, msg);
}

// блокировка скроллбара, если курсор не на нём
int __stdcall Y_NewScenarioDlg_BlockScrollBar(HiHook* hook, void* this_)
{
    if (focusedItemID != 338)
        return 0;

    return CALL_1(int, __thiscall, hook->GetDefaultFunc(), this_);
}

void __stdcall Y_NewScenarioDlg_Create(HiHook* hook, _NewScenarioDlg_* this_, int type)
{
    CALL_2(void, __thiscall, hook->GetDefaultFunc(), this_, type);
    this_->AddItem(_DlgTextButton_::Create(622, 105, 4444, "GSPBUT2.DEF", json_WoGOpt[1], n_SmallFont, 0, 1, 0, 0, 1));
    focusedItemID = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dlg_WoGOptions(PatcherInstance* _PI)
{
    // создание кнопки WoG Options
    _PI->WriteDword((0x779100 + 3), 640);
    _PI->WriteDword((0x779119 + 3), 640);
    _PI->WriteByte((0x779132 + 3), 110);
    _PI->WriteByte((0x779147 + 3), 110);
    _PI->WriteHiHook(0x579CE0, SPLICE_, EXTENDED_, THISCALL_, Y_NewScenarioDlg_Create);
    _PI->WriteHiHook(0x587FD0, SPLICE_, EXTENDED_, THISCALL_, Y_NewScenarioDlg_Proc);
    _PI->WriteHiHook(0x57CB70, SPLICE_, EXTENDED_, THISCALL_, Y_NewScenarioDlg_BlockScrollBar);
    // диалог WoG Опций
     _PI->WriteHiHook(0x779213, CALL_, EXTENDED_, CDECL_, Dlg_WoG_Options_Show);

}
