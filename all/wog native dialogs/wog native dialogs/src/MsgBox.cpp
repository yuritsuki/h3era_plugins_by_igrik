////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// Callback диалога MsgBox
/////////////////////////////////////////////////////////
#include <memory>

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
_LHF_(test_Faerie);

int my_TimeClick_MsgBox;
int my_TimeAnimate_MsgBox;
struct Dlg8ItemInfo;

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

    BOOL isValid = FALSE;
    std::string assetName;
    int initialDefFrame = 0;
    std::string text;
    std::string rmcHint;
    _DlgStaticDef_ *defPtr = nullptr;

  public:
    static int initCounter;
};
int Dlg8ItemInfo::initCounter = 0;

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
        {
            dlg8ItemInfos[i] = {};
        }
        // memset(dlg8ItemInfos, 0, sizeof(dlg8ItemInfos));
    }
};

struct Dlg8Manager
{
    std::vector<std::unique_ptr<Dlg8Info>> dlg8ItemInfosVector;
    Dlg8Info dlg8InfoBuffer;
    BOOL bufferInUse = FALSE;
    Dlg8Info *currentDlg8Info;
    Patch *patches[5];

  public:
    inline void ClearPicturesBuffer()
    {
        for (size_t i = 0; i < Dlg8Info::MAX_SIZE; i++)
        {
            dlg8InfoBuffer.dlg8ItemInfos[i].assetName = {};
            dlg8InfoBuffer.dlg8ItemInfos[i].initialDefFrame = {};
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
    inline void Init(PatcherInstance *_PI)
    {
        _PI->WriteLoHook(0x04A8BC2, test_Faerie);
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
        const int itemsToAnimate = dlg8Info->usedSize;
        int time = o_GetTime();
        if (time - my_TimeAnimate_MsgBox > 100)
        {
            BOOL redraw = FALSE;
            for (size_t i = 0; i < itemsToAnimate; i++)
            {
                auto &info = dlg8Info->dlg8ItemInfos[i];
                if (info.initialDefFrame != -1 || info.defPtr == nullptr)
                    continue;
                _DlgStaticDef_ *it = info.defPtr;
                const int defFrameIndex = it->def_frame_index;
                const int lastFrameIndex = it->def->groups[0]->frames_count - 1;
                if (defFrameIndex < lastFrameIndex)
                    it->SetFrame(defFrameIndex + 1);
                else
                    it->SetFrame(0);
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
    SetMaskMsgBoxItId((1 << Dlg8ItemInfo::initCounter) - 1);

    return CALL_1(signed int, __thiscall, hook->GetDefaultFunc(), gm);
}

struct H3Dlg8Item
{
    int picType;
    int picSubType;
    _HStr_ spriteName;
    _HStr_ textBelow;
    int spriteFrameIndex;
    POINT spritePos;
    int spriteHeight;
    int spriteWidth;
    POINT textPos;
    int textHeight;
    int textWidth;
};

char *nameArt = "dlg_npc1.def";
_LHF_(test_Faerie)
{
    c->esp += 4;

    c->Push((DWORD)nameArt);

    return EXEC_DEFAULT;
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
            auto &info = dlg8ItemInfos[infoCounter++];
            info.isValid = TRUE;
            info.assetName = assetName;
            const int defFrame = frameIds[i];
            info.initialDefFrame = defFrame;
            if (defFrame == -1)
                dlg8Info.hasAnimation = TRUE;
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
        if (dlg8ItemInfos[i].isValid)
        {
            if (descriptions[i])
                dlg8ItemInfos[i].text = descriptions[i];
            if (rmcHints[i])
                dlg8ItemInfos[i].rmcHint = rmcHints[i];
        }
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
            item.assetName = info.assetName.c_str();
            item.defFrame = info.initialDefFrame;
        }
        items.Append(item);
    }
    // dlg8Manager.ApplyPatches();
    dlg8Manager.bufferInUse = TRUE;
    Dlg8ItemInfo::initCounter = 0;
    dlg8Manager.currentDlg8Info = &dlg8Manager.dlg8InfoBuffer;

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

    // dlg8Manager.UndoPatches();

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
        if (dlg8Info->dlg8ItemInfos[index].isValid && !hint.empty())
            b_MsgBox(const_cast<char *>(hint.c_str()), 4);
    }

    return EXEC_DEFAULT;
}

_LHF_(H3Dlg8Item_Parser)
{

    const int id = Dlg8ItemInfo::initCounter++;

    if (id == 0 &&  dlg8Manager.bufferInUse)
    {
        dlg8Manager.currentDlg8Info = &dlg8Manager.dlg8InfoBuffer;
    }
    auto currentDlg8Info = dlg8Manager.currentDlg8Info;

    if (!currentDlg8Info)
    {

        // if (c->edi > 0xFFFF)
        //{
        //     H3Dlg8Item *item = reinterpret_cast<H3Dlg8Item *>(c->ebx);
        //     // item->spriteName.Set((char*)c->edi);
        //     item->textBelow.Set(Era::IntToStr(c->edi).c_str());
        //     if (c->edi > 1000000000)
        //     {
        //         item->spriteName.Set("resource.def");
        //     }

        //    c->return_address = 0x4F6229;
        //    return EXEC_DEFAULT;
        //    // 0x4F61DD
        //}
        return EXEC_DEFAULT;
    }

    auto &info = currentDlg8Info->dlg8ItemInfos[id];
    if (!info.isValid)
        return EXEC_DEFAULT;
    H3Dlg8Item *item = reinterpret_cast<H3Dlg8Item *>(c->ebx);

    if (!info.assetName.empty())
    {
        item->picType = (DWORD)info.assetName.data();
        item->spriteName.Set(info.assetName.c_str());
    }

    int picSubtype = c->eax;

    if (!info.text.empty())
        item->textBelow.Set(info.text.c_str());
    if (info.initialDefFrame >= 0)
        item->spriteFrameIndex = info.initialDefFrame;

    c->return_address = 0x4F6229;

    return NO_EXEC_DEFAULT;
}

_LHF_(H3Dlg8_H3DlgDef_Ctor)
{

    if (dlg8Manager.bufferInUse)
    {
        const int index = IntAt(c->ebp - 0x14);
        dlg8Manager.dlg8InfoBuffer.dlg8ItemInfos[index].defPtr = reinterpret_cast<_DlgStaticDef_ *>(c->ecx);
    }
    return EXEC_DEFAULT;
}

_LHF_(Y_New_MsgBox_ChangeDefName)
{
    c->ecx = (int)nameArt;
    return NO_EXEC_DEFAULT;
}
_LHF_(Y_New_MsgBox_ChangeDefType)
{
    c->Push((int)nameArt);
    return NO_EXEC_DEFAULT;
}
_LHF_(H3Dlg8_RightBeforeShow)
{

    auto &vec = dlg8Manager.dlg8ItemInfosVector;
    vec.push_back(nullptr);

    if (dlg8Manager.bufferInUse)
    {
        dlg8Manager.bufferInUse = FALSE;

        auto& buffer = dlg8Manager.dlg8InfoBuffer;
        // clear not used items

        const size_t itemsCreated = Dlg8ItemInfo::initCounter;
        buffer.usedSize = itemsCreated;
        for (size_t i = itemsCreated; i < Dlg8ItemInfo::MAX_SIZE; i++)
        {
            buffer.dlg8ItemInfos[i] = {};
        }


        vec.back() = std::make_unique<Dlg8Info>(buffer);
        dlg8Manager.dlg8InfoBuffer.Clear();
    }

    Dlg8ItemInfo::initCounter = 0;

    dlg8Manager.currentDlg8Info = vec.back().get();
    return EXEC_DEFAULT;
}
_LHF_(H3Dlg8_RightAfterClose)
{
    auto &vec = dlg8Manager.dlg8ItemInfosVector;
    if (!vec.empty())
        vec.pop_back();

    dlg8Manager.currentDlg8Info = vec.empty() ? nullptr : vec.back().get();

    return EXEC_DEFAULT;
}
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

void Dlg_MsgBox(PatcherInstance *_PI)
{
    // новый Callback диалога MsgBox
    _PI->WriteHiHook(0x4F1650, SPLICE_, EXTENDED_, THISCALL_, Y_New_MsgBox_Proc);

    // установка дефолтной желтой рамки (как буд-то она уже выбранна)
    _PI->WriteLoHook(0x4F7B46, Y_New_MsgBox_SetDefaultFrameEnabled);
    //  _PI->WriteLoHook(0x04510B5, Y_New_MsgBox_ChangeDefType);
    // _PI->WriteLoHook(0x045107D, Y_New_MsgBox_ChangeDefName);

    // создание элемента для хранения "разрешенных к выбору элементов"
    _PI->WriteHiHook(0x4F71BB, CALL_, EXTENDED_, THISCALL_, Y_New_MsgBox_GetBitMask);
    _PI->WriteLoHook(0x4F11E7, H3Dlg8_H3DlgDef_RightClick);
    _PI->WriteLoHook(0x4F558D, H3Dlg8Item_Parser);
    _PI->WriteLoHook(0x4F7838, H3Dlg8_H3DlgDef_Ctor);
    _PI->WriteLoHook(0x4F7B5E, H3Dlg8_RightBeforeShow);
    _PI->WriteLoHook(0x4F7BD5, H3Dlg8_RightAfterClose);

    // правильное смещение для жёлтых рамок
    _PI->WriteByte(0x4F7985 + 2, 1); // увеличение ширины
    _PI->WriteByte(0x4F7988 + 2, 1); // увеличение высоты

    // увеличение высоты скролл текста
    if (o_HD_Y >= 664)
        _PI->WriteDword(0x4F662F + 1, o_HD_Y - 440);

    // установка хуков для ПКМ процедуры и конструктора H3Dlg8Item
    dlg8Manager.Init(_PI);

    if (HINSTANCE hEra = GetModuleHandleA("era.dll"))
    {
        GetDefaultMsgBoxItId = (TGetDefaultMsgBoxItId)GetProcAddress(hEra, "_GetPreselectedDialog8ItemId");
        GetMaskMsgBoxItId = (TGetMaskMsgBoxItId)GetProcAddress(hEra, "_GetDialog8SelectablePicsMask");
        SetMaskMsgBoxItId = (TSetMaskMsgBoxItId)GetProcAddress(hEra, "_SetDialog8SelectablePicsMask");
    }
}
