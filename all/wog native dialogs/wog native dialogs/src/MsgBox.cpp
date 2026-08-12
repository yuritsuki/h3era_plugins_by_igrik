///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////// Callback диалога MsgBox ///////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma comment(linker, "/EXPORT:PrepareDlgPictures=_PrepareDlgPictures@12")
#pragma comment(linker, "/EXPORT:PrepareDlgText=_PrepareDlgText@12")
#pragma comment(linker, "/EXPORT:ShowPreparedDlg=_ShowPreparedDlg@24")

// по большей части тупо переписан оригинальный код функции 0x4F1650,
// но в этом коде расширено кол-во кликабельных желтых рамок.
// В оригинале было всего 2 кликабельные рамки: 30729 и 30730.
// Теперь же работают все рамки: 30729-30736
// И в довесок реализован даббл.клик выбора
// UPD: добавлена возможность ототображать любой деф и текст экспортными def
#define DllExport extern "C" __declspec(dllexport)
#define P_Dlg_MsgBox (*(_Dlg_ **)0x6995E0)

constexpr auto ERA_SPEC_DLG_ITEM_ID = 1525;

constexpr auto MSG_10_OK = 30720;
constexpr auto MSG_10_CANCEL = 30721;

constexpr auto MSG_1_OK = 30722;
constexpr auto MSG_2_OK = 30725;
constexpr auto MSG_2_CANCEL = 30726;

constexpr auto MSG_10_IT0 = 30729;
constexpr auto MSG_10_IT1 = 30730;
constexpr auto MSG_10_IT2 = 30731;
constexpr auto MSG_10_IT3 = 30732;
constexpr auto MSG_10_IT4 = 30733;
constexpr auto MSG_10_IT5 = 30734;
constexpr auto MSG_10_IT6 = 30735;
constexpr auto MSG_10_IT7 = 30736;

constexpr INT MINIMUM_ADDRESS_TO_READ = 0x100000;
constexpr INT ERA_ERM_STRING_ADDRESS = 1000000000;
constexpr INT MAXIMUM_DEFAULT_PIC_TYPE = 36;

int my_TimeClick_MsgBox;
int my_TimeAnimate_MsgBox;
static char *validExtensions[] = {".pcx", ".def", ".pcx16"};

// Adventure-map info-panel extension.  Type 9 is intentionally kept here,
// together with the existing native-dialog helpers, so the panel lifetime is
// handled by the same game code as the stock resource panel.
std::string GetStringFromInt(const int vAddress, const BOOL isExtraFile);

enum eAssetType : int
{
    ASSET_TYPE_NO = 0,
    ASSET_TYPE_PCX = 0x1,
    ASSET_TYPE_DEF = 0x2,
    ASSET_TYPE_PCX16 = 0x4,
    ASSET_TYPE_ANY_PCX = ASSET_TYPE_PCX | ASSET_TYPE_PCX16,
    ASSET_TYPE_DEF_ANIMATED = 0x8,
    ASSET_TYPE_ANY_DEF = ASSET_TYPE_DEF | ASSET_TYPE_DEF_ANIMATED,
    ASSET_TYPE_ALL = 0xF,
};
static eAssetType IsGameAssetType(const std::string &s, const eAssetType type)
{

    const size_t len = s.size();
    if (len < 5 || len > 12)
        return ASSET_TYPE_NO;
    const char *ext = s.c_str() + len - 4;
    constexpr size_t validExtensionsCount = std::size(validExtensions);
    for (size_t i = 0; i < validExtensionsCount; i++)
    {
        if (type & (1 << i))
        {
            if (_stricmp(ext, validExtensions[i]) == 0)
                return static_cast<eAssetType>(1 << i);
        }
    }

    return ASSET_TYPE_NO;
}

struct ItemInfo
{
    union {
        int defType = -1;
        const char *assetName;
    };
    union {
        int defFrame = 0;
        const char *hintPtr;
    };
};

struct Dlg8ItemInfo
{
    static constexpr size_t MAX_SIZE = 8;

  public:
    BOOL assetIsValid = FALSE;
    ItemInfo itemInfo;
    eAssetType assetType = ASSET_TYPE_NO;
    _HStr_ assetName;
    _HStr_ externalPicturePath;
    _DlgStaticDef_ *defPtr = nullptr;

  public:
    BOOL InitCustomAsset(const ItemInfo &itemInfo)
    {
        *this = {};
        this->itemInfo = itemInfo;
        std::string normalized = GetStringFromInt(itemInfo.defType, FALSE);
        switch (assetType = IsGameAssetType(normalized, ASSET_TYPE_ALL))
        {
        case eAssetType::ASSET_TYPE_PCX16:
            break;
        case eAssetType::ASSET_TYPE_PCX:
            if (itemInfo.defFrame == 16 || itemInfo.defFrame > MINIMUM_ADDRESS_TO_READ)
            {
                assetType = eAssetType::ASSET_TYPE_PCX16;
                if (itemInfo.defFrame != 16)
                    externalPicturePath.Set(GetStringFromInt(itemInfo.defFrame, TRUE).c_str());
            }
            break;
        case eAssetType::ASSET_TYPE_DEF:
            if (itemInfo.defFrame <= -1)
            {
                assetType = eAssetType::ASSET_TYPE_DEF_ANIMATED;
                this->itemInfo.defFrame = -1;
            }
            break;
        default:
            return FALSE;
        }
        assetName.Set(normalized.c_str());
        assetIsValid = TRUE;

        return assetIsValid;
    }
};
struct AdventureInfoDefPanelData
{
    _HStr_ text;
    Dlg8ItemInfo dlg8ItemInfos[2];
    int itemCount;
    int timeToShow;
    RECT position;

} adventureInfoDefPanel;

