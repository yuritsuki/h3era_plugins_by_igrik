#pragma once

//#include "Base.h"
//#include "HoMM3.h"
//#include "Logging.h"

//#include "BattleFigures.h"
// Основная группа def`а.
#define DG_MAIN 0

// Группы def`а стеков в бою.
#define DG_MOVE                 0 // Анимация движения
#define DG_RANDANIM             1 // Случайная анимация, кривляние
#define DG_STAY                 2 // Анимация стойки
#define DG_DAMAGE               3 // Анимация получения повреждений
#define DG_DEFENSE              4 // Анимация получения повреждений в защитной стойке
#define DG_DEATH                5 // Анимация смерти
#define DG_SPEC_DEATH           6 // Особая анимация смерти(есть у Бехолдеров, Дурных Глаз, Золотых Големов, не используется в оригинале)
#define DG_TURN_TO_RIGHT_BEGIN  7 // Анимация начала поворота вправо
#define DG_TURN_TO_LEFT_END     8 // Анимация конца поворота влево
#define DG_TURN_TO_LEFT_BEGIN   9 // Анимация начала поворота влево
#define DG_TURN_TO_RIGHT_END   10 // Анимация конца поворота вправо
#define DG_HIT_UP              11 // Анимация ближней атаки вверх
#define DG_HIT_STRAIGHT        12 // Анимация ближней атаки прямо
#define DG_HIT_DOWN            13 // Анимация ближней атаки вниз
#define DG_SHOT_UP             14 // Анимация дальней атаки вверх
#define DG_SHOT_STRAIGHT       15 // Анимация дальней атаки прямо
#define DG_SHOT_DOWN           16 // Анимация дальней атаки вниз
#define DG_CAST_UP             17 // Анимация колдовства вверх
#define DG_CAST_STRAIGHT       18 // Анимация колдовства прямо
#define DG_CAST_DOWN           19 // Анимация колдоства вниз
#define DG_BEGIN_MOVE          20 // Анимация начала движения
#define DG_END_MOVE            21 // Анимация конца движения


#pragma once


// Порядок отображения подо всеми.
#define MPO_BELOW_ALL -2

// Порядок отображения надо всеми.
#define MPO_ABOVE_ALL -1


// Порядок отображения под стеком.
#define MPO_BELOW_STACK -3

// Порядок отображения между стеком и его прямоугольником с количеством (и под анимацией).
#define MPO_BELOW_STACK_ANIM -2

// Порядок отображения над стеком.
#define MPO_ABOVE_STACK -1


// Нет гекса (вместо номера).
#define NO_GEX -1


// Нет стороны (вместо номера).
#define NO_SIDE -1

// Нет стека (вместо номера).
#define NO_STACK -1


// Размер - полный размер def`а.
#define SIZE_FULL -1


// Тип позиции - нет.
#define BF_NO_POS_TYPE -1
// Тип позиции - для стека или гекса - с земли.
#define BF_POS_TYPE_DOWN 0
// Тип позиции - для стека или гекса - в центре.
#define BF_POS_TYPE_CENTER 1
// Тип позиции - для стека - сверху.
#define BF_POS_TYPE_UP 2
// Тип позиции - для стека - перед.
#define BF_POS_TYPE_FRONT 3
// Тип позиции - для гекса - на гексе начиная справа-снизу.
#define BF_POS_TYPE_HEX 4



// Тип изображения - с поддержкой включения полупрозрачности.
#define IMG_TRANSPARENT 0
// Тип изображения - с поддержкой замены жёлтого спеццвета.
#define IMG_SPECCOLOR_REPLACE 1



// Цвет на замену спеццвета - прозрачный.
#define R_NO_COLOR 0


// Отсутствие границы кадров анимации.
#define ALL_FRAMES -1



// Максимальное случайное отклонение от среднего значения периода анимаций.
#define MAX_PERIOD_RAND_DEVI 0.25



// Добавка к высоте существа на стрелковой башне для Медузы.
#define TOWER_MEDUSA_ADD_HEIGHT -3



// Код клавиши ESC как подтип сообщения события.
#define KEY_ESC 1



// Скорость - нет анимации.
#define NO_ANIM -1


// Максимальное количесвто пропускаемых кадров.
#define MAX_FRAMESKIP 32





// Является ли текущи проект частью проекта HotA (для включения / отключения некоторых возможностей анимации).
#define IsHotA FALSE


// Максимальная ширина существа.
#define MAX_CREATURE_WIDTH 196

// Максимальная высота изображения существа стека.
#define MAX_CREATURE_HEIGHT 267



// Ширина гекса
#define HEX_WIDTH 22


// Специальный сдвиг по-горизонтали для Медузы на башне.
#define TOWER_MEDUSA_X_SHIFT 5



// Сдвиг на центр гекса по-вертикали снизу.
#define GEX_CENTER_Y_SHIFT -37

// Сдвиг на нижнюю годную для отрисовки позицию свеху.
#define GEX_STD_DOWN_SHIFT 52


// Боевая фигура, изображение на поле боя.
struct _BattleFigure_;





// Длительность активности последнего диалога.
extern _int_ Battle_LastDialog_Time;


// Включено ли упрощённое удаление боевых фигур.
extern _bool_ Fgrs_Simple_Destruct;



// Список всех существующих боевых фигур на заднем плане.
extern _List_<_BattleFigure_*> BattleFgrs_Background;

// Список всех существующих боевых фигур на гексах.
extern _List_<_BattleFigure_*> BattleFgrs_Gexes[BATTLE_HEXES_COUNT];

// Список всех существующих боевых фигур на стеках.
extern _List_<_BattleFigure_*> BattleFgrs_Stacks[2][BATTLE_SIDE_STACKS_COUNT];

// Список всех существующих боевых фигур на переднем плане.
extern _List_<_BattleFigure_*> BattleFgrs_Foreground;





// Текущая отрисовывающаяся стрелковая башня.
extern _ArrowTower_* CurrDrawingTower;



// Текущая добавка к X-координате стека при движении или полёте.
extern _int_ Curr_Stack_Moving_X_Add;

// Текущая добавка к Y-координате стека при движении или полёте.
extern _int_ Curr_Stack_Moving_Y_Add;





// Инициализация модуля боевых фигур.
void BattleFigures_Init();



// Корректен ли номер гекса.
_bool_ Gex_IsCorrect(_int_ GexNum);

// Корректен ли номер стека.
_bool_ Stack_IsCorrect(_int_ Side, _int_ StackNum);

// Корректен ли порядок отображения.
_bool_ MappingOrder_IsCorrect(_int_ MappingOrder);




// Определение списка по определяющим параметрам фигуры.
_List_<_BattleFigure_*>* DetermineList_By_FigureParams(_int_ GexNum, _int_ MappingOrder, _int_ Side, _int_ StackNum);




// Удаление всех фигур из списка.
void Delete_Figures_From_List(_List_<_BattleFigure_*>* List);


// Очищение списков боевых фигур.
void Delete_All_Battle_Figures();




// Отрисовка  боевых фигур из общего списка в текущем режиме (эта функция должна быть вызвана при каждом режиме).
// GexNum - текущий гекс (-1 - нет).
// MappingOrder - текущий порядок отображения.
void BattleFgrs_Draw(_int_ GexNum, _int_ MappingOrder, _int_ Side, _int_ StackNum, _int_ X_Add = 0, _int_ Y_Add = 0);




// Отрисовка или настройка границ отображения для фигур в списке.
void BattleFgrs_List_Draw(_List_<_BattleFigure_*>* List);

// Отрисовка или настройка границ отображения для нестековых фигур, не принадлежащих стекам.
void BattleFgrs_Draw_NoStacks();




// Подготавливаем проходы анимаций всех боевых фигур.
void BattleFgrs_ParallelAnimPrepare();

// Параллельное проигрывание проходов анимаций списка.
void BattleFgrs_List_ParallelAnim(_List_<_BattleFigure_*>* List);

// Проигрываем проходы анимаций всех боевых фигур.
void BattleFgrs_ParallelAnim();



// Восстанавливаем перерисовку всех боевых фигур.
void BattleFgrs_RestoreRedraw();

// Сохраняем перерисовку всех боевых фигур.
void BattleFgrs_SaveRedraw();




// При начале отрисовки отрисовываем боевые фигуры заднего плана.
int __stdcall HookOn_Fgr_DrawBegin(LoHook* h, HookContext* c);


// При отрисовке порядка на гексе отрисовываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_GexesDraw(LoHook* h, HookContext* c);


// В конце отрисовки отрисовываем боевые фигуры переднего плана.
int __stdcall HookOn_Fgr_DrawEnd(LoHook* h, HookContext* c);



// Перед отрисовкой стека проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_StackBeforeBlit(LoHook* h, HookContext* c);

// После отрисовки стека проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_StackAfterBlit(LoHook* h, HookContext* c);

// После отрисовки анимации стека проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_StackAfterBlitAnims(LoHook* h, HookContext* c);



// Перед отрисовкой стрелковой башни обнуляем ссылку на неё.
int __stdcall HookOn_Fgr_TowerCreatureBeforeDraw(LoHook* h, HookContext* c);

// Перед отрисовкой существа стрелковой башни проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_TowerCreatureBeforeBlit(LoHook* h, HookContext* c);

// После отрисовки существа стрелковой башни проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_TowerCreatureAfterBlit(LoHook* h, HookContext* c);



// При настройке границ перерисовки боя настраиваем границы перерисовки не принадлежащих стекам фигур.
void __stdcall HookOn_Battle_SettingRedrawBorders(HiHook* h, _BattleMgr_* this_);



// Рассчитываем добавки к координатам отрисовки стека.
int __stdcall HookOn_StackDraw_Calcs_PosAdds_For_Moving(LoHook* h, HookContext* c);



// Перед боем очищаем списки боевых фигур.
void __stdcall HookOn_Battle_Start_Fgrs(HiHook* h, _BattleMgr_* this_, _dword_ a2);

// После сообщения о конце боя очищаем списки боевых фигур.
void __stdcall HookOn_Battle_EndMessage_Fgrs(HiHook* h, _BattleMgr_* this_, _dword_ a2);



// При проигрывании анимации ожидания в бою проигрываем параллельную анимацию боевых фигур.
// int __stdcall HookOn_WaitAnim_Fgrs(LoHook* h, HookContext* c);

// Очищаем перерисовки боевых фигур (inline-подстановка всего одна, и та стирается в другом месте).
void __stdcall HookOn_BattleClearRedraws(HiHook* h, _BattleMgr_* this_);



