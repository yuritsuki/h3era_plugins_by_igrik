////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Callback диалога MsgBox
/////////////////////////////////////////////////////////

static bool IsPcxResourceName(const char *name)
{
    if (!name)
        return false;

    const std::string normalized = name;
    return normalized.size() >= 4 && normalized.compare(normalized.size() - 4, 4, ".pcx") == 0;
}

// Adventure-map info-panel extension.  Type 9 is intentionally kept here,
// together with the existing native-dialog helpers, so the panel lifetime is
// handled by the same game code as the stock resource panel.
struct AdventureInfoAsset
{
    int type;
    int frame;
    char *name = "AVArnd3.def";
};

struct AdventureInfoDefPanelData
{
    _HStr_ text;
    AdventureInfoAsset assetInfos[2];
    int timeToShow;
    RECT backPos;
};

static AdventureInfoDefPanelData g_adventureInfoDefPanel;

static void __fastcall AdventureInfoPanel_Dtor(_ptr_ panel)
{
    CALL_1(void, __thiscall, 0x451470, panel);
}
static void __fastcall AdventureInfoPanel_Update(_ptr_ panel)
{
    CALL_1(void, __thiscall, 0x451470, panel);
}

struct PanelInfoVTable
{
    DWORD _delete = 0x451440;
};
static void ShowAdventureInfoPanelMessageBox(AdventureInfoDefPanelData &data)
{
    _AdvMgrSave_ *mgr = reinterpret_cast<_AdvMgrSave_ *>(o_AdvMgr);
    if (!mgr)
        return;
    mgr->infoPanelToCreate.type = EBottomViewType::CUSTOM_MSG_BOX;
    if (!data.timeToShow)
        data.timeToShow = 5000; // default timer value

    mgr->infoPanelToCreate.endTime = o_GetTime() + data.timeToShow;
}
static _ptr_ AdventureInfoPanel_Build(_ptr_ panel, _Dlg_ *parent, AdventureInfoDefPanelData &panelData)
{

    CALL_1(_ptr_, __thiscall, 0x5AA650, panel); // panel ctor

    *(_ptr_ *)panel = (_ptr_)0x63BB04;
    auto &rect = panelData.backPos;
    CALL_6(void, __thiscall, 0x5AA780, panel, rect.left, rect.top, rect.right, rect.bottom, parent);
    *(_ptr_ *)panel = (_ptr_)0x63BB1C;

    auto *background = b_DlgStaticPcx8_Create(0, 0, 176, 166, 2000, const_cast<char *>("AdStatOt.pcx"), 2048);
    if (background)
        CALL_3(void, __thiscall, 0x5AA7B0, panel, background, -1);

    for (int i = 0; i < 2; ++i)
    {
        const auto &defInfo = panelData.assetInfos[i];
        if (!defInfo.name)
            continue;

        _Def_ *def = o_LoadDef(defInfo.name);
        if (!def)
            continue;
        const int x = (176 / 2) + (i ? 44 : -44) - static_cast<int>(def->width / 2);
        auto *item =
            b_DlgStaticDef_Create(x, 50, def->width, def->height, 2103 + i, defInfo.name, defInfo.frame, 0, 0, 0, 16);

        if (!item)
            continue;
        CALL_3(void, __thiscall, 0x5AA7B0, panel, item, -1);
    }

    return panel;
}