static bool TryReadString(const char *ptr, char *buffer, size_t maxLength)
{
    if (!ptr || !buffer || !maxLength)
        return false;

    __try
    {
        for (size_t i = 0; i <= maxLength; ++i)
        {
            buffer[i] = ptr[i];

            if (!buffer[i])
                return i != 0;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return false;
}

static bool TryReadStringPointer(int value, char *buffer, size_t maxLength)
{
    if (value <= MINIMUM_ADDRESS_TO_READ)
        return false;

    __try
    {
        const char *ptr = *reinterpret_cast<const char **>(value);
        return TryReadString(ptr, buffer, maxLength);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

static bool TryGetString(int value, char *buffer, size_t maxLength)
{
    if (value <= MINIMUM_ADDRESS_TO_READ)
        return false;

    if (TryReadString(reinterpret_cast<const char *>(value), buffer, maxLength)) // char*
        return true;

    return TryReadStringPointer(value, buffer, maxLength); // char**
}

std::string GetStringFromInt(const int vAddress, const BOOL isExtraFile)
{
    if (vAddress > ERA_ERM_STRING_ADDRESS)
    {
        // store prev erm data
        const int savedY1 = Era::y[1];
        strcpy_s(myString2, 512, Era::z[1]);

        Era::y[1] = vAddress;

        Era::ExecErmCmd("SN:Bzy1/d/?z1");
        strcpy_s(myString1, 512, Era::z[1]);

        // restore prev erm data
        Era::y[1] = savedY1;
        strcpy_s(Era::z[1], 512, myString2);

        if (_stricmp(myString1, "STRING NOT FOUND") != 0)
            return myString1;
    }

    if (TryGetString(vAddress, myString1, isExtraFile ? MAX_PATH : 12))
        return myString1;

    return {};
}

struct Dlg8Info
{
  public:
    static constexpr size_t MAX_SIZE = Dlg8ItemInfo::MAX_SIZE;

  public:
    BOOL hasAnimation = FALSE;
    size_t usedSize = 0;
    Dlg8ItemInfo dlg8ItemInfos[MAX_SIZE];
    struct Dlg8ItemTextInfo
    {
        _HStr_ textBelow;
        _HStr_ rmcHint;
    } dlg8ItemTextInfos[MAX_SIZE];

  public:
    inline void Clear()
    {
        for (size_t i = 0; i < MAX_SIZE; i++)
        {
            dlg8ItemInfos[i] = {};
            dlg8ItemTextInfos[i] = {};
        }

        hasAnimation = FALSE;
        usedSize = 0;
    }
};

struct Dlg8Manager
{
    std::vector<std::unique_ptr<Dlg8Info>> dlg8ItemInfosVector;
    Dlg8Info dlg8InfoBuffer;
    int itemsParsedPerDlg8 = 0;
    BOOL bufferInUse = FALSE;
    Dlg8Info *currentDlg8Info = nullptr;
    Dlg8ItemInfo *nextDlg8ItemsInfos = nullptr;

  public:
    inline void ClearPicturesBuffer()
    {
        for (size_t i = 0; i < Dlg8Info::MAX_SIZE; i++)
        {
            dlg8InfoBuffer.dlg8ItemInfos[i].itemInfo = {};
            dlg8InfoBuffer.dlg8ItemInfos[i].assetName.Destruct(1);
            dlg8InfoBuffer.dlg8ItemInfos[i].externalPicturePath.Destruct(1);
        }
        dlg8InfoBuffer.usedSize = 0;
    }
    inline void ClearTextBuffer()
    {
        for (size_t i = 0; i < Dlg8Info::MAX_SIZE; i++)
        {
            dlg8InfoBuffer.dlg8ItemTextInfos[i].textBelow = {};
            dlg8InfoBuffer.dlg8ItemTextInfos[i].rmcHint = {};
        }
    }
    inline void ClearNextDlgPreparation()
    {
        itemsParsedPerDlg8 = 0;
        bufferInUse = FALSE;
        nextDlg8ItemsInfos = nullptr;
    }
} dlg8Manager;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DllExport BOOL __stdcall PrepareDlgPictures(char **assetNames, int *frameIds, size_t length)
{

    dlg8Manager.ClearPicturesBuffer();
    auto &dlg8Info = dlg8Manager.dlg8InfoBuffer;

    length = Clamp(0, length, Dlg8ItemInfo::MAX_SIZE);
    int infoCounter = 0;
    for (size_t i = 0; i < length; i++)
    {
        if (char *assetName = assetNames[i])
        {
            auto &info = dlg8Info.dlg8ItemInfos[infoCounter];
            ItemInfo itemInfo;
            itemInfo.assetName = assetName;
            itemInfo.defFrame = frameIds[i];
            if (info.InitCustomAsset(itemInfo))
            {
                if (info.itemInfo.defFrame == -1)
                    dlg8Info.hasAnimation = TRUE;

                infoCounter++;
            }
        }
    }
    dlg8Info.usedSize = infoCounter;
    dlg8Manager.bufferInUse = dlg8Info.usedSize > 0;

    return infoCounter;
}

DllExport BOOL __stdcall PrepareDlgText(char **descriptions, char **rmcHints, size_t length)
{
    auto &dlg8Info = dlg8Manager.dlg8InfoBuffer;
    dlg8Manager.ClearTextBuffer();

    length = Clamp(0, length, Dlg8ItemInfo::MAX_SIZE);
    auto &dlg8ItemTextInfos = dlg8Info.dlg8ItemTextInfos;
    for (size_t i = 0; i < length; i++)
    {
        if (descriptions[i])
            dlg8ItemTextInfos[i].textBelow.Set(descriptions[i]);
        if (rmcHints[i])
            dlg8ItemTextInfos[i].rmcHint.Set(rmcHints[i]);
    }
    // dlg8Manager.bufferInUse = TRUE;

    return length;
}

DllExport BOOL __stdcall ShowPreparedDlg(char *text, int type, int x, int y, int timeToShow, int *result)
{

    if (!text)
        text = o_NullString;

    _List_<ItemInfo> items;
    auto &dlg8ItemInfos = dlg8Manager.dlg8InfoBuffer.dlg8ItemInfos;
    int count = 0;

    type = Clamp(1, type, 10);

    for (size_t i = 0; i < Dlg8ItemInfo::MAX_SIZE; i++)
    {
        auto &info = dlg8ItemInfos[i];
        ItemInfo item;

        if (info.assetIsValid)
        {
            count++;
            item.assetName = info.assetName.c_str;
            item.defFrame = info.itemInfo.defFrame;
        }
        items.Append(item);
    }
    dlg8Manager.ClearNextDlgPreparation();

    dlg8Manager.bufferInUse = TRUE;
    dlg8Manager.nextDlg8ItemsInfos = dlg8ItemInfos;

    if (count > 3)
    {
        Era::TComplexDialogOpts patchedTimeToShow{timeToShow, type};
        CALL_5(void, __fastcall, 0x4F7D20, text, &items, x, y, patchedTimeToShow.value);
    }
    else
    {
        CALL_12(void, __fastcall, 0x4F6C00, text, type, x, y, items[0].defType, items[0].defFrame, items[1].defType,
                items[1].defFrame, -1, timeToShow, items[2].defType, items[2].defFrame);
    }

    int selection = o_WndMgr->result_dlg_item_id;
    selection = selection >= MSG_10_IT0 ? selection - MSG_10_IT0 : -1;
    if (result)
    {
        *result = selection;
    }
    return selection;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int F_MsgBox_ItemExists(_Dlg_ *dlg, int itemId)
{
    int bitAccess = dlg->GetItem(ERA_SPEC_DLG_ITEM_ID)->field_28;

    if (bitAccess == -1 || bitAccess & (1 << (itemId - MSG_10_IT0)))
    {
        return dlg->GetItem(itemId) != nullptr;
    }

    return 0;
}

// общая функция обновления всех желтых рамок диалога
int F_MsgBox_ResetYellowFrames(_Dlg_ *dlg, int itemID)
{
    _DlgItem_ *it;

    for (int i = MSG_10_IT0; i <= MSG_10_IT7; i++)
    {
        if (it = dlg->GetItem(i))
        {
            it->SendCommand(6, 4);
            it = NULL;
        }
    }
    it = dlg->GetItem(itemID);
    if (it)
        it->SendCommand(5, 4);

    return 2;
}

// чтобы не дублировать код, пишем универсальную функцию выхода из MsgBox_Proc
int F_MsgBox_Return(_EventMsg_ *msg, int itemID)
{
    msg->type = 0x200;
    msg->subtype = 10;
    msg->item_id = 10;
    o_WndMgr->result_dlg_item_id = itemID;
    o_TimeClick = 0;
    my_TimeClick_MsgBox = 0;
    my_TimeAnimate_MsgBox = 0;

    return 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// *** по неоднократной просьбе Berserker'a ***
// функция поиска следующего элемента от текущего активного
// при выборе клавишами влево(-1) или вправо(+1).
// поддерживает циклический (круговой) выбор элементов

int Y_New_MsgBox_GetNextItem(int way, int currentItem)
{
    _Dlg_ *dlg = P_Dlg_MsgBox;

    // получаем список (в битах) разрешённых элементов
    int bitAccess = dlg->GetItem(ERA_SPEC_DLG_ITEM_ID)->field_28;
    if (bitAccess == -1)
        bitAccess = 255;

    // получаем список (в битах) существующих элементов
    int bitExist = 0;
    for (int i = 0; i <= 7; i++)
    {
        _DlgItem_ *it = dlg->GetItem(i + MSG_10_IT0);

        if (it) // элемент существует - создаём его бит доступности
            bitExist |= (1 << i);
    }

    // прокрутка вправо (поиск следующего элемента)
    if (way == 1)
    {
        // отменяем круговую прокрутку, если есть 2 первых элемента
        // ибо она визуально запутывает игрока
        if (bitExist == 3 && currentItem == 1)
            return -1;
        if (bitExist == 3 && currentItem == -1)
            return 1;

        int nextItem = 0;
        if (currentItem != -1)
            nextItem = currentItem + 1;

        for (int i = 0; i <= 7; i++)
        {
            int temp = i + nextItem;
            if (temp > 7)
                temp -= 8;

            if (bitExist >> temp & 1)
                if (bitAccess >> temp & 1)
                    return temp;
        }
    }

    // прокрутка влево (поиск предыдущего элемента)
    if (way == -1)
    {
        // отменяем круговую прокрутку, если есть 2 первых элемента
        // ибо она визуально запутывает игрока
        if (bitExist == 3 && currentItem == 0)
            return -1;
        if (bitExist == 3 && currentItem == -1)
            return 0;

        int nextItem = 0;
        if (currentItem != -1)
            nextItem = currentItem - 1 + 1;

        for (int i = 7; i >= 0; i--)
        {
            int temp = i + nextItem;
            if (temp > 7)
                temp -= 8;

            if (bitExist >> temp & 1)
                if (bitAccess >> temp & 1)
                    return temp;
        }
    }

    // элемент не найден
    return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall Y_New_MsgBox_Call(HiHook *h, const char *Mes, int MType, int PosX, int PosY, int Type1, int SType1,
                                 int Type2, int SType2, int Par, int Time2Show, int Type3, int SType3)
{

    if (MType == EBottomViewType::CUSTOM_MSG_BOX)
    {
        auto &data = adventureInfoDefPanel;
        int timeToShow = 5000; // default timer value
        if (Mes)
            data.text.Set(Mes);
        if (Type3 > 0)
            timeToShow = Type3;
        if (Time2Show > 0)
            timeToShow = Time2Show;

        ItemInfo assetInfos[2] = {{Type1, SType1}, {Type2, SType2}};
        data.itemCount = 0;
        for (size_t i = 0; i < 2; i++)
        {
            const int defType = assetInfos[i].defType;
            if (data.dlg8ItemInfos[data.itemCount].InitCustomAsset(assetInfos[i]) ||
                defType >= 0 && defType <= MAXIMUM_DEFAULT_PIC_TYPE)
                data.itemCount++;
        }

        _AdvMgrSave_ *mgr = reinterpret_cast<_AdvMgrSave_ *>(o_AdvMgr);
        mgr->infoPanelToCreate.type = EBottomViewType::CUSTOM_MSG_BOX;
        mgr->infoPanelToCreate.endTime = o_GetTime() + timeToShow;
        if (Par > 0 || SType3 > 0)
        {
            mgr->RedrawInfoPanel(1);
        }

        return;
    }

    return CALL_12(void, __fastcall, h->GetDefaultFunc(), Mes, MType, PosX, PosY, Type1, SType1, Type2, SType2, Par,
                   Time2Show, Type3, SType3);
}

// Callback диалога MsgBox (ПКМ обрабатывается в другом месте)
signed int __stdcall Y_New_MsgBox_Proc(HiHook *hook, _EventMsg_ *msg)
{
    if (o_TimeClick)
    {
        int time = o_GetTime() - o_TimeClick;

        if (time >= 0)
            return F_MsgBox_Return(msg, 9999);
    }

    // получаем структуру диалога
    _Dlg_ *dlg = P_Dlg_MsgBox;

    // анимируем спрайты, если заказано
    Dlg8Info *dlg8Info = dlg8Manager.currentDlg8Info;
    if (dlg8Info && dlg8Info->hasAnimation)
    {
        int time = o_GetTime();
        if (time - my_TimeAnimate_MsgBox > 100)
        {
            const int itemsToAnimate = dlg8Info->usedSize;
            BOOL redraw = FALSE;
            for (size_t i = 0; i < itemsToAnimate; i++)
            {
                auto &info = dlg8Info->dlg8ItemInfos[i];
                if (info.itemInfo.defFrame != -1 || info.defPtr == nullptr)
                    continue;

                _DlgStaticDef_ *it = info.defPtr;
                const int framesCount = it->def->groups[0]->frames_count;
                if (framesCount < 2)
                    continue;

                const int defFrameIndex = it->def_frame_index;
                const int lastFrameIndex = it->def->groups[0]->frames_count - 1;
                it->SetFrame(defFrameIndex < lastFrameIndex ? defFrameIndex + 1 : 0);
                redraw = TRUE;
            }

            if (redraw)
                dlg->Redraw();
            my_TimeAnimate_MsgBox = time;
        }
    }

    if (msg->type == MT_KEYDOWN)
    {
        // если тип сообщения: с выбором элементов
        if (b_MsgBox_Style_id == 7 || b_MsgBox_Style_id == 10)
        {
            // *** по неоднократной просьбе Berserker'a ***
            // прокрутка элементов списка по стрелкам влево/вправо
            switch (msg->subtype)
            {
            case HK_ARROW_LEFT:
            case HK_TAB:
            case HK_ARROW_RIGHT: {
                // получаем текущий выбранный элемент в диалоге
                int currentItem = b_MsgBox_Result_id;

                // если элемент есть, нужно передать его id в виде 0-7
                if (currentItem != -1)
                    currentItem -= MSG_10_IT0;

                // функция нахождения следующего существующего и доступного элемента
                const int way = msg->subtype != HK_ARROW_LEFT ? 1 : -1;
                int nextItem = Y_New_MsgBox_GetNextItem(way, currentItem);

                // если элемент найден - делаем его активным
                if (nextItem != -1)
                {
                    msg->type = MT_MOUSEBUTTON;
                    msg->subtype = MST_LBUTTONCLICK;
                    msg->item_id = nextItem + MSG_10_IT0;
                }
                break;
            }
            default:
                break;
            }
            // *** по просьбе Bes'a ***
            // выбор элементов клавишами 1 или 2 (при двух картинках)
            // UPD: *** по желанию daemon_n'a ***
            // добавлена поддержка выбора клавишами 1-8 (при 8 картинках)
            if (msg->subtype >= HK_1 && msg->subtype <= HK_8)
            {
                const int itemIdToCheck = msg->subtype - 2 + MSG_10_IT0;
                if (F_MsgBox_ItemExists(dlg, itemIdToCheck))
                {
                    b_MsgBox_Result_id = itemIdToCheck;
                    F_MsgBox_ResetYellowFrames(dlg, itemIdToCheck);
                    dlg->GetItem(MSG_1_OK)->SetEnabled(1);
                    dlg->Redraw(1);
                }
            }
        }
    }

    int result = 1;

    if (msg->type == MT_MOUSEBUTTON)
    {
        if (msg->subtype == MST_LBUTTONCLICK)
        {
            int temp = 0;

            switch (msg->item_id)
            {
            case MSG_10_OK:
            case MSG_10_CANCEL:
            case MSG_2_OK:
            case MSG_2_CANCEL:
                result = F_MsgBox_Return(msg, msg->item_id);
                break;

            case MSG_1_OK:
                if (b_MsgBox_Style_id == 7 || b_MsgBox_Style_id == 10)
                    temp = b_MsgBox_Result_id;
                else
                    temp = msg->item_id;

                result = F_MsgBox_Return(msg, temp);
                break;

            case MSG_10_IT0:
            case MSG_10_IT1:
            case MSG_10_IT2:
            case MSG_10_IT3:
            case MSG_10_IT4:
            case MSG_10_IT5:
            case MSG_10_IT6:
            case MSG_10_IT7:
                if (b_MsgBox_Style_id == 7 || b_MsgBox_Style_id == 10)
                {
                    // проверяем маску ERA: разрешён ли элемент для выбора
                    int bitAccess = dlg->GetItem(ERA_SPEC_DLG_ITEM_ID)->field_28;

                    if (bitAccess >> (msg->item_id - MSG_10_IT0) & 1)
                    {
                        b_MsgBox_Result_id = msg->item_id;
                        F_MsgBox_ResetYellowFrames(dlg, msg->item_id);
                        dlg->GetItem(MSG_1_OK)->SetEnabled(1);
                        dlg->Redraw(1);
                    }
                }
                break;

            default:
                break;
            }
        }
        if (msg->subtype == MST_LBUTTONDOWN) // реализация даббл.клика
        {
            if (msg->item_id >= MSG_10_IT0 && msg->item_id <= MSG_10_IT7)
            {
                if (b_MsgBox_Style_id == 7 || b_MsgBox_Style_id == 10)
                {
                    if ((o_GetTime() - my_TimeClick_MsgBox) < 300 && msg->item_id == b_MsgBox_Result_id)
                    {
                        e_ClickSound();
                        result = F_MsgBox_Return(msg, msg->item_id);
                    }
                    else
                        my_TimeClick_MsgBox = o_GetTime();
                }
            }
        }
    }

    // оригинальную функцию НЕ вызываем
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef int(__stdcall *TGetDefaultMsgBoxItId)();
typedef int(__stdcall *TGetMaskMsgBoxItId)();
typedef int(__stdcall *TSetMaskMsgBoxItId)(int);
TGetDefaultMsgBoxItId GetDefaultMsgBoxItId = NULL;
TGetMaskMsgBoxItId GetMaskMsgBoxItId = NULL;
TSetMaskMsgBoxItId SetMaskMsgBoxItId = NULL;
// установка жёлтой рамки (создание активного элемента по умолчанию для ERA 3)
int __stdcall Y_New_MsgBox_SetDefaultFrameEnabled(LoHook *h, HookContext *c)
{
    _Dlg_ *dlg = P_Dlg_MsgBox;

    // дефолтная маска: читаем маску, которую передала нам ERA
    int bitAccess = GetMaskMsgBoxItId();
    dlg->GetItem(ERA_SPEC_DLG_ITEM_ID)->field_28 = bitAccess;

    if (b_MsgBox_Style_id == 7 || b_MsgBox_Style_id == 10)
    {
        if (ERA_VERSION < 3009)
            return EXEC_DEFAULT;

        // получаем дефолтный активный элемент
        int itemID = GetDefaultMsgBoxItId();

        if (itemID >= 0 && itemID <= 7)
        {
            // проверяем дефолтный элемент на разрешенную маску
            if (bitAccess >> itemID & 1)
            {
                // создаём правильный id элемента
                itemID += MSG_10_IT0;

                // получаем структуры акт.элемента и кнопки ОК
                _DlgItem_ *it = P_Dlg_MsgBox->GetItem(itemID);

                if (it)
                {
                    // подсвечиваем акт.элемент
                    it->SendCommand(5, 4);

                    // заносим результат в результирующую глобальную переменную
                    b_MsgBox_Result_id = itemID;
                    _DlgItem_ *ok = P_Dlg_MsgBox->GetItem(MSG_1_OK);
                    if (ok) // включаем кнопку ОК
                        ok->SetEnabled(1);
                }
            }
        }
    }
    // выполнить затёртый хуком код
    return EXEC_DEFAULT;
}

// для ERA 3: создание специального элемента диалога,
// который будет хранить битовую маски "разрешенных к выбору элементов"
signed int __stdcall Y_New_MsgBox_GetBitMask(HiHook *hook, _GameMgr_ *gm)
{
    _Dlg_ *dlg = P_Dlg_MsgBox;
    dlg->AddItem(_DlgItem_::Create(0, 0, 0, 0, ERA_SPEC_DLG_ITEM_ID, 1));
    SetMaskMsgBoxItId((1 << dlg8Manager.itemsParsedPerDlg8) - 1);

    return CALL_1(signed int, __thiscall, hook->GetDefaultFunc(), gm);
}

_LHF_(H3Dlg8_H3DlgDef_RightClick)
{
    if (const auto dlg8Info = dlg8Manager.currentDlg8Info)
    {
        const auto item = reinterpret_cast<_DlgItem_ *>(c->ecx);
        const int index = item->id - MSG_10_IT0;
        const auto &hint = dlg8Info->dlg8ItemTextInfos[index].rmcHint;
        if (!hint.Empty())
        {
            b_MsgBox(hint.c_str, 4);
            // jump over original code
            c->return_address = 0x4F15C0;
            return NO_EXEC_DEFAULT;
        }
    }
    return EXEC_DEFAULT;
}
// @daemon_n
// Исправление отображения кадров иконок с моралью и удачей в месседжбоксе
void __stdcall _Dlg8Item_Parser(HiHook *h, _Dlg8Item_ *dlg8Item, int picType, const int picSubtype)
{

    // получить тип картинки
    INT storedPicSubtype = picSubtype;
    const BOOL isMoraleOrLuck = (picType == 11 || picType == 14 || picType == 13 || picType == 16);

    // if we have text it is changed unconditionally
    auto &info = dlg8Manager.dlg8InfoBuffer.dlg8ItemTextInfos[dlg8Manager.itemsParsedPerDlg8];
    if (!info.textBelow.Empty())
        dlg8Item->textBelow.Set(info.textBelow.c_str);
    else if (isMoraleOrLuck)
    {
        int value = picSubtype ? picSubtype : 1;
        char buffer[16];
        char *format =
            picType == 11 || picType == 14 ? "%+d" : "%d"; // для положительных и отрицательных значений удачи и морали
        sprintf(buffer, format, value);
        dlg8Item->textBelow.Set(buffer);
    }

    CALL_3(void, __thiscall, h->GetDefaultFunc(), dlg8Item, picType, picSubtype);
    info;
    if (isMoraleOrLuck)
    {
        // если +/- удача или мораль и значение выше 1
        const int absSubtype = abs(storedPicSubtype);
        if (absSubtype > 1 && absSubtype < 4)
            dlg8Item->spriteFrameIndex = 3 + storedPicSubtype;
    }
    dlg8Manager.itemsParsedPerDlg8++;
}

/**
*
  1. Тип элемента может хранить имя def, тогда:
    - если подтип `>=0`, то интерпретируется как номер кадра;
    - если подтип `==-1`, то трактуется как "анимировать";
  2. Тип элемента может хранить имя pcx (только если имя валидно), тогда вместо DlgDef создаётся иной элемент:
    - если подтип `==0`, то создаётся DlgPcx8;
    - если подтип `==16`, то создаётся DlgPcx16;
    - если подтип интерпретируется в строку, то безусловно загружается вызывается `Era::LoadImageAsPcx16`;
    */
_LHF_(H3Dlg8Item_CompareItemTypeSetText)
{
    _Dlg8Item_ *dlg8Item = reinterpret_cast<_Dlg8Item_ *>(c->ebx);
    auto &info = dlg8Manager.nextDlg8ItemsInfos[dlg8Manager.itemsParsedPerDlg8];

    return EXEC_DEFAULT;
}
_LHF_(H3Dlg8Item_CompareItemTypeInsideParser)
{

    _Dlg8Item_ *dlg8Item = reinterpret_cast<_Dlg8Item_ *>(c->ebx);
    const int picType = c->edi;
    const int picSubtype = c->eax;
    if (dlg8Manager.nextDlg8ItemsInfos == nullptr)
        dlg8Manager.nextDlg8ItemsInfos = dlg8Manager.dlg8InfoBuffer.dlg8ItemInfos;

    auto &info = dlg8Manager.nextDlg8ItemsInfos[dlg8Manager.itemsParsedPerDlg8];
    if (!info.assetIsValid && picType <= MAXIMUM_DEFAULT_PIC_TYPE)
        return EXEC_DEFAULT;

    // if prepared info is not valid, we try to get asset name from picType
    if (!info.assetIsValid && picType > MINIMUM_ADDRESS_TO_READ)
    {
        ItemInfo itemInfo;
        itemInfo.defType = picType;
        itemInfo.defFrame = picSubtype;

        if (!info.InitCustomAsset(itemInfo))
        // if we can't init custom asset, we set picType to -1 to avoid crash
        {

            dlg8Item->picType = -1;
            dlg8Item->picSubType = 0;
            c->return_address = 0x04F6388;
            return NO_EXEC_DEFAULT;
        }
    }

    if (info.assetIsValid)
    {

        dlg8Manager.bufferInUse = TRUE;

        char *buf = info.assetName.c_str;
        //  store asset name inside dlg8item
        dlg8Item->spriteName.Set(buf);
        dlg8Item->picType = DWORD(buf);

        dlg8Item->picSubType = 0;
        dlg8Item->spriteFrameIndex = 0;

        if (info.assetType & eAssetType::ASSET_TYPE_ANY_PCX)
        {

            // assume we have address of pcx file in picSubtype, if it is > MINIMUM_ADDRESS_TO_READ

            if (!info.externalPicturePath.Empty())
                Era::LoadImageAsPcx16(info.externalPicturePath.c_str, buf, 0, 0, 0, 0, Era::RESIZE_ALG_NO_RESIZE);

            // else if (picSubtype > MINIMUM_ADDRESS_TO_READ)
            //{
            //     const std::string pathToExternalFile = GetStringFromInt(picSubtype, TRUE);
            //     info.assetType = eAssetType::ASSET_TYPE_PCX16;
            // }

            _Pcx_ *pcx = o_LoadPcx8(buf);
            dlg8Item->spriteWidth = pcx->width + 2;
            dlg8Item->spriteHeight = pcx->height + 2;
            // deref or destruct pcx to avoid memory leak
            pcx->DerefOrDestruct();

            // jump over def-deref
            c->return_address = 0x04F6255;
        }
        else
        {
            const int defFrame = info.itemInfo.defFrame;
            if (defFrame == -1)
            {
                dlg8Manager.dlg8InfoBuffer.hasAnimation = TRUE;
            }
            else
            {
                dlg8Item->picSubType = defFrame;
                dlg8Item->spriteFrameIndex = defFrame;
            }
            c->return_address = 0x4F6229;
        }
    }
    else
    {
        // if we can't init custom asset, we set picType to -1 to avoid crash
        dlg8Item->picType = -1;
        dlg8Item->picSubType = 0;
        c->return_address = 0x04F6388;
    }

    return NO_EXEC_DEFAULT;
}

// if dlg8ItemInfo->createDlgPcx is TRUE, then create a new DlgStaticPcx8 or DlgStaticPcx16
// and return it at address after original DlgStaticDef constructor
_LHF_(H3Dlg8_H3DlgDef_BeforeCtor)
{
    const int index = IntAt(c->ebp - 0x14);
    auto dlg8ItemInfos = dlg8Manager.nextDlg8ItemsInfos;

    if (!dlg8ItemInfos || !(dlg8ItemInfos[index].assetType & eAssetType::ASSET_TYPE_ANY_PCX))
        return EXEC_DEFAULT;

    auto &dlg8ItemInfo = dlg8ItemInfos[index];

    ByteAt(c->ebp - 0x4) = 0x13;
    c->esp += 4; // remove prev allocation size from stack

    _Dlg8Item_ *dlg8Item = reinterpret_cast<_Dlg8Item_ *>(c->esi - 0x1C);

    _DlgItem_ *dlgPcx = nullptr;
    if (dlg8ItemInfo.assetType == eAssetType::ASSET_TYPE_PCX16)
        dlgPcx =
            b_DlgStaticPcx16_Create(dlg8Item->spritePos.x + 1, dlg8Item->spritePos.y + 1, dlg8Item->spriteWidth - 2,
                                    dlg8Item->spriteHeight - 2, -1, dlg8Item->spriteName.c_str, 2048);
    else
        dlgPcx = b_DlgStaticPcx8_Create(dlg8Item->spritePos.x + 1, dlg8Item->spritePos.y + 1, dlg8Item->spriteWidth - 2,
                                        dlg8Item->spriteHeight - 2, -1, dlg8Item->spriteName.c_str, 2048);
    IntAt(c->ebp - 0x5C) = (int)dlgPcx;
    c->eax = (int)dlgPcx;
    c->return_address = 0x4F783D;
    return NO_EXEC_DEFAULT;
}

// else we store DlgStaticDef pointer to allow animation of it
_LHF_(H3Dlg8_H3DlgDef_Ctor)
{
    if (dlg8Manager.bufferInUse)
    {
        const int index = IntAt(c->ebp - 0x14);
        dlg8Manager.dlg8InfoBuffer.dlg8ItemInfos[index].defPtr = reinterpret_cast<_DlgStaticDef_ *>(c->ecx);
    }

    return EXEC_DEFAULT;
}

_LHF_(H3Dlg8_RightBeforeShow)
{

    auto &vec = dlg8Manager.dlg8ItemInfosVector;
    vec.push_back(nullptr);

    if (dlg8Manager.bufferInUse) // this var is cleared in the method below
    {
        const size_t itemsCreated = dlg8Manager.itemsParsedPerDlg8;

        auto &buffer = dlg8Manager.dlg8InfoBuffer;

        // clear not used items

        buffer.usedSize = itemsCreated;
        for (size_t i = itemsCreated; i < Dlg8ItemInfo::MAX_SIZE; i++)
            buffer.dlg8ItemInfos[i] = {};

        vec.back() = std::make_unique<Dlg8Info>(buffer);
        buffer.Clear();
    }

    dlg8Manager.ClearNextDlgPreparation();

    dlg8Manager.currentDlg8Info = vec.back().get();
    return EXEC_DEFAULT;
}
_LHF_(H3Dlg8_RightAfterClose)
{
    auto &vec = dlg8Manager.dlg8ItemInfosVector;
    size_t size = vec.size();
    if (size--)
        vec.pop_back();

    dlg8Manager.currentDlg8Info = size ? vec.back().get() : nullptr;

    // if last dialog closed, clear data
    if (c->ecx == 0 && size)
        vec.clear();

    return EXEC_DEFAULT;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static _ptr_ AdventureInfoPanel_Build(_ptr_ panel, _Dlg_ *parent, AdventureInfoDefPanelData &panelData)
{

    CALL_1(_ptr_, __thiscall, 0x5AA650, panel); // panel ctor

    *reinterpret_cast<_ptr_ *>(panel) = 0x63BB04;
    auto &rect = panelData.position;

    const int panelWidth = rect.right;
    const int panelHeight = rect.bottom;

    CALL_6(void, __thiscall, 0x5AA780, panel, rect.left, rect.top, panelWidth, panelHeight, parent);
    *reinterpret_cast<_ptr_ *>(panel) = 0x63BB1C;

    auto *background =
        b_DlgStaticPcx8_Create(0, 0, panelWidth, panelHeight, 2000, const_cast<char *>("AdStatOt.pcx"), 2048);
    if (background)
        CALL_3(void, __thiscall, 0x5AA7B0, panel, background, -1);

    int textHeight = 0;

    const int maxTextHeight = panelHeight - 20;
    const int maxTextWidth = panelWidth - 20;
    // init items positions and sizes
    if (!panelData.text.Empty())
    {
        auto font = o_Smalfont_Fnt;
        const int linesCount = font->GetLinesCountInText(panelData.text.c_str, maxTextWidth);
        textHeight = Clamp(0, linesCount * font->height, maxTextHeight);
    }

    int totalHeight = panelHeight;
    int availableForAssetsHeight = panelHeight - textHeight - 20;
    _Dlg8Item_ dlg8Items[2];

    const int itemCount = panelData.itemCount;

    int maxAssetHeight = 0;
    int maxAssetTextHeight = 0;
    int itemsCount = 0;
    dlg8Manager.ClearNextDlgPreparation();
    dlg8Manager.nextDlg8ItemsInfos = panelData.dlg8ItemInfos;

    for (int i = 0; i < itemCount; ++i)
    {
        auto &dlg8ItemInfo = panelData.dlg8ItemInfos[i];

        if (!dlg8ItemInfo.assetIsValid && dlg8ItemInfo.itemInfo.defType > MAXIMUM_DEFAULT_PIC_TYPE)
            continue;
        auto &dlg8Item = dlg8Items[i];
        dlg8Item.spritePos = {};
        dlg8Item.textPos = {};

        CALL_3(void, __thiscall, 0x4F5540, &dlg8Item, dlg8ItemInfo.itemInfo.defType, dlg8ItemInfo.itemInfo.defFrame);
        if (!dlg8Item.spriteName.Empty())
        {
            if (dlg8Item.spriteHeight > maxAssetHeight)
                maxAssetHeight = dlg8Item.spriteHeight;
            if (dlg8Item.textHeight > maxAssetTextHeight)
                maxAssetTextHeight = dlg8Item.textHeight;
            itemsCount++;
        }
        if (!dlg8Item.textBelow.Empty())
        {
            if (dlg8Item.textHeight > maxAssetTextHeight)
                maxAssetTextHeight = dlg8Item.textHeight;
            if (dlg8Item.spriteHeight > 0)
            {
                dlg8Item.textPos.y = dlg8Item.spritePos.y + dlg8Item.spriteHeight + 2;
            }
        }
    }
    dlg8Manager.ClearNextDlgPreparation();

    // now create items and add them to panel
    //  int hightLeft =
    if (textHeight)
    {
        int textY = (totalHeight - textHeight - 10) >> 1;
        auto *textItem = b_DlgStaticText_Create(10, textY, panelWidth - 20, textHeight, panelData.text.c_str,
                                                o_Smalfont_Fnt->name, 1, 2100, 1, 0, 8);
        if (textItem)
            CALL_3(void, __thiscall, 0x5AA7B0, panel, textItem, -1);
    }

    for (size_t i = 0; i < itemCount; i++)
    {

        auto &dlg8ItemInfo = panelData.dlg8ItemInfos[i];
        if (!dlg8ItemInfo.assetIsValid && dlg8ItemInfo.itemInfo.defType > MAXIMUM_DEFAULT_PIC_TYPE)
            continue;
        _Dlg8Item_ &dlg8Item = dlg8Items[i];
        _DlgItem_ *item = nullptr;
        int itemId = 2103 + i;

        const eAssetType assetType =
            dlg8ItemInfo.itemInfo.defType > MAXIMUM_DEFAULT_PIC_TYPE ? dlg8ItemInfo.assetType : ASSET_TYPE_DEF;

        int itemX = dlg8Item.spritePos.x + 1 + i * 64;
        int itemY = dlg8Item.spritePos.y + 1;
        int itemWidth = dlg8Item.spriteWidth - 2;
        int itemHeight = dlg8Item.spriteHeight - 2;
        switch (assetType)
        {
        case eAssetType::ASSET_TYPE_PCX:
            item = b_DlgStaticPcx8_Create(itemX, itemY, itemWidth, itemHeight, itemId, dlg8Item.spriteName.c_str, 2048);
            break;

        case eAssetType::ASSET_TYPE_PCX16:
            item =
                b_DlgStaticPcx16_Create(itemX, itemY, itemWidth, itemHeight, itemId, dlg8Item.spriteName.c_str, 2048);
            break;
        case eAssetType::ASSET_TYPE_DEF:
        case eAssetType::ASSET_TYPE_DEF_ANIMATED:
            item = b_DlgStaticDef_Create(itemX, itemY, itemWidth, itemHeight, itemId, dlg8Item.spriteName.c_str,
                                         dlg8Item.spriteFrameIndex, 0, 0, 0, 16);
            break;
        default:
            break;
        }

        if (item)
            CALL_3(void, __thiscall, 0x5AA7B0, panel, item, -1);

        itemId = 2105 + i;
        if (dlg8Item.textHeight > 0)
        {
            item = b_DlgStaticText_Create(itemX, itemY + dlg8Item.spriteHeight + 2, itemWidth, dlg8Item.textHeight,
                                          dlg8Item.textBelow.c_str, o_Smalfont_Fnt->name, 1, itemId, 1, 0, 8);
            if (item)
                CALL_3(void, __thiscall, 0x5AA7B0, panel, item, -1);
        }
    }

    return panel;
}

static bool AdventureInfoPanel_Draw(_AdvMgrSave_ *advMgr, BOOL rebuildPanel)
{
    if (!rebuildPanel && advMgr->currentInfoPanelToDraw == EBottomViewType::CUSTOM_MSG_BOX)
        return 0;

    _Dlg_ *dlg = advMgr->dlg;                // *reinterpret_cast<_Dlg_**>(reinterpret_cast<char*>(advMgr) + 68);
    CALL_1(void, __thiscall, 0x403EE0, dlg); // H3AdventureMgrDlg::ClearInfoPanel
    advMgr->currentInfoPanelToDraw = EBottomViewType::CUSTOM_MSG_BOX;

    _ptr_ panel = o_New(0x34);
    if (panel)
        AdventureInfoPanel_Build(panel, dlg, adventureInfoDefPanel);
    CALL_2(void, __thiscall, 0x402C10, dlg, panel);
    return 1;
}

_LHF_(AdventureInfoPanel_Type11)
{
    auto *advMgr = reinterpret_cast<_AdvMgrSave_ *>(c->esi);
    const int panelType = advMgr->infoPanelToCreate.type;
    if (panelType != EBottomViewType::CUSTOM_MSG_BOX)
        return EXEC_DEFAULT;

    const BOOL rebuildPanel = IntAt(c->ebp + 8);

    Era::ExecErmCmd("IF:L^^");
    c->eax = AdventureInfoPanel_Draw(advMgr, rebuildPanel);
    c->return_address = 0x415E3C;
    return NO_EXEC_DEFAULT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Dlg_MsgBox(PatcherInstance *_PI)
{

    // новый конструктор диалога MsgBox
    _PI->WriteHiHook(0x4F6C00, SPLICE_, EXTENDED_, FASTCALL_, Y_New_MsgBox_Call);

    // новый Callback диалога MsgBox
    _PI->WriteHiHook(0x4F1650, SPLICE_, EXTENDED_, THISCALL_, Y_New_MsgBox_Proc);

    // установка дефолтной желтой рамки (как буд-то она уже выбранна)
    _PI->WriteLoHook(0x4F7B46, Y_New_MsgBox_SetDefaultFrameEnabled);

    // создание элемента для хранения "разрешенных к выбору элементов"
    _PI->WriteHiHook(0x4F71BB, CALL_, EXTENDED_, THISCALL_, Y_New_MsgBox_GetBitMask);

    // правильное смещение для жёлтых рамок
    _PI->WriteByte(0x4F7985 + 2, 1); // увеличение ширины
    _PI->WriteByte(0x4F7988 + 2, 1); // увеличение высоты

    // увеличение высоты скролл текста
    if (o_HD_Y >= 664)
        _PI->WriteDword(0x4F662F + 1, o_HD_Y - 440);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // добавление отображения любого ассета в диалогах с 8 картинками (Dlg8/ он же MessageBox)

    _PI->WriteLoHook(0x4F11E7, H3Dlg8_H3DlgDef_RightClick);
    // Исправление отображения лишь 1 удачи и морали в иконках (@daemon_n)
    // основной парсер картинок
    _PI->WriteHiHook(0x4F5540, SPLICE_, EXTENDED_, THISCALL_, _Dlg8Item_Parser);
    // внутри парсера надо изменить def, чтобы ф-ция сама посчитала размеры и текст
    _PI->WriteLoHook(0x4F558D, H3Dlg8Item_CompareItemTypeInsideParser);
    _PI->WriteLoHook(0x4F625A, H3Dlg8Item_CompareItemTypeSetText);

    _PI->WriteLoHook(0x4F77F8, H3Dlg8_H3DlgDef_BeforeCtor);
    _PI->WriteLoHook(0x4F7838, H3Dlg8_H3DlgDef_Ctor);
    _PI->WriteLoHook(0x4F7B5E, H3Dlg8_RightBeforeShow);
    _PI->WriteLoHook(0x4F7BD5, H3Dlg8_RightAfterClose);

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Adventure-map info panel extension: type 11 (EBottomViewType::CUSTOM_MSG_BOX) draws two caller-supplied DEFs.
    _PI->WriteLoHook(0x415D6E, AdventureInfoPanel_Type11);
    RECT &rect = adventureInfoDefPanel.position;
    rect.left = IntAt(0x450F35 + 1);
    rect.top = IntAt(0x450F30 + 1);
    rect.right = IntAt(0x450F2B + 1);
    rect.bottom = IntAt(0x450F26 + 1);

    if (HINSTANCE hEra = GetModuleHandleA("era.dll"))
    {
        GetDefaultMsgBoxItId = (TGetDefaultMsgBoxItId)GetProcAddress(hEra, "_GetPreselectedDialog8ItemId");
        GetMaskMsgBoxItId = (TGetMaskMsgBoxItId)GetProcAddress(hEra, "_GetDialog8SelectablePicsMask");
        SetMaskMsgBoxItId = (TSetMaskMsgBoxItId)GetProcAddress(hEra, "_SetDialog8SelectablePicsMask");
    }
}