// Боевая фигура, изображение на поле боя.
NOALIGN struct _BattleFigure_
{
    // Следующие поля определяют активность фигуры.

    // Включена ли.
    // Если нет - фигура не отрисовывается и не меняет кадры.
    _bool_ Enabled;
    // Видна ли.
    // если нет - фигура не отрисовывается (но может менять кадры, если не отключена).
    _bool_ Visible;


    // Следующие поля определяют изображение фигуры.

private:
    // Загруженный def фигуры.
    // Не следует устанавливать это поле вручную - это может вызвать утечку памяти.
    // Для установки def`а фигуры можно использовать специальные методы.
    _Def_* Def;
public:
    // Текущая отображаемая секция def`а.
    _int_ CurrDefGroup;
    // Текущий отображаемый кадр текущей секции def`а.
    _int_ CurrFrame;


    // Следующие поля используются для возможности отображения не всего кадра def`а, а только его прямоугольной части.

    // Начальная x-координата изображения как части кадра def`а.
    _int_ Img_Def_X;
    // Начальная y-координата изображения как части кадра def`а.
    _int_ Img_Def_Y;
    // Ширина изображения как части def`а.
    // SIZE_FULL - вся ширина кадра, в этом случае начальная координата Img_Def_X не учитывается.
    _int_ Img_Width;
    // Высота изображения как части def`а.
    // SIZE_FULL - вся ширина кадра, в этом случае начальная координата Img_Def_Y не учитывается.
    _int_ Img_Height;


    // Отражено ли изображение (по-горизонтали). Если отражено - кадр def`а будет отриосван зеркально.
    // Для привязанной к стеку фигуры отражение определяется направлением стека.
    _bool_ Reflected;


    // Следующие поля определяют особые свойства изображения фигуры.

    // Тип изображения. Если особые свойства типов не нужны - лучше выбрать IMG_TRANSPARENT, установив IsTransparent в FALSE.
    // IMG_TRANSPARENT - с поддержкой добавления общей полупрозрачности (используется для боевых анимаций).
    // IMG_SPECCOLOR_REPLACE - с поддержкой замены жёлтого спеццвета на произвольный (используется при отрисовке стеков в бою).
    _int_ ImageType;
    // Является ли изображение полупрозрачным (работает только для типа IMG_TRANSPARENT, иначе считаеся FALSE).
    // Если является - всё изображение целиком, независимо от того, каково оно в def`е отрисовывается полупрозрачно.
    _bool_ IsTransparent;
    // Произвольный цвет, заменяющий жёлтый спеццвет при отображении (для типа IMG_SPECCOLOR_REPLACE, иначе считается R_NO_COLOR).
    // R_NO_COLOR - нет цвета (прозрачный).
    _int_ ColorReplaceSpec;


    // Следующие поля определяют проигрывание анимации фигуры.

    // Время между подряд идущими кадрами (без учёта настроек скорости боя).
    _int_ FrameTime;
    // Время следующей смены кадров.
    // Это поле используется при анимации, чтобы определить, когда должен проигрываться следующий кадр.
    // При смене кадров оно автоматически меняется.
    _int_ NextFrameTime;
    // Зависит ли скорость анимации фигуры от настроек скорости боя.
    // Если зависит - время между кадрами при использовании будет умножаться на соответствующее настройкам значение.
    // Рекомендуется устанавливать в TRUE для фигур, приостанавливающих бой во время анимации, и в FALSE для анимирующих параллельно бою.
    _bool_ Depend_On_BattleSpeed;
    // Фигура анимирует параллельно бою, циклически прокручивая кадры текущей секции def`а.
    _bool_ ParallelAnim;
    // Шаг смены кадров анимации (может быть отрицательным).
    _int_ FrameStep;
    // После окончания кадров в секции def`а фигура будет уничтожена.
    // Для фигуры будет вызван деструктор прямо при смене кадров.
    _bool_ Temporary;
    // Техническое поле: фигуру необходимо включить в границы перерисовки.
    _bool_ NeedRedraw;
    // Техническое поле: фигуру необходимо включить в границы перерисовки - сохранённое значение.
    _bool_ NeedRedrawSaved;

    // Следующие поля определяют привязку и порядок отрисовки фигуры.
    // Не следует напрямую обращаться к этим полям, это может вызвать утечку памяти.
    // Для обращения к ним есть специальные методы.

private:
    // Порядок отображения на поле боя, отображается поверх изображений своего порядка.
    // Значения 0-7 - стандартные геройские, в одном ряду гексов поля боя элементы отображаются в порядке возрастания их номеров порядков.
    // MPO_BELOW_ALL - отображается под всеми активными элементами поля боя.
    // MPO_ABOVE_ALL  - отображается поверх всех элементов поля боя.
    // MPO_BELOW_STACK - отображается подо стеком (только для привязанной к стеку).
    // MPO_BELOW_STACK_ANIM - отображается над стеком, но под его анимацией и прямоугольником с количеством (только для привязанной к стеку).
    // MPO_ABOVE_STACK - отображается над стеком и его анимацией (только для привязанной к стеку).
    _int_ MappingOrder;
    // Номер гекса фигуры, NO_GEX - нет (в этом случае фигура не привязана к гексу, а порядок отрисовки не должен быть 0-7).
    // Если у фигура привязана к стеку, это поле не учитывается.
    _int_ GexNum;
    // Сторона стека фигуры, NO_SIDE - нет (в этом случае фигура не привязана к стеку).
    // Порядок отрисовки привязанной к стеку фигуры должен быть MPO_BELOW_STACK, MPO_BELOW_STACK_ANIM или MPO_ABOVE_STACK.
    _int_ Side;
    // Номер стека фигуры, NO_STACK - нет (в этом случае фигура не привязана к стеку).
    // Порядок отрисовки привязанной к стеку фигуры должен быть MPO_BELOW_STACK, MPO_BELOW_STACK_ANIM или MPO_ABOVE_STACK.
    _int_ StackNum;


    // Следующие поля определяют позицию изображения.

public:
    // Тип позиции изображения. Определяет позицию привязанной к гексу или стеку фигуры.
    // Тип позиции - нет.
    // BF_NO_POS_TYPE - нет типа позиции, позиция не определяется положением гекса или стека.
    // BF_POS_TYPE_DOWN - для привязанной к стеку или гексу - на стеке с земли  (как анимация медлительности) или с низа гекса.
    // BF_POS_TYPE_CENTER - для привязанной к стеку или гексу - на середине гекса или изображения стека (как анимация щита).
    // BF_POS_TYPE_UP - только для привязанной к стеку - над головой стека (как молния в полёте).
    // BF_POS_TYPE_FRONT - только для привязанной к стеку - перед стеком (как анимация сопротивления магии).
    // BF_POS_TYPE_HEX - только для привязанной к гексу - на гексе (как силоое поле). 
    _int_ PositionType;
    // Смещение по X (если нет привязуи к стеку или гексу или тип позиции - BF_NO_POS_TYPE, X-координата).
    _int_ X_Shift;
    // Смещение по Y (если нет привязуи к стеку или гексу или тип позиции - BF_NO_POS_TYPE, Y-координата).
    _int_ Y_Shift;
    // Полная ли снихронизация с изображением стека.
    // Если нет - проигрывается как стандартная анимация (синхронизация с кадром 0 анимации стойки стека).
    // Не учитывается, если нет привязки к стеку.
    _bool_ Full_Sync_To_Stack;
    // Использовать ли добавки к позиции стека (нужные для синхронизации со стеком при движении), не использующиеся в стандартных анимациях.
    // Рекомендуется использовать для привязанных к стеку параллельных анимаций.
    // Не учитывается, если нет привязки к стеку.
    _bool_ Use_Position_Additions;







    // Управление def`ом фигуры.


    // Удаление def`а фигуры.
    inline void DeleteDef()
    {
        // Если def существует...
        if (this->Def)
        {
            // Удаляем def.
            this->Def->DerefOrDestruct();
            // Обнуляем параметры def`а.
            this->Def = 0;
            this->CurrDefGroup = 0;
            this->CurrFrame = 0;
        }
    }


    // Загрузка и установка нового def`а по имени, а так же установление безопасных текущих группы, кадра и границ изображения для него.
    // Старый def удаляется автоматически.
    inline void SetNewDef(char* DefName)
    {
        // Удаляем старый def.
        this->DeleteDef();

        // Загружаем новый def.
        this->Def = _Def_::Load(DefName);

        // Обнуляем параметры def`а, если они стали некорректны.

        if (this->Def->groups_count > this->CurrDefGroup && ((DWORD*)(this->Def->active_groups))[this->CurrDefGroup])
        {
            if (this->Def->groups[this->CurrDefGroup]->frames_count <= this->CurrFrame)
            {
                this->CurrFrame = 0;
            }
        }
        else
        {
            this->CurrDefGroup = 0;
            this->CurrFrame = 0;
        }

        if (this->Img_Width != SIZE_FULL && (this->Img_Def_X < 0 || this->Img_Def_X >= this->Def->width
            || this->Img_Def_X + this->Img_Width < 0 || this->Img_Def_X + this->Img_Width >= this->Def->width))
        {
            this->Img_Def_X = 0;
            this->Img_Width = SIZE_FULL;
        }

        if (this->Img_Height != SIZE_FULL && (this->Img_Def_Y < 0 || this->Img_Def_Y >= this->Def->height
            || this->Img_Def_Y + this->Img_Height < 0 || this->Img_Def_Y + this->Img_Height >= this->Def->height))
        {
            this->Img_Def_Y = 0;
            this->Img_Height = SIZE_FULL;
        }
    }








    // Управление глобальными списками фигуры.
    // Не рекомендуется использовать эти методы - некорректное их исользование может вызвать утечку памяти.


    // Определение списка, в котором должна находиться фигура.
    inline private _List_<_BattleFigure_*>* DetermineList()
    {
        // Определяем список по соответствующим параметрам фгуры.
        return DetermineList_By_FigureParams(this->GexNum, this->MappingOrder, this->Side, this->StackNum);
    }


    // Удаление фигуры из списков.
    inline private void DeleteFromLists()
    {
        // Определяем список фигуры и удаляем её оттуда.
        this->DetermineList()->DeleteLastValue(this);
    }



    // Добавление боевой фигуры в списки.
    inline private void AddToLists()
    {
        // Определяем список фигуры и добавляем её туда.
        this->DetermineList()->Append(this);
    }












    // Управление полями, отвечающими за привязку и порядок отрисовки фигуры.


    // Установка полей, отвечающих за привязку и порядок отрисовки фигуры.
    // В целях оптимизации рекомендуется использовать эту команду, если надо поменять сразу несколько полей.
    inline void Set_New_ListParams(_int_ GexNum, _int_ MappingOrder, _int_ Side, _int_ StackNum)
    {

        // Исправляем некорректные параметры.
        if (!Gex_IsCorrect(GexNum))
        {
            GexNum = NO_GEX;
        }
        if (!MappingOrder_IsCorrect(MappingOrder))
        {
            MappingOrder = MPO_ABOVE_ALL;
        }
        if (!Stack_IsCorrect(Side, StackNum))
        {
            Side = NO_SIDE;
            StackNum = NO_STACK;
        }


        // Необходимо ли менять список фигуры.
        _bool_ NeedReList = (this->DetermineList() != DetermineList_By_FigureParams(GexNum, MappingOrder, Side, StackNum));

        // Если надо поменять список, удаляем фигуру из списков.
        if (NeedReList) this->DeleteFromLists();

        // Устанавливаем новые параметры.
        this->GexNum = GexNum;
        this->MappingOrder = MappingOrder;
        this->Side = Side;
        this->StackNum = StackNum;

        // Если надо поменять список, добавляем фигуру в списки.
        if (NeedReList) this->AddToLists();
    }





    // Установка нового гекса фигуры.
    inline void Set_GexNum(_int_ GexNum)
    {
        // Устанавливаем новый номер гекса.
        this->Set_New_ListParams(GexNum, this->MappingOrder, this->Side, this->StackNum);
    }

    // Получние номера гекса фигуры.
    inline _int_ Get_GexNum()
    {
        return this->GexNum;
    }



    // Установка нового порядка отображения фигуры.
    inline void Set_MappingOrder(_int_ MappingOrder)
    {
        // Устанавливаем новый номер гекса.
        this->Set_New_ListParams(this->GexNum, MappingOrder, this->Side, this->StackNum);
    }

    // Получние порядка отображения фигуры.
    inline _int_ Get_MappingOrder()
    {
        return this->MappingOrder;
    }



    // Установка нового стека (со стороной) фигуры.
    inline void Set_Stack(_int_ Side, _int_ StackNum)
    {
        // Устанавливаем новый номер гекса.
        this->Set_New_ListParams(this->GexNum, this->MappingOrder, Side, StackNum);
    }

    // Получние стороны фигуры.
    inline _int_ Get_Side()
    {
        return this->Side;
    }

    // Получние номера стека фигуры.
    inline _int_ Get_StackNum()
    {
        return this->StackNum;
    }












    // Конструктор (с возможностью обозначить определители добавления в нужный список).
    inline _BattleFigure_(_int_ GexNum = NO_GEX, _int_ MappingOrder = MPO_ABOVE_ALL, _int_ Side = NO_SIDE, _int_ StackNum = NO_STACK)
    {
        // Значения по-умолчанию.
        this->Enabled = FALSE;
        this->Visible = FALSE;
        this->Def = 0;
        this->CurrDefGroup = 0;
        this->CurrFrame = 0;
        this->Img_Def_X = 0;
        this->Img_Def_Y = 0;
        this->Img_Width = SIZE_FULL;
        this->Img_Height = SIZE_FULL;
        this->Reflected = FALSE;
        this->ImageType = IMG_TRANSPARENT;
        this->IsTransparent = FALSE;
        this->ColorReplaceSpec = R_NO_COLOR;
        this->FrameTime = STD_FRAME_PERIOD;
        this->NextFrameTime = 0;
        this->Depend_On_BattleSpeed = TRUE;
        this->ParallelAnim = TRUE;
        this->FrameStep = 1;
        this->Temporary = FALSE;
        this->MappingOrder = MappingOrder;
        this->GexNum = GexNum;
        this->Side = Side;
        this->StackNum = StackNum;
        this->PositionType = BF_NO_POS_TYPE;
        this->X_Shift = 0;
        this->Y_Shift = 0;
        this->Full_Sync_To_Stack = FALSE;
        this->Use_Position_Additions = FALSE;

        // Добавляем фигуру в её список.
        this->AddToLists();
    }

    // Деструктор.
    inline ~_BattleFigure_()
    {
        // Вызываем деструктор def`а.
        this->DeleteDef();

        // Если деструктор не упрощён, удаляем фигуру из списков.
        if (!Fgrs_Simple_Destruct) this->DeleteFromLists();
    }






    // Примитивное создание боевой фигуры с полным списком параметров.
    // DefName - имя def`а, который будет загружен при создании фигуры (NULL - нет def`а).
    static inline _BattleFigure_* Create(_bool_ Enabled, _bool_ Visible,
        char* DefName, _int_ CurrDefGroup, _int_ CurrFrame, _int_ Img_Def_X, _int_ Img_Def_Y,
        _int_ Img_Width, _int_ Img_Height, _bool_ Reflected, _int_ ImageType, _bool_ IsTransparent,
        _int_ ColorReplaceSpec, _int_ FrameTime, _int_ NextFrameTime, _bool_ Depend_On_BattleSpeed,
        _bool_ ParallelAnim, _int_ FrameStep, _bool_ Temporary, _int_ MappingOrder, _int_ GexNum,
        _int_ Side, _int_ StackNum, _int_ PositionType, _int_ X_Shift, _int_ Y_Shift,
        _bool_ Full_Sync_To_Stack, _bool_ Use_Position_Additions)
    {
        // Новая боевая фигура.
        _BattleFigure_* this_ = new _BattleFigure_(GexNum, MappingOrder, Side, StackNum);

        // Устанавляваем параметры.
        this_->Enabled = Enabled;
        this_->Visible = Visible;

        this_->CurrDefGroup = CurrDefGroup;
        this_->CurrFrame = CurrFrame;
        this_->Img_Def_X = Img_Def_X;
        this_->Img_Def_Y = Img_Def_Y;
        this_->Img_Width = Img_Width;
        this_->Img_Height = Img_Height;

        if (DefName) this_->SetNewDef(DefName);

        this_->Reflected = Reflected;
        this_->ImageType = ImageType;
        this_->IsTransparent = IsTransparent;
        this_->ColorReplaceSpec = ColorReplaceSpec;
        this_->FrameTime = FrameTime;
        this_->NextFrameTime = NextFrameTime;
        this_->Depend_On_BattleSpeed = Depend_On_BattleSpeed;
        this_->ParallelAnim = ParallelAnim;
        this_->FrameStep = FrameStep;
        this_->Temporary = Temporary;

        this_->PositionType = PositionType;
        this_->X_Shift = X_Shift;
        this_->Y_Shift = Y_Shift;
        this_->Full_Sync_To_Stack = Full_Sync_To_Stack;
        this_->Use_Position_Additions = Use_Position_Additions;

        // Возвращаем созданную боевую фигуру.
        return this_;
    }









    // Получение параметров, необходимых для отрисовки фигуры.


    // Получение позиции фигуры по горизонтали с учётом типа позиции.
    // Addition - добавка к координате, используется для синхронизации позиции при движении стека.
    inline _int_ Get_X_Position(_int_ Addition = 0)
    {
        // Если добавка к позиции не используется, обнуляем её.
        if (!this->Use_Position_Additions) Addition = 0;

        // Если у фигуры нет def`а, возвращаем её смещение.
        if (!this->Def) return this->X_Shift + Addition;

        // Вычисляемая позиция.
        _int_ X_Pos = 0;

        // Если есть привязка к стеку...
        if (this->Get_StackNum() != NO_STACK && this->Get_Side() != NO_SIDE)
        {
            // Стек фигуры.
            _BattleStack_* Stack = &(o_BattleMgr->stack[this->Get_Side()][this->Get_StackNum()]);

            // Выбираем расположение в зависимости от типа позиции.
            switch (this->PositionType)
            {
                // От земли.
            case BF_POS_TYPE_DOWN:
                // В центре стека.
            case BF_POS_TYPE_CENTER:
                // Выше стека.
            case BF_POS_TYPE_UP:

                // Для стрелковой башни.
                if (Stack->creature_id == CID_ARROW_TOWER)
                {
                    // Стрелковая башня.
                    _ArrowTower_* Tower = (_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36 * (Stack->Get_ArrowTowerNum()));

                    // Позиция существа на башне.
                    X_Pos = Tower->X_Position - this->Def->width / 2;
                }
                // Не для стрелковой башни.
                else
                {
                    // Позиция гекса.
                    X_Pos = o_BattleMgr->hex[Stack->hex_ix].X_Position - this->Def->width / 2;
                    // Если стек - большое существо, учитываем это.
                    if (Stack->creature.flags & 1)
                    {
                        X_Pos += (Stack->orientation == 0) ? -HEX_WIDTH : HEX_WIDTH;
                    }
                }
                // Учитываем смещение.
                return X_Pos + this->X_Shift + Addition;

                // Перед стеком.
            case BF_POS_TYPE_FRONT:

                // Направление стека.
                _bool_ Orient;

                // Размер нужного кадра def`а стека.
                _int_ StackDefSz;

                // Для стрелковой башни.
                if (Stack->creature_id == CID_ARROW_TOWER)
                {
                    // Стрелковая башня.
                    _ArrowTower_* Tower = ((_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36 * (Stack->Get_ArrowTowerNum())));

                    // Направление существа на башне.
                    Orient = Tower->Orientation;

                    // Позиция существа на башне.
                    X_Pos = Tower->X_Position;
                    // Если существо - большое, то на башне оно теряет это свойство.
                    if (o_CreatureInfo[Tower->CreatureType].flags & 1)
                    {
                        X_Pos += (Orient == 0) ? HEX_WIDTH : -HEX_WIDTH;
                    }
                    // Если существо - медуза, то на башне у неё особая позиция.
                    if (Tower->CreatureType == CID_MEDUSA)
                    {
                        X_Pos += (Orient == 0) ? TOWER_MEDUSA_X_SHIFT : -TOWER_MEDUSA_X_SHIFT;
                    }


                    // Размер нужного кадра def`а башни.

                    // Если идёт полная синхронизация со стеком, берём параметры его текущего кадра.
                    if (this->Full_Sync_To_Stack)
                    {
                        StackDefSz = MAX_CREATURE_WIDTH - (Tower->Def->groups[Tower->AnimSectionNum]->frames[Tower->AnimFrameNum]->frame_left + Tower->Def->groups[Tower->AnimSectionNum]->frames[Tower->AnimFrameNum]->frame_width);
                    }
                    // Если не идёт полная синхронизация со стеком, берём параметры его основного кадра (стойка, кадр 0).
                    else
                    {
                        StackDefSz = MAX_CREATURE_WIDTH - (Tower->Def->groups[DG_STAY]->frames[0]->frame_left + Tower->Def->groups[DG_STAY]->frames[0]->frame_width);
                    }
                }
                // Не для стрелковой башни.
                else
                {
                    // Позиция гекса.
                    X_Pos = o_BattleMgr->hex[Stack->hex_ix].X_Position;
                    // Направление стека.
                    Orient = Stack->orientation;

                    // Размер нужного кадра def`а стека.

                    // Если идёт полная синхронизация со стеком, берём параметры его текущего кадра.
                    if (this->Full_Sync_To_Stack)
                    {
                        StackDefSz = MAX_CREATURE_WIDTH - (Stack->def->groups[Stack->def_group_ix]->frames[Stack->def_frame_ix]->frame_left + Stack->def->groups[Stack->def_group_ix]->frames[Stack->def_frame_ix]->frame_width);
                    }
                    // Если не идёт полная синхронизация со стеком, берём параметры его основного кадра (стойка, кадр 0).
                    else
                    {
                        StackDefSz = MAX_CREATURE_WIDTH - (Stack->def->groups[DG_STAY]->frames[0]->frame_left + Stack->def->groups[DG_STAY]->frames[0]->frame_width);
                    }
                }

                // Позиция зависит от направления стека.
                if (Orient)
                {
                    X_Pos -= StackDefSz;
                }
                else
                {
                    X_Pos += StackDefSz - this->Def->width;
                }
                // Учитываем дополнительные смещения.
                return X_Pos + this->X_Shift + Addition;

                // По-умолчанию возвращаем смещение по X.
            default:
                return this->X_Shift + Addition;
            }
        }

        // Если нет привязки к стеку, но есть к гексу...
        else if (this->Get_GexNum() != NO_GEX)
        {
            // Выбираем расположение в зависимости от типа позиции.
            switch (this->PositionType)
            {
                // От земли.
            case BF_POS_TYPE_DOWN:
                // По центру.
            case BF_POS_TYPE_CENTER:
                // Позиция гекса.
                X_Pos = o_BattleMgr->hex[this->Get_GexNum()].X_Position;
                // Учитываем размер def`а и смещения.
                return X_Pos - this->Def->width / 2 + this->X_Shift + Addition;

                // На гексе слева-снизу.
            case BF_POS_TYPE_HEX:
                // Левая граница гекса.
                X_Pos = o_BattleMgr->hex[this->Get_GexNum()].Left;
                // Учитываем смещения.
                return X_Pos + this->X_Shift + Addition;

                // По-умолчанию возвращаем смещение по X.
            default:
                return this->X_Shift + Addition;
            }
        }

        // Если нет привязки ни к чему, возвращаем смещение.
        else
        {
            return this->X_Shift + Addition;
        }
    }







    // Получение позиции фигуры по вертикали с учётом типа позиции.
    // Addition - добавка к координате, используется для синхронизации позиции при движении стека.
    inline _int_ Get_Y_Position(_int_ Addition = 0)
    {
        // Если добавка к позиции не используется, обнуляем её.
        if (!this->Use_Position_Additions) Addition = 0;

        // Если у фигуры нет def`а, возвращаем её смещение.
        if (!this->Def) return this->Y_Shift + Addition;

        // Вычисляемая позиция.
        _int_ Y_Pos = 0;

        // Если есть привязка к стеку...
        if (this->Get_StackNum() != NO_STACK && this->Get_Side() != NO_SIDE)
        {
            // Стек фигуры.
            _BattleStack_* Stack = &(o_BattleMgr->stack[this->Get_Side()][this->Get_StackNum()]);

            // Выбираем расположение в зависимости от типа позиции.
            switch (this->PositionType)
            {
                // От земли.
            case BF_POS_TYPE_DOWN:

                // Для стрелковой башни.
                if (Stack->creature_id == CID_ARROW_TOWER)
                {
                    // Позиция существа на башне.
                    Y_Pos = ((_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36 * (Stack->Get_ArrowTowerNum())))->Y_Position;
                }
                // Не для стрелковой башни.
                else
                {
                    // Позиция гекса.
                    Y_Pos = o_BattleMgr->hex[Stack->hex_ix].Y_Position;
                }
                // Учитываем смещение def`а и стека.
                return Y_Pos - this->Def->height + this->Y_Shift + Addition;

                // В центре стека.
            case BF_POS_TYPE_CENTER:
                // Перед стеком.
            case BF_POS_TYPE_FRONT:

                // Для стрелковой башни.
                if (Stack->creature_id == CID_ARROW_TOWER)
                {
                    // Стрелковая башня.
                    _ArrowTower_* Tower = ((_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36 * (Stack->Get_ArrowTowerNum())));

                    // Позиция существа на башне (с учётом высоты изображения).

                    // Если идёт полная синхронизация со стеком, берём параметры его текущего кадра.
                    if (this->Full_Sync_To_Stack)
                    {
                        Y_Pos = Tower->Y_Position - (MAX_CREATURE_HEIGHT - Tower->Def->groups[Tower->AnimSectionNum]->frames[Tower->AnimFrameNum]->frame_top) / 2;
                    }
                    // Если не идёт полная синхронизация со стеком, берём параметры его основного кадра (стойка, кадр 0).
                    else
                    {
                        Y_Pos = Tower->Y_Position - (MAX_CREATURE_HEIGHT - Tower->Def->groups[DG_STAY]->frames[0]->frame_top) / 2;
                    }
                }
                // Не для стрелковой башни.
                else
                {
                    // Позиция гекса с учётом высоты существа.

                    // Если идёт полная синхронизация со стеком, берём параметры его текущего кадра.
                    if (this->Full_Sync_To_Stack)
                    {
                        Y_Pos = o_BattleMgr->hex[Stack->hex_ix].Y_Position - (MAX_CREATURE_HEIGHT - Stack->def->groups[Stack->def_group_ix]->frames[Stack->def_frame_ix]->frame_top) / 2;
                    }
                    // Если не идёт полная синхронизация со стеком, берём параметры его стандартную высоту.
                    else
                    {
                        Y_Pos = o_BattleMgr->hex[Stack->hex_ix].Y_Position - Stack->image_height / 2;
                    }
                }
                // Учитываем смещение def`а и стека.
                return Y_Pos - this->Def->height / 2 + this->Y_Shift + Addition;

                // Выше стека.
            case BF_POS_TYPE_UP:

                // Для стрелковой башни.
                if (Stack->creature_id == CID_ARROW_TOWER)
                {
                    // Стрелковая башня.
                    _ArrowTower_* Tower = ((_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36 * (Stack->Get_ArrowTowerNum())));

                    // Позиция существа на башне.

                    // Если идёт полная синхронизация со стеком, берём параметры его текущего кадра.
                    if (this->Full_Sync_To_Stack)
                    {
                        Y_Pos = Tower->Y_Position - (MAX_CREATURE_HEIGHT - Tower->Def->groups[Tower->AnimSectionNum]->frames[Tower->AnimFrameNum]->frame_top);
                    }
                    // Если не идёт полная синхронизация со стеком, берём параметры его основного кадра (стойка, кадр 0).
                    else
                    {
                        Y_Pos = Tower->Y_Position - (MAX_CREATURE_HEIGHT - Tower->Def->groups[DG_STAY]->frames[0]->frame_top);
                    }
                }
                // Не для стрелковой башни.
                else
                {
                    // Позиция гекса с учётом высоты существа.

                    // Если идёт полная синхронизация со стеком, берём параметры его текущего кадра.
                    if (this->Full_Sync_To_Stack)
                    {
                        Y_Pos = o_BattleMgr->hex[Stack->hex_ix].Y_Position - (MAX_CREATURE_HEIGHT - Stack->def->groups[Stack->def_group_ix]->frames[Stack->def_frame_ix]->frame_top);
                    }
                    // Если не идёт полная синхронизация со стеком, берём параметры его стандартную высоту.
                    else
                    {
                        Y_Pos = o_BattleMgr->hex[Stack->hex_ix].Y_Position - Stack->image_height;
                    }
                }
                // Учитываем смещение def`а и стека.
                return Y_Pos - this->Def->height + this->Y_Shift + Addition;

                // По-умолчанию возвращаем смещение по Y.
            default:
                return this->Y_Shift + Addition;
            }
        }

        // Если нет привязки к стеку, но есть к гексу...
        else if (this->Get_GexNum() != NO_GEX)
        {
            // Выбираем расположение в зависимости от типа позиции.
            switch (this->PositionType)
            {
                // По центру.
            case BF_POS_TYPE_CENTER:
                // Позиция гекса.
                Y_Pos = o_BattleMgr->hex[this->Get_GexNum()].Y_Position + GEX_CENTER_Y_SHIFT;
                // Учитываем размер def`а и смещения.
                return Y_Pos - this->Def->height / 2 + this->Y_Shift + Addition;

                // От земли.
            case BF_POS_TYPE_DOWN:
                // На гексе слева-снизу.
            case BF_POS_TYPE_HEX:
                // Верхняя граница гекса. со специальным смещением.
                Y_Pos = o_BattleMgr->hex[this->Get_GexNum()].Top + GEX_STD_DOWN_SHIFT;
                // Учитываем размер def`а и смещения.
                return Y_Pos - this->Def->height + this->Y_Shift + Addition;

                // По-умолчанию возвращаем смещение по Y.
            default:
                return this->Y_Shift + Addition;
            }
        }

        // Если нет привязки ни к чему, возвращаем смещение.
        else
        {
            return this->Y_Shift + Addition;
        }
    }


    // Получение необходимости отражения def`а с учётом привязки к стеку.
    inline _bool_ Get_Reflected()
    {
        // Стек фигуры.
        _BattleStack_* Stack = &(o_BattleMgr->stack[this->Get_Side()][this->Get_StackNum()]);

        // Если анимация привязана к стеку, определяем неоьходимость поворота по нему.
        if (this->Get_StackNum() != NO_STACK && this->Get_Side() != NO_SIDE)
        {
            // Для стрелковой башни возвращаем результат сравнения её направления с 0.
            if (Stack->creature_id == CID_ARROW_TOWER)
            {
                return ((_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36 * (Stack->Get_ArrowTowerNum())))->Orientation == 0;
            }
            // Для стека возвращаем результат сравнения его направления с 0.
            else
            {
                return Stack->orientation == 0;
            }
        }
        // Если фигура не привязана к стеку, возвращаем её реальное значение переворота.
        else
        {
            return this->Reflected;
        }
    }










    // Отрисовка фигуры или настройка границ перерисовки/обновлния экрана для неё (в зависимсти от определённых глобальных параметров боя).
    // X_Add, Y_Add - добавки к координате, используются для синхронизации позиции при движении стека.
    // Не выполняется для фигур с Enabled = FALSE или Visible = FALSE.
    inline void Draw(_int_ X_Add = 0, _int_ Y_Add = 0)
    {
        // Если фигура включена и видима, отрисовываем её.
        if (this->Enabled && this->Visible && this->Def)
        {

            // Начальная x-координата изображения как части def`а
            _int_ Img_Def_X = this->Img_Def_X;
            // Начальная y-координата изображения как части def`а
            _int_ Img_Def_Y = this->Img_Def_Y;
            // Ширина изображения как части def`а
            _int_ Img_Width = this->Img_Width;
            // Высота изображения как части def`а
            _int_ Img_Height = this->Img_Height;

            // При отсутствии ширины указываем ширину всего def`а.
            if (Img_Width == -1)
            {
                Img_Width = this->Def->width;
                Img_Def_X = 0;
            }

            // При отсутствии высоты указываем ширину всего def`а.
            if (Img_Height == -1)
            {
                Img_Height = this->Def->height;
                Img_Def_Y = 0;
            }

            // Получение позиции def`а.
            _int_ X_Position = Get_X_Position(X_Add);
            _int_ Y_Position = Get_Y_Position(Y_Add);


            // Добавляем рисующуюся фигуру к области перерисовки поля боя, если это стек.
            if (!o_BattleMgr->AddUpdateArea(X_Position, Y_Position, Img_Width, Img_Height)) return;



            // Границы отрисовки.
            _RedrawBorders_* brd = &BattleRedraw_Borders;

            // Огриначиваем размеры изображения.
            if (X_Position + Img_Def_X < brd->Left)
            {
                Img_Width -= brd->Left - X_Position - Img_Def_X;
                Img_Def_X = brd->Left - X_Position;
                X_Position = brd->Left;
            }
            if (Y_Position + Img_Def_Y < brd->High)
            {
                Img_Height -= brd->High - Y_Position - Img_Def_Y;
                Img_Def_Y = brd->High - Y_Position;
                Y_Position = brd->High;
            }
            if (X_Position + Img_Def_X + Img_Width > brd->Right)
            {
                Img_Width = brd->Right - X_Position - Img_Def_X;
            }
            if (Y_Position + Img_Def_Y + Img_Height > brd->Low)
            {
                Img_Height = brd->Low - Y_Position - Img_Def_Y;
            }



            // Отрисовываем текущий кадр фигуры на экран.

            // Если фигура может быть прозрачной, отрисовывае её соответствующей функцией.
            if (this->ImageType == IMG_TRANSPARENT)
            {
                // Обрабатывается HD и так.
                this->Def->Draw_Transparent(this->CurrDefGroup, this->CurrFrame, Img_Def_X, Img_Def_Y, Img_Width, Img_Height,
                    o_WndMgr->screen_pcx16->buffer, X_Position, Y_Position,
                    o_WndMgr->screen_pcx16->width, o_WndMgr->screen_pcx16->height, o_WndMgr->screen_pcx16->scanline_size,
                    this->Get_Reflected(), this->IsTransparent);
            }
            // Если фигура может быть иметь замену спеццвета, отрисовывае её соответствующей функцией.
            else if (this->ImageType == IMG_SPECCOLOR_REPLACE)
            {
                // Поправка для корректной работы с HD.
                if (X_Position < 0)
                {
                    Img_Def_X -= X_Position;
                    Img_Width += X_Position;
                    X_Position = 0;
                }
                if ((X_Position + Img_Width) > 800)
                {
                    Img_Width = 800 - X_Position;
                }

                if (Y_Position < 0)
                {
                    Img_Def_Y -= Y_Position;
                    Img_Height += Y_Position;
                    Y_Position = 0;
                }
                if ((Y_Position + Img_Height) > 600)
                {
                    Img_Height = 600 - Y_Position;
                }

                X_Position += HD_Battle_X;
                Y_Position += HD_Battle_Y;


                // Отрисовка.
                this->Def->Draw_SpecColorReplace(this->CurrDefGroup, this->CurrFrame, Img_Def_X, Img_Def_Y, Img_Width, Img_Height,
                    o_WndMgr->screen_pcx16->buffer, X_Position, Y_Position,
                    o_WndMgr->screen_pcx16->width, o_WndMgr->screen_pcx16->height,
                    o_WndMgr->screen_pcx16->scanline_size, this->Get_Reflected(), this->ColorReplaceSpec);
            }

        }
    }









    // Анимация фигуры.


    // Получение реального времени между кадрами анимации фигуры, с учётом настроек скорости боя.
    inline _int_ GetRealFrameTime()
    {
        // Если время проигрывания кадра зависит от настроек скорости битвы, учитываем это.
        if (this->Depend_On_BattleSpeed)
        {
            return (_int_)(((double)(this->FrameTime)) * BattleAnimPeriodFactors[Settind_BattleFast]);
        }
        // Если время проигрывания кадра не зависит от настроек скорости битвы, берём стандартное время.
        else
        {
            return this->FrameTime;
        }
    }




    // Проигрывание кадра def`а
    // Возвращает, удалилась ли фигура, для фигур с Temporary = TRUE.
    // NeedCycle - нужно ли зациклить анимацию, если закончились кадры.
    inline _bool_ PlayFrame(_bool_ NeedCycle)
    {

        // Если def`а или фигура неактивна нет - ничего не происходит
        if (!this->Def || !this->Enabled) return FALSE;

        // Увеличиваем счётчик кадров.
        this->CurrFrame += this->FrameStep;

        // Надо перерисовывать.
        this->NeedRedraw = TRUE;

        // Если фигура анимирует на стеке, он будет перерисовываться.
        if (this->Get_Side() != NO_SIDE && this->Get_StackNum() != NO_STACK)
        {
            // Только если стек существует, настраиваем его поле обновления.
            if (o_BattleMgr->countMonsters[this->Get_Side()] > this->Get_StackNum())
            {
                o_BattleMgr->Set_Stack_Redrawable(&(o_BattleMgr->stack[this->Get_Side()][this->Get_StackNum()]));
            }
        }

        // Количество кадров def`а.
        _int_ Def_FramesCount;

        // Определям количество кадров def`а.
        if (this->Def->groups_count > this->CurrDefGroup && ((DWORD*)(this->Def->active_groups))[this->CurrDefGroup])
        {
            Def_FramesCount = this->Def->groups[this->CurrDefGroup]->frames_count;
        }
        else
        {
            Def_FramesCount = 0;
        }

        // Если кадры кончились...
        if (this->CurrFrame >= Def_FramesCount || this->CurrFrame < 0)
        {
            // Если фигура была временной - удаляем её.
            if (this->Temporary)
            {
                delete this;
                return TRUE;
            }
            // Если фигура не временная - зацикливам её кадры, если нужно.
            else if (NeedCycle)
            {
                // Если в def`е нет подходящих кадров, обнуляем текущий кадр.
                if (Def_FramesCount <= 0)
                {
                    this->CurrFrame = 0;
                }
                // Если в def`е есть кадры, зацикливаем их.
                else
                {
                    this->CurrFrame = this->CurrFrame % Def_FramesCount;
                    if (this->CurrFrame < 0) this->CurrFrame += Def_FramesCount;
                }
            }
        }

        return FALSE;
    }




    // Один проход проигрывания анимации фигуры, т. е. кадр проигрывается только если наступило время.
    // Возвращает, удалилась ли фигура, для фигур с Temporary = TRUE.
    // NeedCycle - нужно ли зациклить анимацию, если закончились кадры.
    // out_Animed - указатель на переменную, в которую надо вернуть, проанимировала ли фигура (0 - не определять).
    inline _bool_ PlayOnce(_bool_ NeedCycle, _bool_* out_Animed = NULL, _bool_ only_prepare = FALSE)
    {
        // Если фигура неактивна, она не играет анимацию.
        if (!this->Enabled) return FALSE;

        // Текущее время.
        _int_ CurrTime = o_GetTime();

        if (out_Animed) *out_Animed = FALSE;

        // Если наступило время проигрывания кадра, проигрываем его и устанавливаем новое время последней анимации.
        if (CurrTime - this->NextFrameTime >= 0)
        {

            // Надо перерисовывать.
            this->NeedRedraw = TRUE;

            // Если фигура анимирует на стеке, он будет перерисовываться.
            if (this->Get_Side() != NO_SIDE && this->Get_StackNum() != NO_STACK)
            {
                // Только если стек существует, настраиваем его поле обновления.
                if (o_BattleMgr->countMonsters[this->Get_Side()] > this->Get_StackNum())
                {
                    o_BattleMgr->Set_Stack_Redrawable(&(o_BattleMgr->stack[this->Get_Side()][this->Get_StackNum()]));
                }
            }

            this->NextFrameTime = CurrTime + this->GetRealFrameTime();
            if (out_Animed) *out_Animed = TRUE;

            if (!only_prepare)
            {
                return this->PlayFrame(NeedCycle);
            }
        }

        return FALSE;
    }




    // Проигрывание анимации def`а до конца кадров группы (или до начала, если поле FrameStep фигуры отрицательно).
    // Возвращает, удалилась ли фигура, для фигур с Temporary = TRUE.
    // Устананавливает поля Enabled и Depend_On_BattleSpeed фигуры в TRUE на время анимации и возвращает их в исходное состояние после.
    // На время проигрывания анимации бой приостанавливается (как и при любой стандартной анимации).
    // LowFrame - кадр, при доходе до которого или меньше анимация останавливается, ALL_FRAMES - нет (до 0 кадра).
    // HighFrame - кадр, при доходе до большего, чем который, анимация останавливается), ALL_FRAMES - нет (до последнего кадра секции def`а).
    inline _bool_ Play_ToDefGroupEnd(_int_ LowFrame = ALL_FRAMES, _int_ HighFrame = ALL_FRAMES)
    {
        // Если у фигуры нет def`а, не проигрываем.
        if (!this->Def) return FALSE;

        // Количество кадров def`а.
        _int_ Def_FramesCount;


        // Определям количество кадров def`а.
        if (this->Def->groups_count > this->CurrDefGroup && ((DWORD*)(this->Def->active_groups))[this->CurrDefGroup])
        {
            Def_FramesCount = this->Def->groups[this->CurrDefGroup]->frames_count;
        }
        else
        {
            Def_FramesCount = 0;
        }


        // Время подобного проигрывания всегда зависит от настроек скорости боя.
        _bool_ Depend_On_BattleSpeed = this->Depend_On_BattleSpeed;
        this->Depend_On_BattleSpeed = TRUE;

        // На время проигрывания фигура включается.
        _bool_ Enabled = this->Enabled;
        this->Enabled = TRUE;

        // Устанавливаем первое время следующего кадра.
        this->NextFrameTime = o_GetTime() + this->GetRealFrameTime();

        // Проанимировала ли фигура.
        _bool_ Animed = TRUE;

        // Проигрываем анимацию.
        while (this->CurrFrame >= 0 && this->CurrFrame < Def_FramesCount
            && (LowFrame == ALL_FRAMES || this->CurrFrame >= LowFrame) && (HighFrame == ALL_FRAMES || this->CurrFrame < HighFrame))
        {

            // Отрисовываем поле боя и, если фигура проанимировала, ожидаем.
            if (IsHotA)
            {
                o_BattleMgr->ClearRedrawFields();
                o_BattleMgr->PlayWaitAnim();
            }

            // Проигрываем проход для фигуры (прерываемся, если фигура удалена).
            if (this->PlayOnce(FALSE, &Animed)) return TRUE;

            o_BattleMgr->RedrawBattlefield(TRUE, TRUE, TRUE, this->FrameTime, TRUE, Animed);
        }

        // Последний кадр.
        this->CurrFrame -= this->FrameStep;

        // Восстанавливаем зависимость от настроек скорости боя и активность.
        this->Depend_On_BattleSpeed = Depend_On_BattleSpeed;
        this->Enabled = Enabled;

        return FALSE;
    }









    // Проигрывание стандартных боевых анимаций при помощи боевых фигур.



    // Создание фигуры на основе боевой анимации.
    // BattleAnim - адрес структуры нужной боевой анимации.
    // FrameTime - время между кадрами для анимации (без учёта настроек скорости боя).
    // GexNum - номер гекса (должно быть NO_GEX, если анимация не на стеке).
    // Above - должна ли анимация быть поверх всего, вместо общего порядка (для гекса) и должна ли быть поверх обычной анимации (для стека).
    // Side - сторона (не вводить, если анимация на гексе).
    // StackNum - номер стека (не вводить, если анимация на гексе).
    inline static _BattleFigure_* Create_FromBattleAnim(_BattleAnim_* BattleAnim, _int_ FrameTime, _int_ GexNum, _bool_ Above = TRUE, _int_ Side = NO_SIDE, _int_ StackNum = NO_STACK)
    {
        // Порядок отображения.
        _int_ MappingOrder;

        // Порядок отображения для стека.
        if (Side != NO_SIDE && StackNum != NO_STACK)
        {
            if (Above)
            {
                MappingOrder = MPO_ABOVE_STACK;
            }
            else
            {
                MappingOrder = MPO_BELOW_STACK_ANIM;
            }
        }
        // Порядок отображения для гекса.
        else
        {
            if (Above)
            {
                MappingOrder = MPO_ABOVE_ALL;
            }
            else
            {
                MappingOrder = 2;
            }
        }

        return Create(TRUE, TRUE, BattleAnim->DefName, DG_MAIN, 5, 0, 0, SIZE_FULL, SIZE_FULL, FALSE, IMG_TRANSPARENT,
            (BattleAnim->Properties >> 8) & 1, R_NO_COLOR, FrameTime, 0, TRUE, FALSE, 1, TRUE, MappingOrder,
            GexNum, Side, StackNum, BattleAnim->Properties & 0xF, 0, 0, FALSE, FALSE);
    }


    // Создание фигуры на основе боевой анимации стека.
    // BattleAnimNum - номер нужной боевой анимации.
    // FrameTime - время между кадрами для анимации (без учёта настроек скорости боя).
    // Side - сторона.
    // StackNum - номер стека.
    inline static _BattleFigure_* Create_FromBattleAnimStack(_int_ BattleAnimNum, _int_ FrameTime, _int_ Side, _int_ StackNum)
    {
        return Create_FromBattleAnim(&(o_BattleAnimation[BattleAnimNum]), FrameTime, NO_GEX, TRUE, Side, StackNum);
    }

    // Создание фигуры на основе боевой анимации гекса.
    // BattleAnimNum - номер нужной боевой анимации.
    // FrameTime - время между кадрами для анимации (без учёта настроек скорости боя).
    // GexNum - номер гекса.
    // Above - должна ли анимация быть поверх всего, вместо отображения в общем порядке.
    inline static _BattleFigure_* Create_FromBattleAnimGex(_int_ BattleAnimNum, _int_ FrameTime, _int_ GexNum, _bool_ Above)
    {
        return Create_FromBattleAnim(&(o_BattleAnimation[BattleAnimNum]), FrameTime, GexNum, Above, NO_SIDE, NO_STACK);
    }





    // Проигрывание анимации на гексе при помощи боевых фигур.
    // BattleAnimNum - номер нужной боевой анимации.
    // FrameTime - время между кадрами для анимации (без учёта настроек скорости боя).
    // GexNum - номер гекса.
    // Above - должна ли анимация быть поверх всего, вместо отображения в общем порядке.
    // NoRedrawWithout - нужно ли перерисовывать поле боя последний раз без фигуры.
    inline static void PlayGexAnim(_int_ BattleAnimNum, _int_ FrameTime, _int_ GexNum, _bool_ Above, _bool_ NoRedrawWithout)
    {
        // Запускам тактильный эффект анимации.
        CALL_2(void, __fastcall, 0x4B6750, o_BattleAnimation[BattleAnimNum].TouchEffect_Name, 1);

        // Создаём и проигрываем боевую фигуру.
        _BattleFigure_::Create_FromBattleAnimGex(BattleAnimNum, FrameTime, GexNum, Above)->Play_ToDefGroupEnd();

        // Если нужно, перерисовываем поле боя без фигуры.
        if (!NoRedrawWithout)
        {
            o_BattleMgr->RedrawBattlefield(TRUE, FALSE, FALSE, 0, TRUE, FALSE);
        }

    }



    // Проигрывание анимации на стеке при помощи боевых фигур.
    // BattleAnimNum - номер нужной боевой анимации.
    // FrameTime - время между кадрами для анимации (без учёта настроек скорости боя).
    // Side - сторона.
    // StackNum - номер стека.
    inline static void PlayStackAnim(_int_ BattleAnimNum, _int_ FrameTime, _int_ Side, _int_ StackNum)
    {
        // Запускам тактильный эффект анимации.
        CALL_2(void, __fastcall, 0x4B6750, o_BattleAnimation[BattleAnimNum].TouchEffect_Name, 1);

        // Создаём и проигрываем боевую фигуру.
        _BattleFigure_::Create_FromBattleAnimStack(BattleAnimNum, FrameTime, Side, StackNum)->Play_ToDefGroupEnd();
    }


};