static bool AdventureInfoPanel_Draw(_AdvMgrSave_ *advMgr, BOOL rebuildPanel)
{
    if (!rebuildPanel && advMgr->currentInfoPanelToDraw == EBottomViewType::CUSTOM_MSG_BOX)
        return 0;

    _Dlg_ *dlg = advMgr->dlg; // *reinterpret_cast<_Dlg_**>(reinterpret_cast<char*>(advMgr) + 68);
    CALL_1(void, __thiscall, 0x403EE0, dlg);
    advMgr->currentInfoPanelToDraw = EBottomViewType::CUSTOM_MSG_BOX;

    _ptr_ panel = o_New(0x34);
    if (panel)
        AdventureInfoPanel_Build(panel, dlg, g_adventureInfoDefPanel);
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

extern "C" __declspec(dllexport) void __stdcall PrepareAdventureInfoPanelDefs(char **defNames, int *frames)
{
}

// по большей части тупо переписан оригинальный код функции 0x4F1650,
// но в этом коде расширено кол-во кликабельных желтых рамок.
// В оригинале было всего 2 кликабельные рамки: 30729 и 30730.
// Теперь же работают все рамки: 30729-30736
// И в довесок реализован даббл.клик выбора
// UPD: добавлена возможность ототображать любой деф и текст экспортными def
// #define DllImport extern "C" __declspec(dllimport)
#define DllExport extern "C" __declspec(dllexport)
#define ERA_SPEC_DLG_ITEM_ID 1525
#define P_Dlg_MsgBox (*(_Dlg_ **)0x6995E0)

#define MSG_10_OK 30720
#define MSG_10_CANCEL 30721

#define MSG_1_OK 30722
#define MSG_2_OK 30725
#define MSG_2_CANCEL 30726

#define MSG_10_IT0 30729
#define MSG_10_IT1 30730
#define MSG_10_IT2 30731
#define MSG_10_IT3 30732
#define MSG_10_IT4 30733
#define MSG_10_IT5 30734
#define MSG_10_IT6 30735
#define MSG_10_IT7 30736

//
#pragma comment(linker, "/EXPORT:PrepareDlgPictures=_PrepareDlgPictures@12")
#pragma comment(linker, "/EXPORT:PrepareDlgText=_PrepareDlgText@12")
#pragma comment(linker, "/EXPORT:ShowPreparedDlg=_ShowPreparedDlg@24")
#pragma comment(linker, "/EXPORT:PrepareAdventureInfoPanelDefs=_PrepareAdventureInfoPanelDefs@8")
_LHF_(test_Faerie);

int my_TimeClick_MsgBox;
int my_TimeAnimate_MsgBox;

struct ItemInfo
{
    union {
        const char *assetName;
        int defType = -1;
    };
    union {
        const char *hintPtr;
        int defFrame = 0;
    };
};

struct Dlg8ItemInfo
{
    static constexpr size_t MAX_SIZE = 8;

  public:
    BOOL isValid = FALSE;
    _HStr_ assetName;
    _HStr_ text;
    _HStr_ rmcHint;
    union {
        int defFrame;
        BOOL isPcx16 = FALSE;
    };
    union {
        BOOL createDlgPcx;
        _DlgStaticDef_ *defPtr = nullptr;
    };

  public:
    static ItemInfo externalCallItemInfo;
    static int itemsParsedPerDlg8;
};
int Dlg8ItemInfo::itemsParsedPerDlg8 = 0;
ItemInfo Dlg8ItemInfo::externalCallItemInfo;

struct Dlg8Info
{
  public:
    static constexpr size_t MAX_SIZE = Dlg8ItemInfo::MAX_SIZE;

  public:
    BOOL hasAnimation = FALSE;
    size_t usedSize = 0;
    Dlg8ItemInfo dlg8ItemInfos[MAX_SIZE];

  public:
    inline void Clear()
    {
        for (size_t i = 0; i < MAX_SIZE; i++)
            dlg8ItemInfos[i] = {};
    }
};

struct Dlg8Manager
{
    std::vector<std::unique_ptr<Dlg8Info>> dlg8ItemInfosVector;
    Dlg8Info dlg8InfoBuffer;
    BOOL bufferInUse = FALSE;
    Dlg8Info *currentDlg8Info = nullptr;
    Dlg8Info *nextDlg8Info = nullptr;

  public:
    inline void ClearPicturesBuffer()
    {
        for (size_t i = 0; i < Dlg8Info::MAX_SIZE; i++)
        {
            dlg8InfoBuffer.dlg8ItemInfos[i].assetName = {};
            dlg8InfoBuffer.dlg8ItemInfos[i].defFrame = {};
        }
    }
    inline void ClearTextBuffer()
    {
        for (size_t i = 0; i < Dlg8Info::MAX_SIZE; i++)
        {
            dlg8InfoBuffer.dlg8ItemInfos[i].text = {};
            dlg8InfoBuffer.dlg8ItemInfos[i].rmcHint = {};
        }
    }

} dlg8Manager;

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

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
        _DlgItem_ *it = dlg->GetItem(i);
        if (it)
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

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void __stdcall Y_New_MsgBox_Call(HiHook *h, const char *Mes, int MType, int PosX, int PosY, int Type1, int SType1,
                                 int Type2, int SType2, int Par, int Time2Show, int Type3, int SType3)
{

    if (MType == EBottomViewType::CUSTOM_MSG_BOX)
    {
        auto &data = g_adventureInfoDefPanel;
        data.timeToShow = 1000;
        if (Mes)
            data.text.Set(Mes);

        PrepareAdventureInfoPanelDefs(nullptr, nullptr);
        int itmesCount = 0;
        if (Type1 >= 0)
        {
            _Dlg8Item_ item;
            CALL_3(void, __thiscall, 0x4F5540, &item, Type1, SType1);
            itmesCount++;
        }
        if (Type2 >= 0)
        {
            _Dlg8Item_ item;
            CALL_3(void, __thiscall, 0x4F5540, &item, Type2, SType2);
            itmesCount++;
        }
        if (Type3 >= 0 && itmesCount < 2)
        {
            _Dlg8Item_ item;
            CALL_3(void, __thiscall, 0x4F5540, &item, Type3, SType3);
            itmesCount++;
        }

        // 0x4F5540
        ShowAdventureInfoPanelMessageBox(g_adventureInfoDefPanel);
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
                if (info.defFrame != -1 || info.defPtr == nullptr)
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

//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////
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
    SetMaskMsgBoxItId((1 << Dlg8ItemInfo::itemsParsedPerDlg8) - 1);

    return CALL_1(signed int, __thiscall, hook->GetDefaultFunc(), gm);
}

DllExport BOOL __stdcall PrepareDlgPictures(char **assetNames, int *frameIds, size_t length)
{

    auto &dlg8Info = dlg8Manager.dlg8InfoBuffer;
    dlg8Manager.ClearPicturesBuffer();

    auto &dlg8ItemInfos = dlg8Info.dlg8ItemInfos;

    length = Clamp(0, length, Dlg8ItemInfo::MAX_SIZE);
    int infoCounter = 0;
    for (size_t i = 0; i < length; i++)
    {
        if (char *assetName = assetNames[i])
        {
            auto &info = dlg8ItemInfos[infoCounter];
            info.assetName.Set(assetName); // ResolveAssetName(assetName);
            if (info.assetName.Empty())
            {
                info = {};
                continue;
            }

            if (IsPcxResourceName(assetName))
            {
                info.createDlgPcx = TRUE;
            }
            else
            {
                const int defFrame = frameIds[i];
                info.defFrame = defFrame;
                if (defFrame <= -1)
                    dlg8Info.hasAnimation = TRUE;
            }
            // only if name isn't changed , we can use the defPtr to animate the icon

            info.isValid = TRUE;
            infoCounter++;
        }
    }
    dlg8Manager.bufferInUse = TRUE;

    return infoCounter;
}

DllExport BOOL __stdcall PrepareDlgText(char **descriptions, char **rmcHints, size_t length)
{
    auto &dlg8Info = dlg8Manager.dlg8InfoBuffer;
    dlg8Manager.ClearTextBuffer();

    length = Clamp(0, length, Dlg8ItemInfo::MAX_SIZE);
    auto &dlg8ItemInfos = dlg8Info.dlg8ItemInfos;
    for (size_t i = 0; i < length; i++)
    {
        if (descriptions[i])
            dlg8ItemInfos[i].text.Set(descriptions[i]);
        if (rmcHints[i])
            dlg8ItemInfos[i].rmcHint.Set(rmcHints[i]);
    }
    dlg8Manager.bufferInUse = TRUE;

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

        if (info.isValid)
        {
            count++;
            item.assetName = info.assetName.c_str;
            item.defFrame = info.defFrame;
        }
        items.Append(item);
    }

    dlg8Manager.bufferInUse = TRUE;
    Dlg8ItemInfo::itemsParsedPerDlg8 = 0;
    dlg8Manager.nextDlg8Info = &dlg8Manager.dlg8InfoBuffer;

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
_LHF_(H3Dlg8_H3DlgDef_RightClick)
{
    if (const auto dlg8Info = dlg8Manager.currentDlg8Info)
    {
        const auto item = reinterpret_cast<_DlgItem_ *>(c->ecx);
        const int index = item->id - 30729;
        const auto &hint = dlg8Info->dlg8ItemInfos[index].rmcHint;
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
    if (isMoraleOrLuck)
    {
        int value = picSubtype ? picSubtype : 1;
        char buffer[16];
        char *format =
            picType == 11 || picType == 14 ? "%+d" : "%d"; // для положительных и отрицательных значений удачи и морали
        sprintf(buffer, format, value);
        dlg8Item->textBelow.Set(buffer);
    }

    CALL_3(void, __thiscall, h->GetDefaultFunc(), dlg8Item, picType, picSubtype);

    if (isMoraleOrLuck)
    {
        // если +/- удача или мораль и значение выше 1
        const int absSubtype = abs(storedPicSubtype);
        if (absSubtype > 1 && absSubtype < 4)
            dlg8Item->spriteFrameIndex = 3 + storedPicSubtype;
    }
}
BOOL IsDefOfPcx(std::string &assetName)
{
    if (assetName.empty())
        return FALSE;

    const size_t size = assetName.size();
    if (size < 5)
        return FALSE;
    std::string toLower = assetName;
    std::transform(toLower.begin(), toLower.end(), toLower.begin(), ::tolower);
    return toLower.compare(toLower.size() - 4, 4, ".pcx") == 0 || toLower.compare(toLower.size() - 4, 4, ".def") == 0;
}
std::string GetAssetNameFromType(const int picType)
{

    if (picType > 1000000000)
    {
        Era::y[1] = picType; // read string from erm
        Era::ExecErmCmd("SN:Bzy1/d/?z1");
        std::string result = Era::z[1];
        if (IsDefOfPcx(result))
            return result;
    }

    std::string result = reinterpret_cast<LPCSTR>(picType);

    if (IsDefOfPcx(result))
        return result;

    if (picType > 1000000000)
    {
        result = *reinterpret_cast<LPCSTR *>(picType);
        if (IsDefOfPcx(result))
            return result;
    }
    return {};
}
_LHF_(H3Dlg8Item_CompareItemTypeInsideParser)
{

    const int id = Dlg8ItemInfo::itemsParsedPerDlg8++;

    if (id == 0 && dlg8Manager.bufferInUse)
    {
        dlg8Manager.nextDlg8Info = &dlg8Manager.dlg8InfoBuffer;
    }
    auto dlg8Info = dlg8Manager.nextDlg8Info;
    _Dlg8Item_ *item = reinterpret_cast<_Dlg8Item_ *>(c->ebx);
    _HStr_ *defNameToSet = nullptr;

    Dlg8ItemInfo *dlg8ItemInfoCurrent = nullptr;

    BOOL assetNameChanged = FALSE;
    const int picType = c->edi;
    const int picSubtype = c->eax;

    if (dlg8Info)
    {
        auto &info = dlg8Info->dlg8ItemInfos[id];
        dlg8ItemInfoCurrent = &info;

        if (info.isValid)
        {
            defNameToSet = &info.assetName;
            item->spriteFrameIndex = info.defFrame >= 0 ? info.defFrame : 0;
            assetNameChanged = true;
        }

        if (!info.text.Empty())
            item->textBelow.Set(info.text.c_str);
    }

    if (!assetNameChanged && picType > 100000)
    {
        dlg8Manager.bufferInUse = TRUE;
        dlg8Info = dlg8Manager.nextDlg8Info = &dlg8Manager.dlg8InfoBuffer;
        
        auto &info = dlg8Info->dlg8ItemInfos[id];
        dlg8ItemInfoCurrent = &info;

        std::string &assetName = GetAssetNameFromType(picType);

        if (assetName.empty())
            return EXEC_DEFAULT;

        info.isValid = TRUE;
        info.assetName.Set(assetName.c_str());

        if (picSubtype < 0)
        {
            info.defFrame = -1;
            dlg8Info->hasAnimation = TRUE;
            item->spriteFrameIndex = 0;
        }
        else
            item->spriteFrameIndex = picSubtype;

        defNameToSet = &info.assetName;

        dlg8Manager.nextDlg8Info = &dlg8Manager.dlg8InfoBuffer;
    }

    if (defNameToSet)
    {
        char *buf = defNameToSet->c_str;

        // char newName[12] = "tefeff.pcx";
        // Era::LoadImageAsPcx16("text.png", newName, 0, 0, 0, 0, Era::RESIZE_ALG_NO_RESIZE);
        // buf = newName;
        // store asset name inside dlg8item
        item->spriteName.Set(buf);
        item->picType = DWORD(buf);

        // if not pcx allow return to default proc
        if (!IsPcxResourceName(buf))
        {
            c->return_address = 0x4F6229;
            return NO_EXEC_DEFAULT;
        }

        // sest flag to create DlgStaticPcx8 instead of DlgStaticDef
        dlg8ItemInfoCurrent->createDlgPcx = TRUE;

        _Pcx_ *pcx = o_LoadPcx8(buf);
        dlg8ItemInfoCurrent->isPcx16 = 1; // picSubtype > 0; // pcx->v_table == (_ptr_*)0x063B9C8;

        auto dlg8Item = reinterpret_cast<_Dlg8Item_ *>(c->ebx);
        dlg8Item->spriteWidth = pcx->width + 2;
        dlg8Item->spriteHeight = pcx->height + 2;

        pcx->DerefOrDestruct();
        // jump over def-deref
        c->return_address = 0x04F6255;
        return NO_EXEC_DEFAULT;
    }
    return EXEC_DEFAULT;
}

// if dlg8ItemInfo->createDlgPcx is TRUE, then create a new DlgStaticPcx8 or DlgStaticPcx16
// and return it at address after original DlgStaticDef constructor
_LHF_(H3Dlg8_H3DlgDef_BeforeCtor)
{
    const int index = IntAt(c->ebp - 0x14);
    auto dlg8Info = dlg8Manager.nextDlg8Info;

    if (!dlg8Info || !dlg8Info->dlg8ItemInfos[index].createDlgPcx)
        return EXEC_DEFAULT;

    auto &dlg8ItemInfo = dlg8Info->dlg8ItemInfos[index];

    ByteAt(c->ebp - 0x4) = 0x13;
    c->esp += 4; // remove prev allocation size from stack

    _Dlg8Item_ *item = reinterpret_cast<_Dlg8Item_ *>(c->esi - 0x1C);

    _DlgItem_ *dlgPcx = nullptr;
    if (dlg8ItemInfo.isPcx16)
        dlgPcx = b_DlgStaticPcx16_Create(item->spritePos.x + 1, item->spritePos.y + 1, item->spriteWidth - 2,
                                         item->spriteHeight - 2, -1, item->spriteName.c_str, 2048);
    else
        dlgPcx = b_DlgStaticPcx8_Create(item->spritePos.x + 1, item->spritePos.y + 1, item->spriteWidth - 2,
                                        item->spriteHeight - 2, -1, item->spriteName.c_str, 2048);

    dlg8ItemInfo.createDlgPcx = FALSE;
    IntAt(c->ebp - 0x5C) = (int)dlgPcx;
    c->eax = (int)dlgPcx;
    c->return_address = 0x4F783D;
    return NO_EXEC_DEFAULT;
}

// else we store DlgStaticDef pointer to allow animation of it
_LHF_(H3Dlg8_H3DlgDef_Ctor)
{
    const int index = IntAt(c->ebp - 0x14);
    dlg8Manager.dlg8InfoBuffer.dlg8ItemInfos[index].defPtr = reinterpret_cast<_DlgStaticDef_ *>(c->ecx);
    return EXEC_DEFAULT;
}

_LHF_(H3Dlg8_RightBeforeShow)
{

    auto &vec = dlg8Manager.dlg8ItemInfosVector;
    vec.push_back(nullptr);

    if (dlg8Manager.bufferInUse)
    {
        dlg8Manager.bufferInUse = FALSE;

        const size_t itemsCreated = Dlg8ItemInfo::itemsParsedPerDlg8;

        auto &buffer = dlg8Manager.dlg8InfoBuffer;

        // clear not used items

        buffer.usedSize = itemsCreated;
        for (size_t i = itemsCreated; i < Dlg8ItemInfo::MAX_SIZE; i++)
            buffer.dlg8ItemInfos[i] = {};

        vec.back() = std::make_unique<Dlg8Info>(buffer);
        buffer.Clear();
    }

    Dlg8ItemInfo::itemsParsedPerDlg8 = 0;

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
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

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
    _PI->WriteLoHook(0x4F11E7, H3Dlg8_H3DlgDef_RightClick);
    // Исправление отображения лишь 1 удачи и морали в иконках (@daemon_n)
    // основной парсер картинок
    _PI->WriteHiHook(0x4F5540, SPLICE_, EXTENDED_, THISCALL_, _Dlg8Item_Parser);
    // внутри парсера надо изменить def, чтобы ф-ция сама посчитала размеры и текст
    _PI->WriteLoHook(0x4F558D, H3Dlg8Item_CompareItemTypeInsideParser);

    _PI->WriteLoHook(0x4F77F8, H3Dlg8_H3DlgDef_BeforeCtor);
    _PI->WriteLoHook(0x4F7838, H3Dlg8_H3DlgDef_Ctor);
    _PI->WriteLoHook(0x4F7B5E, H3Dlg8_RightBeforeShow);
    _PI->WriteLoHook(0x4F7BD5, H3Dlg8_RightAfterClose);

    // правильное смещение для жёлтых рамок
    _PI->WriteByte(0x4F7985 + 2, 1); // увеличение ширины
    _PI->WriteByte(0x4F7988 + 2, 1); // увеличение высоты

    // увеличение высоты скролл текста
    if (o_HD_Y >= 664)
        _PI->WriteDword(0x4F662F + 1, o_HD_Y - 440);

    // Adventure-map info panel extension: type 11 (EBottomViewType::CUSTOM_MSG_BOX) draws two caller-supplied DEFs.
    _PI->WriteLoHook(0x415D6E, AdventureInfoPanel_Type11);
    auto &rect = g_adventureInfoDefPanel.backPos;
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