// Сообщение о возможности пропуска предбитвенного звука.
extern TString PreBattleSound_SkippingMessage;

// Стандартное сообщение о возможности пропуска предбитвенного звука, если оно не настроенно отдельно.
#define PRE_BATTLE_SOUND_SKIPPING_MESSAGE_STD ""


// Отрисовывается ли сейчас стрелковая башня с Медузой.
extern _bool_ MedusaTower_Drawing;




// Нужно ли обновлять препятствие фона.
extern _bool_ NeedUpdateBackgroundObst;



// Периоды анимации стойки стеков.
extern _int_ StackStayAnimPeriod[2][21];
// Периоды случайной анимации стеков.
extern _int_ StackRandAnimPeriod[2][21];

// Времена последних проигрываний кадров анимации стойки стеков.
extern _int_ StackLastStayAnimTime[2][21];

// Необходимость проигрывания анимацию стойки стеков.
extern _bool_ StackStayAnimNeedPlay[2][21];



// Периоды анимаций флага сторон.
extern _int_ SideFlagAnimPeriod[2];
// Времена последних проигрываний кадров анимации флагов сторон.
extern _int_ SideFlagLastAnimTime[2];


// Периоды анимаций героя сторон.
extern _int_ SideHeroAnimPeriod[2];
// Времена последних проигрываний кадров анимации героя сторон.
extern _int_ SideHeroLastAnimTime[2];


// Периоды случайных анимаций героя сторон.
extern _int_ SideHeroRandAnimPeriod[2];


// Период анимации рамки вокруг стека.
extern _int_ BorderPeriod;
// Время последнего проигрывания рамок вокруг стеков.
extern _int_ BorderLastTime;


// Идёт ли сейчас предбитвенный звук.
//extern _bool_ IsPreBattleSound;


// Текущее время для всей анимации ожидания.
extern _int_ WaitAnimCurrTime;


// Сохранённые значения необходимости обновления сторон при анимации ожидания.
extern _bool8_ WaitAnimBackupSidesRedraw[2];


// Необходимость отрисовывать рамки вокруг стеков при анимации ожидания.
extern _bool_ NeedRedrawBorders;

// Необходимость начинать анимации кривляния стеков при анимации ожидания.
extern _bool_ NeedBeginRandomAnims;


// Был ли последний раунд тактическим (при смене раунда).
extern _bool_ RoundWasTactic;



// Необходимо ли отрисовывать изображения активных элементов поля боя.
extern _bool_ NeedDraw_Active_Elemenst;


// Тукущий кадр луча (выстрел непрерывным прямым лучом).
extern _int_ RayCurrFrame;

// Тукущий максимальный кадр луча (выстрел непрерывным прямым лучом).
extern _int_ RayMaxFrame;

// Закончился ли луч только что.
extern _bool_ RayWasEnded;

// Начальное зерно ГСЧ луча.
extern _dword_ RayStartSeed;

// Сохранённые начальные значения переменных для луча.
extern _dword_ RaySavedVars[10];



// Другой способ модификации луча, оказалось - более медленный.
/*
// Структура линии луча для хранения.
struct RayLn;
NOALIGN struct RayLn
{
  // Количество секций линии луча.
  _int_ SecCount;
  // Ссылка на луч.
  _ptr_ Ray;
};

// Сохранённые линии луча.
extern RayLn RayLns;

// Максимальное количество секций в частях луча.
extern _int_ RayMaxSec;

// Рассчитывать ли продолжение луча сейчас.
extern _bool_ CalcRay;
*/



// Есть расширения HD, из-за которых надо перерисовывать всё поле боя.
extern _bool_ HD_TE_Exists;





// Функция, отрисовывающая изменения для текущего плавного изменения экрана.
extern void (__stdcall* SmoothAnimSpec_Draw)();
// Функция, возвращающая изменения для текущего плавного изменения экрана.
extern void (__stdcall* SmoothAnimSpec_Redo)();
// Функция, откатывающая изменения для текущего плавного изменения экрана.
extern void (__stdcall* SmoothAnimSpec_Undo)();
// Функция, играющая проход анимации для текущего плавного изменения экрана.
extern void (__stdcall* SmoothAnimSpec_Anim)();
// Функция, обновляющая экран для текущего плавного изменения экрана.
extern void (__stdcall* SmoothAnimSpec_Flip)();






// Стек текущего плавного изменения экрана при вызове.
extern _BattleStack_* SmoothAnimSpec_Summon_Stack;

// Стек текущего плавного изменения экрана при телепорте.
extern _BattleStack_* SmoothAnimSpec_Teleport_Stack;
// Номер начального гекса текущего плавного изменения экрана при телепорте.
extern _int_ SmoothAnimSpec_Teleport_StartGexNum;
// Номер целевого гекса текущего плавного изменения экрана при телепорте.
extern _int_ SmoothAnimSpec_Teleport_TargetGexNum;

// Номер препятствия текущего плавного изменения экрана при уничтожении препятствий.
extern _int_ SmoothAnimSpec_RemoveObstacle_ObstacleNum;
// Видимость текущего препятствия.
extern _bool_ SmoothAnimSpec_RemoveObstacle_ObstacleVisible;





// Время между кадрами баллистического взрыва.
extern _int_ BallisticExplFrameTime;

// Время следующей смены кадров баллистического взрыва.
extern _int_ BallisticExplNextTime;




// Время между кадрами огненного шара Магога.
extern _int_ MGFireballFrameTime;

// Время следующей смены кадров огненного шара Магога.
extern _int_ MGFireballNextTime;




// Время между кадрами облака смерти Лича и Могущественного лича.
extern _int_ LichDClFrameTime;

// Время следующей смены кадров облака смерти Лича и Могущественного лича.
extern _int_ LichDClNextTime;




// Время между кадрами армагеддона.
extern _int_ ArmageddonFrameTime;

// Время следующей смены кадров армагеддона.
extern _int_ ArmageddonNextTime;





// Функции, отрисовывающие изменения для текущего удержания нажатия кнопки (последняя - текущая, остальные - родительских диалогов).
extern _List_<void (__stdcall*)()> ButtonWhileClicked_Draw_List;

// Был ли диалог скрытым (как диалоги наложения заклинаний).
extern _bool_ Dlg_WasHidden;



// Атакующий в текущем выстреле.
extern _BattleStack_* CurrShot_Attacker;


// Позиция текущего удаляемого препятствия.
extern _int_ CurrDelObst_Hex_IX;



// Префикс звука особой смерти.
extern _cstr_ SpecDeathSoundName;



// Def большого препятствия в бою.
extern _Def_* LargeObstackleDef;

// Количество кадров в def`е большого препятствия в бою.
extern _int_ LargeObstackleDef_FramesCount;



// Массив больших препятствий.
extern _BattleObstackleLarge_* LargeObstackles;

// Оригинальное количество больших препятствий.
#define LargeObstackles_CountB 34

// Количество больших препятствий.
extern _int_ LargeObstackles_Count;

// Массив обычных препятствий.
extern _BattleObstackleInfo_* Obstackles;

// Оригинальное количество обычных препятствий.
#define Obstackles_CountB 91

// Количество обычных препятствий.
extern _int_ Obstackles_Count;




// Инициализация модуля.
void BattleAnimInit();



// Двигаем таблицу анимациий.
void MoveBattleAnim_Table();




// Очистка поля боя.
void Clear_Battlefield();


// Сохраняем значения перерисоввки.
void SaveBattleRedraws(_BattleMgr_* b_mgr);

// Восстанавливаем значения перерисоввки.
void RestoreBattleRedraws(_BattleMgr_* b_mgr);



// Проигрывание нового кадра анимации.
void PlayNextFrame(_BattleStack_* Stack, int DefGroup);


// Проверка способности стека анимировать.
_bool_ CanStackAnim(_BattleStack_* Stack);


// Функция шага показа анимации стойки.
void StayAnimNext();


// Проверка возможности отрисовки битвы.
_bool_ CanDrawBattle();




// Проигрывание плавного изменения экрана. Изменений не производится, только анимация.
// В случае отсутствия (NULL) функций управления изменение будет отрисовываться стандартно, тогда перед его отрисовкой нужна подготовка.
// Draw_Func - функция отрисовки (без обновления).
// Redo_Func - функция, производящая изменения, к которым нужно прийти плавно.
// Undo_Func - функция, отменяющая изменения Redo_Func.
// Anim_Func - функция, проводящая поход анимации.
// Flip_Func - функция, обновляющая экран (в дополнение к стандартной).
// X_Pos - X-координата изменяемой области экрана.
// Y_Pos - Y-координата изменяемой области экрана.
// Width - ширина изменяемой области экрана.
// Height - высота изменяемой области экрана.
// FrameTime - время между кадрами, если их 8 (общее время/8), -1 - минимальное.
void PlaySmoothAnim(void (__stdcall* Draw_Func)(), void (__stdcall* Redo_Func)(), void (__stdcall* Undo_Func)(),
                    void (__stdcall* Anim_Func)(), void (__stdcall* Flip_Func)(),
                    _int_ X_Pos, _int_ Y_Pos, _int_ Width, _int_ Height, _int_ FrameTime);









// Функция, отрисовывающая изменения для плавного изменения экрана в битве.
void __stdcall SmoothAnimSpec_Battle_Draw();


// Функция, производящая анимацию для плавного изменения экрана в битве.
void __stdcall SmoothAnimSpec_Battle_Anim();


// Функция, производящая анимацию для плавного изменения экрана в битве.
void __stdcall SmoothAnimSpec_Battle_Flip();




// Функция, возвращающая изменения для плавного изменения экрана при исчезновении трупов.
void __stdcall SmoothAnimSpec_RemoveDead_Redo();

// Функция, откатывающая изменения для плавного изменения экрана при исчезновении трупов.
void __stdcall SmoothAnimSpec_RemoveDead_Undo();



// Функция, возвращающая изменения для плавного изменения экрана при вызове.
void __stdcall SmoothAnimSpec_Summon_Redo();

// Функция, откатывающая изменения для плавного изменения экрана при вызове.
void __stdcall SmoothAnimSpec_Summon_Undo();



// Функция, возвращающая изменения для плавного изменения экрана при телепорте.
void __stdcall SmoothAnimSpec_Teleport_Redo();

// Функция, откатывающая изменения для плавного изменения экрана при телепорте.
void __stdcall SmoothAnimSpec_Teleport_Undo();



// Функция, возвращающая изменения для плавного изменения экрана при уничтожении препятствий.
void __stdcall SmoothAnimSpec_RemoveObstacle_Redo();

// Функция, откатывающая изменения для плавного изменения экрана при уничтожении препятствий.
void __stdcall SmoothAnimSpec_RemoveObstacle_Undo();






// Функция, отрисовывающая изменения для текущего удержания нажатия кнопки в бою.
void __stdcall ButtonWhileClicked_Battle_Draw();








// При загрузке и старте звука делаем его параллельным.
_Sample_ __stdcall HookOn_Load_Start_Sample(HiHook* h, _cstr_ Name);


// При загрузке и старте звука предбитвенного звука загружаем и начинаем его как обычно.
_Sample_ __stdcall HookOn_Load_Start_PreBattle_Sample(HiHook* h, _cstr_ Name);


// Пропуск стандартного окончания звука.
void __stdcall HookOn_End_Sample_Std(_Sample_ Sample);


// При ожидании и окончании проигрывания звука пропускаем это для всех звуков, кроме предбитвенного.
void __stdcall HookOn_Wait_End_Close_Sample(HiHook* h, int Time, _Sample_ Sample);

// При ожидании и окончании проигрывания предбитвенного звука...
void __stdcall HookOn_Wait_End_Close_PreBattleSample(HiHook* h, int Time, _Sample_ Sample);

// При расчёте времени проигрывании звука в бою учитываем его настройки скорости.
int __stdcall HookOn_Wait_End_Close_Sample_CalcTime(LoHook* h, HookContext* c);

// При ожидании и окончании звука в бою также отрисовываем анимацию.
int __stdcall HookOn_Wait_End_Close_Sample_Play(LoHook* h, HookContext* c);


// При ожидании проигрывания звука пропускаем его.
void __stdcall HookOn_Wait_Sample(HiHook* h, _ptr_ SoundMgr, _dword_ SampleInd, int Time);

// При расчёте времени ожидания звука в бою учитываем его настройки скорости.
int __stdcall HookOn_Wait_Sample_CalcTime(LoHook* h, HookContext* c);

// При ожидании звука в бою также отрисовываем анимацию.
int __stdcall HookOn_Wait_Sample_Play(LoHook* h, HookContext* c);


// При инициализации интерфейса битвы отключаем возможность тактического режима, чтобы кнопки не закрывали окно лога.
DWORD __stdcall HookOn_PreBattle_InterfaceInit(HiHook* h, DWORD a1, _byte_ IsTactiс);

// Позже инициализируем интерфейс как надо и инициализируем времена случайной анимации.
int __stdcall HookOn_LaterPreBattle_InterfaceInit(LoHook* h, HookContext* c);



// Вместо загрузки некоторых звуков возвращаем их имя.
_cstr_ __fastcall HookOn_Some_LoadWav(_cstr_ Name);

// Вместо старта некоторых звуков по ссылке на них запускаем их распараллеленно по имени.
_dword_ __stdcall HookOn_Some_StartSound(HiHook* h, _ptr_ SoundMgr, _cstr_ Name);





// При вызове функции перехода к следующему раунду при тактической фазе сохраняем то, что это была тактическая фаза.
void __stdcall HookOn_Battle_NextRoundTactic(HiHook* h, _BattleMgr_* this_);

// При добавлении в лог информации о следующем раунде учитываем тактическую фазу.
int __stdcall HookOn_BattleNextRoundLog(LoHook* h, HookContext* c);







// Перед отрисовкой башни проверяем существо на ней на Медузу и запоминаем результат проверки.
int __stdcall HookOn_Battle_DrawTower_Check_Medusa(LoHook* h, HookContext* c);

// После отрисовки башни стираем результат проверки на Медузу.
int __stdcall HookOn_Battle_DrawTower_UnCheck_Medusa(LoHook* h, HookContext* c);

// При отрисовке башни учитываем возможность нахождения на ней Медузы.
void __stdcall HookOn_Battle_DrawTower_Height(HiHook* h, _Def_* CrDef, _int32_ AnimSection, _int32_ AnimFrame, DWORD a4, DWORD a5, DWORD a6, DWORD Height, DWORD a8, _int32_ X_Pos, _int32_ Y_Pos, DWORD a11, DWORD a12, DWORD a13, _int32_ Orientation, DWORD a15);







// При вызове функции отрисовки тени перемещения стека вне общей отрисовки для стёрки...
_bool32_ __stdcall HookOn_BattleMShadowDraw(HiHook* h, _BattleMgr_* this_, _dword_ a1, _dword_ a2);





// При выделении стека учитываем то, что он может быть неспособен анимировать.
int __stdcall HookOn_BattleSelectStack(LoHook* h, HookContext* c);

// При выделении стека площадным заклинанием учитываем то, что он может быть неспособен анимировать.
int __stdcall HookOn_BattleSpellSelectStack(LoHook* h, HookContext* c);




// При инициализации битвы...
_bool32_ __stdcall HookOn_BattleInit(HiHook* h, _BattleMgr_* this_, DWORD a2);

// При инициализации графики героя в бою...
int __stdcall HookOn_BattleHeroInitDef(LoHook* h, HookContext* c);

// При инициализации начальных времён участников битвы добавляем случайности к случайной анимации героя.
int __stdcall HookOn_BattleInitTimes(LoHook* h, HookContext* c);

// При инициализвации стека прописываем ему параметры анимации стойки.
void __stdcall HookOn_StackInit(HiHook* h, _BattleStack_* this_, int creature_id, int count, _Hero_* hero_owner, int side, int index_on_side, int position_hex_ix,  int army_slot_ix);






// При проигрывании анимации ожидания в бою...
void __stdcall HookOn_WaitAnimDraw(HiHook* h, _BattleMgr_* this_);


// При стандартном проигрывании анимации ожидания в бою...
void __stdcall HookOn_WaitAnimDrawStd(HiHook* h, _BattleMgr_* this_);




// Не отрисовываем невидимые стеки.
void __stdcall HookOn_BattleStack_Draw(HiHook* h, _BattleStack_* this_, _int_ X_Pos, _int_ Y_Pos, _bool_ OnlyCalcBorders);





// При выборе цвета рамки вокруг стека не отрисовываем её, когда этого не требуется.
int __stdcall Hook_StackDraw_ChoseBorderColor(LoHook* h, HookContext* c);

// При выборе цвета рамки вокруг существа стрелковой башни не отрисовываем её, когда этого не требуется.
int __stdcall Hook_ArrowTower_Creature_Draw_ChoseBorderColor(LoHook* h, HookContext* c);





// При подсчёте количества кадров анимации при действии отрисовки учитываем защитную стойку (недочёт SoD).
int __stdcall Hook_OnDrawActPlay_CalcFrames_DefendPos(LoHook* h, HookContext* c);

// При проигрывании анимации при действии отрисовки учитываем защитную стойку (недочёт SoD).
int __stdcall Hook_OnDrawActPlay_Play_DefendPos(LoHook* h, HookContext* c);

// При проигрывании кадра выстрела анимации при действии отрисовки пропускаем кадры стойки и кривляния.
int __stdcall Hook_OnDrawActPlay_DrawNewShotFrame(LoHook* h, HookContext* c);

// При проигрывании кадра анимации при действии отрисовки пропускаем кадры стойки и кривляния.
int __stdcall Hook_OnDrawActPlay_DrawNewFrame(LoHook* h, HookContext* c);

// При проигрывании кадра доигрывающейся анимации при действии отрисовки пропускаем кадры стойки и кривляния.
int __stdcall Hook_OnDrawActPlay_LastDrawNewFrame(LoHook* h, HookContext* c);

// После проигрывания последнего кадра основной анмации отключаем боевую анимацию.
int __stdcall Hook_OnDrawActPlay_EndBAnim(LoHook* h, HookContext* c);




// При ожидании определённого времени в бою прокручиваем анимацию ожидания.
void __stdcall WaitForTime_Draw(HiHook* h, _int32_ time);


// Перед отрисовкой...
void __stdcall HookOn_Battle_Draw(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait);

// При необходимости пропускаем отрисовку активной части поля боя.
int __stdcall Hook_OnDraw_NeedDraw_ActivePart(LoHook* h, HookContext* c);

// При отрисовке, параллельной действиям боя с отрисовкой рамок стеков.
void __stdcall HookOn_Battle_ParallelDraw_Borders(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait);

// Перед нужной отрисовкой проигрываем анимацию ожидания.
void __stdcall HookOn_BattleDraw_WaitAnim(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait);

// Перед нужной отрисовкой проигрываем анимацию ожидания.
void __stdcall HookOn_BattleDraw_WaitAnim_ClearRedraws(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait);

// Пропускаем отрисовку.
void __stdcall HookOn_BattleDraw_Skip(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait);

// Мгновенный проигрыш анимаций ожидания и отрисовка с обновлением.
int __stdcall HookOn_Draw_WaitAnim_Low_Redraw(LoHook* h, HookContext* c);




// При задержке между кадрами...
void __stdcall HookOn_FramesDraw_Delay(HiHook* h, _int32_ Time);





// Определение первого времени следующей смены кадров снаряда.
int __stdcall HookOn_BulletDraw_InitTime(LoHook* h, HookContext* c);

// При смене кадров отрисовки полёта снаряда...
int __stdcall HookOn_BulletDraw_FrameChange(LoHook* h, HookContext* c);





// Определение первого времени следующей смены кадров снаряда стрелковой башни.
int __stdcall HookOn_ArrowTower_BulletDraw_InitTime(LoHook* h, HookContext* c);

// При смене кадров отрисовки полёта снаряда стрелковой башни...
int __stdcall HookOn_ArrowTower_BulletDraw_FrameChange(LoHook* h, HookContext* c);





// Инициализация луча.
int __stdcall HookOn_RayInit(LoHook* h, HookContext* c);

// Инициализация каждой отрисовки луча.
int __stdcall HookOn_RayDrawingInit(LoHook* h, HookContext* c);

// При переходе к следующему кадру при отрисовке луча...
int __stdcall HookOn_RayNextFrame(LoHook* h, HookContext* c);

// При окончании отрисовки луча...
int __stdcall HookOn_RayEnd(LoHook* h, HookContext* c);



// Другой способ модификации луча, оказалось - более медленный.
/*
// Инициализация луча.
int __stdcall HookOn_RayInitSD(LoHook* h, HookContext* c);

// При переходе к следующей части луча сохраняем её.
int __stdcall HookOn_RayNewSecSD(LoHook* h, HookContext* c);

// При переходе к следующему кадру при отрисовке луча...
int __stdcall HookOn_RayNextFrameSD(LoHook* h, HookContext* c);


// При удалении данных о луче...
int __stdcall HookOn_RayDestructSD(LoHook* h, HookContext* c);
*/



// Определение первого времени следующей смены кадров снаряда-заклинания.
int __stdcall HookOn_SpellBulletDraw_InitTime(LoHook* h, HookContext* c);

// При смене кадров отрисовки полёта снаряда-заклинания...
int __stdcall HookOn_SpellBulletDraw_FrameChange(LoHook* h, HookContext* c);




// Определение первого времени следующей смены кадров баллистического снаряда.
int __stdcall HookOn_BallisticBulletDraw_InitTime(LoHook* h, HookContext* c);

// При смене кадров отрисовки полёта баллистического снаряда...
int __stdcall HookOn_BallisticBulletDraw_FrameChange(LoHook* h, HookContext* c);





// Инициализация времени взрыва баллистического выстрела.
int __stdcall HookOn_BallisticExplDraw_InitTime(LoHook* h, HookContext* c);

// При отрисовке взрыва баллистического выстрела.
int __stdcall HookOn_BallisticExplDraw(LoHook* h, HookContext* c);

// При отрисовке def`а взрыва баллистического выстрела.
void __stdcall HiHook_BallisticExpl_BlitDef(HiHook* h, _Def_* this_, _int32_ group_ix, _int32_ frame_ix, _int32_ def_x, _int32_ def_y, _int32_ img_width, _int32_ img_height, _ptr_ pcx_buffer,
                      _int32_ dest_x, _int32_ dest_y, _int32_ dest_width, _int32_ dest_height,
                      _dword_ scanline_size, _bool32_ reflected, _bool8_ use_spec_colors);







// Добавление зависимости максимальной скорости от настроек скорости боя.
int __stdcall HookOn_SmoothImageChangeDraw_MinFrameTime(LoHook* h, HookContext* c);

// Определение первого времени следующей смены кадров при плавном изменении изображения.
int __stdcall HookOn_SmoothImageChangeDraw_InitTime(LoHook* h, HookContext* c);

// При смене кадров при плавном изменении изображения...
int __stdcall HookOn_SmoothImageChangeDraw_FrameChange(LoHook* h, HookContext* c);

// При задержке между кадрами при плавном изменении изображения...
void __stdcall HookOn_SmoothImageChangeDraw_FramesDraw_Delay(HiHook* h, _int32_ Time);






// При анимации исчезновения трупов.
void __stdcall HiHook_RemoveDeadDraw(HiHook* h, _BattleMgr_* this_);



// При отрисовке вызова стека.
int __stdcall HookOn_SummonDraw(LoHook* h, HookContext* c);



// При отрисовке телепорта.
int __stdcall HookOn_TeleportDraw(LoHook* h, HookContext* c);

// При отрисовке прямоурольника, отображающего количество существ.
int __stdcall HookOn_DrawStackRectShowingCount(LoHook* h, HookContext* c);



// Подготавливаем плавное исчезновение объекта.
int __stdcall HookOn_RemoveObstacleDraw_Prepare(LoHook* h, HookContext* c);

// При плавном исчезновеним объекта.
void __stdcall HookOn_RemoveObstacleDraw(HiHook* h, _WndMgr_* this_, _int_ X_Pos, _int_ Y_Pos, _int_ Width, _int_ Height, _int_ FrameTime);

// Делаем возможной невидимость препятсвия (1).
int __stdcall HookOn_RemoveObstacleDraw_Invisible1(LoHook* h, HookContext* c);

// Делаем возможной невидимость препятсвия (2).
int __stdcall HookOn_RemoveObstacleDraw_Invisible2(LoHook* h, HookContext* c);




// Инициализация отрисовки движения.
void __stdcall HiHook_StackMoveDraw_Init(HiHook* h, _BattleStack_* this_, _int_ def_group, _int_ frames_count, _int_ start_frame);

// При смене кадров отрисовки движения стека...
int __stdcall HookOn_StackMoveDraw_FrameChange(LoHook* h, HookContext* c);




// При инициализации координат отрисовки полёта стека...
int __stdcall HookOn_StackFlightDraw_InitCoords(LoHook* h, HookContext* c);

// При задержке между кадрами при полёте...
int __stdcall HookOn_FlightFramesDraw_Delay(LoHook* h, HookContext* c);

// При смене кадров отрисовки полёта стека...
int __stdcall HookOn_StackFlightDraw_FrameChange(LoHook* h, HookContext* c);




// При сбросе анимации стека при открытии моста при движении...
int __stdcall HookOn_StackAnim_MoveBridgeOpenReset(LoHook* h, HookContext* c);

// При сбросе анимации стека после заклинания...
int __stdcall HookOn_StackAnim_AfterSpellReset(LoHook* h, HookContext* c);

// При сбросе анимации стека перед действием...
int __stdcall HookOn_StackAnim_BeforeActionReset(LoHook* h, HookContext* c);

// При сбросе анимации стека после действия...
int __stdcall HookOn_StackAnim_AfterActionReset(LoHook* h, HookContext* c);

// При сбросе анимации стека при открытии моста перед полётом...
int __stdcall HookOn_StackAnim_BeforeFlightBridgeOpenReset(LoHook* h, HookContext* c);

// При общем сбросе анимации стеков...
int __stdcall HookOn_StackAnimReset(LoHook* h, HookContext* c);



// Заменяем способ проигрывания анимации гекса.
void __stdcall HookOn_GexAnim(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast);

// Заменяем способ проигрывания анимации гекса там, где надо проиграть на конкретном месте (а не поверх всего).
void __stdcall HookOn_GexAnimNotAboveAll(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast);


// Пропускаем анимацию на гексе.
void __stdcall HookOn_GexSkip(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast);

// Появление стены огня.
int __stdcall LoHook_FirewallAppearAnim(LoHook* h, HookContext* c);



// Инициализация управления временем при отрисовке тряски землетрясения.
int __stdcall HookOn_Earthquake_Effect_InitTime(LoHook* h, HookContext* c);

// Перерисовка при отрисовке тряски землетрясения.
int __stdcall HookOn_Earthquake_Effect(LoHook* h, HookContext* c);

// Инициализация времени взрыва землетрясения.
int __stdcall HookOn_Earthquake_Expl_InitTime(LoHook* h, HookContext* c);

// Перерисовка для взрыва землетрясения.
int __stdcall HookOn_Earthquake_Expl_Draw(LoHook* h, HookContext* c);

// Добавляем взрывы в границы перерисовки, вместо обновления экрана.
int __stdcall HookOn_Earthquake_Expl_FlipBrd(LoHook* h, HookContext* c);

// Обновляем экран при отрисовке взрыва при землетрясении.
int __stdcall HookOn_Earthquake_Expl_Flip(LoHook* h, HookContext* c);




// Инициализация времени огненного шара Магога.
int __stdcall HookOn_MGFireballDraw_InitTime(LoHook* h, HookContext* c);

// При отрисовке огненного шара Магога.
int __stdcall HookOn_MGFireballDraw(LoHook* h, HookContext* c);



// Инициализация времени облака смерти Лича и Могущественного лича.
int __stdcall HookOn_LichDClDraw_InitTime(LoHook* h, HookContext* c);

// При отрисовке облака смерти Лича и Могущественного лича.
int __stdcall HookOn_LichDClDraw(LoHook* h, HookContext* c);



// Инициализация времени армагеддона.
int __stdcall HookOn_ArmageddonDraw_InitTime(LoHook* h, HookContext* c);

// При отрисовке армагеддона.
int __stdcall HookOn_ArmageddonDraw(LoHook* h, HookContext* c);

// При отрисовке при ожидании выбора целей некоторых заклинаний (телепорта и жертвы).
_dword_ __stdcall HookOn_SpellSelectTagetsWaitingDraw(HiHook* h, _int_ Param);






// Перед боем инициализируем функцию отрисовки нажатия кнопки.
void __stdcall HookOn_Battle_Start_ButtonClickDraw(HiHook* h, _BattleMgr_* this_, _dword_ a2);


// После сообщения о конце боя деинициализируем функцию отрисовки нажатия кнопки.
void __stdcall HookOn_Battle_EndMessage_ButtonClickDraw(HiHook* h, _BattleMgr_* this_, _dword_ a2);






// При вызове диалога обнуляем функцию отрисовки удержания нажатия кнопки для него.
_dword_ __stdcall HookOn_Show_Dialog(HiHook* h, _WndMgr_* this_, _dword_ a2, _int_ (__fastcall* DalogFunc)(_DlgMsg_* Msg), _dword_ a4);

// При отрисовке скрытого диалога включаем отсутствие учёта его существования.
_dword_ __stdcall HookOn_Show_HiddenDialog(HiHook* h, _WndMgr_* this_, _dword_ a2, _int_ (__fastcall* DalogFunc)(_DlgMsg_* Msg), _dword_ a4);


// Отрисовка при удержании нажатия на кнопку.
int __stdcall HookOn_WhileButtonClickedDraw(LoHook* h, HookContext* c);




// При стёрке окна предпросмотра стека или героя перерисовывам участок поля боя.
int __stdcall LoHook_HidePreviewImage_Redraw(LoHook* h, HookContext* c);




// Перед отрисовкой выстрела забираем информацию об атакующем.
void __stdcall Hook_On_BattleStack_DrawShot_GetInfo(HiHook* h, _BattleStack_* this_, _BattleStack_* target);


// Перед отрисовкой выстрела стрелковой башни забираем информацию об атакующем.
int __stdcall HookOn_BattleStack_DrawArrowTowerShot_GetInfo(LoHook* h, HookContext* c);


// При отрисовке выстрела добавлям взрыв снаряда.
void __stdcall Hook_On_BattleStack_DrawShot_Expl(HiHook* h, _BattleMgr_* this_, _int_ anim_id, _bool8_ need_clear_redraws);





// При настройке анимации смерти стека учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_DrawActionPlay_SetSpecDeath(LoHook* h, HookContext* c);

// При настройке количества кадров анимации смерти стека учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_DrawActionPlay_SpecDeath_FramesCount(LoHook* h, HookContext* c);

// При проверке анимации стека учитываем то, что он мог умереть особой смертью (1).
int __stdcall HookOn_DrawActionPlay_SpecDeath_Check1(LoHook* h, HookContext* c);

// При проверке анимации стека учитываем то, что он мог умереть особой смертью (2).
int __stdcall HookOn_DrawActionPlay_SpecDeath_Check2(LoHook* h, HookContext* c);

// При проверке анимации стека учитываем то, что он мог умереть особой смертью (3).
int __stdcall HookOn_DrawActionPlay_SpecDeath_Check3(LoHook* h, HookContext* c);


// При настройке количества кадров анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_Armageddon_SpecDeath_FramesCount(LoHook* h, HookContext* c);

// При настройке анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью (1).
int __stdcall HookOn_Armageddon_SetSpecDeath1(LoHook* h, HookContext* c);

// При настройке анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью (2).
int __stdcall HookOn_Armageddon_SetSpecDeath2(LoHook* h, HookContext* c);


// При настройке анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_MassSpell_SetSpecDeath(LoHook* h, HookContext* c);

// При настройке количества кадров анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью (1).
int __stdcall HookOn_MassSpell_SpecDeath_FramesCount1(LoHook* h, HookContext* c);

// При настройке количества кадров анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью (2).
int __stdcall HookOn_MassSpell_SpecDeath_FramesCount2(LoHook* h, HookContext* c);


// Учитываем особую смерть при воскрешении.
int __stdcall LoHook_SpecDeath_Resurrect(LoHook* h, HookContext* c);


// Во время нанесения урона (для магической атаки).
int __stdcall HookOn_MakeMagicDamage(HiHook* h, _BattleStack_* this_, _int_ damage);


// При уничтожении препятсвия убираем его время жизни, дабы избежать его повтороного удаления (баг SoD).
void __stdcall HookOn_RemoveObstackle(HiHook* h, _BattleMgr_* this_, _int_ obstackle_ix);



// При заклинании уничтожения препятствия запоминаем его гекс.
int __stdcall LoHook_RemoveObstackle_GetHex(LoHook* h, HookContext* c);

// Заменяем способ проигрывания анимации уничтожения магического препятствия.
void __stdcall HookOn_RemoveObstackleMagic_Anim(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast);








// При постановке стека на гекс после полёта предварительно убираем его с предыдущих гексов, если надо.
void __stdcall HiHook_AfterFly_ChangeStackPosition(HiHook* h, _BattleMgr_* this_, _BattleStack_* stack, _int32_ tar_hex_ix);




// Добавляем звук взрыва волшебной стреле.
int __stdcall LoHook_MagicArrow_ExplSound(LoHook* h, HookContext* c);

// Добавляем звук взрыва волшебной стреле - конец.
int __stdcall LoHook_MagicArrow_ExplSoundEnd(LoHook* h, HookContext* c);
