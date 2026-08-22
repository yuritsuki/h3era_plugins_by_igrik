#include "CombatFixes.h"
#define  o_GENRLTXT_Txt_ o_GENRLTXT_TXT

//#include "HotA_MonstersTable.h"
//#include "Hota_HeroClasses.h"
// #include "HotA_ids.h"
// Проверка возможности отрисовки битвы.
_bool_ CanDrawBattle()
{
    if (*(DWORD*)((DWORD)o_BattleMgr + 78584) || o_BattleMgr->ShouldNotRenderBattle() || !*(DWORD*)((DWORD)o_BattleMgr + 78592))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}



// Длительность активности последнего диалога.
_int_ Battle_LastDialog_Time = 0;


// Включено ли упрощённое удаление боевых фигур.
_bool_ Fgrs_Simple_Destruct = FALSE;


// Список всех существующих боевых фигур на заднем плане.
_List_<_BattleFigure_*> BattleFgrs_Background;

// Список всех существующих боевых фигур на гексах.
_List_<_BattleFigure_*> BattleFgrs_Gexes[BATTLE_HEXES_COUNT];

// Список всех существующих боевых фигур на стеках.
_List_<_BattleFigure_*> BattleFgrs_Stacks[2][BATTLE_SIDE_STACKS_COUNT];

// Список всех существующих боевых фигур на переднем плане.
_List_<_BattleFigure_*> BattleFgrs_Foreground;






// Текущая отрисовывающаяся стрелковая башня.
_ArrowTower_* CurrDrawingTower;





// Текущая добавка к X-координате стека при движении или полёте.
_int_ Curr_Stack_Moving_X_Add;

// Текущая добавка к Y-координате стека при движении или полёте.
_int_ Curr_Stack_Moving_Y_Add;



// Период анимации стойки существ.
DWORD* StayAnimPeriod = 0;

// Период случайной анимации существ.
DWORD* RandAnimPeriod = 0;

// Случайное отклонение времени между кадрами анимации стойки от настроенного значения.
double* StayAnim_Rand_Devi = 0;

// Необходимость проигрывать анимацию стойки.
_bool_* StayAnimNeedPlay = 0;

// Номера анимаций взрыва снаряда существ.
_int_* BulletExplAnim_ID = 0;

// Имена звуков взрывов снарядов существ.
_cstr_* BulletExpl_SoundName = 0;

// Количество жилищ каждого существа.
_int_* Creatures_DwellingsCount = 0;





// Инициализация модуля боевых фигур.
void BattleFigures_Init()
{
    // При начале отрисовки отрисовываем боевые фигуры заднего плана.
    _PI->WriteLoHook(0x494145, HookOn_Fgr_DrawBegin);

    // При отрисовке порядка на гексе отрисовываем соответствующие боевые фигуры.
    _PI->WriteLoHook(0x49455E, HookOn_Fgr_GexesDraw);

    // В конце отрисовки отрисовываем боевые фигуры переднего плана.
    _PI->WriteLoHook(0x494624, HookOn_Fgr_DrawEnd);



    // Перед отрисовкой стека проигрываем соответствующие боевые фигуры.
    _PI->WriteLoHook(0x43E210, HookOn_Fgr_StackBeforeBlit);

    // После отрисовки стека проигрываем соответствующие боевые фигуры.
    _PI->WriteLoHook(0x43E255, HookOn_Fgr_StackAfterBlit);

    // После отрисовки анимации стека проигрываем соответствующие боевые фигуры.
    _PI->WriteLoHook(0x43E6DC, HookOn_Fgr_StackAfterBlitAnims);



    // Перед отрисовкой стрелковой башни обнуляем ссылку на неё.
    _PI->WriteLoHook(0x494982, HookOn_Fgr_TowerCreatureBeforeDraw);

    // Перед отрисовкой существа стрелковой башни проигрываем соответствующие боевые фигуры.
    _PI->WriteLoHook(0x494A76, HookOn_Fgr_TowerCreatureBeforeBlit);

    // После отрисовки существа стрелковой башни проигрываем соответствующие боевые фигуры.
    _PI->WriteLoHook(0x494A86, HookOn_Fgr_TowerCreatureAfterBlit);


    // При настройке границ перерисовки боя настраиваем границы перерисовки не принадлежащих стекам фигур.
    _PI->WriteHiHook(0x495770, SPLICE_, EXTENDED_, THISCALL_, HookOn_Battle_SettingRedrawBorders);


    // Рассчитываем добавки к координатам отрисовки стека.
    _PI->WriteLoHook(0x43DF45, HookOn_StackDraw_Calcs_PosAdds_For_Moving);



    // Перед боем очищаем списки боевых фигур.
    _PI->WriteHiHook(0x462600, SPLICE_, EXTENDED_, THISCALL_, HookOn_Battle_Start_Fgrs);

    // После сообщения о конце боя очищаем списки боевых фигур.
    _PI->WriteHiHook(0x475CFD, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_EndMessage_Fgrs);



    // При проигрывании анимации ожидания в бою проигрываем параллельную анимацию боевых фигур.
    // _PI->WriteLoHook(0x495CFF, HookOn_WaitAnim_Fgrs); - делается в другом месте.

    // Очищаем перерисовки боевых фигур (inline-подстановка всего одна, и та стирается в другом месте).
    _PI->WriteHiHook(0x493290, SPLICE_, EXTENDED_, THISCALL_, HookOn_BattleClearRedraws);

}






// Корректен ли номер гекса.
_bool_ Gex_IsCorrect(_int_ GexNum)
{
    return ((GexNum >= 0 && GexNum < BATTLE_HEXES_COUNT) || GexNum == NO_GEX);
}


// Корректен ли номер стека.
_bool_ Stack_IsCorrect(_int_ Side, _int_ StackNum)
{
    return ((Side == ATTACKER || Side == DEFENDER || Side == NO_SIDE) && ((StackNum >= 0 && StackNum < BATTLE_SIDE_STACKS_COUNT) || StackNum == NO_STACK));
}


// Корректен ли порядок отображения.
_bool_ MappingOrder_IsCorrect(_int_ MappingOrder)
{
    return ((MappingOrder >= 0 && MappingOrder < BATTLE_MAPPING_PRIORITIES_COUNT)
        || MappingOrder == MPO_BELOW_ALL || MappingOrder == MPO_ABOVE_ALL
        || MappingOrder == MPO_BELOW_STACK || MappingOrder == MPO_BELOW_STACK_ANIM || MappingOrder == MPO_ABOVE_STACK);
}





// Определение списка по определяющим параметрам.
_List_<_BattleFigure_*>* DetermineList_By_FigureParams(_int_ GexNum, _int_ MappingOrder, _int_ Side, _int_ StackNum)
{
    // Если стек корректный и существующий, берём список по нему.
    if (Stack_IsCorrect(Side, StackNum) && Side != NO_SIDE && StackNum != NO_STACK)
    {
        return &(BattleFgrs_Stacks[Side][StackNum]);
    }
    // Если стека нет, но есть корректный гекс и порядок отображения, берём список по гексу.
    else if (Gex_IsCorrect(GexNum) && GexNum != NO_GEX && MappingOrder >= 0 && MappingOrder < BATTLE_MAPPING_PRIORITIES_COUNT)
    {
        return &(BattleFgrs_Gexes[GexNum]);
    }
    // Если нет привязок и отображение подо всем - берём соответствующий список.
    else if (MappingOrder == MPO_BELOW_ALL)
    {
        return &BattleFgrs_Background;
    }
    // Если отображение надо всем или нет привязки к гексу и стеку, но есть порядок отображения на гексе...
    else
    {
        return &BattleFgrs_Foreground;
    }
}





// Удаление всех фигур из списка.
void Delete_Figures_From_List(_List_<_BattleFigure_*>* List)
{
    // Включаем упрощённое удаление боевых фигур.
    Fgrs_Simple_Destruct = TRUE;

    // Вызываем деструкторы всех элементов списка.
    for (_int_ i = 0; i < List->GetItemsCount(); i++)
    {
        delete (*List)[i];
    }

    // Выключаем упрощённое удаление боевых фигур.
    Fgrs_Simple_Destruct = FALSE;

    // Очищаем список.
    List->DeleteAll();
}





// Очищение списков боевых фигур.
void Delete_All_Battle_Figures()
{
    // Очищаем список фигур заднего плана.
    Delete_Figures_From_List(&BattleFgrs_Background);

    // Очищаем списки фигур гексов.
    for (_int_ i = 0; i < BATTLE_HEXES_COUNT; i++)
    {
        Delete_Figures_From_List(&(BattleFgrs_Gexes[i]));
    }

    // Очищаем списки фигур стеков.
    for (_int_ i = ATTACKER; i <= DEFENDER; i++)
    {
        for (_int_ j = 0; j < BATTLE_SIDE_STACKS_COUNT; j++)
        {
            Delete_Figures_From_List(&(BattleFgrs_Stacks[i][j]));
        }
    }

    // Очищаем список фигур переднего плана.
    Delete_Figures_From_List(&BattleFgrs_Foreground);
}











// Отрисовка  боевых фигур в текущем режиме (эта функция должна быть вызвана при каждом режиме).
void BattleFgrs_Draw(_int_ GexNum, _int_ MappingOrder, _int_ Side, _int_ StackNum, _int_ X_Add, _int_ Y_Add)
{
    // Определяем текущий список фигур.
    _List_<_BattleFigure_*>* FgrsList = DetermineList_By_FigureParams(GexNum, MappingOrder, Side, StackNum);


    // Проходим по всем боевым фигурам текущего списка.
    for (_int_ i = 0; i < FgrsList->GetItemsCount(); i++)
    {
        // Отрисовываем фигуру, если сейчас её очередь.
        if ((*FgrsList)[i]->Get_MappingOrder() == MappingOrder) (*FgrsList)[i]->Draw(X_Add, Y_Add);
    }
}







// Отрисовка или настройка границ отображения для фигур в списке.
void BattleFgrs_List_Draw(_List_<_BattleFigure_*>* List, _bool_ only_redrawed)
{
    // Проходим по всем фигурам списка.
    for (_int_ i = 0; i < List->GetItemsCount(); i++)
    {
        // Настраиваем границы отображения.
        (*List)[i]->Draw();
    }
}





// Отрисовка или настройка границ отображения для нестековых фигур, не принадлежащих стекам.
void BattleFgrs_Draw_NoStacks(_bool_ only_redrawed)
{
    // Настройка границ отображения списка фигур заднего плана.
    BattleFgrs_List_Draw(&BattleFgrs_Background, only_redrawed);

    // Настройка границ отображения списков фигур гексов.
    for (_int_ i = 0; i < BATTLE_HEXES_COUNT; i++)
    {
        BattleFgrs_List_Draw(&(BattleFgrs_Gexes[i]), only_redrawed);
    }

    // Настройка границ отображения списков фигур переднего плана.
    BattleFgrs_List_Draw(&BattleFgrs_Foreground, only_redrawed);
}












// Восстанавливаем перерисовку фигур из списка.
void BattleFgrs_List_RestoreRedraw(_List_<_BattleFigure_*>* List)
{
    // Проходим по всем фигурам списка с конца.
    for (_int_ i = List->GetItemsCount() - 1; i >= 0; i--)
    {
        // Восстанавливаем перерисовку фигуры.
        (*List)[i]->NeedRedraw = (*List)[i]->NeedRedrawSaved;
    }
}



// Восстанавливаем перерисовку всех боевых фигур.
void BattleFgrs_RestoreRedraw()
{
    // Восстанавливаем перерисовку фигур заднего плана.
    BattleFgrs_List_RestoreRedraw(&BattleFgrs_Background);

    // Восстанавливаем перерисовку фигур гексов.
    for (_int_ i = 0; i < BATTLE_HEXES_COUNT; i++)
    {
        BattleFgrs_List_RestoreRedraw(&(BattleFgrs_Gexes[i]));
    }

    // Восстанавливаем перерисовку фигур стеков.
    for (_int_ i = ATTACKER; i <= DEFENDER; i++)
    {
        for (_int_ j = 0; j < BATTLE_SIDE_STACKS_COUNT; j++)
        {
            BattleFgrs_List_RestoreRedraw(&(BattleFgrs_Stacks[i][j]));
        }
    }

    // Восстанавливаем перерисовку фигур переднего плана.
    BattleFgrs_List_RestoreRedraw(&BattleFgrs_Foreground);
}



// Сохраняем перерисовку фигур из списка.
void BattleFgrs_List_SaveRedraw(_List_<_BattleFigure_*>* List)
{
    // Проходим по всем фигурам списка с конца.
    for (_int_ i = List->GetItemsCount() - 1; i >= 0; i--)
    {
        // Сохраняем перерисовку фигуры.
        (*List)[i]->NeedRedrawSaved = (*List)[i]->NeedRedraw;
    }
}



// Сохраняем перерисовку всех боевых фигур.
void BattleFgrs_SaveRedraw()
{
    // Сохраняем перерисовку фигур заднего плана.
    BattleFgrs_List_SaveRedraw(&BattleFgrs_Background);

    // Сохраняем перерисовку фигур гексов.
    for (_int_ i = 0; i < BATTLE_HEXES_COUNT; i++)
    {
        BattleFgrs_List_SaveRedraw(&(BattleFgrs_Gexes[i]));
    }

    // Сохраняем перерисовку фигур стеков.
    for (_int_ i = ATTACKER; i <= DEFENDER; i++)
    {
        for (_int_ j = 0; j < BATTLE_SIDE_STACKS_COUNT; j++)
        {
            BattleFgrs_List_SaveRedraw(&(BattleFgrs_Stacks[i][j]));
        }
    }

    // Сохраняем перерисовку фигур переднего плана.
    BattleFgrs_List_SaveRedraw(&BattleFgrs_Foreground);
}





// Сбрасываем перерисовку фигур из списка.
void BattleFgrs_List_ResetRedraw(_List_<_BattleFigure_*>* List)
{
    // Проходим по всем фигурам списка с конца.
    for (_int_ i = List->GetItemsCount() - 1; i >= 0; i--)
    {
        // Сбрасываем перерисовку фигуры.
        (*List)[i]->NeedRedraw = FALSE;
    }
}



// Сбрасываем перерисовку всех боевых фигур.
void BattleFgrs_ResetRedraw()
{
    // Сбрасываем перерисовку фигур заднего плана.
    BattleFgrs_List_ResetRedraw(&BattleFgrs_Background);

    // Сбрасываем перерисовку фигур гексов.
    for (_int_ i = 0; i < BATTLE_HEXES_COUNT; i++)
    {
        BattleFgrs_List_ResetRedraw(&(BattleFgrs_Gexes[i]));
    }

    // Сбрасываем перерисовку фигур стеков.
    for (_int_ i = ATTACKER; i <= DEFENDER; i++)
    {
        for (_int_ j = 0; j < BATTLE_SIDE_STACKS_COUNT; j++)
        {
            BattleFgrs_List_ResetRedraw(&(BattleFgrs_Stacks[i][j]));
        }
    }

    // Сбрасываем перерисовку фигур переднего плана.
    BattleFgrs_List_ResetRedraw(&BattleFgrs_Foreground);
}


// Подготавливаем проигрывание проходов анимаций списка.
void BattleFgrs_List_ParallelAnimPrepare(_List_<_BattleFigure_*>* List)
{
    // Проходим по всем фигурам списка с конца.
    for (_int_ i = List->GetItemsCount() - 1; i >= 0; i--)
    {
        // Если фигура должна проигрываться параллельно - проигрываем её.
        if ((*List)[i]->ParallelAnim)
        {
            (*List)[i]->PlayOnce(TRUE, NULL, TRUE);
        }
    }
}



// Подготавливаем проходы анимаций всех боевых фигур.
void BattleFgrs_ParallelAnimPrepare()
{
    // Проигрываем список фигур заднего плана.
    BattleFgrs_List_ParallelAnimPrepare(&BattleFgrs_Background);

    // Проигрываем списки фигур гексов.
    for (_int_ i = 0; i < BATTLE_HEXES_COUNT; i++)
    {
        BattleFgrs_List_ParallelAnimPrepare(&(BattleFgrs_Gexes[i]));
    }

    // Проигрываем списки фигур стеков.
    for (_int_ i = ATTACKER; i <= DEFENDER; i++)
    {
        for (_int_ j = 0; j < BATTLE_SIDE_STACKS_COUNT; j++)
        {
            BattleFgrs_List_ParallelAnimPrepare(&(BattleFgrs_Stacks[i][j]));
        }
    }

    // Проигрываем список фигур переднего плана.
    BattleFgrs_List_ParallelAnimPrepare(&BattleFgrs_Foreground);
}



// Параллельное проигрывание проходов анимаций списка.
void BattleFgrs_List_ParallelAnim(_List_<_BattleFigure_*>* List)
{
    // Проходим по всем фигурам списка с конца.
    for (_int_ i = List->GetItemsCount() - 1; i >= 0; i--)
    {
        // Если фигура должна проигрываться параллельно и должна перерисовываться - проигрываем её.
        if ((*List)[i]->ParallelAnim && (*List)[i]->NeedRedraw)
        {
            (*List)[i]->PlayFrame(TRUE);
        }
    }
}



// Проигрываем проходы анимаций всех боевых фигур.
void BattleFgrs_ParallelAnim()
{
    // Проигрываем список фигур заднего плана.
    BattleFgrs_List_ParallelAnim(&BattleFgrs_Background);

    // Проигрываем списки фигур гексов.
    for (_int_ i = 0; i < BATTLE_HEXES_COUNT; i++)
    {
        BattleFgrs_List_ParallelAnim(&(BattleFgrs_Gexes[i]));
    }

    // Проигрываем списки фигур стеков.
    for (_int_ i = ATTACKER; i <= DEFENDER; i++)
    {
        for (_int_ j = 0; j < BATTLE_SIDE_STACKS_COUNT; j++)
        {
            BattleFgrs_List_ParallelAnim(&(BattleFgrs_Stacks[i][j]));
        }
    }

    // Проигрываем список фигур переднего плана.
    BattleFgrs_List_ParallelAnim(&BattleFgrs_Foreground);
}





// При начале отрисовки отрисовываем боевые фигуры заднего плана.
int __stdcall HookOn_Fgr_DrawBegin(LoHook* h, HookContext* c)
{
    // Отрисовываем боевые фигуры текущего режима (гекс один из существующих, порядок один из 0-7).
    BattleFgrs_Draw(NO_GEX, MPO_BELOW_ALL, NO_SIDE, NO_STACK);

    return EXEC_DEFAULT;
}


// При отрисовке порядка на гексе отрисовываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_GexesDraw(LoHook* h, HookContext* c)
{
    // Отрисовываем боевые фигуры текущего режима (порядок нижний).
    BattleFgrs_Draw(*(_int_*)(c->ebp - 4), *(_int_*)(c->ebp + 24), NO_SIDE, NO_STACK);

    return EXEC_DEFAULT;
}


// В конце отрисовки отрисовываем боевые фигуры переднего плана.
int __stdcall HookOn_Fgr_DrawEnd(LoHook* h, HookContext* c)
{
    // Отрисовываем боевые фигуры текущего режима (порядок верхний).
    BattleFgrs_Draw(NO_GEX, MPO_ABOVE_ALL, NO_SIDE, NO_STACK);

    return EXEC_DEFAULT;
}




// Перед отрисовкой стека проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_StackBeforeBlit(LoHook* h, HookContext* c)
{
    // Отрисовываем боевые фигуры текущего режима (стек, порядок нижний).
    BattleFgrs_Draw(NO_GEX, MPO_BELOW_STACK, ((_BattleStack_*)c->ebx)->side, ((_BattleStack_*)c->ebx)->index_on_side, Curr_Stack_Moving_X_Add, Curr_Stack_Moving_Y_Add);

    return EXEC_DEFAULT;
}


// После отрисовки стека проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_StackAfterBlit(LoHook* h, HookContext* c)
{
    // Отрисовываем боевые фигуры текущего режима (стек, порядок средний).
    BattleFgrs_Draw(NO_GEX, MPO_BELOW_STACK_ANIM, ((_BattleStack_*)c->ebx)->side, ((_BattleStack_*)c->ebx)->index_on_side, Curr_Stack_Moving_X_Add, Curr_Stack_Moving_Y_Add);

    return EXEC_DEFAULT;
}


// После отрисовки анимации стека проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_StackAfterBlitAnims(LoHook* h, HookContext* c)
{
    // Отрисовываем боевые фигуры текущего режима, если отрисовка нужна (стек, порядок верхний).
    if (!(*(_dword_*)((_ptr_)o_BattleMgr + 78584)) && !o_BattleMgr->ShouldNotRenderBattle() && !(*(_dword_*)(c->ebp + 16)))
    {
        BattleFgrs_Draw(NO_GEX, MPO_ABOVE_STACK, ((_BattleStack_*)c->ebx)->side, ((_BattleStack_*)c->ebx)->index_on_side, Curr_Stack_Moving_X_Add, Curr_Stack_Moving_Y_Add);
    }

    return EXEC_DEFAULT;
}






// Перед отрисовкой стрелковой башни обнуляем ссылку на неё.
int __stdcall HookOn_Fgr_TowerCreatureBeforeDraw(LoHook* h, HookContext* c)
{
    // Обнуляем теущую отрисовывающуюся стрелковую башню.
    CurrDrawingTower = 0;

    return EXEC_DEFAULT;
}


// Перед отрисовкой существа стрелковой башни проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_TowerCreatureBeforeBlit(LoHook* h, HookContext* c)
{
    // Теущая отрисовывающаяся стрелковая башня.
    CurrDrawingTower = (_ArrowTower_*)c->ecx;

    // Отрисовываем боевые фигуры текущего режима (стек, порядок нижний).
    BattleFgrs_Draw(NO_GEX, MPO_BELOW_STACK, DEFENDER, CurrDrawingTower->StackNum);

    return EXEC_DEFAULT;
}


// После отрисовки существа стрелковой башни проигрываем соответствующие боевые фигуры.
int __stdcall HookOn_Fgr_TowerCreatureAfterBlit(LoHook* h, HookContext* c)
{
    // Если рисовалась башня, отрисовываем боевые фигуры текущего режима (стек, порядок средний, затем верхний).
    if (CurrDrawingTower)
    {
        BattleFgrs_Draw(NO_GEX, MPO_BELOW_STACK_ANIM, DEFENDER, CurrDrawingTower->StackNum);
        BattleFgrs_Draw(NO_GEX, MPO_ABOVE_STACK, DEFENDER, CurrDrawingTower->StackNum);
    }

    return EXEC_DEFAULT;
}






// При настройке границ перерисовки боя настраиваем границы перерисовки не принадлежащих стекам фигур.
void __stdcall HookOn_Battle_SettingRedrawBorders(HiHook* h, _BattleMgr_* this_)
{
    // Настраиваем границы перерисовки не принадлежащих стекам фигур.
    *(_bool8_*)((_ptr_)this_ + 81196) = TRUE;
    *(_bool32_*)((_ptr_)this_ + 81204) = TRUE;
    BattleFgrs_Draw_NoStacks(TRUE);
    *(_bool8_*)((_ptr_)this_ + 81196) = FALSE;
    *(_bool32_*)((_ptr_)this_ + 81204) = FALSE;

    // Вызываем функцию настройки границ перерисовки.
    CALL_1(void, __thiscall, h->GetDefaultFunc(), this_);
}









// Рассчитываем добавки к координатам отрисовки стека.
int __stdcall HookOn_StackDraw_Calcs_PosAdds_For_Moving(LoHook* h, HookContext* c)
{
    // Запоминаем добавки к координатам отрисовки.
    Curr_Stack_Moving_X_Add = *(_int_*)(c->ebp + 8) - o_BattleMgr->hex[((_BattleStack_*)c->ebx)->hex_ix].X_Position;
    Curr_Stack_Moving_Y_Add = *(_int_*)(c->ebp + 12) - o_BattleMgr->hex[((_BattleStack_*)c->ebx)->hex_ix].Y_Position;

    return EXEC_DEFAULT;
}





// Перед боем очищаем списки боевых фигур.
void __stdcall HookOn_Battle_Start_Fgrs(HiHook* h, _BattleMgr_* this_, _dword_ a2)
{
    // Очищаем списки боевых фигур.
    if (!this_->ShouldNotRenderBattle()) Delete_All_Battle_Figures();

    // Вызываем инициализацию битвы.
    CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, a2);

}




// После сообщения о конце боя очищаем списки боевых фигур.
void __stdcall HookOn_Battle_EndMessage_Fgrs(HiHook* h, _BattleMgr_* this_, _dword_ a2)
{
    // Вызываем сообщение о конце боя.
    CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, a2);

    // Очищаем списки боевых фигур.
    if (!this_->ShouldNotRenderBattle()) Delete_All_Battle_Figures();
}



// Очищаем перерисовки боевых фигур (inline-подстановка всего одна, и та стирается в другом месте).
void __stdcall HookOn_BattleClearRedraws(HiHook* h, _BattleMgr_* this_)
{
    CALL_1(void, __thiscall, h->GetDefaultFunc(), this_);

    // Очищаем списки боевых фигур.
    BattleFgrs_ResetRedraw();
}



// При проигрывании анимации ожидания в бою проигрываем параллельную анимацию боевых фигур.
// int __stdcall HookOn_WaitAnim_Fgrs(LoHook* h, HookContext* c)
// {
//   // Проигрываем параллельные анимации фигур.
//   BattleFgrs_ParallelAnim();
//   
//   return EXEC_DEFAULT;
// }



// Сообщение о возможности пропуска предбитвенного звука.
TString PreBattleSound_SkippingMessage = "";


// Отрисовывается ли сейчас стрелковая башня с Медузой.
_bool_ MedusaTower_Drawing = FALSE;




// Периоды анимации стойки стеков.
_int_ StackStayAnimPeriod[2][21];
// Периоды случайной анимации стеков.
_int_ StackRandAnimPeriod[2][21];

// Времена последних проигрываний кадров анимации стойки стеков.
_int_ StackLastStayAnimTime[2][21];


// Необходимость проигрывания анимацию стойки стеков.
_bool_ StackStayAnimNeedPlay[2][21];


// Периоды анимаций флага сторон.
_int_ SideFlagAnimPeriod[2];
// Времена последних проигрываний кадров анимации флагов сторон.
_int_ SideFlagLastAnimTime[2];


// Периоды анимаций героя сторон.
_int_ SideHeroAnimPeriod[2];
// Времена последних проигрываний кадров анимации героя сторон.
_int_ SideHeroLastAnimTime[2];


// Периоды случайных анимаций героя сторон.
_int_ SideHeroRandAnimPeriod[2];



// Период анимации рамки вокруг стека.
_int_ BorderPeriod;
// Время последнего проигрывания рамок вокруг стеков.
_int_ BorderLastTime;



// Идёт ли сейчас предбитвенный звук.
_bool_ IsPreBattleSound = FALSE;


// Текущее время для всей анимации ожидания.
_int_ WaitAnimCurrTime = 0;


// Необходимость отрисовывать рамки вокруг стеков при анимации ожидания.
_bool_ NeedRedrawBorders = FALSE;




// Необходимость начинать анимации кривляния стеков при анимации ожидания.
_bool_ NeedBeginRandomAnims = FALSE;


// Был ли последний раунд тактическим (при смене раунда).
_bool_ RoundWasTactic = FALSE;


// Необходимо ли отрисовывать изображения активных элементов поля боя.
_bool_ NeedDraw_Active_Elemenst = TRUE;


// Тукущий кадр луча (выстрел непрерывным прямым лучом).
_int_ RayCurrFrame;

// Тукущий максимальный кадр луча (выстрел непрерывным прямым лучом).
_int_ RayMaxFrame;

// Закончился ли луч только что.
_bool_ RayWasEnded;

// Начальное зерно ГСЧ луча.
_dword_ RayStartSeed;

// Сохранённые начальные значения переменных для луча.
_dword_ RaySavedVars[10];




// Другой способ модификации луча, оказалось - более медленный.
/*
// Сохранённые линии луча.
RayLn RayLns;

// Максимальное количество секций в частях луча.
_int_ RayMaxSec;

// Рассчитывать ли продолжение луча сейчас.
_bool_ CalcRay;
*/






// Функция, отрисовывающая изменения для текущего плавного изменения экрана.
void (__stdcall* SmoothAnimSpec_Draw)() = NULL;
// Функция, возвращающая изменения для текущего плавного изменения экрана.
void (__stdcall* SmoothAnimSpec_Redo)() = NULL;
// Функция, откатывающая изменения для текущего плавного изменения экрана.
void (__stdcall* SmoothAnimSpec_Undo)() = NULL;
// Функция, играющая проход анимации для текущего плавного изменения экрана.
void (__stdcall* SmoothAnimSpec_Anim)() = NULL;
// Функция, обновляющая экран для текущего плавного изменения экрана.
void (__stdcall* SmoothAnimSpec_Flip)() = NULL;




#define NO_GEX -1
// Стек текущего плавного изменения экрана при вызове.
_BattleStack_* SmoothAnimSpec_Summon_Stack;

// Стек текущего плавного изменения экрана при телепорте.
_BattleStack_* SmoothAnimSpec_Teleport_Stack;
// Номер начального гекса текущего плавного изменения экрана при телепорте.
_int_ SmoothAnimSpec_Teleport_StartGexNum = NO_GEX;
// Номер целевого гекса текущего плавного изменения экрана при телепорте.
_int_ SmoothAnimSpec_Teleport_TargetGexNum = NO_GEX;

// Номер препятствия текущего плавного изменения экрана при уничтожении препятствий.
_int_ SmoothAnimSpec_RemoveObstacle_ObstacleNum;
// Видимость текущего препятствия.
_bool_ SmoothAnimSpec_RemoveObstacle_ObstacleVisible = TRUE;






// Время между кадрами баллистического взрыва.
_int_ BallisticExplFrameTime;

// Время следующей смены кадров баллистического взрыва.
_int_ BallisticExplNextTime;




// Время между кадрами огненного шара Магога.
_int_ MGFireballFrameTime;

// Время следующей смены кадров огненного шара Магога.
_int_ MGFireballNextTime;




// Время между кадрами облака смерти Лича и Могущественного лича.
_int_ LichDClFrameTime;

// Время следующей смены кадров облака смерти Лича и Могущественного лича.
_int_ LichDClNextTime;




// Время между кадрами армагеддона.
_int_ ArmageddonFrameTime;

// Время следующей смены кадров армагеддона.
_int_ ArmageddonNextTime;




// Функции, отрисовывающие изменения для текущего удержания нажатия кнопки (последняя - текущая, остальные - родительских диалогов).
_List_<void (__stdcall*)()> ButtonWhileClicked_Draw_List;

// Был ли диалог скрытым (как диалоги наложения заклинаний).
_bool_ Dlg_WasHidden = FALSE;




// Атакующий в текущем выстреле.
_BattleStack_* CurrShot_Attacker;


// Позиция текущего удаляемого препятствия.
_int_ CurrDelObst_Hex_IX;



// Префикс звука особой смерти.
_cstr_ SpecDeathSoundName = NULL;


// Идёт ли сейчас инициализация битвы.
_bool_ IsBattleInit = FALSE;



// Def большого препятствия в бою.
_Def_* LargeObstackleDef;

// Количество кадров в def`е большого препятствия в бою.
_int_ LargeObstackleDef_FramesCount;



// Массив больших препятствий.
//_BattleObstackleLarge_* LargeObstackles = (_BattleObstackleLarge_*)0x63BEC0;

// Количество больших препятствий.
_int_ LargeObstackles_Count = LargeObstackles_CountB;

// Массив обычных препятствий.
//_BattleObstackleInfo_* Obstackles = (_BattleObstackleInfo_*)0x63C7C8;

// Количество обычных препятствий.
_int_ Obstackles_Count = Obstackles_CountB;





// Сравнение для 2 переменных времени (которые можно использовать только в виде разности).
_int_ t_min(_int_ a, _int_ b)
{
  if (a - b < 0)
  {
    return a;
  }
  else
  {
    return b;
  }
}






// Не подсвечиваем гексы с существами противника в тактической фазе.
int __stdcall LoHook_BattleTaktik_Shadow(LoHook* h, HookContext* c)
{
  // Тактическая фаза - пропускаем выделение гекса с врагом.
  if (o_BattleMgr->Field<_bool8_>(81256))
  {
    c->return_address = 0x49347F;
    
    return NO_EXEC_DEFAULT;
  }
  // Иначе - стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}



// Исправляем исчезновение тени перемещения.
int __stdcall LoHook_Redraw_Shadow(LoHook* h, HookContext* c)
{
  // Отрисовка тени.
  CALL_3(void, __thiscall, 0x4934B0, o_BattleMgr, 0, 1);
  
  // Перерисовываем поле боя.
  o_BattleMgr->RedrawBattlefield(TRUE, FALSE, FALSE, 0, TRUE, FALSE);
  
  return EXEC_DEFAULT;
}




// Исправляем задержку кадра анимации колдовства на стеке.
int __stdcall LoHook_BattleStack_Cast_Reset(LoHook* h, HookContext* c)
{
  // Колдующий стек.
  _BattleStack_* stack = (_BattleStack_*)c->esi;
  
  // Стек в анимации колдовства.
  if (stack->def_group_ix == DG_CAST_UP || stack->def_group_ix == DG_CAST_STRAIGHT || stack->def_group_ix == DG_CAST_DOWN
      || stack->def_group_ix == DG_HIT_UP || stack->def_group_ix == DG_HIT_STRAIGHT || stack->def_group_ix == DG_HIT_DOWN)
  {
    // Берём границы перерисовки стека.
    o_BattleMgr->ClearRedrawFields();
    o_BattleMgr->Set_Stack_Redrawable(stack);
    o_BattleMgr->SetRedrawBorders();
    
    // Меняем кадр стека.
    stack->def_group_ix = DG_STAY;
    stack->def_frame_ix = 0;
    
    // Отрисовка.
    o_BattleMgr->RedrawBattlefield(TRUE, TRUE, TRUE, 0, TRUE, FALSE);
  }
  
  return EXEC_DEFAULT;
}


// Перерисовываем поле боя после анимации на стеке.
void __stdcall HiHook_Battle_StdAnimRedraw(HiHook* h, _BattleMgr_* this_, _int32_ anim_ix, _BattleStack_* stack, _int32_ length, _bool8_ anim_damage)
{
  // Играем анимацию.
  CALL_5(void, __thiscall, h->GetDefaultFunc(), this_, anim_ix, stack, length, anim_damage);
  
  // Перерисовка.
  this_->RedrawBattlefield(TRUE, FALSE, FALSE, 0, TRUE, FALSE);
}



// Устанавливаем минимальное время для первой анимации цепной молнии.
void __stdcall HookOn_Battle_ChainLightingAnimFirst(HiHook* h, _BattleMgr_* this_, _int32_ anim_ix, _BattleStack_* stack, _int32_ length, _bool8_ anim_damage)
{
  // Заменяем длительность.
  CALL_5(void, __thiscall, h->GetDefaultFunc(), this_, anim_ix, stack, 10, anim_damage);
}




// Ограничение границами отрисовки простого def`а в бою.
void __stdcall HiHook_Battle_BlitDefSimple_UseBorders(HiHook* h, _Def_* this_, _int32_ group_ix, _int32_ frame_ix, _int32_ img_x, _int32_ img_y, _int32_ img_width, _int32_ img_height, _ptr_ pcx_buffer,
                      _int32_ dest_x, _int32_ dest_y, _int32_ dest_width, _int32_ dest_height,
                      _dword_ scanline_size, _bool32_ reflected, _bool8_ use_spec_colors)
{
  
  // Границы отрисовки.
  _RedrawBorders_* brd = &BattleRedraw_Borders;
  
  // Огриначиваем размеры изображения.
  if (dest_x + img_x < brd->Left)
  {
    img_width -= brd->Left - dest_x - img_x;
    img_x = brd->Left - dest_x;
    dest_x = brd->Left;
  }
  if (dest_y + img_y < brd->High)
  {
    img_height -= brd->High - dest_y - img_y;
    img_y = brd->High - dest_y;
    dest_y = brd->High;
  }
  if (dest_x + img_x + img_width > brd->Right)
  {
    img_width = brd->Right - dest_x - img_x;
  }
  if (dest_y + img_y + img_height > brd->Low)
  {
    img_height = brd->Low - dest_y - img_y;
  }
  
  
  CALL_15(void, __thiscall, h->GetDefaultFunc(), this_, group_ix, frame_ix, img_x, img_y, img_width, img_height, pcx_buffer,
                      dest_x, dest_y, dest_width, dest_height,
                      scanline_size, reflected, use_spec_colors);
}




// Ограничение границами отрисовки def`а боевой анмации.
void __stdcall HiHook_Battle_BlitDefBattleAnim_UseBorders(HiHook* h, _Def_* this_, _int32_ group_ix, _int32_ frame_ix, _int32_ img_x, _int32_ img_y, _int32_ img_width, _int32_ img_height, _ptr_ pcx_buffer,
                      _int32_ dest_x, _int32_ dest_y, _int32_ dest_width, _int32_ dest_height,
                      _dword_ scanline_size, _bool32_ reflected, _bool8_ is_transparent)
{
  
  // Границы отрисовки.
  _RedrawBorders_* brd = &BattleRedraw_Borders;
  
  // Огриначиваем размеры изображения.
  if (dest_x + img_x < brd->Left)
  {
    img_width -= brd->Left - dest_x - img_x;
    img_x = brd->Left - dest_x;
    dest_x = brd->Left;
  }
  if (dest_y + img_y < brd->High)
  {
    img_height -= brd->High - dest_y - img_y;
    img_y = brd->High - dest_y;
    dest_y = brd->High;
  }
  if (dest_x + img_x + img_width > brd->Right)
  {
    img_width = brd->Right - dest_x - img_x;
  }
  if (dest_y + img_y + img_height > brd->Low)
  {
    img_height = brd->Low - dest_y - img_y;
  }
  
  
  CALL_15(void, __thiscall, h->GetDefaultFunc(), this_, group_ix, frame_ix, img_x, img_y, img_width, img_height, pcx_buffer,
                      dest_x, dest_y, dest_width, dest_height,
                      scanline_size, reflected, is_transparent);
}



// Ограничение границами отрисовки def`а с заменяющимся спеццветом.
void __stdcall HiHook_Battle_BlitDefSpecColorReplace_UseBorders(HiHook* h, _Def_* this_, _int32_ group_ix, _int32_ frame_ix, _int32_ img_x, _int32_ img_y, _int32_ img_width, _int32_ img_height, _ptr_ pcx_buffer,
                      _int32_ dest_x, _int32_ dest_y, _int32_ dest_width, _int32_ dest_height,
                      _dword_ scanline_size, _bool32_ reflected, _int32_ spec_color)
{
  
  // Границы отрисовки.
  _RedrawBorders_* brd = &BattleRedraw_Borders;
  
  // Огриначиваем размеры изображения.
  if (dest_x + img_x < brd->Left)
  {
    img_width -= brd->Left - dest_x - img_x;
    img_x = brd->Left - dest_x;
    dest_x = brd->Left;
  }
  if (dest_y + img_y < brd->High)
  {
    img_height -= brd->High - dest_y - img_y;
    img_y = brd->High - dest_y;
    dest_y = brd->High;
  }
  if (dest_x + img_x + img_width > brd->Right)
  {
    img_width = brd->Right - dest_x - img_x;
  }
  if (dest_y + img_y + img_height > brd->Low)
  {
    img_height = brd->Low - dest_y - img_y;
  }
  
  
  CALL_15(void, __thiscall, h->GetDefaultFunc(), this_, group_ix, frame_ix, img_x, img_y, img_width, img_height, pcx_buffer,
                      dest_x, dest_y, dest_width, dest_height,
                      scanline_size, reflected, spec_color);
}




// Ограничение границами отрисовки pcx в бою.
void __stdcall HiHook_Battle_Pcx16_Draw_UseBorders(HiHook* h, _Pcx16_* this_, _int32_ img_x, _int32_ img_y, _int32_ img_width, _int32_ img_height, _ptr_ pcx_buffer,
                      _int32_ dest_x, _int32_ dest_y, _int32_ dest_width, _int32_ dest_height,
                      _dword_ scanline_size, _bool8_ reflected)
{
  
  // Границы отрисовки.
  _RedrawBorders_* brd = &BattleRedraw_Borders;
  
  // Огриначиваем размеры изображения.
  if (dest_x + img_x < brd->Left)
  {
    img_width -= brd->Left - dest_x - img_x;
    img_x = brd->Left - dest_x;
    dest_x = brd->Left;
  }
  if (dest_y + img_y < brd->High)
  {
    img_height -= brd->High - dest_y - img_y;
    img_y = brd->High - dest_y;
    dest_y = brd->High;
  }
  if (dest_x + img_x + img_width > brd->Right)
  {
    img_width = brd->Right - dest_x - img_x;
  }
  if (dest_y + img_y + img_height > brd->Low)
  {
    img_height = brd->Low - dest_y - img_y;
  }
  
  
  CALL_12(void, __thiscall, h->GetDefaultFunc(), this_, img_x, img_y, img_width, img_height, pcx_buffer,
                      dest_x, dest_y, dest_width, dest_height,
                      scanline_size, reflected);
}

// Ограничение границами упрощённой отрисовки pcx в бою.
void __stdcall HiHook_Battle_Pcx16_DrawSimple_UseBorders(HiHook* h, _Pcx16_* this_, _int32_ img_x, _int32_ img_y, _int32_ img_width, _int32_ img_height, _Pcx16_* dest,
                      _int32_ dest_x, _int32_ dest_y, _bool8_ unk9)
{
  
  // Границы отрисовки.
  _RedrawBorders_* brd = &BattleRedraw_Borders;
  
  // Огриначиваем размеры изображения.
  if (dest_x + img_x < brd->Left)
  {
    img_width -= brd->Left - dest_x - img_x;
    img_x = brd->Left - dest_x;
    dest_x = brd->Left;
  }
  if (dest_y + img_y < brd->High)
  {
    img_height -= brd->High - dest_y - img_y;
    img_y = brd->High - dest_y;
    dest_y = brd->High;
  }
  if (dest_x + img_x + img_width > brd->Right)
  {
    img_width = brd->Right - dest_x - img_x;
  }
  if (dest_y + img_y + img_height > brd->Low)
  {
    img_height = brd->Low - dest_y - img_y;
  }
  
  
  CALL_9(void, __thiscall, h->GetDefaultFunc(), this_, img_x, img_y, img_width, img_height, dest,
                      dest_x, dest_y, unk9);
}







// Инициализация звуков особой смерти стеков.
//void __stdcall HiHook_BattleStack_InitDefsAndWavs(HiHook* h, _BattleStack_* this_)
//{
//  // Инициализация оригинальных ресурсов.
//  CALL_1(void, __thiscall, h->GetDefaultFunc(), this_);
//  
//  // Есть имя звука особой смерти.
//  if (SpecDeathSoundName)
//  {
//    // У существа есть особая смерть.
//    if (this_->creature_id == CID_BEHOLDER || this_->creature_id == CID_EVIL_EYE || this_->creature_id == CID_GOLD_GOLEM)
//    {
//      // Получаем имя звука.
//      sprintf(o_TextBuffer, SpecDeathSoundName, this_->creature.sound_name);
//      
//      // Если звук особой смерти уже был - удаляем.
//      if (this_->ext->spec_death_sound)
//      {
//        this_->ext->spec_death_sound->DerefOrDestruct();
//      }
//      
//      // Загружаем новый звук.
//      this_->ext->spec_death_sound = LoadWav(o_TextBuffer);
//    }
//  }
//}



// Проигрывание новых звуков стека.
void __stdcall HiHook_BattleStack_PlaySound(HiHook* h, _BattleStack_* this_, _int32_ sound_id)
{
  // Проигрывание оригинальных звуков.
    CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, sound_id);

  //if (sound_id < 8)
  //{
  //  CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, sound_id);
  //}
  //else
  //{
  //  // Надо отрисовывать бой.
  //  if (!o_BattleMgr->ShouldNotRenderBattle())
  //  {
  //    // Проигрываемый звук.
  //    _Wav_* sound = (&this_->ext->spec_death_sound)[sound_id - 8];
  //    
  //    // Если есть соответствующий звук - запускаем его.
  //    if (sound)
  //    {
  //      o_SoundMgr->StartSample(sound);
  //    }
  //    // Иначе, если это звук особой смерти, запускаем обычный зук смерти.
  //    else if (sound_id == 8)
  //    {
  //      CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, 4);
  //    }
  //  }
  //}
}





// Проигрывание звука особой смерти стека.
void __stdcall HiHook_BattleStack_PlaySpecDeathSound(HiHook* h, _BattleStack_* this_, _int32_ sound_id)
{
  // Существо умирает особой смертью - проигрываем соответствующий звук.
  if (this_->Field<_int8_>(234) == 2)
  {
    CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, 8);
  }
  // Иначе - стандартно.
  else
  {
    CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, sound_id);
  }
}



// Информация о заклинании, для которого будет рисоваться тень перемещения.
_int_ Spell_For_Drawing_Shadow;
_int_ Spell_For_Drawing_Shadow_Skill;
_int_ Spell_For_Drawing_Shadow_Side;
_bool8_ Spell_For_Drawing_Shadow_IsFirstDlg;
_int_ Spell_For_Drawing_Shadow_ByStack;

// Был ли диалог колдовства подвержен установке новой подсветки.
_bool_ Spell_NewShadow_WasSetted;

// Выбранные диалогом заклинания стеки.
_bool8_ StacksSelectedBySpellDlg[2][21];

// Получение номера гекса магического препятствия.
_int_ MagicObst_GetHexIx(_int_ spell_id, _int_ side, _int_ skill_level, _int_ main_hex, _int_ hex_shift)
{
  // Главный гекс
  if (hex_shift == 0)
  {
    return main_hex;
  }
  // Силовое поле
  else if (spell_id == SPL_FORCE_FIELD)
  {
    // Выбираем структуру препятствия.
    _Struct_* obst = (_Struct_*)0x63CF18;
    if (skill_level >= 2) obst = (_Struct_*)0x63CF2C;
    
    // Недоступный в данной ситуации гекс.
    if (hex_shift < 0 || hex_shift >= obst->Field<_byte_>(6))
    {
      return ID_NONE;
    }
    else
    {
      // Номер гекса.
      _int_ hex_ix = main_hex + obst->PField<_int8_>(8)[hex_shift];
      
      // Нечётный ряд основного гекса и чётный ряд текущего гекса.
      if ((main_hex/17 & 1) && !(hex_ix/17 & 1))
      {
        hex_ix--;
      }
      
      return hex_ix;
    }
  }
  // Стена огня.
  else
  {
    // Недоступный в данной ситуации гекс.
    if (hex_shift < 0 || hex_shift > 2 || skill_level < 2 && hex_shift == 2)
    {
      return ID_NONE;
    }
    else
    {
      // Смещение на 1 гекс.
      if (hex_shift == 1)
      {
        // Номер гекса.
        _int_ hex_ix = main_hex - 17;
        
        // Нечётный ряд основного гекса.
        if (main_hex/17 & 1)
        {
          // Колдует защищающийся.
          if (side == DEFENDER)
          {
            hex_ix--;
          }
        }
        // Чётный ряд основного гекса.
        else
        {
          // Колдует атакующий.
          if (side == ATTACKER)
          {
            hex_ix++;
          }
        }
        
        return hex_ix;
      }
      // Смещение на 2 гекса.
      else
      {
        // Ровно на 2 ряда выше.
        return main_hex - 34;
      }
    }
  }
}




// Получение номера главного гекса магического препятствия по известному неглавному.
_int_ MagicObst_GetMainHexIxFromShift(_int_ spell_id, _int_ side, _int_ skill_level, _int_ curr_hex, _int_ hex_shift)
{
  // Главный гекс
  if (hex_shift == 0)
  {
    return curr_hex;
  }
  // Силовое поле
  else if (spell_id == SPL_FORCE_FIELD)
  {
    // Выбираем структуру препятствия.
    _Struct_* obst = (_Struct_*)0x63CF18;
    if (skill_level >= 2) obst = (_Struct_*)0x63CF2C;
    
    // Недоступный в данной ситуации гекс.
    if (hex_shift < 0 || hex_shift >= obst->Field<_byte_>(6))
    {
      return ID_NONE;
    }
    else
    {
      // Номер гекса.
      _int_ hex_ix = curr_hex - obst->PField<_int8_>(8)[hex_shift];
      
      // Нечётный ряд основного гекса и чётный ряд текущего гекса.
      if (!(curr_hex/17 & 1) && (hex_ix/17 & 1))
      {
        hex_ix++;
      }
      
      return hex_ix;
    }
  }
  // Стена огня.
  else
  {
    // Недоступный в данной ситуации гекс.
    if (hex_shift < 0 || hex_shift > 2 || skill_level < 2 && hex_shift == 2)
    {
      return ID_NONE;
    }
    else
    {
      // Смещение на 1 гекс.
      if (hex_shift == 1)
      {
        // Номер гекса.
        _int_ hex_ix = curr_hex + 17;
        
        // Нечётный ряд основного гекса.
        if (hex_ix/17 & 1)
        {
          // Колдует защищающийся.
          if (side == DEFENDER)
          {
            hex_ix++;
          }
        }
        // Чётный ряд основного гекса.
        else
        {
          // Колдует атакующий.
          if (side == ATTACKER)
          {
            hex_ix--;
          }
        }
        
        return hex_ix;
      }
      // Смещение на 2 гекса.
      else
      {
        // Ровно на 2 ряда ниже.
        return curr_hex + 34;
      }
    }
  }
}


// Получение гекса, на который можно колдовать препятствие.
_int_ MagicObst_GetHexToCast(_BattleMgr_* b_mgr, _int_ base_hex, _int_ spell_id, _int_ side, _int_ skill_level)
{
  // Некорректный гекс.
  if (base_hex < 0 || base_hex > 186 || base_hex%17 == 0 || base_hex%17 == 16)
  {
    return ID_NONE;
  }
  
  if (skill_level < 2)
  {
    // Пытаемся поставить препятствие на выбранный гекс первым гексом.
    if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, spell_id, skill_level, base_hex, side, TRUE, FALSE))
    {
      return base_hex;
    }
    
    // Пытаемся поставить препятствие на выбранный гекс вторым гексом.
    _int_ hex_ix = MagicObst_GetMainHexIxFromShift(spell_id, side, skill_level, base_hex, 1);
    
    // Можно колдовать на второй гекс.
    if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, spell_id, skill_level, hex_ix, side, TRUE, FALSE))
    {
      return hex_ix;
    }
  }
  else
  {
    // Пытаемся поставить препятствие на выбранный гекс вторым гексом.
    _int_ hex_ix = MagicObst_GetMainHexIxFromShift(spell_id, side, skill_level, base_hex, 1);
    
    // Можно колдовать на второй гекс.
    if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, spell_id, skill_level, hex_ix, side, TRUE, FALSE))
    {
      return hex_ix;
    }
    
    // Пытаемся поставить препятствие на выбранный гекс первым гексом.
    if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, spell_id, skill_level, base_hex, side, TRUE, FALSE))
    {
      return base_hex;
    }
    
    // Пытаемся поставить препятствие на выбранный гекс третьим гексом.
    hex_ix = MagicObst_GetMainHexIxFromShift(spell_id, side, skill_level, base_hex, 2);
    
    // Можно колдовать на третий гекс.
    if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, spell_id, skill_level, hex_ix, side, TRUE, FALSE))
    {
      return hex_ix;
    }
  }
  
  // Не удалось поставить никак.
  return ID_NONE;
}




// Получение гекса, на который можно телепортировать стек.
_int_ Teleport_GetHexToCast(_BattleMgr_* b_mgr, _int_ base_hex, _BattleStack_* stack)
{
  // Некорректный гекс.
  if (base_hex < 0 || base_hex > 186 || base_hex%17 == 0 || base_hex%17 == 16)
  {
    return ID_NONE;
  }
  
  // Стек 2-гексовый.
  if (stack->creature.flags & BCF_2HEX_WIDE)
  {
     
    // Пытаемся поставить стек на выбранный гекс головным гексом.
    _int_ hex_ix = base_hex + (stack->Field<_bool32_>(68) ? -1 : 1);
    
    // Можно колдовать на гекс к хвосту от выбранного.
    if (CALL_3(_bool8_, __thiscall, 0x5A3A10, b_mgr, stack, hex_ix))
    {
      return hex_ix;
    }
    
    // Пытаемся поставить стек на выбранный гекс хвостовым гексом.
    if (CALL_3(_bool8_, __thiscall, 0x5A3A10, b_mgr, stack, base_hex))
    {
      return base_hex;
    }
  }
  // Стек 1-гексовый.
  else
  {
    // Пытаемся поставить стек на выбранный гекс единственным гексом.
    if (CALL_3(_bool8_, __thiscall, 0x5A3A10, b_mgr, stack, base_hex))
    {
      return base_hex;
    }
  }
  
  // Не удалось поставить никак.
  return ID_NONE;
}



// Подсветка гексов в диалоге выбора гекса телепорта.
void Teleport_SelectHex_Highlight(_BattleMgr_* b_mgr, _int_ base_hex, _int_ sel_hex, _BattleStack_* stack)
{
  // Список гексов для подсветки.
  _List_<_int32_> hexes_lst;
  
  // Основной гекс для подсветки.
  _int_ hex_ix = ID_NONE;
  
  // Выбранный гекс для телепорта корректен.
  if (sel_hex != ID_NONE)
  {
    hex_ix = sel_hex;
    
    // Подсвечиваем основной гекс.
    hexes_lst.Append(sel_hex);
    
    // Стек 2-гексовый.
    if (stack->creature.flags & BCF_2HEX_WIDE)
    {
      // Подсвечиваем головной гекс.
      hexes_lst.Append(sel_hex + (stack->Field<_bool32_>(68) ? 1 : -1));
    }
  }
  else
  {
    // Гекс не вне поля боя.
    if (base_hex >= 0 && base_hex <= 186 && base_hex%17 != 0 && base_hex%17 != 16)
    {
      hex_ix = base_hex;
      
      // Подсвечиваем основной гекс.
      hexes_lst.Append(base_hex);
    }
  }
  
  // Подсвечиваем гексы.
  CALL_4(void, __thiscall, 0x493A20, b_mgr, hex_ix, &hexes_lst, FALSE);
}






// Устанавливаем параметры заклинания, для которого будет рисоваться тень.
void __stdcall HiHook_Battle_SelectSpellTarget_SaveSpell(HiHook* h, _BattleMgr_* this_, _int32_ spell_id, _bool32_ by_stack)
{
  // Устанавливаем параметры заклинания для отрисовки тени.
  Spell_For_Drawing_Shadow = spell_id;
  Spell_For_Drawing_Shadow_Side = this_->currentActiveSide;
  if (this_->hero[Spell_For_Drawing_Shadow_Side])
  {
    Spell_For_Drawing_Shadow_Skill = CALL_3(_int32_, __thiscall, 0x4E52F0, this_->hero[Spell_For_Drawing_Shadow_Side], spell_id, this_->special_Ground);
  }
  else
  {
    Spell_For_Drawing_Shadow_Skill = 0;
  }
  Spell_For_Drawing_Shadow_IsFirstDlg = TRUE;
  Spell_For_Drawing_Shadow_ByStack = by_stack;
  
  // Новой подсветки пока не было.
  Spell_NewShadow_WasSetted = FALSE;
  
  CALL_3(void, __thiscall, h->GetDefaultFunc(), this_, spell_id, by_stack);
}


// Индикация того, что далее будет проигрываться второй диалог заклинания.
int __stdcall LoHook_Battle_SelectSpellTarget_SecondDlg(LoHook* h, HookContext* c)
{
  Spell_For_Drawing_Shadow_IsFirstDlg = FALSE;
  
  // Выбранный гекс.
  _int_ sel_hex_ix = o_BattleMgr->Field<_int32_>(68);
  if (sel_hex_ix >= 0 && Spell_For_Drawing_Shadow == SPL_TELEPORT)
  {
    // Выбранный стек.
    _BattleStack_* sel_stack = o_BattleMgr->hex[sel_hex_ix].GetCreature();
    if (sel_stack)
    {
      // Выделяем стек.
      StacksSelectedBySpellDlg[sel_stack->side][sel_stack->index_on_side] = TRUE;
    }
  }
  
  return EXEC_DEFAULT;
}


// Настройка новой тени для заклинания.
int __stdcall LoHook_SelectSpell_NewShadow(LoHook* h, HookContext* c)
{
  // Была новая подсветка.
  Spell_NewShadow_WasSetted = TRUE;
  
  // Менеджер боя.
  _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
  
  // Если нужно подсвечивать гекс, сбрасываем подсветку.
  if (DwordAt(0x698810) && Spell_For_Drawing_Shadow != ID_NONE)
  {
    CALL_3(void, __thiscall, 0x493F10, b_mgr, -1, 0);
  }
  
  // Если есть тень перемещения - меняем её.
  if (DwordAt(0x698814) && Spell_For_Drawing_Shadow != ID_NONE)
  {
    // Надо ли затенять гексы.
    _bool8_* hex_msk = b_mgr->PField<_bool8_>(263);
    
    // Нет подсвеченых гексов.
    MemSet(hex_msk, 0, 187);
    
    // Определяем подсветку для заклинания.
    switch (Spell_For_Drawing_Shadow)
    {
      // Силовое поле.
      case SPL_FORCE_FIELD:
      // Стена огня.
      case SPL_FIRE_WALL:
        
        // Отмечаем недоступные для постановки гексы.
        for (_int_ i = 0; i < 187; i++)
        {
          if (MagicObst_GetHexToCast(b_mgr, i, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_Skill) != ID_NONE)
          {
            hex_msk[i] = 3;
          }
        }
        
        break;
      
      // Жертва.
      case SPL_SACRIFICE:
       // Выбор стека для воскрешения.
        if (Spell_For_Drawing_Shadow_IsFirstDlg)
        {
          // Отмечаем доступные гексы-цели.
          for (_int_ i = 0; i < 187; i++)
          {
            // Можно колдовать на этот стек.
            if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, i, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack))
            {
              hex_msk[i] = 1;
            }
          }
        }
        // Выбор жертвы.
        else
        {
          // Есть выбранный стек, который воскрешаем.
          if (b_mgr->Field<_int32_>(68) >= 0)
          {
            // Воскрешаемый стек.
            _BattleStack_* ressur_stack = b_mgr->hex[b_mgr->Field<_int32_>(68)].GetCreature();
            
            // Отмечаем доступные гексы-цели.
            for (_int_ i = 0; i < 187; i++)
            {
              // Можно колдовать на этот стек и он не жертвуемый.
              if (b_mgr->hex[i].GetCreature() != ressur_stack && CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, i, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack))
              {
                hex_msk[i] = 1;
              }
            }
          }
        }
        break;
      
      // Телепорт.
      case SPL_TELEPORT:
        // Выбор стека.
        if (Spell_For_Drawing_Shadow_IsFirstDlg)
        {
          // Отмечаем доступные гексы-цели.
          for (_int_ i = 0; i < 187; i++)
          {
            // Можно колдовать на этот стек.
            if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, i, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack))
            {
              hex_msk[i] = 1;
            }
          }
        }
        // Постановка стека на гекс.
        else
        {
          // Есть выбранный гекс, с которого телепортируем.
          if (b_mgr->Field<_int32_>(68) >= 0)
          {
            // Телепортируемый стек.
            _BattleStack_* teleport_stack = b_mgr->hex[b_mgr->Field<_int32_>(68)].GetCreature();
            
            // Отмечаем доступные для постановки гексы.
            for (_int_ i = 0; i < 187; i++)
            {
              // Можно телепортирвоать стек туда.
              if (Teleport_GetHexToCast(b_mgr, i, teleport_stack) != ID_NONE)
              {
                hex_msk[i] = 3;
              }
            }
          }
        }
        break;
      
      // Цепная молния.
      case SPL_CHAIN_LIGHTNING:
        // Отмечаем доступные гексы-цели.
        for (_int_ i = 0; i < 187; i++)
        {
          // Можно колдовать на этот стек и он вражеский.
          if ((_int8_)b_mgr->hex[i].bstack_side >= 0)
          {
            if ((b_mgr->hex[i].GetCreature()->side != Spell_For_Drawing_Shadow_Side) && CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, i, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack))
            {
              hex_msk[i] = 1;
            }
          }
        }
        break;
      
      // Площадные заклинания.
      case SPL_FROST_RING:
      case SPL_FIREBALL:
      case SPL_INFERNO:
      case SPL_METEOR_SHOWER:
      case SPL_BERSERK:
        // Отмечаем доступные гексы-цели.
        for (_int_ i = 0; i < 187; i++)
        {
          // Там есть стек и можно колдовать на него.
          if ((_int8_)b_mgr->hex[i].bstack_side >= 0)
          {
            if (CALL_7(double, __thiscall, 0x5A83A0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Side, b_mgr->hex[i].GetCreature(), FALSE, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack) > 0.0)
            {
              hex_msk[i] = 1;
            }
          }
        }
        break;
      
      // Обычные заклинания.
      default:
        // Отмечаем доступные гексы-цели.
        for (_int_ i = 0; i < 187; i++)
        {
          // Можно колдовать на этот гекс.
          if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, i, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack))
          {
            hex_msk[i] = 1;
          }
        }
        break;
    }
  }
  
  // Перерисовка поля боя.
  if ((DwordAt(0x698810) || DwordAt(0x698814)) && Spell_For_Drawing_Shadow != ID_NONE)
  {
    b_mgr->Field<_bool32_>(21432) = FALSE;
    _bool_ BNRB = NeedRedrawBorders;
    NeedRedrawBorders = TRUE;
    b_mgr->RedrawBattlefield(TRUE, FALSE, FALSE, 0, TRUE, FALSE);
    NeedRedrawBorders = BNRB;
  }
  
  return EXEC_DEFAULT;
}


// Возвращение обычной тени.
int __stdcall LoHook_SelectSpell_RestoreShadow(LoHook* h, HookContext* c)
{
  // Если была установлена новая подсветка - возвращаем обычную.
  if (Spell_NewShadow_WasSetted)
  {
    // Менеджер боя.
    _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
    
    // Если нужно подсвечивать гекс, сбрасываем подсветку.
    if (DwordAt(0x698810) && Spell_For_Drawing_Shadow != ID_NONE)
    {
      CALL_3(void, __thiscall, 0x493F10, b_mgr, -1, 0);
    }
    
    // Если есть тень перемещения - возвращаем её.
    if (DwordAt(0x698814) && Spell_For_Drawing_Shadow != ID_NONE)
    {
      CALL_2(void, __thiscall, 0x493350, b_mgr, &b_mgr->stack[b_mgr->currentStackSide][b_mgr->currentStackIndex]);
    }
    
    // Перерисовка поля боя.
    if ((DwordAt(0x698810) || DwordAt(0x698814)) && Spell_For_Drawing_Shadow != ID_NONE)
    {
      b_mgr->Field<_bool32_>(21432) = FALSE;
      if (b_mgr->Field<_int32_>(60) == 0)
      {
        _bool_ BNRB = NeedRedrawBorders;
        NeedRedrawBorders = TRUE;
        b_mgr->RedrawBattlefield(TRUE, FALSE, FALSE, 0, TRUE, FALSE);
        NeedRedrawBorders = BNRB;
      }
    }
  }
  
  return EXEC_DEFAULT;
}





// Добавление учёта одиночных заклинаний в функцию определения текущих целей заклинания.
int __stdcall LoHook_SelectSpellTargets_SingleSpell(LoHook* h, HookContext* c)
{
  // По номеру заклинания.
  switch (c->eax)
  {
    // Площадные заклинания.
    case SPL_FROST_RING:
    case SPL_FIREBALL:
    case SPL_INFERNO:
    case SPL_METEOR_SHOWER:
    case SPL_BERSERK:
      
      // Корректный гекс.
      if (IntAt(c->ebp + 8) >= 0 && IntAt(c->ebp + 8) <= 186 && IntAt(c->ebp + 8)%17 != 0 && IntAt(c->ebp + 8)%17 != 16)
      {
        return EXEC_DEFAULT;
      }
      // Некорректный гекс.
      else
      {
        // Восстанавливаем затёртые команды.
        IntAt(c->ebp - 4) = c->edi;
        ByteAt(c->ebp - 13) = 0;
        
        // Очищаем выделенные стеки-цели.
        MemSet(o_BattleMgr->PField<_bool8_>(21628), 0, 40);
        
        // Пропускаем расчёт.
        c->return_address = 0x59FC2D;
        
        return NO_EXEC_DEFAULT;
      }
      break;
    
    // Уничтожение препятствий.
    case SPL_REMOVE_OBSTACLE:
      // Восстанавливаем затёртые команды.
      IntAt(c->ebp - 4) = c->edi;
      ByteAt(c->ebp - 13) = 0;
      
      // Очищаем выделенные стеки-цели.
      MemSet(o_BattleMgr->PField<_bool8_>(21628), 0, 40);
      
      // Пропускаем расчёт выделенных стеков.
      c->return_address = 0x59FC2D;
      
      return NO_EXEC_DEFAULT;
      break;
    
    // Жертва.
    case SPL_SACRIFICE:
      // Восстанавливаем затёртые команды.
      IntAt(c->ebp - 4) = c->edi;
      ByteAt(c->ebp - 13) = 0;
      
      // Очищаем выделенные стеки-цели.
      MemSet(o_BattleMgr->PField<_bool8_>(21628), 0, 40);
      
      // Список только из целевого стека.
      if (IntAt(c->ebp + 8) >= 0 && IntAt(c->ebp + 8) <= 186
          && (_int8_)o_BattleMgr->hex[IntAt(c->ebp + 8)].bstack_side >= 0)
      {
        // Целевой стек.
        _BattleStack_* stack = o_BattleMgr->hex[IntAt(c->ebp + 8)].GetCreature();
        // Если стек - не воскрешаемый жертвой, выбираем его.
        if (Spell_For_Drawing_Shadow_IsFirstDlg || stack != o_BattleMgr->hex[o_BattleMgr->Field<_int32_>(68)].GetCreature())
        {
          // Стек не мёртв.
          if (!((stack->creature.flags >> 21) & 1))
          {
            ((_List_<_BattleStack_*>*)(c->ebp - 64))->Append(stack);
            o_BattleMgr->PField<_bool8_>(21628)[stack->side*20 + stack->index_on_side] = TRUE;
          }
        }
      }
      
      // Пропускаем расчёт для площадных.
      c->return_address = 0x59FC2D;
      
      return NO_EXEC_DEFAULT;
      break;
    
    // Не площадные заклинания.
    default:
      // Восстанавливаем затёртые команды.
      IntAt(c->ebp - 4) = c->edi;
      ByteAt(c->ebp - 13) = 0;
      
      // Очищаем выделенные стеки-цели.
      MemSet(o_BattleMgr->PField<_bool8_>(21628), 0, 40);
      
      // Список только из целевого стека.
      if (IntAt(c->ebp + 8) >= 0 && IntAt(c->ebp + 8) <= 186
          && (_int8_)o_BattleMgr->hex[IntAt(c->ebp + 8)].bstack_side >= 0)
      {
        // Целевой стек - выбираем его.
        _BattleStack_* stack = o_BattleMgr->hex[IntAt(c->ebp + 8)].GetCreature();
        // Стек не мёртв.
        if (!((stack->creature.flags >> 21) & 1))
        {
          ((_List_<_BattleStack_*>*)(c->ebp - 64))->Append(stack);
          o_BattleMgr->PField<_bool8_>(21628)[stack->side*20 + stack->index_on_side] = TRUE;
        }
      }
      
      // Пропускаем расчёт для площадных.
      c->return_address = 0x59FC2D;
      
      return NO_EXEC_DEFAULT;
      break;
  }
}

// Добавление учёта одиночных заклинаний в функцию определения текущих целей заклинания (подсветка курсора).
int __stdcall LoHook_SelectSpellTargets_SingleSpellCursorHighlight(LoHook* h, HookContext* c)
{
  // По номеру заклинания.
  switch (c->ecx)
  {
    // Площадные заклинания.
    case SPL_FROST_RING:
    case SPL_FIREBALL:
    case SPL_INFERNO:
    case SPL_METEOR_SHOWER:
    case SPL_BERSERK:
      // Корректный гекс.
      if (IntAt(c->ebp + 8) >= 0 && IntAt(c->ebp + 8) <= 186 && IntAt(c->ebp + 8)%17 != 0 && IntAt(c->ebp + 8)%17 != 16)
      {
        return EXEC_DEFAULT;
      }
      // Некорректный гекс.
      else
      {
        // Отмечаем отсутствие гекса.
        IntAt(c->ebp + 8) = -1;
        
        // Пропускаем расчёт.
        c->return_address = 0x59FD44;
        
        return NO_EXEC_DEFAULT;
      }
      break;
    
    // Уничтожение препятствий.
    case SPL_REMOVE_OBSTACLE:
      // Есть гекс, он корректен.
      if (IntAt(c->ebp + 8) >= 0 && IntAt(c->ebp + 8) <= 186 && IntAt(c->ebp + 8)%17 != 0 && IntAt(c->ebp + 8)%17 != 16)
      {
        // Список только из целевого гекса.
        ((_List_<_int32_>*)(c->ebp - 48))->Append(IntAt(c->ebp + 8));
      }
      else
      {
        IntAt(c->ebp + 8) = -1;
      }
      // Пропускаем расчёт для площадных.
      c->return_address = 0x59FD44;
      
      return NO_EXEC_DEFAULT;
      break;
    
    // Жертва.
    case SPL_SACRIFICE:
      // Есть гекс - либо он корректен, либо на нём есть стек.
      if (IntAt(c->ebp + 8) >= 0 && IntAt(c->ebp + 8) <= 186 && (IntAt(c->ebp + 8)%17 != 0 && IntAt(c->ebp + 8)%17 != 16 || (_int8_)o_BattleMgr->hex[IntAt(c->ebp + 8)].bstack_side >= 0))
      {
        // Если это не воскрешаемый стек - список только из целевого гекса.
        if (Spell_For_Drawing_Shadow_IsFirstDlg || o_BattleMgr->hex[IntAt(c->ebp + 8)].GetCreature() != o_BattleMgr->hex[o_BattleMgr->Field<_int32_>(68)].GetCreature())
        {
          ((_List_<_int32_>*)(c->ebp - 48))->Append(IntAt(c->ebp + 8));
        }
        else
        {
          IntAt(c->ebp + 8) = -1;
        }
      }
      else
      {
        IntAt(c->ebp + 8) = -1;
      }
      
      // Пропускаем расчёт для площадных.
      c->return_address = 0x59FD44;
      
      return NO_EXEC_DEFAULT;
      break;
    
    // Не площадные заклинания.
    default:
      // Есть гекс - либо он корректен, либо на нём есть стек.
      if (IntAt(c->ebp + 8) >= 0 && IntAt(c->ebp + 8) <= 186 && (IntAt(c->ebp + 8)%17 != 0 && IntAt(c->ebp + 8)%17 != 16 || (_int8_)o_BattleMgr->hex[IntAt(c->ebp + 8)].bstack_side >= 0))
      {
        // Список только из целевого гекса.
        ((_List_<_int32_>*)(c->ebp - 48))->Append(IntAt(c->ebp + 8));
      }
      else
      {
        IntAt(c->ebp + 8) = -1;
      }
      
      // Пропускаем расчёт для площадных.
      c->return_address = 0x59FD44;
      
      return NO_EXEC_DEFAULT;
      break;
  }
}


// Запрещаем выделять целью цепной молнии дружественные отряды.
double __stdcall HiHook_GetSpellSelectChance(HiHook* h, _BattleMgr_* this_, _int32_ spell_id, _int32_ side, _BattleStack_* target_stack, _bool8_ check_side, _bool8_ is_first_dlg, _bool32_ by_stack)
{
  // Цепную молниию можно посылать только на вражеские отряды.
  if (spell_id == SPL_CHAIN_LIGHTNING && target_stack->side == side)
  {
    return 0.0;
  }
  // Остальные заклинания - стандартно (но заменяем то, первый диалог или нет на верную информацию).
  else
  {
    return CALL_7(double, __thiscall, h->GetDefaultFunc(), this_, spell_id, side, target_stack, check_side, Spell_For_Drawing_Shadow_IsFirstDlg, by_stack);
  }
}




// Вместо невыделения гексов стеков, не выделяем гексы соответствующих препятствий для удаление препятстсвий.
int __stdcall LoHook_SelectSpellTargets_ChangeNotHighlightesHexes(LoHook* h, HookContext* c)
{
  // Это уничтожение препятствий.
  if (IntAt(c->ebp - 28) == SPL_REMOVE_OBSTACLE)
  {
    // Есть препятствие.
    if (o_BattleMgr->hex[c->eax].ObstacleNum >= 0)
    {
      // Препятствие.
      _BattleObstackle_* obst = &o_BattleMgr->Field<_BattleObstackle_*>(81244)[o_BattleMgr->hex[c->eax].ObstacleNum];
      
      // Не можем убрать его.
      if (!CALL_7(_bool8_, __thiscall, 0x5A3CD0, o_BattleMgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, c->eax, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack)
          && (obst->side == o_BattleMgr->currentActiveSide || obst->visible || o_BattleMgr->Field<_bool8_>(81269)))
      {
        // Убираем выделение гекса.
        c->return_address = 0x59FD98;
        
        return NO_EXEC_DEFAULT;
      }
    }
    
    // Не убираем выделение гекса.
    c->return_address = 0x59FDC2;
    
    return NO_EXEC_DEFAULT;
  }
  
  return EXEC_DEFAULT;
}





// Разрешаем неплощадным заклинаниям колдовать на гекс вне поля боя, если там есть стек.
int __stdcall LoHook_SelectSpellTargets_OutHexes(LoHook* h, HookContext* c)
{
  // На гексе есть стек и это не площадное заклинание (и не удаление препятствий).
  if (!ByteAt(c->ebp - 1) && c->esi != SPL_REMOVE_OBSTACLE && (_int8_)o_BattleMgr->hex[c->edi].bstack_side >= 0)
  {
    // Пропускаем проверку крайних гексов.
    c->return_address = 0x59FAB3;
  
    return NO_EXEC_DEFAULT;
  }
  else
  {
    return EXEC_DEFAULT;
  }
}



// Станадртный диалог выбора цели заклинания - номер последнего гекса.
_int_ SelectSpellTarDlg_LastHex_Ix = -1;

// Перевыделяем гекс, если ни один не был выделен.
int __stdcall LoHook_SelectSpellTargetsSingle_ReselectHexIfNoSelected(LoHook* h, HookContext* c)
{
  // Гекс не изменился.
  if (c->eax == SelectSpellTarDlg_LastHex_Ix)
  {
    // Не выделяем гекс.
    c->return_address = 0x5A33E8;
  
    return NO_EXEC_DEFAULT;
  }
  // Гекс изменился.
  else
  {
    // Новый гекс.
    SelectSpellTarDlg_LastHex_Ix = c->eax;
    
    // Рассчитываем выделение гекса.
    c->return_address = 0x5A33DC;
  
    return NO_EXEC_DEFAULT;
  }
}


// Тень курсора препятствия.
int __stdcall LoHook_MagicObst_MouseShadow(LoHook* h, HookContext* c)
{
  // Список подсвечиваемых гексов.
  _List_<_int32_>* hexes_lst = (_List_<_int32_>*)(c->ebp - 44);
  
  // Гекс препятствия.
  _int_ obst_hex_ix = c->esi;
  hexes_lst->Append(obst_hex_ix);
  
  // Второй гекс препятствия.
  hexes_lst->Append(MagicObst_GetHexIx(c->edi, Spell_For_Drawing_Shadow_Side, c->ebx, obst_hex_ix, 1));
  
  // Третий гекс препятствия.
  if (c->ebx >= 2)
  {
    hexes_lst->Append(MagicObst_GetHexIx(c->edi, Spell_For_Drawing_Shadow_Side, c->ebx, obst_hex_ix, 2));
  }
  
  // Пропускаем собственное составление списка выделение гексов.
  c->return_address = 0x5A36F1;
  
  return NO_EXEC_DEFAULT;
}




// Выбранный гекс препятствия.
int __stdcall LoHook_MagicObst_SelectedHex(LoHook* h, HookContext* c)
{
  // Гекс препятствия.
  c->eax = MagicObst_GetHexToCast(o_BattleMgr, c->eax, c->edi, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_Skill);
  
  return EXEC_DEFAULT;
}




// Можно ли колдовать препятствия на гекс.
int __stdcall LoHook_MagicObst_CanCast(LoHook* h, HookContext* c)
{
  // Можно ставить препятствие.
  if (c->esi != ID_NONE)
  {
    c->return_address = 0x5A3628;
  }
  else
  {
    c->return_address = 0x5A3724;
  }
  
  return NO_EXEC_DEFAULT;
}



// Определяем, можно ли телепортировать стек на этот гекс и отрисовываемм тень курсора.
int __stdcall LoHook_Teleport_CanCast_Highlight(LoHook* h, HookContext* c)
{
  // Гекс, на который надо телепортировать.
  _int_ sel_hex = Teleport_GetHexToCast(o_BattleMgr, c->esi, (_BattleStack_*)c->eax);
  
  // Рисуем подсветку.
  if (DwordAt(0x698810))
  {
    Teleport_SelectHex_Highlight(o_BattleMgr, c->esi, sel_hex, (_BattleStack_*)c->eax);
  }
  
  // Можно телепортировать.
  if (sel_hex != ID_NONE)
  {
    // Выбранный гекс для телепорта.
    IntAt(0x688384) = sel_hex;
    
    c->return_address = 0x5A3B63;
  }
  // Нельзя телепортировать.
  else
  {
    c->return_address = 0x5A3B95;
  }
  
  return NO_EXEC_DEFAULT;
}



// Тень курсора для особых заклинаний.
int __stdcall LoHook_SelectSpecSpellTarget_HighlightMouseHex(LoHook* h, HookContext* c)
{
  // Рисуем подсветку.
  CALL_3(void, __fastcall, 0x59FBB0, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, c->esi);
  
  return NO_EXEC_DEFAULT;
}




// Можно ли колдовать жертву.
int __stdcall LoHook_Sacrifice_CanCast(LoHook* h, HookContext* c)
{
  
  // По всем гексам.
  for (_int_ i = 0; i < 187; i++)
  {
    // Гекс корректен и на него можно колдовать.
    if (i%17 != 0 && i%17 != 16 && CALL_7(_bool8_, __thiscall, 0x5A3CD0, o_BattleMgr, SPL_SACRIFICE, Spell_For_Drawing_Shadow_Skill, i, Spell_For_Drawing_Shadow_Side, TRUE, Spell_For_Drawing_Shadow_ByStack))
    {
      // Первый выбранный стек.
      _BattleStack_* first_stack = CALL_6(_BattleStack_*, __thiscall, 0x5A3C60, o_BattleMgr, SPL_SACRIFICE, Spell_For_Drawing_Shadow_Side, i, TRUE, Spell_For_Drawing_Shadow_ByStack);
      
      // По всем гексам.
      for (_int_ j = 0; j < 187; j++)
      {
        // Гекс корректен и на него можно колдовать.
        if (j%17 != 0 && j%17 != 16 && CALL_7(_bool8_, __thiscall, 0x5A3CD0, o_BattleMgr, SPL_SACRIFICE, Spell_For_Drawing_Shadow_Skill, j, Spell_For_Drawing_Shadow_Side, FALSE, Spell_For_Drawing_Shadow_ByStack))
        {
          // Второй целевой стек не тот же, что и первый.
          if (o_BattleMgr->hex[j].GetCreature() != first_stack)
          {
            // Можно колдовать.
            c->return_address = 0x59F628;
            
            return NO_EXEC_DEFAULT;
          }
        }
      }
    }
  }
  
  
  
  // Ничего не нашли - нельзя колдовать.
  c->return_address = 0x59F5E8;
    
  return NO_EXEC_DEFAULT;
}



// Сброс выбранных стеков после диалога выбора цели заклинания.
int __stdcall LoHook_SpellSelectTargetSlg_Reset_Selected_Stacks(LoHook* h, HookContext* c)
{
  // По всем стекам всех сторон.
  for (_int_ side = ATTACKER; side <= DEFENDER; side++)
  {
    for (_int_ i = 0; i < o_BattleMgr->countMonsters[side]; i++)
    {
      // Стек не мёртв.
      if (!((o_BattleMgr->stack[side][i].creature.flags >> 21) & 1))
      {
        // Сбрасываем выделение.
        CALL_2(_bool8_, __thiscall, 0x43ED00, &o_BattleMgr->stack[side][i], FALSE);
      }
    }
  }
  
  // Сбрасываем выбранные диалогом заклинания стеки.
  MemSet(StacksSelectedBySpellDlg, 0, 42);
  
  return EXEC_DEFAULT;
}




// Сброс выбранных диалогом заклинаия стеков.
int __stdcall LoHook_StacksSelectedBySpellDlg_Reset(LoHook* h, HookContext* c)
{
  // Сбрасываем выбранные диалогом заклинания стеки.
  MemSet(StacksSelectedBySpellDlg, 0, 42);
  
  // Сбрасываем последний выделенный гекс.
  SelectSpellTarDlg_LastHex_Ix = -1;
  
  return EXEC_DEFAULT;
}




// Первое выделение - обычное заклинание.
int __stdcall LoHook_FirstSelect_SimpleSpell(LoHook* h, HookContext* c)
{
  // Менеджер битвы.
  _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
  
  // Получаем координаты курсора мыши.
  _int_ x;
  _int_ y;
  o_GetIngameCursorPos(&x, &y);
  
  // Выбранный гекс.
  _int_ hex_ix = b_mgr->GetHexIxAtXY(x, y);
  
  // Выделяем гекс.
  IntAt(0x688374) = CALL_1(_int32_, __fastcall, 0x59FA10, hex_ix);
  
  return EXEC_DEFAULT;
}



// Первое выделение - заклинание-препятствие.
int __stdcall LoHook_FirstSelect_ObstSpell(LoHook* h, HookContext* c)
{
  // Менеджер битвы.
  _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
  
  // Получаем координаты курсора мыши.
  _int_ x;
  _int_ y;
  o_GetIngameCursorPos(&x, &y);
  
  // Выбранный гекс.
  _int_ hex_ix = b_mgr->GetHexIxAtXY(x, y);
  
  // Гекс-цель.
  _int_ sel_hex = MagicObst_GetHexToCast(b_mgr, hex_ix, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_Skill);
  
   // Выбранный гекс.
  IntAt(0x688378) = sel_hex;
  
  // Выбран корректный гекс.
  if (sel_hex != ID_NONE)
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(b_mgr->Field<_int32_>(64) + 1, 3);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A89A0, b_mgr, Spell_For_Drawing_Shadow, sel_hex, TRUE);
    
    // Подсветка курсора.
    if (DwordAt(0x698810))
    {
      // Список подсвечиваемых гексов.
      _List_<_int32_> hexes_lst;
      
      // Гекс препятствия.
      hexes_lst.Append(sel_hex);
      
      // Второй гекс препятствия.
      hexes_lst.Append(MagicObst_GetHexIx(Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_Skill, sel_hex, 1));
      
      // Третий гекс препятствия.
      if (Spell_For_Drawing_Shadow_Skill >= 2)
      {
        hexes_lst.Append(MagicObst_GetHexIx(Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_Skill, sel_hex, 2));
      }
      
      // Рисуем подсветку.
      CALL_4(void, __thiscall, 0x493A20, b_mgr, sel_hex, &hexes_lst, FALSE);
    }
    
  }
  // Выбран некорректный гекс.
  else
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(0, 2);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A2F70, b_mgr, Spell_For_Drawing_Shadow, o_GENRLTXT_Txt_->GetString(24), sel_hex);
    
    // Подсветка курсора.
    if (DwordAt(0x698810))
    {
      // Список подсвечиваемых гексов.
      _List_<_int32_> hexes_lst;
      
      // Рисуем подсветку.
      CALL_4(void, __thiscall, 0x493A20, b_mgr, sel_hex, &hexes_lst, FALSE);
    }
  }
  
  return EXEC_DEFAULT;
}



// Первое выделение - телепорт (выбор стека).
int __stdcall LoHook_FirstSelect_TeleportSpell_Stack(LoHook* h, HookContext* c)
{
  // Менеджер битвы.
  _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
  
  // Получаем координаты курсора мыши.
  _int_ x;
  _int_ y;
  o_GetIngameCursorPos(&x, &y);
  
  // Выбранный гекс.
  _int_ hex_ix = b_mgr->GetHexIxAtXY(x, y);
  
  // Гекс-цель.
  _int_ sel_hex = hex_ix;
  
  IntAt(0x688380) = hex_ix;
  
  // Выбран корректный гекс.
  if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack))
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(b_mgr->Field<_int32_>(64) + 1, 3);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A89A0, b_mgr, Spell_For_Drawing_Shadow, sel_hex, TRUE);
    
    // Выделяем стек.
    CALL_3(void, __fastcall, 0x59FBB0, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex);
    
  }
  // Выбран некорректный гекс.
  else
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(0, 2);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A2F70, b_mgr, Spell_For_Drawing_Shadow, o_GENRLTXT_Txt_->GetString(24), sel_hex);
    
    // Выделяем стек.
    CALL_3(void, __fastcall, 0x59FBB0, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex);
  }
  
  return EXEC_DEFAULT;
}


// Первое выделение - телепорт (выбор гекса).
int __stdcall LoHook_FirstSelect_TeleportSpell_Hex(LoHook* h, HookContext* c)
{
  // Менеджер битвы.
  _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
  
  // Некорректно выбран гекс в первом диалоге.
  if (b_mgr->Field<_int32_>(68) < 0 || b_mgr->Field<_int32_>(68) > 186) return EXEC_DEFAULT;
  
  // Получаем координаты курсора мыши.
  _int_ x;
  _int_ y;
  o_GetIngameCursorPos(&x, &y);
  
  // Выбранный гекс.
  _int_ hex_ix = b_mgr->GetHexIxAtXY(x, y);
  
  IntAt(0x688384) = hex_ix;
  
  // Гекс-цель.
  _int_ sel_hex = Teleport_GetHexToCast(b_mgr, hex_ix, b_mgr->hex[b_mgr->Field<_int32_>(68)].GetCreature());
  
  // Выбран корректный гекс.
  if (sel_hex != ID_NONE)
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(19, 2);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A89A0, b_mgr, Spell_For_Drawing_Shadow, sel_hex, TRUE);
    
    // Подсветка курсора.
    if (DwordAt(0x698810))
    {
      Teleport_SelectHex_Highlight(b_mgr, hex_ix, sel_hex, b_mgr->hex[b_mgr->Field<_int32_>(68)].GetCreature());
    }
    
  }
  // Выбран некорректный гекс.
  else
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(0, 2);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A2F70, b_mgr, Spell_For_Drawing_Shadow, o_GENRLTXT_Txt_->GetString(25), sel_hex);
    
    // Подсветка курсора.
    if (DwordAt(0x698810))
    {
      Teleport_SelectHex_Highlight(b_mgr, hex_ix, sel_hex, b_mgr->hex[b_mgr->Field<_int32_>(68)].GetCreature());
    }
  }
  
  return EXEC_DEFAULT;
}


// Первое выделение - жертва (выбор воскрешаемого).
int __stdcall LoHook_FirstSelect_SacrificeSpell_Ressur(LoHook* h, HookContext* c)
{
  // Менеджер битвы.
  _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
  
  // Получаем координаты курсора мыши.
  _int_ x;
  _int_ y;
  o_GetIngameCursorPos(&x, &y);
  
  // Выбранный гекс.
  _int_ hex_ix = b_mgr->GetHexIxAtXY(x, y);
  
  // Гекс-цель.
  _int_ sel_hex = hex_ix;
  
  IntAt(0x68836C) = hex_ix;
  
  // Выбран корректный гекс.
  if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack))
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(b_mgr->Field<_int32_>(64) + 1, 3);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A89A0, b_mgr, Spell_For_Drawing_Shadow, sel_hex, TRUE);
    
    // Выделяем стек.
    CALL_3(void, __fastcall, 0x59FBB0, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex);
    
    ByteAt(0x6A3D34) = TRUE;
  }
  // Выбран некорректный гекс.
  else
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(0, 2);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A2F70, b_mgr, Spell_For_Drawing_Shadow, o_GENRLTXT_Txt_->GetString(543), sel_hex);
    
    // Выделяем стек.
    CALL_3(void, __fastcall, 0x59FBB0, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex);
    
    ByteAt(0x6A3D34) = FALSE;
  }
  
  return EXEC_DEFAULT;
}


// Первое выделение - жертва (выбор жертвы).
int __stdcall LoHook_FirstSelect_SacrificeSpell_Sacr(LoHook* h, HookContext* c)
{
  // Менеджер битвы.
  _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;
  
  // Получаем координаты курсора мыши.
  _int_ x;
  _int_ y;
  o_GetIngameCursorPos(&x, &y);
  
  // Выбранный гекс.
  _int_ hex_ix = b_mgr->GetHexIxAtXY(x, y);
  
  // Гекс-цель.
  _int_ sel_hex = hex_ix;
  
  IntAt(0x688370) = hex_ix;
  
  // Выбран корректный гекс и это не выбранный ранее стек.
  if (CALL_7(_bool8_, __thiscall, 0x5A3CD0, b_mgr, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex, Spell_For_Drawing_Shadow_Side, Spell_For_Drawing_Shadow_IsFirstDlg, Spell_For_Drawing_Shadow_ByStack)
      && b_mgr->hex[sel_hex].GetCreature() != b_mgr->hex[b_mgr->Field<_int32_>(68)].GetCreature())
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(18, 2);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A89A0, b_mgr, Spell_For_Drawing_Shadow, sel_hex, TRUE);
    
    // Выделяем стек.
    CALL_3(void, __fastcall, 0x59FBB0, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex);
    
    ByteAt(0x6A3D38) = TRUE;
  }
  // Выбран некорректный гекс.
  else
  {
    // Устанавливаем курсор мыши.
    b_MouseMan_SetCursor(0, 2);
    
    // Пишем подсказку в лог.
    CALL_4(void, __thiscall, 0x5A2F70, b_mgr, Spell_For_Drawing_Shadow, o_GENRLTXT_Txt_->GetString(544), sel_hex);
    
    // Выделяем стек.
    CALL_3(void, __fastcall, 0x59FBB0, Spell_For_Drawing_Shadow, Spell_For_Drawing_Shadow_Skill, sel_hex);
    
    ByteAt(0x6A3D38) = FALSE;
  }
  
  return EXEC_DEFAULT;
}





// Первое выделение - установка выбранного гекса обычному и площадному заклинанию.
int __stdcall LoHook_FirstSelect_SimpleOrAreaSpell_SetSelHex(LoHook* h, HookContext* c)
{
  // Устанавливаем гекс.
  IntAt(0x688374) = c->eax;
  
  return EXEC_DEFAULT;
}






// Меняем цвет рамки выбранному целью стеку.
int __stdcall LoHook_SpellSelectTarget_Bordrer(LoHook* h, HookContext* c)
{
  // Отрисовываемый стек.
  _BattleStack_* stack = (_BattleStack_*)c->ebx;
  
  // Стек выбран целью заклинания.
  if (StacksSelectedBySpellDlg[stack->side][stack->index_on_side])
  {
    // Меняем цвет рамки.
    IntAt(c->ebp - 20) = 0x70;
  }
  
  return EXEC_DEFAULT;
}




// Инициализация подсветки диалогов наложения заклинаний.
void SpellsHighlightInit()
{
  
  // Убираем собственную работу с тенью перермещения и подсветкой гексов.
  _PI->WriteCodePatch(0x59F2AE, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x59F2C1, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x59F2C6, "%n", 31); // 31 nop
  _PI->WriteCodePatch(0x59F35B, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x59F368, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x59F36D, "%n", 31); // 31 nop
  _PI->WriteCodePatch(0x59F326, "%n", 48); // 48 nop
  
  // Устанавливаем параметры заклинания, для которого будет рисоваться тень.
  _PI->WriteHiHook(0x59EF60, SPLICE_, EXTENDED_, THISCALL_, HiHook_Battle_SelectSpellTarget_SaveSpell);
  
  // Переход к второму диалогу.
  _PI->WriteLoHook(0x59F486, LoHook_Battle_SelectSpellTarget_SecondDlg); // Телепорт
  _PI->WriteLoHook(0x59F651, LoHook_Battle_SelectSpellTarget_SecondDlg); // Жертва
  
  
  
  
  // Тень доступных гексов.
  
  // Создание тени перемещения.
  _PI->WriteLoHook(0x59F13E, LoHook_SelectSpell_NewShadow); // Обычные заклинания
  _PI->WriteLoHook(0x59F2E5, LoHook_SelectSpell_NewShadow); // Площадные заклинания
  _PI->WriteLoHook(0x59F35B, LoHook_SelectSpell_NewShadow); // Силовое поле, стена огня
  _PI->WriteLoHook(0x59F45D, LoHook_SelectSpell_NewShadow); // Телепорт - выбор стека
  _PI->WriteLoHook(0x59F48C, LoHook_SelectSpell_NewShadow); // Телепорт - выбор гекса
  _PI->WriteLoHook(0x59F631, LoHook_SelectSpell_NewShadow); // Жертва - выбор воскрешаемого
  _PI->WriteLoHook(0x59F657, LoHook_SelectSpell_NewShadow); // Жертва - выбор жертвуемого
  _PI->WriteLoHook(0x59F72B, LoHook_SelectSpell_NewShadow); // Убрать препятствие, клон
  
  // Восстановление обычной тени перемещения.
  _PI->WriteLoHook(0x59F96C, LoHook_SelectSpell_RestoreShadow);
  
  
  
  // Диалог обычных и площадных заклинаний.
  
  // Для одиночных заклинаний вызываем ту же функцию подсветки, что и для площадных.
  _PI->WriteCodePatch(0x59FB32, "%n", 5); // 5 nop
  _PI->WriteHexPatch(0x59FB37, "EB"); // jmp ...
  _PI->WriteCodePatch(0x59FB87, "%n", 7); // 7 nop
  
  // Разрешаем неплощадным заклинаниям колдовать на гекс вне поля боя, если там есть стек.
  _PI->WriteLoHook(0x59FA98, LoHook_SelectSpellTargets_OutHexes);
  
  // Перевыделяем гекс, если ни один не был выделен.
  _PI->WriteLoHook(0x5A33D4, LoHook_SelectSpellTargetsSingle_ReselectHexIfNoSelected);
  
  // Добавление учёта одиночных заклинаний в функцию определения текущих целей заклинания.
  _PI->WriteLoHook(0x59FBED, LoHook_SelectSpellTargets_SingleSpell);
  
  // Добавление учёта одиночных заклинаний в функцию определения текущих целей заклинания (подсветка курсора).
  _PI->WriteLoHook(0x59FD04, LoHook_SelectSpellTargets_SingleSpellCursorHighlight);
  
  // Запрещаем выделять целью цепной молнии дружественные отряды.
  _PI->WriteHiHook(0x59FC73, CALL_, EXTENDED_, THISCALL_, HiHook_GetSpellSelectChance);
  
  // Вместо невыделения гексов стеков, не выделяем гексы соответствующих препятствий для удаление препятстсвий.
  _PI->WriteLoHook(0x59FD6D, LoHook_SelectSpellTargets_ChangeNotHighlightesHexes);
  
  // Первое выделение - установка выбранного гекса обычному заклинанию.
  _PI->WriteLoHook(0x59F16B, LoHook_FirstSelect_SimpleOrAreaSpell_SetSelHex);
  
  // Первое выделение - установка выбранного гекса площадному заклинанию.
  _PI->WriteLoHook(0x59F312, LoHook_FirstSelect_SimpleOrAreaSpell_SetSelHex);
  
  // Первое выделение удаления препятствий и клона.
  _PI->WriteLoHook(0x59F737, LoHook_FirstSelect_SimpleSpell);
  
  
  
  
  // Диалог постановки стены огня и силового поля.
  
  // Тень курсора препятствия.
  _PI->WriteLoHook(0x5A3675, LoHook_MagicObst_MouseShadow);
  
  // Выбранный гекс препятствия.
  _PI->WriteLoHook(0x5A35E1, LoHook_MagicObst_SelectedHex);
  
  // Можно ли колдовать препятствия на гекс.
  _PI->WriteLoHook(0x5A360D, LoHook_MagicObst_CanCast);
  
  // Первое выделение.
  _PI->WriteLoHook(0x59F38C, LoHook_FirstSelect_ObstSpell);
  
  
  // Диалог телепорта.
  
  // Определяем, можно ли телепортировать стек на этот гекс и отрисовываем тень курсора.
  _PI->WriteCodePatch(0x5A3B6D, "%n", 6); // 6 nop
  _PI->WriteLoHook(0x5A3B52, LoHook_Teleport_CanCast_Highlight);
  
  // Тень курсора при выборе стека.
  _PI->WriteCodePatch(0x5A391B, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x5A3967, "%n", 6); // 6 nop
  _PI->WriteLoHook(0x5A3921, LoHook_SelectSpecSpellTarget_HighlightMouseHex);
  _PI->WriteLoHook(0x5A396D, LoHook_SelectSpecSpellTarget_HighlightMouseHex);
  
  // Первое выделение - телепорт (выбор стека).
  _PI->WriteLoHook(0x59F466, LoHook_FirstSelect_TeleportSpell_Stack);
  
  // Первое выделение - телепорт (выбор гекса).
  _PI->WriteLoHook(0x59F495, LoHook_FirstSelect_TeleportSpell_Hex);
  
  // Сброс выбранных стеков после диалога выбора стека телепорта.
  _PI->WriteLoHook(0x5A39AD, LoHook_SpellSelectTargetSlg_Reset_Selected_Stacks);
  
  // Сброс выбранных стеков после диалога выбора гекса телепорта.
  _PI->WriteLoHook(0x5A3C08, LoHook_SpellSelectTargetSlg_Reset_Selected_Stacks);
  
  
  // Диалог жертвы.
  
  // Выбор жертвуемого - тень курсора.
  _PI->WriteCodePatch(0x5A30BC, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x5A3103, "%n", 6); // 6 nop
  _PI->WriteLoHook(0x5A30C2, LoHook_SelectSpecSpellTarget_HighlightMouseHex);
  _PI->WriteLoHook(0x5A3109, LoHook_SelectSpecSpellTarget_HighlightMouseHex);
  
  // Выбор жертвы - тень курсора.
  _PI->WriteCodePatch(0x5A329D, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x5A32E3, "%n", 6); // 6 nop
  _PI->WriteLoHook(0x5A32A3, LoHook_SelectSpecSpellTarget_HighlightMouseHex);
  _PI->WriteLoHook(0x5A32E9, LoHook_SelectSpecSpellTarget_HighlightMouseHex);
  
  // Можно ли колдовать жертву.
  _PI->WriteLoHook(0x59F4BA, LoHook_Sacrifice_CanCast);
  
  // Первое выделение - жертва (выбор воскрешаемого).
  _PI->WriteLoHook(0x59F63A, LoHook_FirstSelect_SacrificeSpell_Ressur);
  
  // Первое выделение - жертва (выбор жертвы).
  _PI->WriteLoHook(0x59F65E, LoHook_FirstSelect_SacrificeSpell_Sacr);
  
  // Сброс выбранных стеков после диалога выбора воскрешаемого жертвы.
  _PI->WriteLoHook(0x5A313A, LoHook_SpellSelectTargetSlg_Reset_Selected_Stacks);
  
  // Сброс выбранных стеков после диалога выбора жертвы.
  _PI->WriteLoHook(0x5A331A, LoHook_SpellSelectTargetSlg_Reset_Selected_Stacks);
  
  
  
  // Особая обводка стека для диалога жертвы и телепорта.
  
  // Сбрасываем выбранные диалогом заклинания стеки.
  MemSet(StacksSelectedBySpellDlg, 0, 42);
  
  // Меняем цвет рамки выбранному целью стеку.
  _PI->WriteLoHook(0x43DF88, LoHook_SpellSelectTarget_Bordrer);
  
  // Сброс выбранных диалогом заклинаия стеков.
  _PI->WriteLoHook(0x5A3474, LoHook_StacksSelectedBySpellDlg_Reset);
  _PI->WriteLoHook(0x5A3518, LoHook_StacksSelectedBySpellDlg_Reset);
  
}





// Во время инициализации битвы trandint всегда работает по времени.
int __stdcall LoHook_Battle_Trandint(LoHook* h, HookContext* c)
{
  // Подготовка битвы - всегда по времени.
  if (IsBattleInit)
  {
    c->return_address = 0x50B3F1;
    
    return NO_EXEC_DEFAULT;
  }
  // Не подготовка битвы - стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}





// Исправлем отсутствие анимации стека после воскрешения до конца боевой анимации.
int __stdcall LoHook_Resurrect_Alive_AfterAnimEnd(LoHook* h, HookContext* c)
{
  // Оживляем стек.
  IntAt(c->esi + 132) &= ~BCF_DEAD;
  
  return EXEC_DEFAULT;
}




// После полёта делаем кадр стека допустимым.
int __stdcall HookOn_StackFlightDraw_MakeFrameAllowed(LoHook* h, HookContext* c)
{
  // Делаем кадр допустимым.
  if (((_BattleStack_*)c->esi)->def_frame_ix >= IntAt(c->ebp - 8))
  {
    ((_BattleStack_*)c->esi)->def_frame_ix = IntAt(c->ebp - 8) - 1;
  }
  
  return EXEC_DEFAULT;
}


// Надо ли пропускать проверку на однотипность действия в реакции на наведение курсора в бою.
_bool_ BWU_SkipChek = FALSE;

// Пропускаем проверку на однотипность действия в реакции на наведение курсора в бою.
int __stdcall LoHook_BWU_SkipCheck(LoHook* h, HookContext* c)
{
  if (BWU_SkipChek)
  {
    // Затёртая команда.
    ByteAt(c->ebp - 1) = 0;
    
    // Пропускаем проверку.
    c->return_address = 0x474CF9;
    
    return NO_EXEC_DEFAULT;
  }
  else
  {
    return EXEC_DEFAULT;
  }
}


// При изменении сообщения в логе сбрасываем ожидание.
int __stdcall LoHook_LogResetWait(LoHook* h, HookContext* c)
{
  // Нет ожидаемых сообщений.
  IntAt(c->ebx + 100) = 0;
  
  return EXEC_DEFAULT;
}




// При начале получения повреждений сбрасываем то, что он играл анимацию выстрела.
int __stdcall LoHook_DrawActionPlay_ResetShootAnim(LoHook* h, HookContext* c)
{
  // Не играет выстрел.
  ByteAt(c->esi - 59) = 0;
  
  return EXEC_DEFAULT;
}





// Вовремя прекращаем анимацию массового заклинания.
int __stdcall LoHook_MassSpell_EndStackAnim(LoHook* h, HookContext* c)
{
  // Кадры не кончились.
  if (c->edx < c->eax)
  {
    return EXEC_DEFAULT;
  }
  // Кадры кончились.
  else
  {
    // Стек.
    _BattleStack_* stack = (_BattleStack_*)(c->esi - 64);
    
    // На стеке не играет анимация.
    stack->Field<_bool8_>(32) = 0;
    
    return EXEC_DEFAULT;
  }
}




// Закрытие менеджера битвы.
void __stdcall HiHook_BattleMgr_Finish(HiHook* h, _BattleMgr_* this_)
{
  // Удаляем def большого препятствия.
  if (LargeObstackleDef)
  {
    LargeObstackleDef->DerefOrDestruct();
  }
  
  
  CALL_1(void, __thiscall, h->GetDefaultFunc(), this_);
}



// При наложении заклинания сбрасываем анимацию стека при наложении обездвиживающего.
void __stdcall HiHook_ApplySpell(HiHook* h, _BattleStack_* this_, _int32_ spell_id, _int32_ spell_power, _int32_ skill, _Hero_* hero)
{
  // Накладываем заклинание.
  CALL_5(void, __thiscall, h->GetDefaultFunc(), this_, spell_id, spell_power, skill, hero);
  
  // Сброс анимации.
  if (spell_id == SPL_BLIND || spell_id == SPL_STONE || spell_id == SPL_PARALYZE)
  {
    this_->def_group_ix = DG_STAY;
    this_->def_frame_ix = 0;
  }
}



// Инициализация модуля.
void CombatFixes(PatcherInstance* _PI)
{
  
    int a = 200;
  // Инициализация модуля боевых фигур.
  BattleFigures_Init();
  
  // Перевыделяем память под периоды анимации существ.
  if (StayAnimPeriod) free(StayAnimPeriod);
  StayAnimPeriod = (DWORD*)MemAlloc(a * 4);

  // Перевыделяем память под периоды случайной анимации существ.
  if (RandAnimPeriod) free(RandAnimPeriod);
  RandAnimPeriod = (DWORD*)MemAlloc(a * 4);

  // Перевыделяем память под случайное отклонение времени между кадрами анимации стойки от настроенного значения.
  if (StayAnim_Rand_Devi) free(StayAnim_Rand_Devi);
  StayAnim_Rand_Devi = (double*)MemAlloc(a * sizeof(double));

  // Перевыделяем память под переменные необходимости проигрывать анимацию стойки.
  if (StayAnimNeedPlay) free(StayAnimNeedPlay);
  StayAnimNeedPlay = (_bool_*)MemAlloc(a * 4);



  // Перевыделяем память под номера анимаций взрыва снаряда существ.
  if (BulletExplAnim_ID) free(BulletExplAnim_ID);
  BulletExplAnim_ID = (_int_*)MemAlloc(a * 4);
  MemSet((_ptr_)BulletExplAnim_ID, -1, a * 4);

  // Перевыделяем память под имена звуков взрывов снарядов существ.
  if (BulletExpl_SoundName) free(BulletExpl_SoundName);
  BulletExpl_SoundName = (_cstr_*)MemAlloc(a * 4);
  MemZero((_ptr_)BulletExpl_SoundName, a * 4);



  // Перевыделяем память под количество жилищ каждого существа.
  if (Creatures_DwellingsCount) free(Creatures_DwellingsCount);
  Creatures_DwellingsCount = (_bool_*)MemAlloc(a * 4);

  
  // Закрытие менеджера битвы.
  _PI->WriteHiHook(0x462E40, SPLICE_, EXTENDED_, THISCALL_, HiHook_BattleMgr_Finish);
  
  
  // Пропускаем проверку на однотипность действия в реакции на наведение курсора в бою.
  _PI->WriteLoHook(0x474C6E, LoHook_BWU_SkipCheck);
  
  // При изменении сообщения в логе сбрасываем ожидание.
  _PI->WriteLoHook(0x472A5B, LoHook_LogResetWait);
  
  
  
  // При наложении заклинания сбрасываем анимацию стека при наложении обездвиживающего.
  _PI->WriteHiHook(0x444610, SPLICE_, EXTENDED_, THISCALL_, HiHook_ApplySpell);
  
  
  
  // Задержки проигрывания звука.
  
  // При загрузке и старте звука делаем его параллельным.
  _PI->WriteHiHook(0x59A770, SPLICE_, EXTENDED_, FASTCALL_1, HookOn_Load_Start_Sample);
  
  // При загрузке и старте звука предбитвенного звука загружаем и начинаем его как обычно.
  _PI->WriteHiHook(0x4626EA, CALL_, EXTENDED_, FASTCALL_1, HookOn_Load_Start_PreBattle_Sample);
  
  
  // Убираем стандартные оканчивания звуков.
  _PI->WriteHiHook(0x419D45, CALL_, DIRECT_, STDCALL_, HookOn_End_Sample_Std);
  _PI->WriteHiHook(0x41A99A, CALL_, DIRECT_, STDCALL_, HookOn_End_Sample_Std);
  _PI->WriteHiHook(0x4AE364, CALL_, DIRECT_, STDCALL_, HookOn_End_Sample_Std);
  
  
  // При ожидании и окончании проигрывания звука пропускаем это для всех звуков, кроме предбитвенного.
  _PI->WriteHiHook(0x59A7C0, SPLICE_, EXTENDED_, THISCALL_, HookOn_Wait_End_Close_Sample);
  // При ожидании и окончании проигрывания предбитвенного звука...
  _PI->WriteHiHook(0x462C2B, CALL_, EXTENDED_, THISCALL_, HookOn_Wait_End_Close_PreBattleSample);
  
  // При расчёте времени проигрывании звука в бою учитываем его настройки скорости.
  _PI->WriteLoHook(0x59A7D2, HookOn_Wait_End_Close_Sample_CalcTime);
  
  // При ожидании и окончании звука в бою также отрисовываем анимацию.
  _PI->WriteLoHook(0x59A7E3, HookOn_Wait_End_Close_Sample_Play);
  
  
  
  // При ожидании проигрывания звука пропускаем его.
  _PI->WriteHiHook(0x59A1C0, SPLICE_, EXTENDED_, THISCALL_, HookOn_Wait_Sample);
  
  // При расчёте времени ожидания звука в бою учитываем его настройки скорости.
  _PI->WriteLoHook(0x59A1DA, HookOn_Wait_Sample_CalcTime);
  
  // При ожидании звука в бою также отрисовываем анимацию.
  _PI->WriteLoHook(0x59A1DF, HookOn_Wait_Sample_Play);
  
  
  
  // Затираем стандартную инициализацию времён случайных анимаций и анимации ожидания.
  _PI->WriteCodePatch(0x462C6C, "%n", 17); // 17 nop
  
  
  // При инициализации интерфейса битвы отключаем возможность тактического режима, чтобы кнопки не закрывали окно лога.
  _PI->WriteHiHook(0x4629AD, CALL_, EXTENDED_, THISCALL_, HookOn_PreBattle_InterfaceInit);
  // Позже инициализируем интерфейс как надо и инициализируем времена случайной анимации.
  _PI->WriteLoHook(0x462DEB, HookOn_LaterPreBattle_InterfaceInit);
  
  
  
  // Подменяем способ запуска звука повреждения стены.
  
  _PI->WriteHiHook(0x445EB9, CALL_, DIRECT_, FASTCALL_1, HookOn_Some_LoadWav);
  _PI->WriteHiHook(0x445FE8, CALL_, EXTENDED_, THISCALL_, HookOn_Some_StartSound);
  // Пропускаем собственное ожидание и деструктор.
  _PI->WriteCodePatch(0x4461E7, "%n", 31); // 31 nop
  
  // Подменяем способ запуска звука колдовства Зыбучих песков.
  
  _PI->WriteHiHook(0x5A067E, CALL_, DIRECT_, FASTCALL_1, HookOn_Some_LoadWav);
  _PI->WriteHiHook(0x5A071A, CALL_, EXTENDED_, THISCALL_, HookOn_Some_StartSound);
  // Пропускаем собственное ожидание и деструктор.
  _PI->WriteCodePatch(0x5A07E1, "%n", 17); // 17 nop
  _PI->WriteCodePatch(0x5A0814, "%n", 45); // 45 nop
  
  // Подменяем способ запуска звука колдовста Минного поля.
  
  _PI->WriteHiHook(0x5A088C, CALL_, DIRECT_, FASTCALL_1, HookOn_Some_LoadWav);
  _PI->WriteHiHook(0x5A0934, CALL_, EXTENDED_, THISCALL_, HookOn_Some_StartSound);
  // Пропускаем собственное ожидание и деструктор.
  _PI->WriteCodePatch(0x5A0A03, "%n", 17); // 17 nop
  _PI->WriteCodePatch(0x5A0A37, "%n", 45); // 45 nop
  
  
  
  
  // Исправление лишнего сообщения о начале раунда в окне лога при тактической фазе.
  
  // При вызове функции перехода к следующему раунду при тактической фазе сохраняем то, что это была тактическая фаза.
  _PI->WriteHiHook(0x473E89, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_NextRoundTactic);
  _PI->WriteHiHook(0x474B8F, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_NextRoundTactic);
  _PI->WriteHiHook(0x4758C9, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_NextRoundTactic);
  
  // При добавлении в лог информации о следующем раунде учитываем тактическую фазу.
  _PI->WriteLoHook(0x475AEB, HookOn_BattleNextRoundLog);
  
  
  
  
  
  // Исправление изображения Медузы на стрелковой башне.
  
  // Перед отрисовкой башни проверяем существо на ней на Медузу и запоминаем результат проверки.
  _PI->WriteLoHook(0x494A76, HookOn_Battle_DrawTower_Check_Medusa);
  
  // После отрисовки башни стираем результат проверки на Медузу.
  _PI->WriteLoHook(0x494A86, HookOn_Battle_DrawTower_UnCheck_Medusa);
  
  // При отрисовке башни учитываем возможность нахождения на ней Медузы.
  _PI->WriteHiHook(0x494D11, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_DrawTower_Height);
  
  
  
  
  
  
  
  // Исправление тени перемещения - выделяем большую чать поля боя под её действие.
  
  // Инициализация максимальных границ перерисовки.
  _PI->WriteDword(0x462220 + 6, 14); // 58 - 44
  _PI->WriteDword(0x462234 + 6, 784); // 740 + 44
  
  // Сами максимальные границы перерисовки (инициализация уже пропущена).
  ((_RedrawBorders_*)0x694F18)->Left = 14; // 58 - 44
  ((_RedrawBorders_*)0x694F18)->Right = 784; // 740 + 44
  
  // Выделение повехности.
  _PI->WriteDword(0x46271F + 1, 771); // 683 + 88
  
  // Обновление изображения.
  _PI->WriteDword(0x4939BF + 1, 771); // 683 + 88
  _PI->WriteByte(0x4939C6 + 1, 14); // 58 - 44
  
  // Отрисовка изображения.
  _PI->WriteByte(0x49376F + 2, -14); // -58 + 44
  
  
  // Не подсвечиваем гексы с существами противника в тактической фазе.
  _PI->WriteLoHook(0x493448, LoHook_BattleTaktik_Shadow);
  
  
  
  // Исправляем исчезновение тени перемещения после автобоя и неудачного колдовства.
  
  // Убираем собственную отрисовку тени перемещения.
  _PI->WriteCodePatch(0x477C9F, "%n", 11); // 11 nop
  
  // Убираем собственную перерисовку.
  _PI->WriteHiHook(0x477CE1, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_Skip);
  
  // Добавляем безусловную отрисовку тени перемещения.
  _PI->WriteLoHook(0x477C65, LoHook_Redraw_Shadow); // После автобоя
  _PI->WriteLoHook(0x5A024F, LoHook_Redraw_Shadow); // После неудачного каста
  
  
  // Исправление выделения стеков.
  
  // При выделении стека учитываем то, что он может быть неспособен анимировать.
  _PI->WriteLoHook(0x477694, HookOn_BattleSelectStack);
  _PI->WriteLoHook(0x4776A8, HookOn_BattleSelectStack);
  
  // При выделении стека площадным заклинанием учитываем то, что он может быть неспособен анимировать.
  _PI->WriteLoHook(0x43ED88, HookOn_BattleSpellSelectStack);
  
  // При отрисовке при выделении стека обновляем без перерисовки выделений стеков.
  _PI->WriteHiHook(0x477728, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  _PI->WriteHiHook(0x59FE1F, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  
  
  
  
  
  
  // Инициализация анимации (времени и периодов) перед битвой.
  
  // Хук на инициализацию битвы.
  _PI->WriteHiHook(0x462600, SPLICE_, EXTENDED_, THISCALL_, HookOn_BattleInit);
  // Хук на инициализацию анимации героя в бою.
  _PI->WriteLoHook(0x463074, HookOn_BattleHeroInitDef);
  // При инициализации начальных времён участников битвы добавляем случайности к случайной анимации героя.
  _PI->WriteLoHook(0x479873, HookOn_BattleInitTimes);
  // Хук на инициализацию анимации стойки стека.
  _PI->WriteHiHook(0x43D5D0, SPLICE_, EXTENDED_, THISCALL_, HookOn_StackInit);
  
  
  // Во время инициализации битвы trandint всегда работает по времени.
  _PI->WriteLoHook(0x50B3CD, LoHook_Battle_Trandint);
  
  
  
  
  
  // Анимация ожидания.
  
  // Перед проигрыванием анимации ожидания.
  _PI->WriteHiHook(0x495C50, SPLICE_, EXTENDED_, THISCALL_, HookOn_WaitAnimDraw);
  
  // Перед стандартными проигрываниями анимации ожидания.
  _PI->WriteHiHook(0x4739EB, CALL_, EXTENDED_, THISCALL_, HookOn_WaitAnimDrawStd);
  _PI->WriteHiHook(0x473A9B, CALL_, EXTENDED_, THISCALL_, HookOn_WaitAnimDrawStd);
  
  
  
  
  // Не отрисовываем невидимые стеки.
  _PI->WriteHiHook(0x43DE60, SPLICE_, EXTENDED_, THISCALL_, HookOn_BattleStack_Draw);
  
  
  
  
  // Исправление лишних отрисовок рамок вокруг стеков.
  
  // При выборе цвета рамки вокруг стека не отрисовываем её, когда этого не требуется.
  _PI->WriteLoHook(0x43DF5A, Hook_StackDraw_ChoseBorderColor);
  
  // При выборе цвета рамки вокруг существа стрелковой башни не отрисовываем её, когда этого не требуется.
  _PI->WriteLoHook(0x494CBC, Hook_ArrowTower_Creature_Draw_ChoseBorderColor);
  
  
  
  
  // При ожидании определённого времени в бою прокручиваем анимацию ожидания.
  _PI->WriteHiHook(0x43F269, CALL_, EXTENDED_, FASTCALL_1, WaitForTime_Draw); // Задержка перед выстрелом лучом
  _PI->WriteHiHook(0x441B3E, CALL_, EXTENDED_, FASTCALL_1, WaitForTime_Draw); // Задержка перед ответным ударом
  _PI->WriteHiHook(0x441BF0, CALL_, EXTENDED_, FASTCALL_1, WaitForTime_Draw); // Задержка перед вторым ударом
  _PI->WriteHiHook(0x5A6827, CALL_, EXTENDED_, FASTCALL_1, WaitForTime_Draw); // Задержка между секциями цепной молнии
  
  
  
  
  
  // Анимации действия отрисовки.
  
  // При подсчёте количества кадров анимации при действии отрисовки учитываем защитную стойку (недочёт SoD).
  _PI->WriteLoHook(0x468838, Hook_OnDrawActPlay_CalcFrames_DefendPos);
  
  // При проигрывании анимации при действии отрисовки учитываем защитную стойку (недочёт SoD).
  _PI->WriteLoHook(0x468A9C, Hook_OnDrawActPlay_Play_DefendPos);
  
  // Затираем собственное обновление экрана функции.
  _PI->WriteCodePatch(0x468C53, "%n", 45); // 45 nop
  
  // Убираем циклическое проигрывание последнего кадра анимации, если она слишком короткая.
  _PI->WriteDataPatch(0x468C38, "7F"); // jg
  
  // Добавялем обновление экрана при основной отрисовке.
  _PI->WriteByte(0x468C4A + 1, TRUE);
  
  // При проигрывании кадра выстрела анимации при действии отрисовки пропускаем кадры стойки и кривляния.
  _PI->WriteLoHook(0x468A14, Hook_OnDrawActPlay_DrawNewShotFrame);
  
  // При проигрывании кадра анимации при действии отрисовки пропускаем кадры стойки и кривляния.
  _PI->WriteLoHook(0x468B81, Hook_OnDrawActPlay_DrawNewFrame);
  
  // При проигрывании кадра доигрывающейся анимации при действии отрисовки пропускаем кадры стойки и кривляния.
  _PI->WriteLoHook(0x468DB6, Hook_OnDrawActPlay_LastDrawNewFrame);
  
  // После проигрывания последнего кадра основной анмации отключаем боевую анимацию.
  _PI->WriteLoHook(0x468C8B, Hook_OnDrawActPlay_EndBAnim);
  
  // При начале получения повреждений сбрасываем то, что он играл анимацию выстрела.
  _PI->WriteLoHook(0x468B78, LoHook_DrawActionPlay_ResetShootAnim);
  
  // Перед отрисовкой при действии отрисовки проигрываем анимацию ожидания и включаем обновление всего экрана боя.
  // _PI->WriteHiHook(0x468C4E, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // - патчем добавляем обновление
  // _PI->WriteHiHook(0x468E43, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // - и так нормально
  // _PI->WriteHiHook(0x468F6A, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // - отрисовка без ожиания, не трогаем.
  
  
  
  
  
  
  // Проигрывание стандартной боевой анимации.
  
  // Перед отрисовкой при проигрывании стандартной боевой анимации проигрываем анимацию ожидания и включаем обновление всего экрана боя.
  // И так всё хорошо.
  // _PI->WriteHiHook(0x4964C0, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);
  // _PI->WriteHiHook(0x49650B, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);
  // _PI->WriteHiHook(0x49655D, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);
  // _PI->WriteHiHook(0x496579, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);  
  
  // Перерисовываем поле боя после анимации на стеке.
  _PI->WriteHiHook(0x4963C0, SPLICE_, EXTENDED_, THISCALL_, HiHook_Battle_StdAnimRedraw);
  
  
  
  
  // Отрисовка боя.
  
  // Стираем собственную проверку необходимости перерисовки.
  _PI->WriteCodePatch(0x493FC9, "%n", 35); // 35 nop
  
  
  // Переделка способа ожидания при отрисовке.
  _PI->WriteHiHook(0x493FC0, SPLICE_, EXTENDED_, THISCALL_, HookOn_Battle_Draw);
  
  // При необходимости пропускаем отрисовку активной части поля боя.
  _PI->WriteLoHook(0x494145, Hook_OnDraw_NeedDraw_ActivePart);
  
  
  
  
  
  // Анимация выстрела и обычного снаряда.
  
  // Убираем конструктор для копирования изображений.
  _PI->WriteCodePatch(0x43F355, "%n", 5); // 5 nop
  _PI->WriteCodePatch(0x43F35C, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x43F364, "%n", 5); // 5 nop
  
  // Убираем собственный расчёт времени следующей смены кадров.
  _PI->WriteCodePatch(0x43F3D3, "%n", 11); // 11 nop
  
  // Убираем изменение координат.
  _PI->WriteCodePatch(0x43F41B, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x43F424, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x43F429, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x43F435, "%n", 9); // 9 nop
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x43F540, "90"); // 1 nop
  
  // Убираем собственную стёрку старого изображения снаряда.
  _PI->WriteCodePatch(0x43F3E5, "%n", 51); // 51 nop
  
  // Убираем копирование изображения фона.
  _PI->WriteCodePatch(0x43F43E, "%n", 38); // 38 nop
  
  // Убираем собственное обновление экрана.
  _PI->WriteCodePatch(0x43F4AF, "%n", 131); // 131 nop
  
  // Убираем собственную стёрку старого изображения снаряда после окончания выстрела.
  _PI->WriteCodePatch(0x43F54C, "%n", 51); // 51 nop
  
  // Убираем собственное обновление экрана после окончания выстрела.
  _PI->WriteCodePatch(0x43F57F, "%n", 18); // 18 nop
  
  // Убираем деструктор для копирования изображений.
  _PI->WriteCodePatch(0x43F591, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x43F59E, "%n", 5); // 5 nop
  
  
  // При отрисовке анимации выстрела проигрываем анимацию ожидания и перерисовываем всё поле боя.
  // _PI->WriteHiHook(0x43F1B5, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // - И так всё нормально
  
  
  // Определение первого времени следующей смены кадров снаряда.
  _PI->WriteLoHook(0x43F3CD, HookOn_BulletDraw_InitTime);
  
  // При смене кадров отрисовки полёта снаряда...
  _PI->WriteLoHook(0x43F464, HookOn_BulletDraw_FrameChange);
  
  // При задержке между кадрами отрисовки полёта снаряда...
  _PI->WriteHiHook(0x43F535, CALL_, EXTENDED_, FASTCALL_1, HookOn_FramesDraw_Delay);
  
  // После отрисовки полёта снаряда отрисовываем поле боя.
  _PI->WriteLoHook(0x43F591, HookOn_Draw_WaitAnim_Low_Redraw);
  
  
  
  
  
  
  
  // Анимация выстрела и снаряда стрелковой башни.
  
  // При отрисовке анимации выстрела стрелковой башни проигрываем анимацию ожидания и перерисовываем всё поле боя.
  // _PI->WriteHiHook(0x4658BE, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // После отрисовки анимации выстрела стрелковой башни проигрываем анимацию ожидания и перерисовываем всё поле боя.
  // _PI->WriteHiHook(0x465A3F, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // При окончании выстрела стрелковой башни проигрываем анимацию ожидания и перерисовываем всё поле боя.
  // _PI->WriteHiHook(0x465A69, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  // Убираем конструктор для копирования изображений.
  _PI->WriteCodePatch(0x468042, "%n", 16); // 16 nop
  
  // Убираем собственный расчёт времени следующей смены кадров.
  _PI->WriteCodePatch(0x4680CE, "%n", 11); // 11 nop
  
  // Убираем изменение координат.
  _PI->WriteCodePatch(0x468113, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x46811C, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x468120, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x46812C, "%n", 9); // 9 nop
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x46821E, "90"); // 1 nop
  
  // Убираем собственную стёрку старого изображения снаряда.
  _PI->WriteCodePatch(0x4680E0, "%n", 48); // 48 nop
  
  // Убираем копирование изображения фона.
  _PI->WriteCodePatch(0x468135, "%n", 35); // 35 nop
  
  // Убираем собственное обновление экрана.
  _PI->WriteCodePatch(0x468192, "%n", 126); // 126 nop
  
  // Убираем собственную стёрку старого изображения снаряда после окончания выстрела.
  _PI->WriteCodePatch(0x46822A, "%n", 49); // 49 nop
  
  // Убираем собственное обновление экрана после окончания выстрела.
  _PI->WriteCodePatch(0x46825B, "%n", 18); // 18 nop
  
  // Убираем деструктор для копирования изображений.
  _PI->WriteCodePatch(0x46826D, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x468277, "%n", 5); // 5 nop
  
  // Определение первого времени следующей смены кадров снаряда.
  _PI->WriteLoHook(0x4680C8, HookOn_ArrowTower_BulletDraw_InitTime);
  
  // При смене кадров отрисовки полёта снаряда...
  _PI->WriteLoHook(0x468158, HookOn_ArrowTower_BulletDraw_FrameChange);
  
  // При задержке между кадрами отрисовки полёта снаряда...
  _PI->WriteHiHook(0x468213, CALL_, EXTENDED_, FASTCALL_1, HookOn_FramesDraw_Delay);
  
  // После отрисовки полёта снаряда отрисовываем поле боя.
  _PI->WriteLoHook(0x46826D, HookOn_Draw_WaitAnim_Low_Redraw);
  
  
  
  
  
  
  
  // Отрисовка луча (цепная молния, бехолдеры, злые глаза, архимаги).
  
  
  // Устанавливаем минимальное время для цепной молнии - 3.
  _PI->WriteCodePatch(0x5A67D4, "6A %b", 3); // push 3
  
  // Убираем бессмысленный вызов ГПСЧ при отрисовке луча.
  _PI->WriteCodePatch(0x5A577B, "%n", 10); // 10 nop
  _PI->WriteCodePatch(0x5A5788, "%n", 5); // 5 nop
  
  
  // Убираем собственную инициализацию управления временем.
  _PI->WriteCodePatch(0x5A600D, "%n", 39); // 39 nop
  _PI->WriteCodePatch(0x5A603B, "%n", 3); // 3 nop
  
  // Убираем собственное ожидание после цепной молнии.
  _PI->WriteCodePatch(0x5A6813, "%n", 25); // 25 nop
  
  // Убираем собственное ожидание перед лучом существа.
  _PI->WriteCodePatch(0x43F24F, "%n", 31); // 31 nop
  
  // Устанавливаем минимальное время для первой анимации цепной молнии.
  _PI->WriteHiHook(0x5A6726, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ChainLightingAnimFirst);
  
  // Инициализация луча.
  _PI->WriteLoHook(0x5A5FD9, HookOn_RayInit);
  
  // Инициализация каждой отрисовки луча.
  _PI->WriteLoHook(0x5A5FE0, HookOn_RayDrawingInit);
  
  // При переходе к следующему кадру при отрисовке луча...
  _PI->WriteLoHook(0x5A60E8, HookOn_RayNextFrame);
  
  // При окончании отрисовки луча...
  _PI->WriteLoHook(0x5A63DA, HookOn_RayEnd);
  
  // Отрисовки после луча.
  // _PI->WriteHiHook(0x5A6402, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  // _PI->WriteHiHook(0x5A64A1, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  
  
  // Другой способ модификации луча, оказалось - более медленный.
  /*
  // Убираем собственную инициализацию управления временем.
  _PI->WriteCodePatch(0x5A600D, "%n", 39); // 39 nop
  _PI->WriteCodePatch(0x5A603B, "%n", 3); // 3 nop
  
  // Инициализация луча.
  _PI->WriteLoHook(0x5A5FD9, HookOn_RayInitSD);
  
  // При переходе к следующей части луча сохраняем её.
  _PI->WriteLoHook(0x5A606F, HookOn_RayNewSecSD);
  
  // При переходе к следующему кадру при отрисовке луча...
  _PI->WriteLoHook(0x5A60E8, HookOn_RayNextFrameSD);
  
  // При удалении данных о луче...
  _PI->WriteLoHook(0x5A63E3, HookOn_RayDestructSD);
  */
  
  
  
  
  
  // Отрисовка полёта снаряда-заклинания.
  
  // Убираем конструктор для копирования изображений.
  _PI->WriteCodePatch(0x467B79, "%n", 1); // 1 nop
  _PI->WriteCodePatch(0x467B7C, "%n", 4); // 4 nop
  _PI->WriteCodePatch(0x467B82, "%n", 5); // 5 nop
  
  // Убираем собственный расчёт времени следующей смены кадров.
  _PI->WriteCodePatch(0x467C03, "%n", 11); // 11 nop
  
  // Убираем изменение координат.
  _PI->WriteCodePatch(0x467C46, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x467C4F, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x467C54, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x467C60, "%n", 9); // 9 nop
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x467D84, "90"); // 1 nop
  
  // Убираем собственное переключение кадров снаряда.
  _PI->WriteCodePatch(0x467D46, "%n", 48); // 48 nop
  
  // Убираем собственную стёрку старого изображения снаряда.
  _PI->WriteCodePatch(0x467C15, "%n", 46); // 46 nop
  
  // Убираем копирование изображения фона.
  _PI->WriteCodePatch(0x467C69, "%n", 35); // 35 nop
  
  // Убираем собственное обновление экрана.
  _PI->WriteCodePatch(0x467CC3, "%n", 131); // 131 nop
  
  // Убираем собственную стёрку старого изображения снаряда после окончания выстрела.
  _PI->WriteCodePatch(0x467D90, "%n", 46); // 46 nop
  
  // Убираем собственное обновление экрана после окончания выстрела.
  _PI->WriteCodePatch(0x467DBE, "%n", 18); // 18 nop
  
  // Убираем деструктор для копирования изображений.
  _PI->WriteCodePatch(0x467DD8, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x467DE2, "%n", 5); // 5 nop
  
  
  // Отрисовка перед полётом снаряда.
  // _PI->WriteHiHook(0x467BC0, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_NotFlip); // Не нужно
  
  
  // Определение первого времени следующей смены кадров снаряда-заклинания.
  _PI->WriteLoHook(0x467BFC, HookOn_SpellBulletDraw_InitTime);
  
  // При смене кадров отрисовки полёта снаряда-заклинания...
  _PI->WriteLoHook(0x467C8C, HookOn_SpellBulletDraw_FrameChange);
  
  // При задержке между кадрами отрисовки полёта снаряда-заклинания...
  _PI->WriteHiHook(0x467D79, CALL_, EXTENDED_, FASTCALL_1, HookOn_FramesDraw_Delay);
  
  // После отрисовки полёта снаряда-заклинания отрисовываем поле боя.
  _PI->WriteLoHook(0x467DD0, HookOn_Draw_WaitAnim_Low_Redraw);
  
  
  
  
  
  
  
  // Отрисовка баллистического выстрела.
  
  // Убираем конструктор для копирования изображений.
  _PI->WriteCodePatch(0x467707, "%n", 1); // 1 nop
  _PI->WriteCodePatch(0x467713, "%n", 1); // 1 nop
  _PI->WriteCodePatch(0x46771A, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x467729, "%n", 5); // 5 nop
  
  // Убираем собственный расчёт времени следующей смены кадров.
  _PI->WriteCodePatch(0x46778C, "%n", 11); // 11 nop
  
  // Убираем изменение координат.
  _PI->WriteCodePatch(0x46779E, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4677AB, "%n", 34); // 34 nop
  _PI->WriteCodePatch(0x4677D7, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x4677E0, "%n", 20); // 20 nop
  
  // Убираем собственное увеличение счётчика кадров и изменение скорости.
  _PI->WriteCodePatch(0x46795A, "%n", 2); // 2 nop
  
  // Убираем собственное изменение x-определителя координаты.
  _PI->WriteCodePatch(0x467949, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x46794F, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x467954, "%n", 3); // 3 nop
  
  // Убираем собственное переключение кадров снаряда.
  _PI->WriteCodePatch(0x46790E, "%n", 48); // 48 nop
  
  // Убираем копирование изображения фона.
  _PI->WriteCodePatch(0x4677F4, "%n", 38); // 38 nop
  
  // Убираем собственную стёрку старого изображения снаряда.
  _PI->WriteCodePatch(0x4678DE, "%n", 48); // 48 nop
  
  // Убираем собственное обновление экрана.
  _PI->WriteCodePatch(0x467850, "%n", 142); // 142 nop
  
   // Убираем деструктор для копирования изображений.
  _PI->WriteCodePatch(0x46796A, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x467977, "%n", 5); // 5 nop
  
  // Убираем собственное ожидание при взрыве.
  _PI->WriteCodePatch(0x4460F7, "6A %b", 0); // push 0, не ждать
  _PI->WriteCodePatch(0x4460FB, "6A %b", 0); // push 0, 0 единиц времени ожидания
  
  // Заменяем параметры отрисовки взрыва.
  _PI->WriteCodePatch(0x4460F9, "6A %b", 1); // push 1, перерисовываем задиний план
  _PI->WriteCodePatch(0x4460FD, "6A %b", 0); // push 0, нет границ обновления
  _PI->WriteCodePatch(0x4460FF, "6A %b", 1); // push 1, настраиваем и используем границы отрисовки
  
  // Заменяем параметры ширины и высоты def`а на настоящие.
  _PI->WriteCodePatch(0x446145, "8B 4E 34 %n", 3); // mov ecx, [esi + 34h]; 3 nop, высота def`а
  _PI->WriteCodePatch(0x44614C, "%n", 6); // 6 nop
  _PI->WriteCodePatch(0x446153, "%n", 8); // 8 nop
  _PI->WriteCodePatch(0x446161, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x446167, "%n", 1); // 1 nop
  _PI->WriteCodePatch(0x44615B, "8B 56 30 %n", 3); // mov edx, [esi + 30h]; 3 nop, ширина def`а
  
  
  // Убираем собственное переключение кадров при взрыве.
   _PI->WriteCodePatch(0x4461AD, "%n", 4); // 4 nop
  
  // Отрисовка анимации баллистического выстрела.
  // _PI->WriteHiHook(0x445F9C, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Определение первого времени следующей смены кадров баллистического снаряда.
  _PI->WriteLoHook(0x467786, HookOn_BallisticBulletDraw_InitTime);
  
  // При смене кадров отрисовки полёта баллистического снаряда...
  _PI->WriteLoHook(0x46781A, HookOn_BallisticBulletDraw_FrameChange);
  
  // При задержке между кадрами отрисовки полёта баллистического снаряда...
  _PI->WriteHiHook(0x467941, CALL_, EXTENDED_, FASTCALL_1, HookOn_FramesDraw_Delay);
  
  // Инициализация времени взрыва баллистического выстрела.
  _PI->WriteLoHook(0x4460AE, HookOn_BallisticExplDraw_InitTime); 
  
  // Перерисовка во время анимации взрыва.
  _PI->WriteHiHook(0x446103, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);
  
  // При отрисовке взрыва баллистического выстрела.
  _PI->WriteLoHook(0x446178, HookOn_BallisticExplDraw);
  
  // При отрисовке def`а взрыва баллистического выстрела.
  _PI->WriteHiHook(0x446173, CALL_, EXTENDED_, THISCALL_, HiHook_BallisticExpl_BlitDef);
  
  // Перерисовка после анимации взрыва.
  // _PI->WriteHiHook(0x4461D1, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  
  
  
  
  
  
  
  
  
  
  // Отрисовка исчезновения трупа.
  
  // Перерисовываем поле боя перед исчезновением трупа - участок кода никогда не выполняется.
  // _PI->WriteHiHook(0x466CCA, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_NoSetRedraws);
  
  // При анимации исчезновения трупов.
  _PI->WriteHiHook(0x4669E0, SPLICE_, EXTENDED_, THISCALL_, HiHook_RemoveDeadDraw);
  
  
  
  
  // Отрисовка вызова.
  
  // При отрисовке вызова стека.
  _PI->WriteLoHook(0x479B4B, HookOn_SummonDraw);
  
  // Исправление перерисовки после инициализации вызванного стека перед анимацией - участок кода никогда не выполняется.
  // _PI->WriteHiHook(0x479BBE, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_NoSetRedraws);
  
  
  
  
  
  // Отрисовка телепорта.
  
  // Затираем собственную плавную отрисовку.
  _PI->WriteCodePatch(0x5A1EE1, "%n", 46); // 46 nop
  
  // Исправление подготовки телепорта - участок кода никогда не выполняется.
  // _PI->WriteHiHook(0x5A1EEF, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_NoSetRedraws);
  
  // При плавной отрисовке телепорта.
  _PI->WriteLoHook(0x5A1E5D, HookOn_TeleportDraw);
  
  // При отрисовке прямоурольника, отображающего количество существ.
  _PI->WriteLoHook(0x43E35D, HookOn_DrawStackRectShowingCount);
  
  
  
  
  
  // Отрисовка исчезновения препятствия.
  
  // Подготавливаем плавное исчезновение объекта.
  _PI->WriteLoHook(0x5A1F53, HookOn_RemoveObstacleDraw_Prepare);
  
  // Исправление подготовки устранения препятствия - участок кода никогда не выполняется.
  // _PI->WriteHiHook(0x5A1FD6, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_NoSetRedraws);
  
  // После плавного исчезновения объекта.
  _PI->WriteHiHook(0x5A2004, CALL_, EXTENDED_, THISCALL_, HookOn_RemoveObstacleDraw);
  
  // Делаем возможной невидимость препятсвия.
  _PI->WriteLoHook(0x494168, HookOn_RemoveObstacleDraw_Invisible1);
  _PI->WriteLoHook(0x49448F, HookOn_RemoveObstacleDraw_Invisible2);
  
  
  
  // Отрисовка плавного изменения экрана.
  // Правящаяся функция - не только боевая.
  // Но её модификация универсальна.
  
  // Правим расчёт силы отображения нового изображения для большего количества кадров (32 весто 8).
  _PI->WriteCodePatch(0x60349A, "C1 F8 %b", 5); // sar eax, 5
  
  // Правим количество кадров (32 весто 8).
  _PI->WriteCodePatch(0x6035DD, "83 F8 %b", 32); // cmp eax, 32
  
  // Убираем собственный расчёт времени следующей смены кадров.
  _PI->WriteCodePatch(0x603475, "%n", 8); // 8 nop
  _PI->WriteCodePatch(0x603480, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x603485, "%n", 3); // 3 nop
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x6035DC, "90"); // 1 nop
  
  // Добавление зависимости максимальной скорости от настроек скорости боя.
  _PI->WriteLoHook(0x603434, HookOn_SmoothImageChangeDraw_MinFrameTime);
  
  // Определение первого времени следующей смены кадров при плавном изменении изображения.
  _PI->WriteLoHook(0x60343B, HookOn_SmoothImageChangeDraw_InitTime);
  
  // При плавном изменении изображения играем производим отрисовку.
  _PI->WriteLoHook(0x603475, HookOn_SmoothImageChangeDraw_FrameChange);
  
  // При задержке между кадрами при плавном изменении изображения...
  _PI->WriteHiHook(0x6035D4, CALL_, EXTENDED_, FASTCALL_1, HookOn_SmoothImageChangeDraw_FramesDraw_Delay);
  
  // После отрисовки при плавном изменении изображения отрисовываем поле боя.
  _PI->WriteLoHook(0x60361A, HookOn_Draw_WaitAnim_Low_Redraw);
  
  
  
  
  
  
  
  
  
  
  
  
  
  // Отрисовка движения стека.
  
  // Перерисовка перед открытием моста.
  // _PI->WriteHiHook(0x441F45, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Перерисовка перед закрытием моста.
  // _PI->WriteHiHook(0x442107, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  // Убираем собственную стёрку старого изображения стека.
  _PI->WriteCodePatch(0x446757, "%n", 5); // 5 nop
  _PI->WriteCodePatch(0x446762, "%n", 5); // 5 nop
  _PI->WriteCodePatch(0x44676A, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x44676E, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x446774, "%n", 42); // 42 nop
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x446939, "90"); // 1 nop
  
  // Убираем собственное управление временем и обновление экрана.
  _PI->WriteCodePatch(0x4468F2, "%n", 65); // 65 nop
  
  // Инициализация отрисовки движения.
  _PI->WriteHiHook(0x446660, SPLICE_, EXTENDED_, THISCALL_, HiHook_StackMoveDraw_Init);
  
  // Убираем собственную отрисовку.
  _PI->WriteHiHook(0x4468D3, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_Skip);
  
  // При задержке между кадрами отрисовки движения стека...
  _PI->WriteHiHook(0x4468ED, CALL_, EXTENDED_, FASTCALL_1, HookOn_FramesDraw_Delay);
  
  // При смене кадров отрисовки движения стека...
  _PI->WriteLoHook(0x4468E7, HookOn_StackMoveDraw_FrameChange);
  
  
  
  
  
  
  
  // Отрисовка полёта.
  
  // Убираем изменение координат.
  _PI->WriteCodePatch(0x4B49DB, "%n", 18); // 18 nop
  _PI->WriteCodePatch(0x4B49F5, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4B4A00, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4B4A11, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4B4A1D, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4B4A23, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4B4A4C, "%n", 3); // 3 nop
  
  // Убираем собственную стёрку старого изображения стека.
  _PI->WriteCodePatch(0x4B49F3, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x4B4A17, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4B4A20, "%n", 3); // 3 nop
  _PI->WriteCodePatch(0x4B4A26, "%n", 74); // 74 nop
  
  // Убираем собственное управление временем и обновление экрана.
  _PI->WriteCodePatch(0x4B4B1A, "%n", 62); // 62 nop
  
  // Убираем собственное увеличение счётчика кадров анимации стека.
  _PI->WriteCodePatch(0x4B4B5E, "90"); // 1 nop
  
  // Убираем установку изображения без стека как заднего плана.
  _PI->WriteCodePatch(0x4B4845, "%n", 7); // 7 nop
  
  
  
  // Перерисовка перед отрисовкой полёта.
  // _PI->WriteHiHook(0x4B4815, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // При инициализации координат отрисовки полёта стека...
  _PI->WriteLoHook(0x4B49A0, HookOn_StackFlightDraw_InitCoords);
  
  // При задержке между кадрами отрисовки полёта стека...
  _PI->WriteHiHook(0x4B4B15, CALL_, EXTENDED_, FASTCALL_1, HookOn_FramesDraw_Delay);
  
  // При задержке между кадрами при полёте...
  _PI->WriteLoHook(0x4B4B0F, HookOn_FlightFramesDraw_Delay);
  
  // При смене кадров отрисовки полёта стека...
  _PI->WriteLoHook(0x4B4B58, HookOn_StackFlightDraw_FrameChange);
  
  // После полёта делаем кадр стека допустимым.
  _PI->WriteLoHook(0x4B4B9B, HookOn_StackFlightDraw_MakeFrameAllowed);
  
  
  
  
  
  // Отрисовки моста замка.
  
  // Отрисовка открытия моста.
  // _PI->WriteHiHook(0x466E67, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Отрисовка закрытия моста.
  // _PI->WriteHiHook(0x466FF9, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);  // Не нужно
  
  
  
  
  
  
  // Модифицируем сбросы анимации стеков.
  
  // При сбросе анимации стека при открытии моста при движении...
  _PI->WriteLoHook(0x441F2D, HookOn_StackAnim_MoveBridgeOpenReset);
  
  // При сбросе анимации стека после заклинания...
  _PI->WriteLoHook(0x477465, HookOn_StackAnim_AfterSpellReset);
  
  // При сбросе анимации стека перед действием...
  _PI->WriteLoHook(0x477CC9, HookOn_StackAnim_BeforeActionReset);
  
  // При сбросе анимации стека после действия...
  _PI->WriteLoHook(0x47962F, HookOn_StackAnim_AfterActionReset);
  
  // При сбросе анимации стека при открытии моста перед полётом...
  _PI->WriteLoHook(0x4B47FD, HookOn_StackAnim_BeforeFlightBridgeOpenReset);
  
  // При общем сбросе анимации стеков...
  _PI->WriteLoHook(0x4797F6, HookOn_StackAnimReset);
  
  
  
  
  
  
  // Отрисовки в основном цикле.
  
  // Отрисовка при нажатии клавиши, изменяющей настройки показа информации о стеках.
  _PI->WriteHiHook(0x475257, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders); 
  
  // Отрисовка при нажатии клавиши, изменяющей настройки тени курсора.
  _PI->WriteHiHook(0x475311, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders); 
  
  // Отрисовка при нажатии клавиши, изменяющей настройки тени перемещения.
  _PI->WriteHiHook(0x47548D, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders); 
  
  // Отрисовка при переводе курсора со стека на интерфес битвы.
  _PI->WriteHiHook(0x474DD6, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  // Отрисовка при нажатии левой кнопки мыши.
  _PI->WriteHiHook(0x474C4D, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  // Отрисовка в конце тела основного цикла.
  _PI->WriteHiHook(0x474BAD, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  // Исправляем отрисовку после закрытия окна настроек битвы.
  _PI->WriteHiHook(0x4682E8, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  
  
  
  // Отрисовка анимации на гексе.
  
  // Заменяем способ проигрывания анимации гекса.
  _PI->WriteHiHook(0x496590, SPLICE_, EXTENDED_, THISCALL_, HookOn_GexAnim);
  
  // Заменяем проигрывании анимаций гекса на анимацицию не поверх всего, где надо.
  _PI->WriteHiHook(0x475AD9, CALL_, EXTENDED_, THISCALL_, HookOn_GexAnimNotAboveAll); // При исчезновении магических препятствий.
  _PI->WriteHiHook(0x5A0733, CALL_, EXTENDED_, THISCALL_, HookOn_GexAnimNotAboveAll); // При появлении зыбучего песка.
  _PI->WriteHiHook(0x5A094D, CALL_, EXTENDED_, THISCALL_, HookOn_GexAnimNotAboveAll); // При появлении мины.
  _PI->WriteHiHook(0x5A0A7F, CALL_, EXTENDED_, THISCALL_, HookOn_GexAnimNotAboveAll); // При появлении силового поля.
  _PI->WriteHiHook(0x5A0B94, CALL_, EXTENDED_, THISCALL_, HookOn_GexSkip); // При появлении стены огня.
  _PI->WriteHiHook(0x5A1A77, CALL_, EXTENDED_, THISCALL_, HookOn_GexAnimNotAboveAll); // При снятия заклинания - магического препятствия.
  _PI->WriteHiHook(0x5A1F45, CALL_, EXTENDED_, THISCALL_, HookOn_GexAnimNotAboveAll); // При анимации уничтожении препятствия.
  _PI->WriteHiHook(0x5A2021, CALL_, EXTENDED_, THISCALL_, HookOn_GexAnimNotAboveAll); // При уничтожении магического препятствия.
  
  // Появление стены огня.
  _PI->WriteLoHook(0x5A0BF4, LoHook_FirewallAppearAnim);
  
  
  
  
  // Отрисовка землетрясения.
  
  // Убираем собственное копирование изображения тряски.
  _PI->WriteCodePatch(0x5A7FB6, "%n", 39); // 39 nop
  
  // Пропускаем собственное ожидание.
  _PI->WriteCodePatch(0x5A8003, "%n", 12); // 12 nop
  
  // Убираем собственную отрисовку и обновление изображения тряски.
  _PI->WriteCodePatch(0x5A8014, "%n", 64); // 64 nop
  
  // Пропускаем собственную смену кадров.
  _PI->WriteCodePatch(0x5A8345, "%n", 3); // 3 nop
  
  // Пропускаем собственное управление временем.
  _PI->WriteCodePatch(0x5A81A5, "%n", 5); // 5 nop
  _PI->WriteCodePatch(0x5A81AC, "%n", 5); // 5 nop
  _PI->WriteCodePatch(0x5A81B7, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x5A81BF, "%n", 3); // 3 nop
  
  // Пропускаем собственное ожидание.
  _PI->WriteCodePatch(0x5A833E, "%n", 7); // 7 nop
  
  
  
  // Инициализация управления временем при отрисовке тряски землетрясения.
  _PI->WriteLoHook(0x5A7FF7, HookOn_Earthquake_Effect_InitTime);
  
  // Перерисовка при отрисовке тряски землетрясения.
  _PI->WriteLoHook(0x5A8054, HookOn_Earthquake_Effect);
  
  
  // Исправление отрисовки при тряске при землетрясении.
  // _PI->WriteHiHook(0x5A8087, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Пропускаем собственную перерисовку.
  _PI->WriteHiHook(0x5A81C2, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_Skip);
  
  
  // Инициализация времени взрыва землетрясения.
  _PI->WriteLoHook(0x5A8162, HookOn_Earthquake_Expl_InitTime);
  
  // Перерисовка для взрыва землетрясения.
  _PI->WriteLoHook(0x5A81C7, HookOn_Earthquake_Expl_Draw);
  
  // Добавляем взрывы в границы перерисовки, вместо обновления экрана.
  _PI->WriteLoHook(0x5A82F1, HookOn_Earthquake_Expl_FlipBrd);
  
  // Обновляем экран при отрисовке взрыва при землетрясении.
  _PI->WriteLoHook(0x5A8348, HookOn_Earthquake_Expl_Flip);
  
  // Перерисовка после анимации взрыва.
  // _PI->WriteHiHook(0x5A8362, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  
  
  
  // Отрисовка огненного шара Магогов.
  
  // Убираем собственное ожидание.
  _PI->WriteCodePatch(0x43F818, "6A %b", 0); // push 0, FALSE для ожидания
  _PI->WriteCodePatch(0x43F81C, "6A %b", 0); // push 0, ожидание 0 единиц
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x43F84D, "%n", 1); // 1 nop
  
  // Инициализация времени огненного шара Магога.
  _PI->WriteLoHook(0x43F7EF, HookOn_MGFireballDraw_InitTime);
  
  // При отрисовке огненного шара Магога.
  _PI->WriteLoHook(0x43F7F4, HookOn_MGFireballDraw);
  
  // Отрисовка анимации огненного шара Магога.
  _PI->WriteHiHook(0x43F824, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_ClearRedraws);
  
  // Отрисовка после анимации огненного шара Магога.
  // _PI->WriteHiHook(0x43F862, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  
  
  // Отрисовка облака смерти Лича и Могущественного лича.
  
  // Убираем собственное ожидание.
  _PI->WriteCodePatch(0x43FC04, "6A %b", 0); // push 0, FALSE для ожидания
  _PI->WriteCodePatch(0x43FC08, "6A %b", 0); // push 0, ожидание 0 единиц
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x43FC39, "%n", 1); // 1 nop
  
  // Инициализация времени облака смерти Лича и Могущественного лича.
  _PI->WriteLoHook(0x43FBDB, HookOn_LichDClDraw_InitTime);
  
  // При отрисовке облака смерти Лича и Могущественного лича.
  _PI->WriteLoHook(0x43FBE0, HookOn_LichDClDraw);
  
  // Отрисовка анимации облака смерти Лича и Могущественного лича.
  _PI->WriteHiHook(0x43FC10, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_ClearRedraws);
  
  // Отрисовка после анимации облака смерти Лича и Могущественного лича.
  // _PI->WriteHiHook(0x43FC4E, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  
  
  // Отрисовка армагеддона.
  
  // Убираем собственное ожидание.
  _PI->WriteCodePatch(0x5A52B0, "6A %b", 0); // push 0, FALSE для ожидания
  _PI->WriteCodePatch(0x5A52B4, "6A %b", 0); // push 0, ожидание 0 единиц
  
  // Убираем собственное увеличение счётчика кадров.
  _PI->WriteCodePatch(0x5A53D0, "%n", 1); // 1 nop
  
  // Инициализация времени армагеддона.
  _PI->WriteLoHook(0x5A51F6, HookOn_ArmageddonDraw_InitTime);
  
  // При отрисовке армагеддона.
  _PI->WriteLoHook(0x5A5201, HookOn_ArmageddonDraw);
  
  // Исправление отрисовки армагеддона.
  _PI->WriteHiHook(0x5A52BE, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_ClearRedraws);
  
  // Исправление отрисовки после армагеддона.
  // _PI->WriteHiHook(0x5A54D2, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  
  
  // Отрисовки при ожидании наложения заклинаний.
  
  // Отрисовка при ожидании выбора стека при телепорте.
  _PI->WriteHiHook(0x5A3880, SPLICE_, EXTENDED_, THISCALL_, HookOn_SpellSelectTagetsWaitingDraw);
  
  // Отрисовка при ожидании выбора гекса при телепорте.
  _PI->WriteHiHook(0x5A3AE0, SPLICE_, EXTENDED_, THISCALL_, HookOn_SpellSelectTagetsWaitingDraw);
  
  // Отрисовка при ожидании выбора воскрешаемого при жертве.
  _PI->WriteHiHook(0x5A3010, SPLICE_, EXTENDED_, THISCALL_, HookOn_SpellSelectTagetsWaitingDraw);
  
  // Отрисовка при ожидании выбора жертвы при жертве.
  _PI->WriteHiHook(0x5A31A0, SPLICE_, EXTENDED_, THISCALL_, HookOn_SpellSelectTagetsWaitingDraw);
  
  
  
  
  
  
  
  
  
  
  // Отрисовки при удержании нажатия кнопки.
  
  // Перед боем инициализируем функцию отрисовки нажатия кнопки.
  _PI->WriteHiHook(0x462600, SPLICE_, EXTENDED_, THISCALL_, HookOn_Battle_Start_ButtonClickDraw);
  
  // После сообщения о конце боя деинициализируем функцию отрисовки нажатия кнопки.
  _PI->WriteHiHook(0x475CFD, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_EndMessage_ButtonClickDraw);
  
  // Игнорируем скрытые диалоги.
  _PI->WriteHiHook(0x59F17A, CALL_, EXTENDED_, THISCALL_, HookOn_Show_HiddenDialog); // Выбор цели заклинания на одну цель
  _PI->WriteHiHook(0x59F321, CALL_, EXTENDED_, THISCALL_, HookOn_Show_HiddenDialog); // Выбор цели заклинания на гексы
  _PI->WriteHiHook(0x59F476, CALL_, EXTENDED_, THISCALL_, HookOn_Show_HiddenDialog); // Выбор цели-стека телепорта
  _PI->WriteHiHook(0x59F49C, CALL_, EXTENDED_, THISCALL_, HookOn_Show_HiddenDialog); // Выбор цели-гекса телепорта
  _PI->WriteHiHook(0x59F641, CALL_, EXTENDED_, THISCALL_, HookOn_Show_HiddenDialog); // Выбор жертвы
  _PI->WriteHiHook(0x59F660, CALL_, EXTENDED_, THISCALL_, HookOn_Show_HiddenDialog); // Выбор воскрешаемого жертвы
  _PI->WriteHiHook(0x59F744, CALL_, EXTENDED_, THISCALL_, HookOn_Show_HiddenDialog); // Выбор цели уничтожения препятствий
  
  
  
  
  
  
  // При вызове диалога обнуляем функцию отрисовки удержания нажатия кнопки для него.
  _PI->WriteHiHook(0x602AE0, SPLICE_, EXTENDED_, THISCALL_, HookOn_Show_Dialog);
  
  
  // Отрисовка при удержании нажатия на кнопку.
  _PI->WriteLoHook(0x456021, HookOn_WhileButtonClickedDraw);
  
  
  
  
  
  
  
  
  
  
  // Прочие отрисовки.
  
  // Не трогаем отрисовку при инициализации битвы - она всё равно самая первая.
  // _PI->WriteHiHook(0x462B5E, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);
  
  // Исправляем отрисовку при срабатывании высокого боевого духа.
  // _PI->WriteHiHook(0x464703, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправляем отрисовку при стёрке изображения стека и взятия фона.
  //_PI->WriteHiHook(0x446604, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim_NotFlip); // Не нужно
  
  // Исправляем отрисовку сброса выделенного стека при смене раундов.
  // _PI->WriteHiHook(0x4759C2, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовок после заклинания.
  // _PI->WriteHiHook(0x47747D, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  // _PI->WriteHiHook(0x47753C, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки при сбросе выделенного стека.
  // _PI->WriteHiHook(0x4777F8, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление перерисовки поля боя перед действием стека.
  // _PI->WriteHiHook(0x4787A9, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление обновления стека после действия.
  // _PI->WriteHiHook(0x47963F, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление обновления после сброса анимаций стеков.
  // _PI->WriteHiHook(0x479853, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление обновления перед выбором целей площадного заклинания.
  _PI->WriteHiHook(0x47999B, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  // Исправление обновления при обновлении экрана при отрисовке тени курсора.
  // Убираем стандартное обновление экрана.
  // _PI->WriteCodePatch(0x493E31, "%n", 32); // 32 nop // Не нужно
  // Отрисовка.
  _PI->WriteHiHook(0x493E2C, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  // При обработке команды чит-меню исправляем обновление экрана боя.
  _PI->WriteHiHook(0x4F4E74, CALL_, EXTENDED_, THISCALL_, HookOn_Battle_ParallelDraw_Borders);
  
  // Исправление отрисовки при задержке перед заклинанием.
  // _PI->WriteHiHook(0x5A0543, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после постановки зыбучего песка.
  // _PI->WriteHiHook(0x5A07D1, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после постановки мины.
  // _PI->WriteHiHook(0x5A09F3, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после постановки стены огня.
  // _PI->WriteHiHook(0x5A0C71, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после заклинания уничтожения нечисти.
  // _PI->WriteHiHook(0x5A117D, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки при посерении при окаменении.
  // _PI->WriteHiHook(0x5A14D4, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после посерения или покраснения при наложении окаменения или жажды крови.
  // _PI->WriteHiHook(0x5A14F9, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim);
  
  // Исправление отрисовки при покраснении при наложении жажды крови.
  // _PI->WriteHiHook(0x5A15DB, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки при снятии красноты при наложении жажды крови.
  // _PI->WriteHiHook(0x5A1614, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки при покраснении при наложении массовой жажды крови.
  // _PI->WriteHiHook(0x5A178D, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки при снятии красноты при наложении массовой жажды крови.
  // _PI->WriteHiHook(0x5A1822, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки перед телепортом.
  // _PI->WriteHiHook(0x5A1E71, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление ожидания после заклинания.
  // _PI->WriteHiHook(0x5A2441, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление окончания ожидания после заклинания.
  // _PI->WriteHiHook(0x5A2471, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки окончания секции цепной молнии.
  // _PI->WriteHiHook(0x5A683A, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки окончания цепной молнии.
  // _PI->WriteHiHook(0x5A699F, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки массового заклинания.
  // _PI->WriteHiHook(0x5A6E49, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после массового заклинания.
  // _PI->WriteHiHook(0x5A6F57, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Вовремя прекращаем анимацию массового заклинания.
  _PI->WriteLoHook(0x5A6DF4, LoHook_MassSpell_EndStackAnim);
  
  // Всегда перерисовываем поле боя после массового заклинания.
  _PI->WriteCodePatch(0x5A6F43, "%n", 2); // 2 nop
  _PI->WriteCodePatch(0x5A6F46, "%n", 2); // 2 nop
  
  // Исправление отрисовки клонирования.
  // _PI->WriteHiHook(0x5A733A, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после клонирования.
  // _PI->WriteHiHook(0x5A7377, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки перед открытием моста замка при вызове.
  // _PI->WriteHiHook(0x5A74FA, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки при поднятии демонов.
  // _PI->WriteHiHook(0x5A77AC, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки при воскрешении.
  // _PI->WriteHiHook(0x5A7B59, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  // Исправление отрисовки после воскрешения.
  // _PI->WriteHiHook(0x5A7B91, CALL_, EXTENDED_, THISCALL_, HookOn_BattleDraw_WaitAnim); // Не нужно
  
  
  
  
  // Исправлем отсутствие анимации стека после воскрешения до конца боевой анимации.
  _PI->WriteLoHook(0x5A7B3D, LoHook_Resurrect_Alive_AfterAnimEnd);
  
  
  
  
  
  // При стёрке окна предпросмотра стека или героя перерисовывам участок поля боя.
  _PI->WriteLoHook(0x5AA96C, LoHook_HidePreviewImage_Redraw);
  
  
  
  
  
  // Анимация взрывов снарядов.
  
  // Перед отрисовкой выстрела забираем информацию об атакующем.
  _PI->WriteHiHook(0x43F620, SPLICE_, EXTENDED_, THISCALL_, Hook_On_BattleStack_DrawShot_GetInfo);
  
  // Перед отрисовкой выстрела стрелковой башни забираем информацию об атакующем.
  _PI->WriteLoHook(0x4656C7, HookOn_BattleStack_DrawArrowTowerShot_GetInfo);
  
  //// При отрисовках ранения от выстрела добавлям взрыв снаряда.
  //// _PI->WriteHiHook(0x43FA11, CALL_, EXTENDED_, THISCALL_, Hook_On_BattleStack_DrawShot_Expl); // Шар магога - не нужно
  //_PI->WriteHiHook(0x43FA7C, CALL_, EXTENDED_, THISCALL_, Hook_On_BattleStack_DrawShot_Expl); // Обычный выстрел
  //// _PI->WriteHiHook(0x43FE0B, CALL_, EXTENDED_, THISCALL_, Hook_On_BattleStack_DrawShot_Expl); // Облако лича - не нужно
  //_PI->WriteHiHook(0x465992, CALL_, EXTENDED_, THISCALL_, Hook_On_BattleStack_DrawShot_Expl); // Стрелковая башня
  
  
  
  
  
  
  // Анимация особой смерти от заклинаний.
  
  // При настройке анимации смерти стека учитываем то, что он мог умереть особой смертью.
  _PI->WriteLoHook(0x468663, HookOn_DrawActionPlay_SetSpecDeath);
  
  // При настройке количества кадров анимации смерти стека учитываем то, что он мог умереть особой смертью.
  _PI->WriteLoHook(0x4687FF, HookOn_DrawActionPlay_SpecDeath_FramesCount);
  
  // При проверке анимации стека учитываем то, что он мог умереть особой смертью (1).
  _PI->WriteLoHook(0x468BB4, HookOn_DrawActionPlay_SpecDeath_Check1);
  
  // При проверке анимации стека учитываем то, что он мог умереть особой смертью (2).
  _PI->WriteLoHook(0x468B5D, HookOn_DrawActionPlay_SpecDeath_Check2);
  
  // При проверке анимации стека учитываем то, что он мог умереть особой смертью (3).
  _PI->WriteLoHook(0x468DEB, HookOn_DrawActionPlay_SpecDeath_Check3);
  
  // При настройке количества кадров анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью.
  _PI->WriteLoHook(0x5A50C8, HookOn_Armageddon_SpecDeath_FramesCount);
  
  // При настройке анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью (1).
  _PI->WriteLoHook(0x5A5106, HookOn_Armageddon_SetSpecDeath1);
  
  // При настройке анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью (2).
  _PI->WriteLoHook(0x5A5114, HookOn_Armageddon_SetSpecDeath2);
  
  // При настройке анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью.
  _PI->WriteLoHook(0x5A6CE3, HookOn_MassSpell_SetSpecDeath);
  
  // При настройке количества кадров анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью (1).
  _PI->WriteLoHook(0x5A6BD9, HookOn_MassSpell_SpecDeath_FramesCount1);
  
  // При настройке количества кадров анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью (2).
  _PI->WriteLoHook(0x5A6BFA, HookOn_MassSpell_SpecDeath_FramesCount2);
  
  // Учитываем особую смерть при воскрешении.
  _PI->WriteLoHook(0x5A7B2B, LoHook_SpecDeath_Resurrect);
  
  // Во время нанесения урона (для магической атаки).
  _PI->WriteHiHook(0x43F95B, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Шар магога
  _PI->WriteHiHook(0x43FD3D, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Облако лича
  _PI->WriteHiHook(0x440858, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Огненный щит
  _PI->WriteHiHook(0x440E70, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Смертельный взгляд
  _PI->WriteHiHook(0x441048, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Удар молнии (птицы грома)
  _PI->WriteHiHook(0x44124C, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Кислотное дыхание
  _PI->WriteHiHook(0x59FF34, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Мина
  _PI->WriteHiHook(0x5A009F, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Огненная стена
  _PI->WriteHiHook(0x5A0D02, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Волшебная стрела
  _PI->WriteHiHook(0x5A0DA8, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Ледяная молния
  _PI->WriteHiHook(0x5A0E6B, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Гром титана
  _PI->WriteHiHook(0x5A0EDB, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Взрыв
  _PI->WriteHiHook(0x5A1065, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Волна смерти
  _PI->WriteHiHook(0x5A1262, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Уничтожить нежить
  _PI->WriteHiHook(0x5A1D17, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Жертва
  _PI->WriteHiHook(0x5A4DED, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Площадное заклинание
  _PI->WriteHiHook(0x5A4FA8, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Армагеддон
  _PI->WriteHiHook(0x5A6886, CALL_, EXTENDED_, THISCALL_, HookOn_MakeMagicDamage); // Цепная молния
  
  
  // Инициализация звуков особой смерти стеков.
//  _PI->WriteHiHook(0x43D710, SPLICE_, EXTENDED_, THISCALL_, HiHook_BattleStack_InitDefsAndWavs);
  
  // Проигрывание новых звуков стека.
  _PI->WriteHiHook(0x43D260, SPLICE_, EXTENDED_, THISCALL_, HiHook_BattleStack_PlaySound);
  
  // Проигрывание звука особой смерти стека (действие отрисовки).
  _PI->WriteHiHook(0x468B6D, CALL_, EXTENDED_, THISCALL_, HiHook_BattleStack_PlaySpecDeathSound);
  
  // Проигрывание звука особой смерти стека (отрисовка армагеддона).
  _PI->WriteHiHook(0x5A5174, CALL_, EXTENDED_, THISCALL_, HiHook_BattleStack_PlaySpecDeathSound);
  
  // Проигрывание звука особой смерти стека (отрисовка массового заклинания).
  _PI->WriteHiHook(0x5A6C70, CALL_, EXTENDED_, THISCALL_, HiHook_BattleStack_PlaySpecDeathSound);
  
  
  
  // При уничтожении препятсвия убираем его время жизни, дабы избежать его повтороного удаления (баг SoD).
  _PI->WriteHiHook(0x466710, SPLICE_, EXTENDED_, THISCALL_, HookOn_RemoveObstackle);
  
  
  
  // При заклинании уничтожения препятствия запоминаем его гекс.
  _PI->WriteLoHook(0x5A1F38, LoHook_RemoveObstackle_GetHex);
  
  // Заменяем способ проигрывания анимации уничтожения магического препятствия.
  _PI->WriteHiHook(0x5A2021, CALL_, EXTENDED_, THISCALL_, HookOn_RemoveObstackleMagic_Anim);
  
  
  // При постановке стека на гекс после полёта предварительно убираем его с предыдущих гексов, если надо.
  _PI->WriteHiHook(0x4B4B84, CALL_, EXTENDED_, THISCALL_, HiHook_AfterFly_ChangeStackPosition);
  
  
  
  
  // Звук взрыва волшебной стрелы.
  
  // Добавляем звук взрыва волшебной стреле.
  _PI->WriteLoHook(0x5A0D07, LoHook_MagicArrow_ExplSound);
  
  // Добавляем звук взрыва волшебной стреле - конец.
  _PI->WriteLoHook(0x5A0D34, LoHook_MagicArrow_ExplSoundEnd);
  
  
  
  
  
  
  
  
  
  
  
  // Добавляем изначальную функцию, отрисовывающую изменения для текущего удержания нажатия кнопки.
  // Самая родительская функция отрисовки - нулевая.
  ButtonWhileClicked_Draw_List.Append(NULL);
  
  
  
  
  
  // Исправляем задержку кадра анимации колдовства на стеке.
  _PI->WriteLoHook(0x4483EC, LoHook_BattleStack_Cast_Reset);
  
  
  
  
  
  // Рисуем изображения только в границах перерисовки.
  _PI->WriteHiHook(0x43F4AA, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Обычный снаряд
  _PI->WriteHiHook(0x46818D, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Снаряд стрелковой башни
  _PI->WriteHiHook(0x467CBE, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Магический снаряд
  _PI->WriteHiHook(0x46784B, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Баллистический снаряд
  _PI->WriteHiHook(0x446173, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Взрыв баллистического снаряда
  _PI->WriteHiHook(0x49675A, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Анимация на гексе старым способом
  _PI->WriteHiHook(0x49675A, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_Pcx16_Draw_UseBorders); // Анимация тряски землетрясения
  _PI->WriteHiHook(0x5A82EC, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Анимация взрыва после землетрясения
  _PI->WriteHiHook(0x5A5388, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefBattleAnim_UseBorders); // Анимация армагеддона
  _PI->WriteHiHook(0x4951AF, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSimple_UseBorders); // Отрисовка простого def`а в бою
  _PI->WriteHiHook(0x495576, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_Pcx16_DrawSimple_UseBorders); // Отрисовка pcx в бою
  _PI->WriteHiHook(0x494F18, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSpecColorReplace_UseBorders); // Отрисовка def`а с заменяющимся спеццветом в бою
  _PI->WriteHiHook(0x495061, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefBattleAnim_UseBorders); // Отрисовка def`а с прозрачностью в бою
  _PI->WriteHiHook(0x494E1A, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSpecColorReplace_UseBorders); // Отрисовка def`а стека
  _PI->WriteHiHook(0x495442, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_Pcx16_DrawSimple_UseBorders); // Отрисовка pcx укрепления города
  _PI->WriteHiHook(0x494D11, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_BlitDefSpecColorReplace_UseBorders); // Отрисовка def`а существа на стрелковой башне
  _PI->WriteHiHook(0x495754, CALL_, EXTENDED_, THISCALL_, HiHook_Battle_Pcx16_DrawSimple_UseBorders); // Отрисовка pcx части рва
  
  
  // Инициализция подсветки в диалогах заклинаний.
  SpellsHighlightInit();
  
}


_int_ BattleAnims_Count = 83;

_BattleAnim_* o_BattleAnimation = ((_BattleAnim_*)0x641E18);

// Двигаем таблицу анимациий.
void MoveBattleAnim_Table()
{
  // Новое количество боевых анимаций.
  #define new_count 99
  
  // Адрес новой таблицы боевых анимаций.
  _ptr_ new_p = MemAlloc(new_count*sizeof(_BattleAnim_));
  
  // Копируем и обнуляем старую таблицу.
  MemCopy(new_p, o_BattleAnimation, BattleAnims_Count*sizeof(_BattleAnim_));
  MemZero(o_BattleAnimation, BattleAnims_Count*sizeof(_BattleAnim_));
  
  // Сохраняем информацию о новой таблице.
  BattleAnims_Count = new_count;
  o_BattleAnimation = (_BattleAnim_*)new_p;
  
  // Перенаправляем ссылки на новую таблицу.
  
  _PI->WriteDword(0x43F77B + 3, new_p); // Шар магога
  _PI->WriteDword(0x43FB67 + 3, new_p); // Облако смерти лича
  _PI->WriteDword(0x4963F9 + 2, new_p); // Стандартная анимация на стеке
  _PI->WriteDword(0x4965CD + 2, new_p); // Стандартная анимация на гексе
  _PI->WriteDword(0x5A5033 + 3, new_p); // Армагеддон
  _PI->WriteDword(0x5A6B11 + 3, new_p); // Массовое колдовство
  _PI->WriteDword(0x5A7A71 + 3, new_p); // Воскрешение
  _PI->WriteDword(0x5A9629 + 3, new_p); // Загрузка анимации
  
  _PI->WriteDword(0x4689C1 + 3, new_p + 4); // Проигрывание действия отрисовки
  _PI->WriteDword(0x496518 + 2, new_p + 4); // Стандартная анимация на стеке
  _PI->WriteDword(0x4966CB + 2, new_p + 4); // Стандартная анимация на гексе
  _PI->WriteDword(0x5A6D2A + 3, new_p + 4); // Массовое колдовство
  _PI->WriteDword(0x5A7B03 + 3, new_p + 4); // Загрузка анимации
  
  _PI->WriteDword(0x43E500 + 3, new_p + 8); // Отрисовка стека
  
  // Правки количества анимаций.
  _PI->WriteByte(0x4963E7 + 2, new_count); // Стандартная анимация на стеке
  _PI->WriteByte(0x4965BB + 2, new_count); // Стандартная анимация на гексе
}








// Очистка поля боя.
void Clear_Battlefield()
{
  // Очищаем поле боя.
  NeedDraw_Active_Elemenst = FALSE;
  o_BattleMgr->RedrawBattlefield(FALSE, FALSE, FALSE, 0, TRUE, FALSE);
  NeedDraw_Active_Elemenst = TRUE;
}



// Сохранённая копия границ перерисовки боя.
_RedrawBorders_ BattleRedrawRect_cpy;

// Сохранённая информация о перерисовке элементов боя.
_byte_ battle_redraw_info[47];


// Сохраняем значения перерисоввки.
void SaveBattleRedraws(_BattleMgr_* b_mgr)
{
  // Сохраняем перерисовку всех боевых фигур.
  BattleFgrs_SaveRedraw();
  
  // Сохраняем границы отрисовки.
  MemCopy(&BattleRedrawRect_cpy, b_mgr->Offset(81208), sizeof(BattleRedrawRect_cpy));
  
  // Сохраняем перерисовку элементов боя.
  MemCopy(battle_redraw_info, b_mgr->Offset(81920), sizeof(battle_redraw_info));
}

// Восстанавливаем значения перерисоввки.
void RestoreBattleRedraws(_BattleMgr_* b_mgr)
{
  // Восстанавливаем перерисовку всех боевых фигур.
  BattleFgrs_RestoreRedraw();
  
  // Восстанавливаем границы отрисовки.
  MemCopy(b_mgr->Offset(81208), &BattleRedrawRect_cpy, sizeof(BattleRedrawRect_cpy));
  
  // Восстанавливаем перерисовку элементов боя.
  MemCopy(b_mgr->Offset(81920), battle_redraw_info, sizeof(battle_redraw_info));
}





// Проигрывание нового кадра анимации.
void PlayNextFrame(_BattleStack_* Stack, int DefGroup)
{
  // Если стек - стрелковая башня...
  if (Stack->creature_id == CID_ARROW_TOWER)
  {
    // Получаем информацию о стрелковой башне.
    _ArrowTower_* Tower = (_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36*(Stack->Get_ArrowTowerNum()));
    
    // Если новая секция анимации корректна и существует у башни...
    if ((DefGroup > 0) && (Tower->Def->groups_count > DefGroup) && (((DWORD*)(Tower->Def->active_groups))[DefGroup]) && (Tower->Def->groups[DefGroup]->frames_count > 0))
    {
      // Если продолжает проигрываться текщая секция...
      if (DefGroup == Tower->AnimSectionNum)
      {
        // Переходим к следующему кадру.
        Tower->AnimFrameNum += 1;
        
        // Номер максимального кадра.
        int MaxFrame = Tower->Def->groups[DefGroup]->frames_count;
        
        // Зацикливаем анимацию.
        if (Tower->AnimFrameNum >= MaxFrame)
        {
          Tower->AnimFrameNum = 0;
        }
      }
      // Если нужно начать новую секцию...
      else
      {
        // Начинаем проигрывать новую секцию анимации.
        Tower->AnimSectionNum = DefGroup;
        Tower->AnimFrameNum = 0;
      }
    }
  }
  else
  {
    // Если новая секция анимации корректна и существует у стека...
    if ((DefGroup > 0) && (Stack->def->groups_count > DefGroup) && (((DWORD*)(Stack->def->active_groups))[DefGroup]) && (Stack->def->groups[DefGroup]->frames_count > 0))
    {
      // Если продолжает проигрываться текщая секция...
      if (DefGroup == Stack->def_group_ix)
      {
        // Переходим к следующему кадру.
        Stack->def_frame_ix += 1;
        
        // Номер максимального кадра.
        int MaxFrame = Stack->def->groups[DefGroup]->frames_count;
        
        // Зацикливаем анимацию.
        if (Stack->def_frame_ix >= MaxFrame)
        {
          Stack->def_frame_ix = 0;
        }
      }
      // Если нужно начать новую секцию...
      else
      {
        // Начинаем проигрывать новую секцию анимации.
        Stack->def_group_ix = DefGroup;
        Stack->def_frame_ix = 0;
      }
    }
  }
}



// Проверка способности стека анимировать.
_bool_ CanStackAnim(_BattleStack_* Stack)
{
  // Если стек жив, не ослеплён, не окаменён, не парализован, он может анимировать.
  if (!(Stack->creature.flags & (1 << 21)) && !(Stack->active_spell_duration[SPL_BLIND]) && !(Stack->active_spell_duration[SPL_STONE])
      && !(Stack->active_spell_duration[SPL_PARALYZE]))
  {
    return TRUE;
  }
  // Иначе - не может.
  else
  {
    return FALSE;
  }
}











// Проигрывание плавного изменения экрана. Изменений не производится, только анимация.
// В случае отсутствия (NULL) функций управления изменение будет отрисовываться стандартно, но потребуется дополнительная подготовка.
// Draw_Func - функция отрисовки (без обновления).
// Redo_Func - функция, производящая изменения, к которым нужно прийти плавно.
// Undo_Func - функция, отменяющая изменения Redo_Func.
// Anim_Func - функция, проводящая проход анимации.
// Flip_Func - функция, обновляющая экран (в дополнение к стандартной).
// X_Pos - X-координата изменяемой области экрана.
// Y_Pos - Y-координата изменяемой области экрана.
// Width - ширина изменяемой области экрана.
// Height - высота изменяемой области экрана.
// FrameTime - время между кадрами, если их 8 (общее время/8), -1 - минимальное.
void PlaySmoothAnim(void (__stdcall* Draw_Func)(), void (__stdcall* Redo_Func)(), void (__stdcall* Undo_Func)(),
                    void (__stdcall* Anim_Func)(), void (__stdcall* Flip_Func)(),
                    _int_ X_Pos, _int_ Y_Pos, _int_ Width, _int_ Height, _int_ FrameTime)
{
  // Устанавливаем функции управления анимацией.
  SmoothAnimSpec_Draw = Draw_Func;
  SmoothAnimSpec_Redo = Redo_Func;
  SmoothAnimSpec_Undo = Undo_Func;
  SmoothAnimSpec_Anim = Anim_Func;
  SmoothAnimSpec_Flip = Flip_Func;
  
  // Сохраняем старое изображение.
  o_WndMgr->MakeBackupScreen(X_Pos, Y_Pos, Width, Height);
  
  
  // Выполняем изменения.
  SmoothAnimSpec_Redo();
  
  // Отрисовываем изменения на экран, но не обновляем.
  if (SmoothAnimSpec_Draw) SmoothAnimSpec_Draw();
  
  // Отрисовываем плавное изменение.
  o_WndMgr->SmoothImageChange(X_Pos, Y_Pos, Width, Height, FrameTime);
  
  // Отменяем изменения.
  SmoothAnimSpec_Undo();
  
  
  // Сбрасываем функции управления анимацией.
  SmoothAnimSpec_Draw = NULL;
  SmoothAnimSpec_Redo = NULL;
  SmoothAnimSpec_Undo = NULL;
  SmoothAnimSpec_Anim = NULL;
  SmoothAnimSpec_Flip = NULL;
}
















// Функция, отрисовывающая изменения для плавного изменения экрана в битве.
void __stdcall SmoothAnimSpec_Battle_Draw()
{
  // Границы отрисовки боя.
  _RedrawBorders_* brd = o_BattleMgr->PField<_RedrawBorders_>(81208);
  
  // Ненастоящие границы: т. к. HD тоже будет их менять.
  brd->Left -= HD_Battle_X;
  brd->High -= HD_Battle_Y;
  brd->Right -= HD_Battle_X;
  brd->Low -= HD_Battle_Y;
  
  // Отрисовываем только в границах отрисовки.
  o_BattleMgr->Field<_bool32_>(81200) = TRUE;
  o_BattleMgr->RedrawBattlefield(FALSE, FALSE, TRUE, 0, TRUE, FALSE);
  
  // Восстанавливаем границы.
  brd->Left += HD_Battle_X;
  brd->High += HD_Battle_Y;
  brd->Right += HD_Battle_X;
  brd->Low += HD_Battle_Y;
}



// Функция, производящая анимацию для плавного изменения экрана в битве.
void __stdcall SmoothAnimSpec_Battle_Anim()
{
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  o_BattleMgr->SetRedrawBorders();
}


// Функция, производящая анимацию для плавного изменения экрана в битве.
void __stdcall SmoothAnimSpec_Battle_Flip()
{
  // Границы отрисовки боя.
  _RedrawBorders_* brd = o_BattleMgr->PField<_RedrawBorders_>(81208);
  
  // Ненастоящие границы: т. к. для HD они будут изменены.
  brd->Left -= HD_Battle_X;
  brd->High -= HD_Battle_Y;
  brd->Right -= HD_Battle_X;
  brd->Low -= HD_Battle_Y;
  
  o_BattleMgr->FlipRedrawRect();
  
  // Восстанавливаем границы.
  brd->Left += HD_Battle_X;
  brd->High += HD_Battle_Y;
  brd->Right += HD_Battle_X;
  brd->Low += HD_Battle_Y;
}





// Функция, возвращающая изменения для плавного изменения экрана при исчезновении трупов.
void __stdcall SmoothAnimSpec_RemoveDead_Redo()
{
  // Скрываем все стеки, которые нужно.
  for (_int_ Side = ATTACKER; Side <= DEFENDER; Side++)
  {
    for (_int_ StackNum = 0; StackNum < BATTLE_SIDE_STACKS_COUNT; StackNum++)
    {
      if (*(_bool8_*)((_ptr_)o_BattleMgr + 78904 + 20*Side + StackNum))
      {
        *(_bool8_*)((_ptr_)(&(o_BattleMgr->stack[Side][StackNum])) + 31) = TRUE;
      }
    }
  }
}

// Функция, откатывающая изменения для плавного изменения экрана при исчезновении трупов.
void __stdcall SmoothAnimSpec_RemoveDead_Undo()
{
  // Показываем все стеки, которые нужно.
  for (_int_ Side = ATTACKER; Side <= DEFENDER; Side++)
  {
    for (_int_ StackNum = 0; StackNum < BATTLE_SIDE_STACKS_COUNT; StackNum++)
    {
      if (*(_bool8_*)((_ptr_)o_BattleMgr + 78904 + 20*Side + StackNum))
      {
        *(_bool8_*)((_ptr_)(&(o_BattleMgr->stack[Side][StackNum])) + 31) = FALSE;
      }
    }
  }
}





// Функция, возвращающая изменения для плавного изменения экрана при вызове.
void __stdcall SmoothAnimSpec_Summon_Redo()
{
  // Показываем стек.
  *(_bool8_*)((_ptr_)SmoothAnimSpec_Summon_Stack + 31) = FALSE;
  
}

// Функция, откатывающая изменения для плавного изменения экрана при вызове.
void __stdcall SmoothAnimSpec_Summon_Undo()
{
  // Скрываем стек.
  *(_bool8_*)((_ptr_)SmoothAnimSpec_Summon_Stack + 31) = TRUE;
}







// Функция, возвращающая изменения для плавного изменения экрана при телепорте.
void __stdcall SmoothAnimSpec_Teleport_Redo()
{
  // Перемещаем стек на новый гекс.
  o_BattleMgr->RemoveStackFromGexes(SmoothAnimSpec_Teleport_Stack);
  o_BattleMgr->PutStackToGex(SmoothAnimSpec_Teleport_Stack, SmoothAnimSpec_Teleport_TargetGexNum);
  SmoothAnimSpec_Teleport_Stack->hex_ix = SmoothAnimSpec_Teleport_TargetGexNum;
}

// Функция, откатывающая изменения для плавного изменения экрана при телепорте.
void __stdcall SmoothAnimSpec_Teleport_Undo()
{
  // Перемещаем стек на старый гекс.
  o_BattleMgr->RemoveStackFromGexes(SmoothAnimSpec_Teleport_Stack);
  o_BattleMgr->PutStackToGex(SmoothAnimSpec_Teleport_Stack, SmoothAnimSpec_Teleport_StartGexNum);
  SmoothAnimSpec_Teleport_Stack->hex_ix = SmoothAnimSpec_Teleport_StartGexNum;
}





// Функция, возвращающая изменения для плавного изменения экрана при уничтожении препятствий.
void __stdcall SmoothAnimSpec_RemoveObstacle_Redo()
{
  // Делаем препятствие невидимым.
  SmoothAnimSpec_RemoveObstacle_ObstacleVisible = FALSE;
}

// Функция, откатывающая изменения для плавного изменения экрана при уничтожении препятствий.
void __stdcall SmoothAnimSpec_RemoveObstacle_Undo()
{
  // Делаем препятствие видимым.
  SmoothAnimSpec_RemoveObstacle_ObstacleVisible = TRUE;
}








// Функция, отрисовывающая изменения для текущего удержания нажатия кнопки в бою.
void __stdcall ButtonWhileClicked_Battle_Draw()
{
  // Проигрываем шаг случайной анимации.
  o_BattleMgr->PlayWaitAnimOnce();
}















// При загрузке и старте звука делаем его параллельным.
_Sample_ __stdcall HookOn_Load_Start_Sample(HiHook* h, _cstr_ Name)
{
  // Предбитвенный звук возвращаем стандартно.
  if (IsPreBattleSound)
  {
    return CALL_1(_Sample_, __fastcall, h->GetDefaultFunc(), Name);
  }
  // Иные звуки...
  else
  {
    // Проигрываем звук распараллеленно.
    CALL_3(void, __fastcall, 0x59A890, Name, -1, 3);
    
    // Возвращаем неудачу при загрузке.
    return EmptySample;
  }
}




// При загрузке и старте звука предбитвенного звука загружаем и начинаем его как обычно.
_Sample_ __stdcall HookOn_Load_Start_PreBattle_Sample(HiHook* h, _cstr_ Name)
{
  IsPreBattleSound = TRUE;
  
  // Инициализуруем и начинаем звук.
  _Sample_ Sample = CALL_1(_Sample_, __fastcall, h->GetDefaultFunc(), Name);;
  
  IsPreBattleSound = FALSE;
  
  return Sample;
}




// Пропуск стандартного окончания звука.
void __stdcall HookOn_End_Sample_Std(_Sample_ Sample)
{
  // Не оканчиваем эти звуки.
  return;
}




// При ожидании и окончании проигрывания звука пропускаем это для всех звуков, кроме предбитвенного.
void __stdcall HookOn_Wait_End_Close_Sample(HiHook* h, int Time, _Sample_ Sample)
{
  // Если звук предбитвенный - проигрываем.
  if (IsPreBattleSound)
  {
    CALL_3(void, __thiscall, h->GetDefaultFunc(), Time, Sample.Wav, Sample.PlayingInd);
  }
}

// При ожидании и окончании проигрывания предбитвенного звука...
void __stdcall HookOn_Wait_End_Close_PreBattleSample(HiHook* h, int Time, _Sample_ Sample)
{
  
  IsPreBattleSound = TRUE;
  
  // Инициализация времени анимации ожидания.
  WaitAnimTime = o_GetTime();
  
  // Сообщение о возможности пропуска.
  strcpy(H3TempStr, PreBattleSound_SkippingMessage.c_str());
  CALL_4(void, __thiscall, 0x4729D0, *(DWORD*)((DWORD)o_BattleMgr + 78588), H3TempStr, 0, 0);
  
  // Проигрываем предбитвенный звук нормально.
  CALL_3(void, __thiscall, h->GetDefaultFunc(), Time, Sample.Wav, Sample.PlayingInd);
  
  // Стираем сообщение о возможности пропуска.
  CALL_4(void, __thiscall, 0x4729D0, *(DWORD*)((DWORD)o_BattleMgr + 78588), &EmptyVar, 0, 0);
  
  IsPreBattleSound = FALSE;
}


// При расчёте времени проигрывании звука в бою учитываем его настройки скорости.
int __stdcall HookOn_Wait_End_Close_Sample_CalcTime(LoHook* h, HookContext* c)
{
  // Если это звук битвы, но не предбитвенная панорама, умножаем на моножитель скорости.
  if (CanDrawBattle() && !IsPreBattleSound)
  {
    c->esi = (DWORD)(((double)(c->esi))*(BattleAnimPeriodFactors[Settind_BattleFast]));
  }
  
  return EXEC_DEFAULT;
}


// При ожидании и окончании звука в бою также отрисовываем анимацию.
int __stdcall HookOn_Wait_End_Close_Sample_Play(LoHook* h, HookContext* c)
{
  // Если это битва и настало время, отрисовываем анимацию ожидания.
  if (CanDrawBattle() && o_GetTime() - WaitAnimTime >= 0)
  {
    // Очистка полей перерисовки.
    o_BattleMgr->ClearRedrawFields();
    // Проигрывание случайной анимации.
    o_BattleMgr->PlayWaitAnim();
    // Отрисовка.
    o_BattleMgr->RedrawBattlefield(TRUE, TRUE, TRUE, 0, TRUE, FALSE);
  }
  
  
  // Если сейчас предбитвенный звук и была нажата клавиша ESC, завершаем его.
  if (IsPreBattleSound)
  {
    // Получаем первое несчитанное событие.
    _EventMsg_ event_msg;
    o_InputMgr->Peek_Event(&event_msg);
    
    // Если событие - нажатие клавиши ESC, заверщаем звук.
    if (event_msg.type == 1 && event_msg.subtype == KEY_ESC)
    {
      // Время завершения - в близжайший момент.
      *(DWORD*)(c->ebp - 4) = 0;
    }
  }
  
  return EXEC_DEFAULT;
}




// При ожидании проигрывания звука пропускаем его.
void __stdcall HookOn_Wait_Sample(HiHook* h, _ptr_ SoundMgr, _dword_ SampleInd, int Time)
{
  // Не ожидаем.
  return;
}



// При расчёте времени ожидания звука в бою учитываем его настройки скорости.
int __stdcall HookOn_Wait_Sample_CalcTime(LoHook* h, HookContext* c)
{
  // Если это звук битвы, но не предбитвенная панорама, умножаем на моножитель скорости.
  if (CanDrawBattle())
  {
    c->esi = (_int32_)(((double)(c->esi))*(BattleAnimPeriodFactors[Settind_BattleFast]));
  }
  
  return EXEC_DEFAULT;
}


// При ожидании звука в бою также отрисовываем анимацию.
int __stdcall HookOn_Wait_Sample_Play(LoHook* h, HookContext* c)
{
  // Если это битва и настало время, отрисовываем анимацию ожидания.
  if (CanDrawBattle() && o_GetTime() - WaitAnimTime >= 0)
  {
    // Очистка полей перерисовки.
    o_BattleMgr->ClearRedrawFields();
    // Проигрывание случайной анимации.
    o_BattleMgr->PlayWaitAnim();
    // Отрисовка.
    o_BattleMgr->RedrawBattlefield(TRUE, TRUE, TRUE, 0, TRUE, FALSE);
  }
  
  return EXEC_DEFAULT;
}







// При инициализации интерфейса битвы отключаем возможность тактического режима, чтобы кнопки не закрывали окно лога.
DWORD __stdcall HookOn_PreBattle_InterfaceInit(HiHook* h, DWORD a1, _byte_ IsTactiс)
{
  // Инициализация обычного режима.
  return CALL_2(DWORD, __thiscall, h->GetDefaultFunc(), a1, 0);
}


// Позже инициализируем интерфейс как надо и инициализируем времена случайной анимации.
int __stdcall HookOn_LaterPreBattle_InterfaceInit(LoHook* h, HookContext* c)
{
  
  // Если необходим тактический режим и есть диалог боя...
  if (*(BYTE*)((DWORD)o_BattleMgr + 81256) && (_ptr_)(o_BattleMgr->dlg))
  { 
    // Если боевые кнопки есть - удаляем их через деструктор.
    if (*(DWORD*)((_ptr_)(o_BattleMgr->dlg) + 112))
    {
      CALL_2(void, __thiscall, ***(_ptr_***)((_ptr_)(o_BattleMgr->dlg) + 112), *(_ptr_*)((_ptr_)(o_BattleMgr->dlg) + 112), 1);
      *(DWORD*)((_ptr_)(o_BattleMgr->dlg) + 112) = 0;
    }
    
    // Инициализируем боевые кнопки тактики.
    *(DWORD*)((_ptr_)(o_BattleMgr->dlg) + 112) = CALL_2(DWORD, __thiscall, 0x46BBA0, o_MemAlloc(60), o_BattleMgr->dlg);
    CALL_3(void, __thiscall, 0x5FF490, o_BattleMgr->dlg, 2008, 8);
    CALL_3(void, __thiscall, 0x5FF490, o_BattleMgr->dlg, 2009, 8);
    CALL_3(void, __thiscall, 0x5FF490, o_BattleMgr->dlg, 2010, 8);
    
  }
  
  
  // Инициализация времён случайных анимаций.
  o_BattleMgr->InitRandAnimsTimes();
  
  
  return EXEC_DEFAULT;
}




// Вместо загрузки некоторых звуков возвращаем их имя.
_cstr_ __fastcall HookOn_Some_LoadWav(_cstr_ Name)
{
  return Name;
}

// Вместо старта некоторых звуков по ссылке на них запускаем их распараллеленно по имени.
_dword_ __stdcall HookOn_Some_StartSound(HiHook* h, _ptr_ SoundMgr, _cstr_ Name)
{
  // Проигрываем звук распараллеленно.
  CALL_3(void, __fastcall, 0x59A890, Name, -1, 3);
  
  return -1;
}












// При вызове функции перехода к следующему раунду при тактической фазе сохраняем то, что это была тактическая фаза.
void __stdcall HookOn_Battle_NextRoundTactic(HiHook* h, _BattleMgr_* this_)
{
  // Был тактический раунд.
  RoundWasTactic = TRUE;
  
  // Вызываем функцию перехода к следующему раунду.
  CALL_1(void, __thiscall, h->GetDefaultFunc(), this_);
  
  // Сбрасываем тактический раунд.
  RoundWasTactic = FALSE;
}


// При добавлении в лог информации о следующем раунде учитываем тактическую фазу.
int __stdcall HookOn_BattleNextRoundLog(LoHook* h, HookContext* c)
{
  // Если предыдущий раунд был тактическим, пропускаем добавление сообщения в лог.
  if (RoundWasTactic)
  {
    c->return_address - 0x475B1E;
    
    return NO_EXEC_DEFAULT;
  }
  // Если предыдущий раунд был не тактическим - всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}






// Перед отрисовкой башни проверяем существо на ней на Медузу и запоминаем результат проверки.
int __stdcall HookOn_Battle_DrawTower_Check_Medusa(LoHook* h, HookContext* c)
{  
  // Если это башня и на ней Медуза, указываем это.
  if (((_ArrowTower_*)(c->ecx))->CreatureType == CID_MEDUSA) // Медуза
  {
    MedusaTower_Drawing = TRUE;
  }
  
  return EXEC_DEFAULT;
}



// После отрисовки башни стираем результат проверки на Медузу.
int __stdcall HookOn_Battle_DrawTower_UnCheck_Medusa(LoHook* h, HookContext* c)
{  
  // Башня с Медузой больше не рисуется.
  MedusaTower_Drawing = FALSE;
  
  return EXEC_DEFAULT;
}


// При отрисовке башни учитываем возможность нахождения на ней Медузы.
void __stdcall HookOn_Battle_DrawTower_Height(HiHook* h, _Def_* CrDef, _int32_ AnimSection, _int32_ AnimFrame, DWORD a4, DWORD a5, DWORD a6, DWORD Height, DWORD a8, _int32_ X_Pos, _int32_ Y_Pos, DWORD a11, DWORD a12, DWORD a13, _int32_ Orientation, DWORD a15)
{
  // Учитываем добавку к высоте для Медузы.
  if (MedusaTower_Drawing)
  {
    Height += TOWER_MEDUSA_ADD_HEIGHT;
  }
  
  // Вызываем функцию отрисовки.
  CALL_15(void, __thiscall, h->GetDefaultFunc(), CrDef, AnimSection, AnimFrame, a4, a5, a6, Height, a8, X_Pos, Y_Pos, a11, a12, a13, Orientation, a15);
}








// При вызове функции отрисовки тени перемещения стека вне общей отрисовки для стёрки...
_bool32_ __stdcall HookOn_BattleMShadowDraw(HiHook* h, _BattleMgr_* this_, _dword_ a1, _dword_ a2)
{
  // Не вызывем функцию отрисовки тени перемещения.
  
  // Устанавливаем флаг необходимости перерисовки фона битвы (FALSE - перерисовывать).
  *(_bool32_*)((DWORD)this_ + 21432) = FALSE;
  
  // Возвращаем значение, как будто перерисовали.
  return TRUE;
}







// При выделении стека учитываем то, что он может быть неспособен анимировать.
int __stdcall HookOn_BattleSelectStack(LoHook* h, HookContext* c)
{
  // Если стек может анимировать - всё стандартно.
  if ((((_BattleStack_*)(c->edi))->creature_id != CID_ARROW_TOWER) && CanStackAnim((_BattleStack_*)(c->edi)))
  {
    // Добавляем границы перерисовки стека до анимации.
    ((_BattleMgr_*)(c->esi))->Set_Stack_Redrawable((_BattleStack_*)(c->edi));
    ((_BattleMgr_*)(c->esi))->SetRedrawBorders();
    
    return EXEC_DEFAULT;
  }
  // Иначе - пропускаем смену кадров.
  else
  {
    c->return_address = 0x4776BE;
  }

  return NO_EXEC_DEFAULT;

}


// При выделении стека площадным заклинанием учитываем то, что он может быть неспособен анимировать.
int __stdcall HookOn_BattleSpellSelectStack(LoHook* h, HookContext* c)
{
  // Если стек может анимировать - всё стандартно.
  if ((((_BattleStack_*)(c->esi))->creature_id != CID_ARROW_TOWER) && CanStackAnim((_BattleStack_*)(c->esi)))
  {
    // Добавляем границы перерисовки стека до анимации.
    o_BattleMgr->SetRedrawBorders();
    
    return EXEC_DEFAULT;
  }
  // Иначе - пропускаем смену кадров.
  else
  {
    c->return_address = 0x43EDC6;
  }
  return NO_EXEC_DEFAULT;

}







// При инициализации битвы...
_bool32_ __stdcall HookOn_BattleInit(HiHook* h, _BattleMgr_* this_, DWORD a2)
{
  IsBattleInit = TRUE;
  
  // Период анимации рамки вокруг стека.
  BorderPeriod = STD_FRAME_PERIOD;
  
  // Время последнего проигрывания рамок вокруг стеков - сейчас.
  BorderLastTime = o_GetTime();
  
  _bool32_ res = CALL_2(_bool32_, __thiscall, h->GetDefaultFunc(), this_, a2);
  
  
  
  
  // Битва отображается.
  if (!this_->ShouldNotRenderBattle())
  {
    // Ходит не игрок за компьютером.
    if (!this_->Field<_bool32_>(78516) || this_->playerID[this_->currentActiveSide] < 0
        || !CALL_2(_bool8_, __thiscall, 0x4CE600, o_GameMgr, this_->playerID[this_->currentActiveSide]))
    {
      b_MouseMan_SetCursor(6, 2);
    }
    // Ходит игрок за компьютером.
    else
    {
      // Нет выбранного курсора.
      o_BattleMgr->Field<_int32_>(78548) = -1;
      
      // Есть диалог.
      if (this_->dlg)
      {
        // Сообщение для установки курсора.
        _EventMsg_ msg;
        msg.type = 4;
        o_GetIngameCursorPos(&msg.subtype, &msg.item_id);
        msg.x_abs = msg.subtype;
        msg.y_abs = msg.item_id;
        msg.flags = 0;
        if ((GetKeyState(17) >> 8) & 0x80) msg.flags = 0x4;
        if ((GetKeyState(18) >> 8) & 0x80) msg.flags |= 0x20;
        if ((GetKeyState(16) >> 8) & 0x80) msg.flags |= 0x1;
        
        // Вызываем установку курсора.
        BWU_SkipChek = TRUE;
        CALL_2(_bool32_, __thiscall, 0x4746B0, this_, &msg);
        BWU_SkipChek = FALSE;
      }
    }
  }
  
  IsBattleInit = FALSE;
  
  return res;
}


// При инициализации графики героя в бою...
int __stdcall HookOn_BattleHeroInitDef(LoHook* h, HookContext* c)
{
  // Присваиваем герою значение периода случайной анимации в соответствии с его городом и полом.
  SideHeroRandAnimPeriod[c->edi] = 0; //ClassesRandAnimPeriod[c->eax];
  
  // Текущее время.
  _int_ CurrTime = o_GetTime();
  
  // Период анимации флага героя.
  SideFlagAnimPeriod[c->edi] = TRandint((_int_)(((double)STD_FRAME_PERIOD)*(1.0 - MAX_PERIOD_RAND_DEVI)), (_int_)(((double)STD_FRAME_PERIOD)*(1.0 + MAX_PERIOD_RAND_DEVI)));
  // Последнее проигрывание анимации флага героя - сейчас.
  SideFlagLastAnimTime[c->edi] = CurrTime;
  
  // Период анимации героя.
  SideHeroAnimPeriod[c->edi] = STD_FRAME_PERIOD;
  // Последнее проигрывание анимации героя  - сейчас
  SideHeroLastAnimTime[c->edi] = CurrTime;
  
  
  // Выполняем затёртое.
  return EXEC_DEFAULT;
}


// Время следующей анимации ожидания.
_int32_ NextWaitAnimT;


// При инициализации начальных времён участников битвы добавляем случайности к случайной анимации героя.
int __stdcall HookOn_BattleInitTimes(LoHook* h, HookContext* c)
{
  // Время следующей анимации ожидания.
  NextWaitAnimT = o_GetTime();
  
  for (int i = 0; i < 2; i++)
  {
    // Время последней случайной анимации героя (добавляем элемент случайности, если нужно).
    if (SideHeroRandAnimPeriod[i])
    {
      *(_int32_*)(c->esi + 21500 + 4*i) = c->eax + TRandint(-(_int32_)(((double)SideHeroRandAnimPeriod[i])*MAX_PERIOD_RAND_DEVI), (_int32_)(((double)SideHeroRandAnimPeriod[i])*MAX_PERIOD_RAND_DEVI));
    }
    else
    {
      *(_int32_*)(c->esi + 21500 + 4*i) = c->eax;
    }
  }
  
  // Пропускаем стандартное получение времени.
  c->return_address = 0x47987F;
  
  return NO_EXEC_DEFAULT;
}




// При инициализвации стека прописываем ему параметры анимации стойки.
void __stdcall HookOn_StackInit(HiHook* h, _BattleStack_* this_, int creature_id, int count, _Hero_* hero_owner, int side, int index_on_side, int position_hex_ix,  int army_slot_ix)
{
  // Вызов иициализации стека.
  CALL_8(void, __thiscall, h->GetDefaultFunc(), this_, creature_id, count, hero_owner, side, index_on_side, position_hex_ix, army_slot_ix);
  
  // Текущее время.
  _int_ CurrTime = o_GetTime();
  
  // Время последней анимации - сейчас.
  StackLastStayAnimTime[side][index_on_side] = CurrTime;
  
  
  // Случайное значение для времени кадра анимации стеков.
  double rand_val = (double)TRandint(-1000, 1000);
  
  // Отдельно обрабатываем стрелковые башни.
  if (creature_id == CID_ARROW_TOWER)
  {
    // Существо на башне.
    _int_ tower_creature = *(DWORD*)((DWORD)o_BattleMgr + 81272 + 36*(this_->Get_ArrowTowerNum()));
    
    // Период анимации стойки.
    StackStayAnimPeriod[side][index_on_side] = (_int_)(((double)(StayAnimPeriod[tower_creature]))*(1.0 + StayAnim_Rand_Devi[tower_creature]*rand_val/1000.0));
    // Период случайной анимации.
    StackRandAnimPeriod[side][index_on_side] = (_int_)(((double)(RandAnimPeriod[tower_creature]))*(1.0 + StayAnim_Rand_Devi[tower_creature]*rand_val/1000.0));
    
    // Необходимость проигрывать анимацию стойки.
    StackStayAnimNeedPlay[side][index_on_side] = StayAnimNeedPlay[tower_creature];
  }
  else
  {
    // Период анимации стойки.
    StackStayAnimPeriod[side][index_on_side] = (_int_)(((double)StayAnimPeriod[creature_id])*(1.0 + StayAnim_Rand_Devi[creature_id]*rand_val/1000.0));
    // Период случайной анимации.
    StackRandAnimPeriod[side][index_on_side] = (_int_)(((double)RandAnimPeriod[creature_id])*(1.0 + StayAnim_Rand_Devi[creature_id]*rand_val/1000.0));
    
    // Необходимость проигрывать анимацию стойки.
    StackStayAnimNeedPlay[side][index_on_side] = StayAnimNeedPlay[creature_id];
  }
  
  
}






// При проигрывании анимации ожидания в бою - заменяем оригинальную функцию...
void __stdcall HookOn_WaitAnimDraw(HiHook* h, _BattleMgr_* this_)
{
  // Текущее время анимации ожидания.
  WaitAnimCurrTime = o_GetTime();
  
  // Проверка необходимости отрисовки битвы и подходящее время для этого.
  if (!CanDrawBattle() || WaitAnimCurrTime - NextWaitAnimT < 0) return;
  
  // Подготавливаем анимацию фигур.
  // BattleFgrs_ParallelAnimPrepare(); // Пока убираем для оптимизации
  
  // Нужно ли анимировать флаг.
  _bool8_ flag_anim[2];
  MemZero(flag_anim, sizeof(flag_anim));
  
  // Анимация флагов - подготовка.
  for (int i = 0; i < 2; i++)
  {
    // Есть def флага героя и герой и наступило время проигрывания анимации флага.
    if (this_->PField<_Def_*>(21516)[i] && this_->hero[i]
        && WaitAnimCurrTime - SideFlagLastAnimTime[i] > SideFlagAnimPeriod[i])
    {
      // Новое время последней анимации флага.
      SideFlagLastAnimTime[i] = WaitAnimCurrTime;
      
      // Флаг должен анимировать.
      flag_anim[i] = TRUE;
      
      // Надо перерисовывать флаг.
      this_->PField<_bool8_>(81962)[i] = TRUE;
    }
  }
  
  // Отряд с жёлтой рамкой.
  _BattleStack_* yellow_stack = this_->Field<_BattleStack_*>(78536);
  _int_ yellow_stack_owner = -1;
  _int_ yellow_stack_ix = -1;
  if (yellow_stack)
  {
    yellow_stack_owner = yellow_stack->side;
    yellow_stack_ix = yellow_stack->index_on_side;
  }
  
  // Выделенный отряд.
  _BattleStack_* selected_stack = NULL;
  _int_ selected_stack_owner = -1;
  _int_ selected_stack_ix = -1;
  if (this_->Field<_bool8_>(78540))
  {
    _BattleHex_* hex = &this_->hex[this_->Field<_int32_>(78544)];
    selected_stack_owner = hex->bstack_side;
    selected_stack_ix = hex->bstack_index;
    selected_stack = &this_->stack[selected_stack_owner][selected_stack_ix];
  }
  
  // Необходимости проигрывать анимацию стеков.
  _bool8_ need_ranim = 0;
  _bool8_ stack_need_ranim[2][20];
  _bool8_ stack_need_stay_anim[2][20];
  MemZero(stack_need_ranim, sizeof(stack_need_ranim));
  MemZero(stack_need_stay_anim, sizeof(stack_need_stay_anim));
  
  // Анимации стеков.
  for (_int_ i = 0; i < 2; i++)
  {
    // Количество стеков стороны.
    _int_ stacks_count = this_->countMonsters[i];
    for (int j = 0; j < stacks_count; j++)
    {
      // Текущий стек.
      _BattleStack_* stack = &this_->stack[i][j];
      
      // По-умолчанию стек не будет играть анимацию стойки.
      stack_need_stay_anim[i][j] = FALSE;
      
      // Стек может анимировать.
      if (CanStackAnim(stack))
      {
        _int_ def_group = stack->def_group_ix;
        // Проигрывается случайная анимация.
        if (def_group == DG_RANDANIM)
        {
          // Если наступило время проигрывания - проигрываем.
          if (StackLastStayAnimTime[i][j] + StackRandAnimPeriod[i][j] - WaitAnimCurrTime <= 0)
          {
            // Новое время последнего кадра кривляния.
            StackLastStayAnimTime[i][j] = WaitAnimCurrTime;
            
            // Устанавливаем поле перерисовки стека.
            o_BattleMgr->Set_Stack_Redrawable(stack);
            
            // Нужно играть случайную анимацию.
            need_ranim = TRUE;
            stack_need_ranim[i][j] = TRUE;
          }
        }
        // Случайная анимация ещё не проигрывается.
        else
        {
          // Если наступило время играть анимацию...
          if (StackStayAnimNeedPlay[i][j]
              && StackLastStayAnimTime[i][j] + StackStayAnimPeriod[i][j] - WaitAnimCurrTime <= 0)
          {
            // Если стек - стрелковая башня...
            if (stack->creature_id == CID_ARROW_TOWER)
            {
              // Получаем информацию о стрелковой башне.
              _ArrowTower_* Tower = &o_BattleMgr->PField<_ArrowTower_>(81272)[stack->Get_ArrowTowerNum()];
              
              // Если башня ожидает...
              if (Tower->AnimSectionNum == DG_STAY)
              {
                // Стек будет играть анимацию стойки.
                need_ranim = TRUE;
                stack_need_stay_anim[i][j] = TRUE;
                
                // Указываем на необходимость перерисовки анимации.
                o_BattleMgr->Set_Stack_Redrawable(stack);
              }
            }
            // Если стек - не стрелковая башня...
            else
            {
              // Если стек стоит...
              if (stack->def_group_ix == DG_STAY)
              {
                // Стек будет играть анимацию стойки.
                need_ranim = TRUE;
                stack_need_stay_anim[i][j] = TRUE;
                
                // Указываем на необходимость перерисовки анимации.
                o_BattleMgr->Set_Stack_Redrawable(stack);
              }
            }
          }
          
          // Можно начинать случайные анимации.
          if (!IsPreBattleSound && NeedBeginRandomAnims)
          {
            // Стек в анимации стойки, может проигрывать анимацию ожидания и настало время её играть.
            _Def_* def = stack->def;
            if (stack->creature_id != CID_ARROW_TOWER && def_group == DG_STAY
                && def->groups_count > DG_RANDANIM && DwordAt(def->active_groups + 4*DG_RANDANIM) && def->groups[DG_RANDANIM]->frames_count > 0
                && 0 > stack->Field<_int32_>(252) + stack->Field<_int32_>(340) - WaitAnimCurrTime)
            {
              // Устанавливаем поле перерисовки стека.
              o_BattleMgr->Set_Stack_Redrawable(stack);
              
              // Нужно играть случайную анимацию.
              need_ranim = TRUE;
              stack_need_ranim[i][j] = TRUE;
              // Стек будет играть случайную анимацию - не будет играть анимацию стойки.
              stack_need_stay_anim[i][j] = FALSE;
            }
          }
        }
      }
      
      // Рамки выделения стеков.
      if (NeedRedrawBorders && WaitAnimCurrTime - BorderLastTime > BorderPeriod)
      {
        // Является целью заклинания.
        if (stack->Field<_bool8_>(1265))
        {
          // Устанавливаем поле перерисовки стека.
          o_BattleMgr->Set_Stack_Redrawable(stack);
        }
      }
    }
  }
  
  // Рамки выделения стеков.
  if (NeedRedrawBorders && WaitAnimCurrTime - BorderLastTime > BorderPeriod)
  {
    // Выделенный жёлтой рамкой стек.
    if (yellow_stack)
    {  
      // Устанавливаем поле перерисовки стека.
      o_BattleMgr->Set_Stack_Redrawable(yellow_stack);
    }
    
    // Выбранный стек.
    if (selected_stack)
    {
      // Устанавливаем поле перерисовки стека.
      o_BattleMgr->Set_Stack_Redrawable(selected_stack);
    }
  }
  
  // Анимация для проигрывания героем.
  _int8_ hero_anim[2];
  
  // Анимация героев.
  for (int i = 0; i < 2; i++)
  {
    // Нет анимации.
    hero_anim[i] = -1;
    
    // Есть def героя.
    _Def_* def = this_->PField<_Def_*>(21508)[i];
    if (def)
    {
      // Текущая анимация героя.
      _int_ curr_anim = this_->PField<_int32_>(21476)[i];
      
      // Нужна ли особая анимация.
      _bool8_ spec_anim = FALSE;
      
      // Герой постоянно анимирует.
      if (!SideHeroRandAnimPeriod[i])
      {
        // В неподвижности.
        if (curr_anim == 0)
        {
          // Герой будет перерисован и запускается случайная анимация.
          // Считаем, что первый кадр случайной анимации такой же, как и кадр без анимации.
          this_->PField<_int32_>(21476)[i] = 1;
          this_->PField<_int32_>(21484)[i] = 0;
          // Время нового кадра.
          if (WaitAnimCurrTime - SideHeroLastAnimTime[i] > SideHeroAnimPeriod[i])
          {
            SideHeroLastAnimTime[i] = WaitAnimCurrTime;
            this_->PField<_bool8_>(81960)[i] = TRUE;
          }
          spec_anim = TRUE;
        }
        // Анимирует.
        else if (curr_anim == 1)
        {
          // Время нового кадра.
          if (WaitAnimCurrTime - SideHeroLastAnimTime[i] > SideHeroAnimPeriod[i])
          {
            SideHeroLastAnimTime[i] = WaitAnimCurrTime;
            this_->PField<_bool8_>(81960)[i] = TRUE;
          }
          spec_anim = TRUE;
        }
        // Особая анимация.
        else if (curr_anim == 2 || curr_anim == 3)
        {
          // Время нового кадра.
          if (WaitAnimCurrTime - SideHeroLastAnimTime[i] > SideHeroAnimPeriod[i])
          {
            SideHeroLastAnimTime[i] = WaitAnimCurrTime;
            this_->PField<_bool8_>(81960)[i] = TRUE;
          }
        }
      }
      
      // Герой уже играет анимацию.
      if (SideHeroRandAnimPeriod[i] && (curr_anim == 1 || curr_anim == 2 || curr_anim == 3))
      {
        // Время нового кадра.
        if (WaitAnimCurrTime - SideHeroLastAnimTime[i] > SideHeroAnimPeriod[i])
        {
          SideHeroLastAnimTime[i] = WaitAnimCurrTime;
          this_->PField<_bool8_>(81960)[i] = TRUE;
        }
      }
      // Герой не играет анимацию.
      else
      {
        // Герой неподвижен, либо играет постоянную анимацию.
        if (curr_anim == 0 || spec_anim)
        {
          // Герой должен начать анимацию печали.
          if (!this_->PField<_bool8_>(21472)[i] && this_->PField<_bool8_>(21468)[i])
          {
            // Владелец героя.
            _int_ owner = this_->playerID[i];
            
            // Сбрасываем необходимость проигрывания радости и печали.
            this_->PField<_bool8_>(21468)[i] = FALSE;
            this_->PField<_bool8_>(21470)[i] = FALSE;
            
            // Владелец - человек.
            if (CALL_2(_bool8_, __thiscall, 0x4CE630, o_GameMgr, owner))
            {
              // Играет анимацию печали.
              this_->PField<_bool8_>(21472)[i] = TRUE;
              if (def->groups_count > 2 && DwordAt(def->active_groups + 8) && def->groups[2]->frames_count)
              {
                hero_anim[i] = 2;
                this_->PField<_bool8_>(81960)[i] = TRUE;
              }
            }
          }
          // Герой должен начать анимацию радости.
          else if (!this_->PField<_bool8_>(21474)[i] && this_->PField<_bool8_>(21470)[i])
          {
            // Владелец героя.
            _int_ owner = this_->playerID[i];
            
            // Сбрасываем необходимость проигрывания радости и печали.
            this_->PField<_bool8_>(21468)[i] = FALSE;
            this_->PField<_bool8_>(21470)[i] = FALSE;
            
            // Владелец - человек.
            if (CALL_2(_bool8_, __thiscall, 0x4CE630, o_GameMgr, owner))
            {
              // Играет анимацию радости.
              this_->PField<_bool8_>(21474)[i] = TRUE;
              if (def->groups_count > 3 && DwordAt(def->active_groups + 12) && def->groups[3]->frames_count)
              {
                hero_anim[i] = 3;
                this_->PField<_bool8_>(81960)[i] = TRUE;
              }
            }
          }
          // Герой, возможно, будет проигрывать случайную анимацию.
          else if (SideHeroRandAnimPeriod[i])
          {
            // Анимация проигрывается постоянно и её надо начать, либо настало время начать.
            if (!IsPreBattleSound
                && WaitAnimCurrTime - this_->Field<_int32_>(21500) > SideHeroRandAnimPeriod[i])
            {
              // Герой будет перерисован и запускается случайная анимация.
              hero_anim[i] = 1;
              this_->PField<_bool8_>(81960)[i] = TRUE;
            }
          }
        }
      }
    }
  }
  
  
  // Определяем границы перерисовки изменяемых элементов.
  this_->SetRedrawBorders();
  
  
  // Проигрываем анимацию фигур.
  // BattleFgrs_ParallelAnim(); // Пока убираем для оптимизации
  
  
  // Проигрываем анимацию флагов героев.
  for (_int_ side = ATTACKER; side <= DEFENDER; side++)
  {
    // Есть герой и надо анимировать.
    if (this_->hero[side] && flag_anim[side])
    {
      // Def флага героя.
      _Def_* flag_def = this_->PField<_Def_*>(21516)[side];
      
      // Переходим к следующему кадру.
      this_->PField<_int32_>(21524)[side] += 1;
      
      // Номер максимального кадра.
      _int_ MaxFrame = 0;
      // Берём номер максимального кадра ходьбы из анимации.
      if (flag_def->groups_count > DG_MAIN && ((DWORD*)(flag_def->active_groups))[DG_MAIN]) MaxFrame = flag_def->groups[DG_MAIN]->frames_count;
      
      // Зацикливаем анимацию.
      if (this_->PField<_int32_>(21524)[side] >= MaxFrame)
      {
        this_->PField<_int32_>(21524)[side] = 0;
      }
    }
  }
  
  
  // Проигрываем анимации стеков.
  if (need_ranim)
  {
    for (_int_ i = 0; i < 2; i++)
    {
      // Количество стеков стороны.
      _int_ stacks_count = this_->countMonsters[i];
      for (int j = 0; j < stacks_count; j++)
      {
        // Стек.
        _BattleStack_* stack = &this_->stack[i][j];
        
        // Нужно играть анимацию.
        if (stack_need_ranim[i][j])
        {
          // Начинаем проигрывать случайную анимацию.
          if (stack->def_group_ix == DG_STAY)
          {
            stack->def_group_ix = DG_RANDANIM;
            stack->def_frame_ix = 0;
          }
          else
          {
            // Период случайной анимации стека.
            _int_ anim_per = stack->Field<_int32_>(340);
            
            // Переходим к следующему кадру.
            if (anim_per > 0)
            {
              stack->def_frame_ix++;
            }
            
            // Количество кадров в def`е.
            _Def_* def = stack->def;
            _int_ frames_count = 0;
            if (def->groups_count > DG_RANDANIM && DwordAt(def->active_groups + 4*DG_RANDANIM))
            {
              frames_count = def->groups[DG_RANDANIM]->frames_count;
            }
            
            // Конец анимации.
            if (stack->def_frame_ix >= frames_count)
            {
              // Переходим к анимации стойки.
              stack->def_group_ix = DG_STAY;
              stack->def_frame_ix = 0;
              stack->Field<_int32_>(252) = WaitAnimCurrTime;
              
              // Настраиваем время следующего проигрывания случайной анимации.
              if (anim_per > 0)
              {
                stack->Field<_int32_>(252) += static_cast<_int_>(0.5*TRandint(0, anim_per) - 0.25*anim_per);
              }
            }
          }
        }
        else
        {
          // Если наступило время играть анимацию...
          if (stack_need_stay_anim[i][j])
          {
            // Проигрываем следующий кадр анимации стойки стека.
            PlayNextFrame(stack, DG_STAY);
            
            // Записываем новое время последней анимации стека.
            StackLastStayAnimTime[i][j] = WaitAnimCurrTime;
            
          }
        }
      }
    }
  }
  
  // Анимация героев.
  for (int i = 0; i < 2; i++)
  {
    // Нужно перерисовывать героя.
    if (this_->PField<_bool8_>(81960)[i])
    {
      // Продолжаем старую анимацию.
      if (hero_anim[i] == -1)
      {
        // Меняем кадр.
        _int_ frame_ix = this_->PField<_int32_>(21484)[i] + 1;
        this_->PField<_int32_>(21484)[i] = frame_ix;
        
        // Количество кадров в def`е.
        _Def_* def = this_->PField<_Def_*>(21508)[i];
        _int_ group = this_->PField<_int32_>(21476)[i];
        _int_ frames_count = 0;
        if (def->groups_count > group && DwordAt(def->active_groups + 4*group))
        {
          frames_count = def->groups[group]->frames_count;
        }
        
        // Анимация завершилась.
        if (frame_ix >= frames_count)
        {
          // Устанавливаем нулевой кадр анимации.
          this_->PField<_int32_>(21484)[i] = 0;
          
          // Если должна постоянно проигрываться случайная анимация.
          if (!SideHeroRandAnimPeriod[i])
          {
            // Будет проигрываться случайная анимация.
            this_->PField<_int32_>(21476)[i] = 1;
            
            // Устанавливаем время последней анимации.
            this_->PField<_int32_>(21500)[i] = WaitAnimCurrTime;
          }
          // Если случайная анимация не проигрывается постоянно - убираем анимацию. 
          else
          {
            this_->PField<_int32_>(21476)[i] = 0;
            
            // Устанавливаем время последней анимации (со случайностью для периода случайной анимации).
            this_->PField<_int32_>(21500)[i] = WaitAnimCurrTime + TRandint(-(DWORD)(((double)SideHeroRandAnimPeriod[i])*MAX_PERIOD_RAND_DEVI), (DWORD)(((double)SideHeroRandAnimPeriod[i])*MAX_PERIOD_RAND_DEVI));
          }
        }
      }
      // Начинаем новую анимацию.
      else
      {
        this_->PField<_int32_>(21476)[i] = hero_anim[i];
        this_->PField<_int32_>(21484)[i] = 0;
      }
    }
  }
  
  // Если наступило время проигрывания - проигрываем общую анимацию боя.
  if (WaitAnimCurrTime - BorderLastTime > BorderPeriod)
  {
    // Новое время последней анимации рамки (и объектов на клетках).
    BorderLastTime = WaitAnimCurrTime;
    
    // Кадр боя.
    this_->Field<_int32_>(81916)++;
    
    // Смещаем палитру рамок выделения.
    CALL_4(void, __thiscall, 0x522E40, PtrAt(0x6AAD18), 96, 103, -1);
    CALL_4(void, __thiscall, 0x522E40, PtrAt(0x6AAD18), 112, 119, -1);
    
    // Обновление изображения препятствия.
    if (LargeObstackleDef_FramesCount > 1)
    {
      // Количество выделенных гексов.
      _int_ hexes_count = ((_List_<_int32_>*)0x696A08)->GetItemsCount();
      
      // Обновляем задник.
      this_->Field<_bool32_>(21432) = FALSE;
      CALL_1(void, __thiscall, 0x493870, this_);
      
      // Есть выделенные гексы.
      if (hexes_count && IntAt(0x6772D8) > 0)
      {
        // Список гексов.
        _List_<_int32_> hexes_lst;
        if (hexes_count)
        {
          hexes_lst.Data = (_int32_*)o_New(4*hexes_count);
          hexes_lst.EndData = hexes_lst.Data + hexes_count;
          hexes_lst.EndMem = (_ptr_)hexes_lst.EndData;
          MemCopy(hexes_lst.Data, ((_List_<_int32_>*)0x696A08)->Data, 4*hexes_count);
        }
        
        // Перерисовываем курсор (вместе со всем полем боя).
        CALL_4(void, __thiscall, 0x493A20, o_BattleMgr, IntAt(0x6772D8), &hexes_lst, TRUE);
      }
      
      // Перерисовка.
      this_->RedrawBattlefield(TRUE, FALSE, FALSE, 0, TRUE, FALSE);
    }
  }
  
  // Время следующей анимации ожидания.
  NextWaitAnimT = WaitAnimCurrTime + MIN_FRAME_PERIOD;
}






// Есть расширения HD, из-за которых надо перерисовывать всё поле боя (уже не надо).
_bool_ HD_TE_Exists;


// При стандартном проигрывании анимации ожидания в бою...
void __stdcall HookOn_WaitAnimDrawStd(HiHook* h, _BattleMgr_* this_)
{
  // Отрисовываем рамки вокруг стеков.
  NeedRedrawBorders = TRUE;
  
  // Начинаем случайные анимации стеков.
  NeedBeginRandomAnims = TRUE;
  
  
  // Назначаем время следующего проигрывания анимации ожидания.
  _int_ next_time = WaitAnimTime + MIN_FRAME_PERIOD;
  Battle_LastDialog_Time = 0;
  
  // Очищаем поля перерисовки битвы.
  this_->ClearRedrawFields();
  
  // Проигрыванием анимации.
  CALL_1(void, __thiscall, h->GetDefaultFunc(), this_);
  
  // Отрисовка.
  this_->RedrawBattlefield(TRUE, TRUE, TRUE, 0, TRUE, FALSE);
  
  // Восстанавливаем время следующего проигрывания анимации ожидания.
  WaitAnimTime = next_time; 
  
  
  // Более не начинаем случайные анимации стеков.
  NeedBeginRandomAnims = FALSE;
  
  // Завершаем отрисовку рамок вокруг стеков.
  NeedRedrawBorders = FALSE;
}







// Не отрисовываем невидимые стеки.
void __stdcall HookOn_BattleStack_Draw(HiHook* h, _BattleStack_* this_, _int_ X_Pos, _int_ Y_Pos, _bool_ OnlyCalcBorders)
{
  // Отриосовываем стек только если он видимый.
  if (!(*(_bool8_*)((_ptr_)this_ + 31)))
  {
    CALL_4(void, __thiscall, h->GetDefaultFunc(), this_, X_Pos, Y_Pos, OnlyCalcBorders);
  }
}







// При выборе цвета рамки вокруг стека не отрисовываем её, когда этого не требуется.
int __stdcall Hook_StackDraw_ChoseBorderColor(LoHook* h, HookContext* c)
{
  // Если отрисовывать рамки существ надо - выбираем цвет стандартно.
  if (NeedRedrawBorders)
  {
    return EXEC_DEFAULT;
  }
  // Иначе - не отрисовываем.
  else
  {
    c->return_address = 0x43DF95;
    
    return NO_EXEC_DEFAULT;
  }
}


// При выборе цвета рамки вокруг существа стрелковой башни не отрисовываем её, когда этого не требуется.
int __stdcall Hook_ArrowTower_Creature_Draw_ChoseBorderColor(LoHook* h, HookContext* c)
{
  // Если отрисовывать рамки существ надо - выбираем цвет стандартно.
  if (NeedRedrawBorders)
  {
    return EXEC_DEFAULT;
  }
  // Иначе - не отрисовываем.
  else
  {
    // Восстанавливаем затёртую команду.
    c->ecx = 0;
    
    // Пропускаем выбор цвета.
    c->return_address = 0x494CCA;
    
    return NO_EXEC_DEFAULT;
  }
}







// При подсчёте количества кадров анимации при действии отрисовки учитываем защитную стойку (недочёт SoD).
int __stdcall Hook_OnDrawActPlay_CalcFrames_DefendPos(LoHook* h, HookContext* c)
{
  // Текущий стек.
  _BattleStack_* stack = (_BattleStack_*)(c->edx - 356);
  
  // Секция анмиации повреждения стека.
  _int8_ anim_sec = stack->Field<_int8_>(3);
  
  // def стека.
  _Def_* def = (_Def_*)c->eax;
  
  // Берём количество кадров.
  if (def->groups_count > anim_sec && DwordAt(def->active_groups + 4*anim_sec))
  {
    c->eax = def->groups[anim_sec]->frames_count;
  }
  else
  {
    c->eax = 0;
  }
  
  // Пропускаем стандартное взятие.
  c->return_address = 0x468854;
  
  return NO_EXEC_DEFAULT;
}


// При проигрывании анимации при действии отрисовки учитываем защитную стойку (недочёт SoD).
int __stdcall Hook_OnDrawActPlay_Play_DefendPos(LoHook* h, HookContext* c)
{
  // В защите.
  if (c->eax == DG_DEFENSE)
  {
    // Идём туда же, куда и при повреждении.
    c->return_address = 0x468AA1;
    
    return NO_EXEC_DEFAULT;
  }
  else
  {
    return EXEC_DEFAULT;
  }
}






// При проигрывании кадра выстрела анимации при действии отрисовки пропускаем кадры стойки и кривляния.
int __stdcall Hook_OnDrawActPlay_DrawNewShotFrame(LoHook* h, HookContext* c)
{
  // Восстанавливаем затёртую команду.
  c->eax = *(DWORD*)(c->esi);
  
  // Если стек стоит или кривляется, не проигрываем его анимацию.
  if (c->eax == DG_STAY || c->eax == DG_RANDANIM)
  {
    c->return_address = 0x468A56;
    return NO_EXEC_DEFAULT;
  }
  // Иначе - анимируем.
  else
  {
    c->return_address = 0x468A1B;
    return NO_EXEC_DEFAULT;
  }
}


// При проигрывании кадра анимации при действии отрисовки пропускаем кадры стойки и кривляния.
int __stdcall Hook_OnDrawActPlay_DrawNewFrame(LoHook* h, HookContext* c)
{
  // Если стек стоит или кривляется, не проигрываем его анимацию.
  if (c->eax == DG_STAY || c->eax == DG_RANDANIM)
  {
    c->return_address = 0x468BD0;
    return NO_EXEC_DEFAULT;
  }
  // Иначе - всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}


// При проигрывании кадра доигрывающейся анимации при действии отрисовки пропускаем кадры стойки и кривляния.
int __stdcall Hook_OnDrawActPlay_LastDrawNewFrame(LoHook* h, HookContext* c)
{
  // Восстанавливаем затёртую команду.
  c->ecx = *(DWORD*)(c->eax);
  
  // Если стек стоит или кривляется, не проигрываем его анимацию.
  if (c->ecx == DG_STAY || c->ecx == DG_RANDANIM)
  {
    c->return_address = 0x468E02;
    return NO_EXEC_DEFAULT;
  }
  // Иначе - анимируем.
  else
  {
    c->return_address = 0x468DBD;
    return NO_EXEC_DEFAULT;
  }
}



// После проигрывания последнего кадра основной анмации отключаем боевую анимацию.
int __stdcall Hook_OnDrawActPlay_EndBAnim(LoHook* h, HookContext* c)
{
  // Если это последний кадр, завершаем боевую анимацию.
  if (c->eax >= c->ecx)
  {
    IntAt(c->edi + 78576) = c->eax;
  }
  
  return EXEC_DEFAULT;
}






// При ожидании определённого времени в бою прокручиваем анимацию ожидания.
void __stdcall WaitForTime_Draw(HiHook* h, _int32_ time)
{
  // Прошедшее время.
  _int_ tm = 0;
  
  // Прокручиваем циклы анимации.
  while (tm < time)
  {
    // Сбрасываем границы перерисовки.
    o_BattleMgr->ClearRedrawFields();
    
    // Прокручиваем анимацию ожидания.
    o_BattleMgr->PlayWaitAnim();
    
    // Проигрываем шаг случайной анимации.
    o_BattleMgr->RedrawBattlefield(TRUE, TRUE, TRUE, 0, TRUE, FALSE);
    
    // Ожидаем.
    CALL_1(void, __fastcall, h->GetDefaultFunc(), std::min<_int_>(MIN_FRAME_PERIOD, time - tm));
    tm += MIN_FRAME_PERIOD;
  }
}





// Перед отрисовкой проигрываем анимацию стойки и заменяем способ ожидания времени на свой (прокрутку отрисовки).
// Ожидание действует так: перед обновлением экрана ожидается период ожидания прошлой отрисовки и записывается текущий период для следующей.
void __stdcall HookOn_Battle_Draw(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait)
{
  
  // Проверка необходимости отрисовки битвы.
  if (!CanDrawBattle()) return;
  
  // Не ждать - просто рисуем.
  if (!Wait)
  {
    // Отрисовка.
    CALL_7(void, __thiscall, h->GetDefaultFunc(), this_, Flip, SetBattleRedraws, UseBattleRedraws, 0, RedrawBackground, FALSE);
  }
  // Если надо ожидать, прокручиваем вместо этого отрисовку.
  else
  {  
    // Текущее время.
    _int32_ CurrTime = o_GetTime();
    Battle_LastDialog_Time = 0;
    
    // Перовая отрисовка.
    if (CurrTime - DrawingWaitTime < 0)
    {
      CALL_7(void, __thiscall, h->GetDefaultFunc(), this_, Flip, SetBattleRedraws, UseBattleRedraws, 0, RedrawBackground, FALSE);
    }
    
    // Сохраняем границы перерисовки.
    SaveBattleRedraws(this_);
    
    while (CurrTime - DrawingWaitTime < 0)
    {
      // Сбрасываем границы перерисовки.
      this_->ClearRedrawFields();
      
      // Прокручиваем анимацию ожидания.
      this_->PlayWaitAnim();
      
      // Отрисовка.
      CALL_7(void, __thiscall, h->GetDefaultFunc(), this_, Flip, TRUE, UseBattleRedraws, 0, RedrawBackground, FALSE);
      
      // Время конца ожидания.
      CurrTime += MIN_FRAME_PERIOD;
      
      // Ждём минимальное время (или до конца, если осталось меньше).
      WaitTill((CurrTime - DrawingWaitTime < 0) ? CurrTime : DrawingWaitTime);
      
      // Текщее время.
      CurrTime = o_GetTime();
      DrawingWaitTime += Battle_LastDialog_Time;
      Battle_LastDialog_Time = 0;
    }
    
    // Восстанавливаем границы перерисовки.
    RestoreBattleRedraws(this_);
    
    // Время окончания следующей отрисовки (записываем глобально и с учётом настроек скорости битвы).
    DrawingWaitTime = o_GetTime() + (_int_)(((double)WaitingTime)*(BattleAnimPeriodFactors[Settind_BattleFast]));
  }
}





// При необходимости пропускаем отрисовку активной части поля боя.
int __stdcall Hook_OnDraw_NeedDraw_ActivePart(LoHook* h, HookContext* c)
{
  // Если нужно отрисовывать активные элементы поля боя, делаем это.
  if (NeedDraw_Active_Elemenst)
  {
    return EXEC_DEFAULT;
  }
  // Иначе - пропускаем отрисовку.
  else
  {
    c->return_address = 0x494641;
    
    return NO_EXEC_DEFAULT;
  }
}




// При отрисовке, параллельной действиям боя с отрисовкой рамок стеков.
void __stdcall HookOn_Battle_ParallelDraw_Borders(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait)
{
  // Обновляем всё поле боя.
  _bool_ BNRB = NeedRedrawBorders;
  NeedRedrawBorders = TRUE;
  CALL_7(void, __thiscall, h->GetDefaultFunc(), this_, Flip, SetBattleRedraws, UseBattleRedraws, WaitingTime, RedrawBackground, Wait);
  NeedRedrawBorders = BNRB;
}





// Перед нужной отрисовкой проигрываем анимацию ожидания.
void __stdcall HookOn_BattleDraw_WaitAnim(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait)
{
  // Проигрываем анимацию ожидания.
  this_->PlayWaitAnim();
  
  // Перерисовываем и обновляем всё поле боя.
  CALL_7(void, __thiscall, h->GetDefaultFunc(), this_, Flip, SetBattleRedraws, UseBattleRedraws, WaitingTime, RedrawBackground, Wait);
}


// Перед нужной отрисовкой проигрываем анимацию ожидания с очисткой границ перерисовки.
void __stdcall HookOn_BattleDraw_WaitAnim_ClearRedraws(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait)
{
  // Проигрываем анимацию ожидания.
  this_->ClearRedrawFields();
  this_->PlayWaitAnim();
  
  // Перерисовываем и обновляем всё поле боя.
  CALL_7(void, __thiscall, h->GetDefaultFunc(), this_, Flip, SetBattleRedraws, UseBattleRedraws, WaitingTime, RedrawBackground, Wait);
}





// Пропускаем отрисовку.
void __stdcall HookOn_BattleDraw_Skip(HiHook* h, _BattleMgr_* this_, _bool8_ Flip, _bool8_ SetBattleRedraws, _bool8_ UseBattleRedraws, _int_ WaitingTime, _bool8_ RedrawBackground, _bool8_ Wait)
{
  return;
}



// Отрисовка с обновлением.
int __stdcall HookOn_Draw_WaitAnim_Low_Redraw(LoHook* h, HookContext* c)
{
  //Отрисовка с обновлением.
  o_BattleMgr->RedrawBattlefield(TRUE, FALSE, FALSE, 0, TRUE, FALSE);
  
  return EXEC_DEFAULT;
}






// При задержке между кадрами...
void __stdcall HookOn_FramesDraw_Delay(HiHook* h, _int32_ Time)
{
  // Обновляем экран в границах перерисовки.
  o_BattleMgr->FlipRedrawRect();
  
  // Ждём не больше 10 милисекунд за раз.
  CALL_1(void, __fastcall, h->GetDefaultFunc(), t_min(o_GetTime() + MIN_FRAME_PERIOD, Time));
}








// Стрелок во время полёта выстрела.
_BattleStack_* BulletDraw_Shooter;
_int_ BulletDraw_Shooter_NextAnimTime;
_int_ BulletDraw_Shooter_Period;


// Определение первого времени следующей смены кадров снаряда.
int __stdcall HookOn_BulletDraw_InitTime(LoHook* h, HookContext* c)
{
  // Следующее время ожидания.
  *(_int32_*)(c->ebp - 72) = o_GetTime();
  Battle_LastDialog_Time = 0;
  
  // Стрелок во время полёта выстрела.
  BulletDraw_Shooter = (_BattleStack_*)c->ebx;
  if (BulletDraw_Shooter->creature_id == CID_CANNON)
  {
    _int_ cl_frame = BulletDraw_Shooter->cr_anim.Field<_int32_>(64);
    if (cl_frame < 0)
    {
      _Def_* def = BulletDraw_Shooter->def;
      _int_ dg = BulletDraw_Shooter->def_group_ix;
      if (dg < def->groups_count && DwordAt(def->active_groups + 4*dg))
      {
        cl_frame = def->groups[dg]->frames_count;
      }
      else
      {
        cl_frame = 0;
      }
    }
    BulletDraw_Shooter_Period = (_int_)((double)(BulletDraw_Shooter->cr_anim.Field<_int32_>(76) / cl_frame) * BattleAnimPeriodFactors[Settind_BattleFast]);
    BulletDraw_Shooter_NextAnimTime = *(_int32_*)(c->ebp - 72) + BulletDraw_Shooter_Period;
  }
  
  return EXEC_DEFAULT;
}


// При смене кадров отрисовки полёта снаряда...
int __stdcall HookOn_BulletDraw_FrameChange(LoHook* h, HookContext* c)
{
  
  // Проигрываем анимацию ожидания.
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  
  // Текущее время.
  _int32_ curr_time = o_GetTime() - Battle_LastDialog_Time;
  
  _int_ Battle_LastDialog_Time1 = Battle_LastDialog_Time;
  
  // Если пришло время - меняем кадр.
  if (*(_int32_*)(c->ebp - 72) - curr_time <= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(_int32_*)(c->ebp - 84) > 0)
    {
      add_frames = std::min<_int_>((curr_time - *(_int32_*)(c->ebp - 72))/(*(_int32_*)(c->ebp - 84)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Увеличиваем счётчик кадров.
    *(DWORD*)(c->ebp - 60) += add_frames;
    
    // Не переходим за максимальный кадр.
    if (*(DWORD*)(c->ebp - 60) > *(DWORD*)(c->ebp - 16))
    {
      add_frames -= *(DWORD*)(c->ebp - 60) - *(DWORD*)(c->ebp - 16);
      *(DWORD*)(c->ebp - 60) = *(DWORD*)(c->ebp - 16);
    }
    
    
    // Следующее время ожидания.
    *(_int32_*)(c->ebp - 72) += (*(_int32_*)(c->ebp - 84))*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
    
    // Изменяем координаты снаряда.
    c->esi += *(DWORD*)(c->ebp - 68)*add_frames;
    c->edi += *(DWORD*)(c->ebp - 52)*add_frames;
    *(DWORD*)(c->ebp - 56) += *(DWORD*)(c->ebp - 68)*add_frames;
    *(DWORD*)(c->ebp - 64) += *(DWORD*)(c->ebp - 52)*add_frames;
    
    
    // Дополнительные границы перерисовки (добавляем + изменившиеся тоже).
    ((_RedrawBorders_*)(c->ebp - 36))->AddRect(c->esi, c->edi, *(DWORD*)(c->ebp - 56), *(DWORD*)(c->ebp - 64));
    o_BattleMgr->AddRedrawRect((_RedrawBorders_*)(c->ebp - 36));
    
  }
  
  // Анимация стрелка: если пришло время - меняем кадр.
  if (BulletDraw_Shooter->creature_id == CID_CANNON && (BulletDraw_Shooter->def_group_ix == DG_SHOT_UP || BulletDraw_Shooter->def_group_ix == DG_SHOT_STRAIGHT || BulletDraw_Shooter->def_group_ix == DG_SHOT_DOWN))
  {
    if (BulletDraw_Shooter_NextAnimTime - curr_time <= 0)
    {
      // Количество добавляемых кадров за прошедшее время.
      _int_ add_frames;
      if (BulletDraw_Shooter_Period > 0)
      {
        add_frames = std::min<_int_>((curr_time - BulletDraw_Shooter_NextAnimTime)/BulletDraw_Shooter_Period + 1, MAX_FRAMESKIP);
      }
      else
      {
        add_frames = MAX_FRAMESKIP;
      }
      
      BulletDraw_Shooter->def_frame_ix += add_frames;
      
      _Def_* def = BulletDraw_Shooter->def;
      _int_ dg = BulletDraw_Shooter->def_group_ix;
      _int_ fr_c;
      if (dg < def->groups_count && DwordAt(def->active_groups + 4*dg))
      {
        fr_c = def->groups[dg]->frames_count;
      }
      else
      {
        fr_c = 0;
      }
      
      // Возвращение в анимацию стойки.
      if (BulletDraw_Shooter->def_frame_ix >= fr_c)
      {
        BulletDraw_Shooter->def_group_ix = DG_STAY;
        BulletDraw_Shooter->def_frame_ix = 0;
      }
      
      // Следующее время ожидания.
      BulletDraw_Shooter_NextAnimTime += BulletDraw_Shooter_Period*add_frames + Battle_LastDialog_Time1;
      Battle_LastDialog_Time = 0;
      
      // Стек будет перерисовываться.
      o_BattleMgr->Set_Stack_Redrawable(BulletDraw_Shooter);
      
    }
  }
  
  
  
  
  //Отрисовка без обновления.
  o_BattleMgr->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
  
  return EXEC_DEFAULT;
}








// Определение первого времени следующей смены кадров снаряда стрелковой башни.
int __stdcall HookOn_ArrowTower_BulletDraw_InitTime(LoHook* h, HookContext* c)
{
  // Следующее время ожидания.
  *(_int32_*)(c->ebp - 44) = o_GetTime();
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}


// При смене кадров отрисовки полёта снаряда стрелковой башни...
int __stdcall HookOn_ArrowTower_BulletDraw_FrameChange(LoHook* h, HookContext* c)
{
  
  // Проигрываем анимацию ожидания.
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  
  // Текущее время.
  _int_ curr_time = o_GetTime() - Battle_LastDialog_Time;
  
  // Если пришло время - меняем кадр.
  if (*(_int32_*)(c->ebp - 44) - curr_time <= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(_int32_*)(c->ebp - 40) > 0)
    {
      add_frames = std::min<_int_>((curr_time - *(_int32_*)(c->ebp - 44))/(*(_int32_*)(c->ebp - 40)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Увеличиваем счётчик кадров.
    *(DWORD*)(c->ebp - 16) += add_frames;
    
    // Не переходим за максимальный кадр.
    if (*(DWORD*)(c->ebp - 16) > *(DWORD*)(c->ebp - 36))
    {
      add_frames -= *(DWORD*)(c->ebp - 16) - *(DWORD*)(c->ebp - 36);
      *(DWORD*)(c->ebp - 16) = *(DWORD*)(c->ebp - 36);
    }
    
    
    // Следующее время ожидания.
    *(_int32_*)(c->ebp - 44) += (*(_int32_*)(c->ebp - 40))*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
    
    // Изменяем координаты снаряда.
    c->esi += *(DWORD*)(c->ebp - 24)*add_frames;
    c->edi += *(DWORD*)(c->ebp - 28)*add_frames;
    *(DWORD*)(c->ebp + 20) += *(DWORD*)(c->ebp - 24)*add_frames;
    *(DWORD*)(c->ebp + 24) += *(DWORD*)(c->ebp - 28)*add_frames;
    
    
    // Дополнительные границы перерисовки (добавляем + изменившиеся тоже).
    _RedrawBorders_ rect;
    rect.AddRect(c->ebx, IntAt(c->ebp - 60), IntAt(c->ebp - 56), IntAt(c->ebp - 52));
    rect.AddRect(c->esi, c->edi, *(DWORD*)(c->ebp + 20), *(DWORD*)(c->ebp + 24));
    o_BattleMgr->AddRedrawRect(&rect);
    
  }
  
  //Отрисовка без обновления.
  o_BattleMgr->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
  
  
  return EXEC_DEFAULT;
}







// Инициализация луча.
int __stdcall HookOn_RayInit(LoHook* h, HookContext* c)
{
  // Луч ещё не закончился.
  RayWasEnded = FALSE;
  
  // Изначальный максимальный кадр луча.
  RayMaxFrame = 1;
  
  // Запоминаем начальное зерно ГСЧ луча.
  RayStartSeed = RandSeed;
  
  // Сохраняем начальные значения луча.
  MemCopy(RaySavedVars, c->ebp + 12, 16);
  RaySavedVars[4] = c->edx;
  RaySavedVars[5] = c->ecx;
  RaySavedVars[6] = c->ebx;
  RaySavedVars[7] = c->edi;
  RaySavedVars[8] = c->esi;
  RaySavedVars[9] = *(DWORD*)(c->ebp + 64);
  
  
  // Инициализируем время ожидания между кадрами.
  *(_int32_*)(c->ebp + 68) = (_int32_)(((double)(*(_int32_*)(c->ebp + 68)))*(BattleAnimPeriodFactors[Settind_BattleFast]));
  
  // Инициализируем первое время проигрывания следующего кадра.
  *(_int32_*)(c->ebp + 36) = o_GetTime();
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}



// Инициализация каждой отрисовки луча.
int __stdcall HookOn_RayDrawingInit(LoHook* h, HookContext* c)
{
  // Проигрываем анимацию ожидания.
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  
  //Отрисовка без обновления.
  o_BattleMgr->RedrawBattlefield(FALSE, FALSE, FALSE, 0, TRUE, FALSE);
  
  // Возвращаем зерно ГСЧ в исходное состояние.
  RandSeed = RayStartSeed;
  
  // Восстанавливаем начальные значения луча.
  MemCopy(c->ebp + 12, RaySavedVars, 16);
  c->edx = RaySavedVars[4];
  c->ecx = RaySavedVars[5];
  c->ebx = RaySavedVars[6];
  c->edi = RaySavedVars[7];
  c->esi = RaySavedVars[8];
  *(DWORD*)(c->ebp + 64) = RaySavedVars[9];
  
  // Изначально кадр луча - нулевой.
  RayCurrFrame = 0;
  
  return EXEC_DEFAULT;
}




// При переходе к следующему кадру при отрисовке луча...
int __stdcall HookOn_RayNextFrame(LoHook* h, HookContext* c)
{
  // Если текущий кадр - не последний в отрисовке...
  if (RayCurrFrame < RayMaxFrame)
  {
    // Добавляем границы отрисовки.
    (*(_BattleMgr_**)(c->ebp - 8))->AddRedrawRect(c->ebx, IntAt(c->ebp + 20), c->esi, c->edi);
    
    // Переходим к следующему кадру.
    RayCurrFrame++;
    
    // Пропускаем расчёт времени и обновление экрана.
    c->return_address = 0x5A615F;
  }
  // Если текущая отриосвка завершилась...
  else
  {
    // Обновляем поле боя.
    (*(_BattleMgr_**)(c->ebp - 8))->AddRedrawRect(c->ebx, IntAt(c->ebp + 20), c->esi, c->edi);
    (*(_BattleMgr_**)(c->ebp - 8))->FlipRedrawRect();
    
    // Короткое ожидание.
    WaitTill(t_min(o_GetTime() + MIN_FRAME_PERIOD, *(_int32_*)(c->ebp + 36)));
    
    // Текущее время.
    _int_ CurrTime = o_GetTime() - Battle_LastDialog_Time;
    
    // Если наступило время следующего кадра не закончившегося луча, прибавляем кадр.
    if (CurrTime - *(_int32_*)(c->ebp + 36) >= 0 && !RayWasEnded)
    {
      
      // Количество добавляемых кадров за прошедшее время.
      _int_ AddFrames;
      if (*(_int32_*)(c->ebp + 68) > 0)
      {
        AddFrames = std::min<_int_>((CurrTime - *(_int32_*)(c->ebp + 36))/(*(_int32_*)(c->ebp + 68)) + 1, MAX_FRAMESKIP);
      }
      else
      {
        AddFrames = MAX_FRAMESKIP;
      }
      
      // Следующее время ожидания.
      *(_int32_*)(c->ebp + 36) += (*(_int32_*)(c->ebp + 68))*AddFrames + Battle_LastDialog_Time;
      Battle_LastDialog_Time = 0;
      
      // Увеличиваем счётчик кадров.
      RayMaxFrame += AddFrames;
    }
    
    // Идём отрисовывать луч сначала.
    c->return_address = 0x5A5FE0;
  }
  
  return NO_EXEC_DEFAULT;
}



// При окончании отрисовки луча...
int __stdcall HookOn_RayEnd(LoHook* h, HookContext* c)
{
  // Обновляем поле боя.
  (*(_BattleMgr_**)(c->ebp - 8))->AddRedrawRect(c->ebx, IntAt(c->ebp + 20), c->esi, c->edi);
  (*(_BattleMgr_**)(c->ebp - 8))->FlipRedrawRect();
  
  // Если луч заканчивается уже не в первый раз или наступило время, завершаем луч.
  if (RayWasEnded && (o_GetTime() - Battle_LastDialog_Time - *(_int32_*)(c->ebp + 36) >= 100*BattleAnimPeriodFactors[Settind_BattleFast]))
  {
    return EXEC_DEFAULT;
  }
  // Если луч ещё не заканчиался, увеличиваем число его заканчиваний.
  else if (!RayWasEnded) RayWasEnded = TRUE;
  
  // Начинаем рисовать луч сначала.
  c->return_address = 0x5A5FE0;
  
  return NO_EXEC_DEFAULT;
}







// Другой способ модификации луча, оказалось - более медленный.
/*
// Инициализация луча.
int __stdcall HookOn_RayInitSD(LoHook* h, HookContext* c)
{
  
  // Инициализация массива лучей.
  RayLns.SecCount = 0;
  RayLns.Ray = 0;
  
  // Максимальное количество секций - пока 0.
  RayMaxSec = 0;
  
  
  // Инициализируем время ожидания между кадрами.
  *(_int32_*)(c->ebp + 68) = (_int32_)(((double)(*(_int32_*)(c->ebp + 68)))*(BattleAnimPeriodFactors[Settind_BattleFast]));
  
  // Инициализируем первое время проигрывания следующего кадра.
  *(_int32_*)(c->ebp + 36) = o_GetTime() + *(_int32_*)(c->ebp + 68);
  
  return EXEC_DEFAULT;
}



// При переходе к следующей части луча сохраняем её.
int __stdcall HookOn_RayNewSecSD(LoHook* h, HookContext* c)
{
  // Добавляем новую часть луча.
  
  // Новый элемент в списке.
  _ptr_ NewList = MemAlloc(8*(RayLns.SecCount + 1));
  if (RayLns.Ray) MemCopy(NewList, RayLns.Ray, 8*(RayLns.SecCount));
  RayLns.Ray = NewList;
  
  // Копируем новую часть луча.
  *(_int_*)(RayLns.Ray + 8*(RayLns.SecCount)) = *(_int_*)(c->ebp + 12);
  *(_ptr_*)(RayLns.Ray + 8*(RayLns.SecCount) + 4) = MemAlloc(120*(*(_int_*)(c->ebp + 12)));
  MemCopy(*(_ptr_*)(RayLns.Ray + 8*(RayLns.SecCount) + 4), *(_ptr_*)(c->ebp - 4), 120*(*(_int_*)(c->ebp + 12)));
  
  // Увеличиваем количество частей луча.
  RayLns.SecCount++;
  
  // Пересчитываем максимальное количество секций в луче.
  if (RayMaxSec < *(_int_*)(c->ebp + 12)) RayMaxSec = *(_int_*)(c->ebp + 12);
  
  return EXEC_DEFAULT;
}




// При переходе к следующему кадру при отрисовке луча...
int __stdcall HookOn_RayNextFrameSD(LoHook* h, HookContext* c)
{
  
  // Обновляем поле боя.
  (*(_BattleMgr_**)(c->ebp - 8))->FlipBattlefield();
  
  
  // Если наступило время следующего кадра не закончившегося луча, прибавляем кадр.
  do
  {
    
    // Короткое ожидание.
    WaitTill(t_min(o_GetTime() + MIN_FRAME_PERIOD, *(_int32_*)(c->ebp + 36)));
    
    // Перерисовываем поле боя.
    (*(_BattleMgr_**)(c->ebp - 8))->ClearRedrawFields();
    (*(_BattleMgr_**)(c->ebp - 8))->PlayWaitAnim();
    (*(_BattleMgr_**)(c->ebp - 8))->RedrawBattlefield(FALSE, FALSE, FALSE, 0, TRUE, FALSE);
    
    
    // Текущий луч - выделяем память.
    _ptr_ CurrRay = MemAlloc(RayMaxSec*120);
    
    // Отрисовываем все части луча.
    for (_int_ i = 0; i < RayLns.SecCount; i++)
    {
      // Копируем в теущий луч.
      MemCopy(CurrRay, *(_ptr_*)(RayLns.Ray + 8*i + 4), (*(_int_*)(RayLns.Ray + 8*i))*120);
      
      // Проходим по секциям части луча.
      for (_int_ j = 0; j < *(_int_*)(RayLns.Ray + 8*i); j++)
      { 
        // Отрисовка.
        CALL_3(void, __thiscall, 0x5A5750, (*(_BattleMgr_**)(c->ebp - 8)), CurrRay + 120*j, *(_int_*)(c->ebp + 56));
      }
    }
    
    // Освобождаем память текущего луча.
    MemFree(CurrRay);
    
    
    // Обновляем экран боя.
    (*(_BattleMgr_**)(c->ebp - 8))->FlipBattlefield();
  }
  while (o_GetTime() - *(_int32_*)(c->ebp + 36) < 0);
  
  
  // Увеличиваем время.
  *(_int32_*)(c->ebp + 36) += *(_int32_*)(c->ebp + 68);
  
  // Пропускаем расчёт времени и обновление экрана.
  c->return_address = 0x5A615F;
  
  return NO_EXEC_DEFAULT;
}



// При удалении данных о луче...
int __stdcall HookOn_RayDestructSD(LoHook* h, HookContext* c)
{
  
  // Удаляем данные о частях луча.
  if (RayLns.Ray)
  {
    for (_int_ i = 0; i < RayLns.SecCount; i++)
    {
      MemFree(*(_ptr_*)(RayLns.Ray + 8*i + 4));
    }
    MemFree(RayLns.Ray);
  }
  
  return EXEC_DEFAULT;
}
*/










// Определение первого времени следующей смены кадров снаряда-заклинания.
int __stdcall HookOn_SpellBulletDraw_InitTime(LoHook* h, HookContext* c)
{
  // Следующее время ожидания.
  *(_int32_*)(c->ebp - 44) = o_GetTime();
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}


// При смене кадров отрисовки полёта снаряда-заклинания...
int __stdcall HookOn_SpellBulletDraw_FrameChange(LoHook* h, HookContext* c)
{
  
  // Проигрываем анимацию ожидания.
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  
  // Текуще время.
  _int32_ curr_time = o_GetTime() - Battle_LastDialog_Time;
  
  // Если пришло время - меняем кадр.
  if (*(_int32_*)(c->ebp - 44) - curr_time <= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(_int32_*)(c->ebp + 32) > 0)
    {
      add_frames = std::min<_int_>((curr_time - *(_int32_*)(c->ebp - 44))/(*(_int32_*)(c->ebp + 32)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Увеличиваем счётчик кадров.
    *(DWORD*)(c->ebp + 20) += add_frames;
    
    // Не переходим за максимальный кадр.
    if (*(DWORD*)(c->ebp + 20) > *(DWORD*)(c->ebp - 28))
    {
      add_frames -= *(DWORD*)(c->ebp + 20) - *(DWORD*)(c->ebp - 28);
      *(DWORD*)(c->ebp + 20) = *(DWORD*)(c->ebp - 28);
    }
    
    
    // Следующее время ожидания.
    *(_int32_*)(c->ebp - 44) += (*(_int32_*)(c->ebp + 32))*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
    
    
    // Меняем счётчик кадров снаряда.
    *(DWORD*)(c->ebp + 24) += add_frames;
    
    // Количество кадров снаряда.
    _int_ frames_count = ((((*(_Def_**)(c->ebp + 28))->groups_count > 0) && ((DWORD*)((*(_Def_**)(c->ebp + 28))->active_groups))[DG_MAIN]) ? (*(_Def_**)(c->ebp + 28))->groups[DG_MAIN]->frames_count : 0);
    
    // Зацикливаем кадры снаряда.
    if ((*(DWORD*)(c->ebp + 24)) >= frames_count)
    {
      (*(DWORD*)(c->ebp + 24)) %= frames_count;
    }
    
    
    // Изменяем координаты снаряда.
    c->esi += *(DWORD*)(c->ebp - 16)*add_frames;
    c->edi += *(DWORD*)(c->ebp - 20)*add_frames;
    *(DWORD*)(c->ebp + 12) += *(DWORD*)(c->ebp - 16)*add_frames;
    *(DWORD*)(c->ebp + 8) += *(DWORD*)(c->ebp - 20)*add_frames;
    
    
    // Дополнительные границы перерисовки (добавляем + изменившиеся тоже).
    ((_RedrawBorders_*)(c->ebp - 60))->AddRect(c->esi, c->edi, *(DWORD*)(c->ebp + 12), *(DWORD*)(c->ebp + 8));
    o_BattleMgr->AddRedrawRect((_RedrawBorders_*)(c->ebp - 60));
  }
  
  
  //Отрисовка без обновления.
  o_BattleMgr->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
  
  
  return EXEC_DEFAULT;
}







// Определение первого времени следующей смены кадров баллистического снаряда.
int __stdcall HookOn_BallisticBulletDraw_InitTime(LoHook* h, HookContext* c)
{
  // Следующее время ожидания.
  *(_int32_*)(c->ebp - 52) = o_GetTime();
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}


// При смене кадров отрисовки полёта баллистического снаряда...
int __stdcall HookOn_BallisticBulletDraw_FrameChange(LoHook* h, HookContext* c)
{
  
  // Проигрываем анимацию ожидания.
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  
  // Текуще время.
  _int32_ curr_time = o_GetTime() - Battle_LastDialog_Time;
  
  // Если пришло время - меняем кадр.
  if (*(_int32_*)(c->ebp - 52) - curr_time <= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(_int32_*)(c->ebp - 68) > 0)
    {
      add_frames = std::min<_int_>((curr_time - *(_int32_*)(c->ebp - 52))/(*(_int32_*)(c->ebp - 68)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Увеличиваем счётчик кадров.
    *(DWORD*)(c->ebp - 36) += add_frames;
    
    // Не переходим за максимальный кадр.
    if (*(DWORD*)(c->ebp - 36) > *(DWORD*)(c->ebp - 32))
    {
      add_frames -= *(DWORD*)(c->ebp - 36) - *(DWORD*)(c->ebp - 32);
      *(DWORD*)(c->ebp - 36) = *(DWORD*)(c->ebp - 32);
    }
    
    // Следующее время ожидания.
    *(_int32_*)(c->ebp - 52) += (*(_int32_*)(c->ebp - 68))*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
    
    
    // Меняем скорость снаряда.
    *(DWORD*)(c->ebp - 40) -= add_frames;
    
    // Меняем x-определитель координаты снаряда.
    *(DWORD*)(c->ebp - 44) += *(DWORD*)(c->ebp - 64)*add_frames;
    
    
    // Меняем счётчик кадров снаряда.
    *(DWORD*)(c->ebp + 16) += add_frames;
    
    // Количество кадров снаряда.
    _int_ frames_count = ((((*(_Def_**)(c->ebp + 24))->groups_count > 0) && ((DWORD*)((*(_Def_**)(c->ebp + 24))->active_groups))[DG_MAIN]) ? (*(_Def_**)(c->ebp + 24))->groups[DG_MAIN]->frames_count : 0);
    
    // Зацикливаем кадры снаряда.
    if ((*(DWORD*)(c->ebp + 16)) >= frames_count)
    {
      (*(DWORD*)(c->ebp + 16)) %= frames_count;
    }
    
    
    
    // Изменяем координаты снаряда.
    c->esi = *(_int_*)(c->ebp + 8) + (*(_int_*)(c->ebp - 44))/(*(_int_*)(c->ebp - 32));
    c->edi = (_int_)((double)(*(_int_*)(c->ebp + 12)) + ((double)(*(_int_*)(c->ebp - 72)) - (double)(*(_int_*)(c->ebp - 40))*(*(double*)(c->ebp - 88)))*((double)(*(_int_*)(c->ebp - 36)))/(*(double*)(c->ebp - 80)));
    
    
    
    // Дополнительные границы перерисовки (добавляем + изменившиеся тоже).
    ((_RedrawBorders_*)(c->ebp - 28))->AddRect(c->esi, c->edi, c->esi + c->ebx - 1, c->edi + *(DWORD*)(c->ebp + 20) - 1);
    o_BattleMgr->AddRedrawRect((_RedrawBorders_*)(c->ebp - 28));
    
  }
  
  
  //Отрисовка без обновления.
  o_BattleMgr->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
  
  return EXEC_DEFAULT;
}





// Границы перерисовки взрыва баллистического снаряда.
_RedrawBorders_  BallisticExplBrd;


// Инициализация времени взрыва баллистического выстрела.
int __stdcall HookOn_BallisticExplDraw_InitTime(LoHook* h, HookContext* c)
{
  
  // Границы перерисовки баллистического снаряда.
  _RedrawBorders_* brd = o_BattleMgr->PField<_RedrawBorders_>(81208);
  BallisticExplBrd.Left = brd->Left;
  BallisticExplBrd.High = brd->High;
  BallisticExplBrd.Right = brd->Right;
  BallisticExplBrd.Low = brd->Low;
  
  // Очищаем поля перерисовки.
  o_BattleMgr->ClearRedrawFields();
  
  // Время между кадрами.
  BallisticExplFrameTime = Round(STD_FRAME_PERIOD*BattleAnimPeriodFactors[Settind_BattleFast]);
  
  // Начальное время следующей смены кадров.
  BallisticExplNextTime = o_GetTime() + BallisticExplFrameTime;
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}


// При отрисовке взрыва баллистического выстрела.
int __stdcall HookOn_BallisticExplDraw(LoHook* h, HookContext* c)
{
  // Обновляем поле боя.
  o_BattleMgr->FlipRedrawRect();
  
  // Текущее время.
  _int_ curr_time = o_GetTime();
  
  // Ожидаем.
  WaitTill(t_min(curr_time + MIN_FRAME_PERIOD, BallisticExplNextTime));
  curr_time += MIN_FRAME_PERIOD - Battle_LastDialog_Time;
  
  // Очищаем поля перерисовки.
  o_BattleMgr->ClearRedrawFields();
  
  // Если пришло время, увеличиваем счётчик кадров.
  if (curr_time - BallisticExplNextTime >= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (BallisticExplFrameTime > 0)
    {
      add_frames = std::min<_int_>((curr_time - BallisticExplNextTime)/BallisticExplFrameTime + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Не пропускаем 5 кадров, чтобы разрушить стену.
    if (*(_int_*)(c->ebp - 8) < 5 && *(_int_*)(c->ebp - 8) + add_frames > 5)
    {
      add_frames = 5 - *(_int_*)(c->ebp - 8);
      *(_int_*)(c->ebp - 8) = 5;
    }
    // Следующий кадр.
    else
    {
      *(_int_*)(c->ebp - 8) += add_frames;
    }
    
    
    // Количество кадров взрыва.
    _int_ frames_count = (((((_Def_*)(c->esi))->groups_count > 0) && ((DWORD*)(((_Def_*)(c->esi))->active_groups))[DG_MAIN]) ? ((_Def_*)(c->esi))->groups[DG_MAIN]->frames_count : 0);
    
    // Игнорируем перескок за конец проигрывания.
    if (*(_int_*)(c->ebp - 8) > frames_count)
    {
      add_frames -= *(_int_*)(c->ebp - 8) - frames_count;
      *(_int_*)(c->ebp - 8) = frames_count;
    }
    
    // Добавляем изображение взрыва в границы перерисовки.
    o_BattleMgr->AddRedrawRect(&BallisticExplBrd);
    
    // Время следующего проигрывания.
    BallisticExplNextTime += BallisticExplFrameTime*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
  }
  
  // Пропускаем собственное обновление экрана
  c->return_address = 0x4461AA;
  
  return NO_EXEC_DEFAULT;
}



// При отрисовке def`а взрыва баллистического выстрела.
void __stdcall HiHook_BallisticExpl_BlitDef(HiHook* h, _Def_* this_, _int32_ group_ix, _int32_ frame_ix, _int32_ def_x, _int32_ def_y, _int32_ img_width, _int32_ img_height, _ptr_ pcx_buffer,
                      _int32_ dest_x, _int32_ dest_y, _int32_ dest_width, _int32_ dest_height,
                      _dword_ scanline_size, _bool32_ reflected, _bool8_ use_spec_colors)
{
  // Заменяем границы взятия изображения def`а.
  CALL_15(void, __thiscall, h->GetDefaultFunc(), this_, group_ix, frame_ix, def_x, def_y,
                      BallisticExplBrd.Right - BallisticExplBrd.Left + 1,
                      BallisticExplBrd.Low - BallisticExplBrd.High + 1, pcx_buffer,
                      dest_x, dest_y, dest_width, dest_height,
                      scanline_size, reflected, use_spec_colors);
}





// Добавление зависимости максимальной скорости от настроек скорости боя.
int __stdcall HookOn_SmoothImageChangeDraw_MinFrameTime(LoHook* h, HookContext* c)
{
  // Если сейчас битва - учитываем её скорость.
  if (CanDrawBattle())
  {
    *(_int_*)(c->ebp + 24) = Round(33.0*BattleAnimPeriodFactors[Settind_BattleFast]);
  }
  // Иначе - стандартное значение.
  else 
  {
    *(_int_*)(c->ebp + 24) = 33;
  }
  
  return NO_EXEC_DEFAULT;
}




// Определение первого времени следующей смены кадров при плавном изменении изображения.
int __stdcall HookOn_SmoothImageChangeDraw_InitTime(LoHook* h, HookContext* c)
{
  // Уменьшаем время одного кадра в 4 раза, чтобы сделать 32 кадра, вместо 8.
  *(_int32_*)(c->ebp + 24) /= 4;
  
  // Следующее время ожидания.
  *(_int32_*)(c->ebp - 64) = o_GetTime() + *(_int32_*)(c->ebp + 24);
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}



// При смене кадров при плавном изменении изображения...
int __stdcall HookOn_SmoothImageChangeDraw_FrameChange(LoHook* h, HookContext* c)
{
  
  // Проигрываем анимацию.
  if (SmoothAnimSpec_Anim) SmoothAnimSpec_Anim();
  
  
  // Если определены обе изменяющие функции, перерисовываем старое и новое изображения
  if (SmoothAnimSpec_Draw && SmoothAnimSpec_Redo && SmoothAnimSpec_Undo)
  {
    // Изображение экрана.
    _Pcx16_* ScrPcx = ((_WndMgr_*)(c->esi))->screen_pcx16;
    
    // Сохраняем границы отображения.
    _RedrawBorders_* brd = o_BattleMgr->PField<_RedrawBorders_>(81208);
    _RedrawBorders_ brd_saved;
    brd_saved.AddRect(brd);
    
    // Всегда перерисовываем изменяющуюся область.
    if (brd->Left > *(_int_*)(c->ebp + 8)) brd->Left = *(_int_*)(c->ebp + 8);
    if (brd->High > c->edi) brd->High = c->edi;
    if (brd->Right < *(_int_*)(c->ebp + 8) + *(_int_*)(c->ebp + 16) - 1) brd->Right = *(_int_*)(c->ebp + 8) + *(_int_*)(c->ebp + 16) - 1;
    if (brd->Low < c->edi + *(_int_*)(c->ebp + 20) - 1) brd->Low = c->edi + *(_int_*)(c->ebp + 20) - 1;
    
    
    
    // Отрисовка без обновления для нового изображения.
    SmoothAnimSpec_Draw();
    
    // Копируем в новое изображение.
    ((_Pcx16_*)(c->ebp - 140))->CopySurface16(ScrPcx->buffer, *(_int_*)(c->ebp + 8), c->edi,
                                              ScrPcx->width, ScrPcx->height, ScrPcx->scanline_size);
    
    
    
    // Возвращаем всё как было в старом изображении.
    SmoothAnimSpec_Undo();
    
    
    // Отрисовка без обновления для старого изображения (+ отрисовка прочей части экрана).
    SmoothAnimSpec_Draw();
    
    // Копируем в старое изображение.
    ((_WndMgr_*)(c->esi))->backup_screen_pcx16->CopySurface16(ScrPcx->buffer, *(_int_*)(c->ebp + 8), c->edi,
                                                              ScrPcx->width, ScrPcx->height, ScrPcx->scanline_size);
    
    
    // Возвращаем всё как должно быть в новом изображении.
    SmoothAnimSpec_Redo();
    
    
    // Восстанавливаем границы перерисовки.
    MemCopy(brd, &brd_saved, sizeof(brd_saved));
    
    
  }
  
  // Текущее время.
  _int_ CurrTime = o_GetTime() - Battle_LastDialog_Time;
  
  // Если пришло время - меняем кадр.
  if (*(_int32_*)(c->ebp - 64) - CurrTime <= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(DWORD*)(c->ebp + 24) > 0)
    {
      add_frames = std::min<_int_>((CurrTime - *(_int32_*)(c->ebp - 64))/(*(_int32_*)(c->ebp + 24)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Увеличиваем счётчик кадров.
    *(DWORD*)(c->ebp - 20) += add_frames;
    
    // Игнорируем перескок за конец проигрывания.
    if (*(DWORD*)(c->ebp - 20) > 32)
    {
      add_frames -= *(DWORD*)(c->ebp - 20) - 32;
      *(DWORD*)(c->ebp - 20) = 32;
    }
    
    // Следующее время ожидания.
    *(_int32_*)(c->ebp - 64) += (*(_int32_*)(c->ebp + 24))*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
  }
  
  return EXEC_DEFAULT;
}



// При задержке между кадрами при плавном изменении изображения...
void __stdcall HookOn_SmoothImageChangeDraw_FramesDraw_Delay(HiHook* h, _int32_ Time)
{
  // Обновляем экран боя.
  if (SmoothAnimSpec_Flip) SmoothAnimSpec_Flip();
  
  // Ждём не больше 20 милисекунд за раз.
  CALL_1(void, __fastcall, h->GetDefaultFunc(), t_min(o_GetTime() + MIN_FRAME_PERIOD, Time));
}











// При анимации исчезновения трупов.
void __stdcall HiHook_RemoveDeadDraw(HiHook* h, _BattleMgr_* this_)
{
  
  // Считаем удаляемые стеки ещё живыми.
  for (_int_ Side = ATTACKER; Side <= DEFENDER; Side++)
  {
    for (_int_ StackNum = 0; StackNum < BATTLE_SIDE_STACKS_COUNT; StackNum++)
    {
      if (this_->PField<_bool8_>(78904)[20*Side + StackNum])
      {
        // Считаем стек живым.
        this_->stack[Side][StackNum].creature.flags &= ~(1 << 21);
      }
    }
  }
  
  // Если битва должна быть отрисована, играем анимацию.
  if (CanDrawBattle())
  {
    // Проигрываем исчезновение трупов.
    PlaySmoothAnim(SmoothAnimSpec_Battle_Draw, SmoothAnimSpec_RemoveDead_Redo, SmoothAnimSpec_RemoveDead_Undo,
                   SmoothAnimSpec_Battle_Anim, SmoothAnimSpec_Battle_Flip,
                   BattleRedraw_Borders.Left + HD_Battle_X, BattleRedraw_Borders.High + HD_Battle_Y,
                   BattleRedraw_Borders.Right - BattleRedraw_Borders.Left + 1,
                   BattleRedraw_Borders.Low - BattleRedraw_Borders.High + 1,
                   Round(150.0*BattleAnimPeriodFactors[Settind_BattleFast]));
  }
  
  // Удаляем все стеки, которые нужно, с гексов.
  for (_int_ Side = ATTACKER; Side <= DEFENDER; Side++)
  {
    for (_int_ StackNum = 0; StackNum < BATTLE_SIDE_STACKS_COUNT; StackNum++)
    {
      if (this_->PField<_bool8_>(78904)[20*Side + StackNum])
      {
        // Стек мёртв.
        this_->stack[Side][StackNum].creature.flags |= (1 << 21);
        this_->RemoveStackFromGexes(&this_->stack[Side][StackNum]);
        // Восстанавливаем видимость стека.
        this_->stack[Side][StackNum].Field<_bool8_>(31) = FALSE;
      }
    }
  }
}







// При отрисовке вызова стека.
int __stdcall HookOn_SummonDraw(LoHook* h, HookContext* c)
{
  
  // Запоминаем стек.
  SmoothAnimSpec_Summon_Stack = (_BattleStack_*)(c->ebx);
  
  // Делаем стек невидимым.
  *(_bool8_*)((_ptr_)SmoothAnimSpec_Summon_Stack + 31) = TRUE;
  
  // Проигрываем вызов (обновляем весь экран).
  PlaySmoothAnim(SmoothAnimSpec_Battle_Draw, SmoothAnimSpec_Summon_Redo, SmoothAnimSpec_Summon_Undo,
                 SmoothAnimSpec_Battle_Anim, SmoothAnimSpec_Battle_Flip,
                 BattleRedraw_Borders.Left + HD_Battle_X, BattleRedraw_Borders.High + HD_Battle_Y,
                 BattleRedraw_Borders.Right - BattleRedraw_Borders.Left + 1,
                 BattleRedraw_Borders.Low - BattleRedraw_Borders.High + 1,
                 Round(75.0*BattleAnimPeriodFactors[Settind_BattleFast]));
  
  
  // Делаем стек видимым.
  *(_bool8_*)((_ptr_)SmoothAnimSpec_Summon_Stack + 31) = FALSE;
  
  // Пропускаем стандартную анимацию.
  c->return_address = 0x479BF2;
  
  return NO_EXEC_DEFAULT;
}








// При отрисовке телепорта.
int __stdcall HookOn_TeleportDraw(LoHook* h, HookContext* c)
{
  // Выполняем затёртую команду.
  *(_int_*)(c->ebp + 12) = *(_int_*)(c->ebp + 20);
  
  // Запоминаем стек.
  SmoothAnimSpec_Teleport_Stack = (_BattleStack_*)(c->edi);
  // Запоминаем номер начального гекса.
  SmoothAnimSpec_Teleport_StartGexNum = SmoothAnimSpec_Teleport_Stack->hex_ix;
  // Запоминаем номер целевого гекса.
  SmoothAnimSpec_Teleport_TargetGexNum = *(_int_*)(c->ebp + 12);
  
  // Проигрываем телепортацию (координаты не важны, т. к. в битве автоматически заменяются).
  PlaySmoothAnim(SmoothAnimSpec_Battle_Draw, SmoothAnimSpec_Teleport_Redo, SmoothAnimSpec_Teleport_Undo,
                 SmoothAnimSpec_Battle_Anim, SmoothAnimSpec_Battle_Flip,
                 BattleRedraw_Borders.Left + HD_Battle_X, BattleRedraw_Borders.High + HD_Battle_Y,
                 BattleRedraw_Borders.Right - BattleRedraw_Borders.Left + 1,
                 BattleRedraw_Borders.Low - BattleRedraw_Borders.High + 1, -1);
  
  
  // Сбрасываем номер начального гекса.
  SmoothAnimSpec_Teleport_StartGexNum = NO_GEX;
  // Сбрасываем номер целевого гекса.
  SmoothAnimSpec_Teleport_TargetGexNum = NO_GEX;
  
  // Пропускаем сохранение начального экрана.
  c->return_address = 0x5A1E8F;
  
  return NO_EXEC_DEFAULT;
}



// При отрисовке прямоурольника, отображающего количество существ.
int __stdcall HookOn_DrawStackRectShowingCount(LoHook* h, HookContext* c)
{
  // Если гекс перед существом участвует в телепортации, смещаем прямоугольник с количеством сушеств.
  if (SmoothAnimSpec_Teleport_StartGexNum != NO_GEX && (c->esi / 112) == SmoothAnimSpec_Teleport_StartGexNum
      || SmoothAnimSpec_Teleport_TargetGexNum != NO_GEX && (c->esi / 112) == SmoothAnimSpec_Teleport_TargetGexNum)
  {
    c->return_address = 0x43E353;
    
    return NO_EXEC_DEFAULT;
  }
  // Иначе - не смещаем.
  else
  {
    return EXEC_DEFAULT;
  }
}










// Подготавливаем плавное исчезновение объекта.
int __stdcall HookOn_RemoveObstacleDraw_Prepare(LoHook* h, HookContext* c)
{
  // Запоминаем номер препятствия.
  SmoothAnimSpec_RemoveObstacle_ObstacleNum = c->esi;
  
  // Устанавливаем границы перерисовки препятствия.
  ((_BattleMgr_*)c->ebx)->Field<_bool8_>(81196) = TRUE;
  ((_BattleMgr_*)c->ebx)->Field<_bool32_>(81204) = TRUE;
  CALL_2(void, __thiscall, 0x4952B0, c->ebx, &(((_BattleMgr_*)c->ebx)->hex[((_Struct_*)(((_BattleMgr_*)c->ebx)->Field<_ptr_>(81244) + 24*SmoothAnimSpec_RemoveObstacle_ObstacleNum))->Field<_byte_>(8)]));
  ((_BattleMgr_*)c->ebx)->Field<_bool32_>(81204) = FALSE;
  ((_BattleMgr_*)c->ebx)->Field<_bool8_>(81196) = FALSE;
  
  
  // Пропускаем собственные подготовку границ отрисовки, сохранение старого экрана и удаление препятствия.
  c->return_address = 0x5A1FDB;
  
  return NO_EXEC_DEFAULT;
}




// При плавном исчезновеним объекта.
void __stdcall HookOn_RemoveObstacleDraw(HiHook* h, _WndMgr_* this_, _int_ X_Pos, _int_ Y_Pos, _int_ Width, _int_ Height, _int_ FrameTime)
{
  
  // Проигрываем плавное исчезновение объекта.
  PlaySmoothAnim(SmoothAnimSpec_Battle_Draw, SmoothAnimSpec_RemoveObstacle_Redo, SmoothAnimSpec_RemoveObstacle_Undo,
                 SmoothAnimSpec_Battle_Anim, SmoothAnimSpec_Battle_Flip, X_Pos + HD_Battle_X, Y_Pos + HD_Battle_Y, Width, Height, FrameTime);
  
  // Удаляем удаляемое препятствие.
  o_BattleMgr->RemoveObstacle(SmoothAnimSpec_RemoveObstacle_ObstacleNum);
}





// Делаем возможной невидимость препятсвия (1).
int __stdcall HookOn_RemoveObstacleDraw_Invisible1(LoHook* h, HookContext* c)
{
  // Препятствие.
  _BattleObstackle_* obst = &o_BattleMgr->Field<_BattleObstackle_*>(81244)[c->eax];
  
  // Если текущее препятсвие исчезающее и отрисовывать его не надо - пропускаем отрисовку.
  if (!SmoothAnimSpec_RemoveObstacle_ObstacleVisible && SmoothAnimSpec_RemoveObstacle_ObstacleNum == c->eax
      || obst->side < 0 && !obst->visible && !o_BattleMgr->Field<_bool8_>(81269)) // Или ничьё и невидимо
  {
    // Пропускаем отрисовку.
    c->return_address = 0x494212;
    
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}


// Делаем возможной невидимость препятсвия (2).
int __stdcall HookOn_RemoveObstacleDraw_Invisible2(LoHook* h, HookContext* c)
{
  // Препятствие.
  _BattleObstackle_* obst = &o_BattleMgr->Field<_BattleObstackle_*>(81244)[c->eax];
  
  // Если текущее препятсвие исчезающее и отрисовывать его не надо - пропускаем отрисовку.
  if (!SmoothAnimSpec_RemoveObstacle_ObstacleVisible && SmoothAnimSpec_RemoveObstacle_ObstacleNum == c->eax
      || obst->side < 0 && !obst->visible && !o_BattleMgr->Field<_bool8_>(81269)) // Или ничьё и невидимо
  {
    // Пропускаем отрисовку.
    c->return_address = 0x494537;
    
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}







// Инициализация отрисовки движения.
void __stdcall HiHook_StackMoveDraw_Init(HiHook* h, _BattleStack_* this_, _int_ def_group, _int_ frames_count, _int_ start_frame)
{
  // Обнуляем задержку последних диалогов.
  Battle_LastDialog_Time = 0;
  
  // Текущие секция и кадр анимации.
  _int_ def_group_ix;
  _int_ def_frame_ix;
  
  if (!o_BattleMgr->ShouldNotRenderBattle())
  {
    // Добавляем изображение стека в границы перерисовки.
    o_BattleMgr->Set_Stack_Redrawable(this_);
    o_BattleMgr->SetRedrawBorders();
    
    // Сохраняем текущую секцию и кадр анимации.
    def_group_ix = this_->def_group_ix;
    def_frame_ix = this_->def_frame_ix;
  }
  
  // Вызываем отрисовку.
  CALL_4(void, __thiscall, h->GetDefaultFunc(), this_, def_group, frames_count, start_frame);
  
  if (!o_BattleMgr->ShouldNotRenderBattle())
  {
    // Нет новой группы - восстанавливаем секцию и кадр анимации.
    if (def_group >= this_->def->groups_count || !DwordAt(this_->def->active_groups + 4*def_group)
        || this_->def->groups[def_group]->frames_count <= 0)
    {
      this_->def_group_ix = def_group_ix;
      this_->def_frame_ix = def_frame_ix;
    }
  }
  
}






// При смене кадров отрисовки движения стека...
int __stdcall HookOn_StackMoveDraw_FrameChange(LoHook* h, HookContext* c)
{
  
  // Сохраняем границы перерисовки стека.
  _RedrawBorders_ brd;
  MemCopy(&brd, o_BattleMgr->PField<_RedrawBorders_>(81208), sizeof(brd));
  
  // Проигрываем анимацию ожидания.
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  
  // Добавляем движущийся стек в границы перерисовки.
  o_BattleMgr->AddRedrawRect(&brd);
  
  // Текущее время.
  _int_ curr_time = o_GetTime() - Battle_LastDialog_Time;
  
  // Если пришло время - меняем кадр.
  if (DrawingWaitTime - curr_time <= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(_int_*)(c->ebp + 16) > 0)
    {
      add_frames = std::min<_int_>((curr_time - DrawingWaitTime)/(*(_int_*)(c->ebp + 16)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    //Отрисовка без обновления.
    o_BattleMgr->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
    
    
    
    // Увеличиваем счётчик кадров.
    (*(_BattleStack_**)(c->ebp - 4))->def_frame_ix += add_frames;
    
    
    // Игнорируем перескок за конец проигрывания.
    if ((*(_BattleStack_**)(c->ebp - 4))->def_frame_ix > *(_int_*)(c->ebp - 8))
    {
      add_frames -= (*(_BattleStack_**)(c->ebp - 4))->def_frame_ix - *(_int_*)(c->ebp - 8);
      (*(_BattleStack_**)(c->ebp - 4))->def_frame_ix = *(_int_*)(c->ebp - 8);
    }
    
    // Следующее время ожидания.
    DrawingWaitTime += (*(_int_*)(c->ebp + 16))*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
  }
  else
  {
    //Отрисовка без обновления.
    o_BattleMgr->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
  }
  
  
  
  return EXEC_DEFAULT;
}









// При инициализации координат отрисовки полёта стека...
int __stdcall HookOn_StackFlightDraw_InitCoords(LoHook* h, HookContext* c)
{
  
  // Изменяем координаты изображния - пропускаемоеое первое смещение.
  *(_float_*)(c->ebp - 20) += (*(_float_*)(c->ebp - 36))/(*(_float_*)(c->ebp - 12));
  *(_float_*)(c->ebp - 16) += (*(_float_*)(c->ebp - 40))/(*(_float_*)(c->ebp - 12));
  
  
  // Перерисовываем область.
  o_BattleMgr->RedrawBattlefield(FALSE, FALSE, TRUE, 0, TRUE, FALSE);
  
  
  // Обнуляем задержку последних диалогов.
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}



// При задержке между кадрами при полёте...
int __stdcall HookOn_FlightFramesDraw_Delay(LoHook* h, HookContext* c)
{
  // Обновляем экран в границах перерисовки.
  o_WndMgr->RedrawScreenRect(HD_Battle_X + IntAt(c->ebp - 68), HD_Battle_Y + IntAt(c->ebp - 64), c->ebx - IntAt(c->ebp - 68) + 1, c->edi - IntAt(c->ebp - 64) + 1);
  
  // Ждём не больше 20 милисекунд за раз.
  CALL_1(void, __fastcall, 0x4F8980, t_min(o_GetTime() + MIN_FRAME_PERIOD, DrawingWaitTime));
  
  // Пропускаем стандартное ожидание.
  c->return_address = 0x4B4B1A;
  
  return NO_EXEC_DEFAULT;
}



// При смене кадров отрисовки полёта стека...
int __stdcall HookOn_StackFlightDraw_FrameChange(LoHook* h, HookContext* c)
{
  
  // Сохраняем границы перерисовки стека.
  _RedrawBorders_ brd;
  MemCopy(&brd, o_BattleMgr->PField<_RedrawBorders_>(81208), sizeof(brd));
  
  
  // Проигрываем анимацию ожидания.
  o_BattleMgr->ClearRedrawFields();
  o_BattleMgr->PlayWaitAnim();
  
  
  // Добавляем движущийся стек в границы перерисовки.
  o_BattleMgr->AddRedrawRect(&brd);
  
  
  // Текущее время.
  _int_ curr_time = o_GetTime() - Battle_LastDialog_Time;
  
  // Если пришло время - меняем кадр.
  if (DrawingWaitTime - curr_time <= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(_int_*)(c->ebp - 48) > 0)
    {
      add_frames = std::min<_int_>((curr_time - DrawingWaitTime)/(*(_int_*)(c->ebp - 48)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Увеличиваем счётчик кадров.
    ((_BattleStack_*)c->esi)->def_frame_ix += add_frames;
    
    // Игнорируем перескок за конец проигрывания.
    if (((_BattleStack_*)c->esi)->def_frame_ix > *(_int_*)(c->ebp - 8))
    {
      add_frames -= ((_BattleStack_*)c->esi)->def_frame_ix - *(_int_*)(c->ebp - 8);
      ((_BattleStack_*)c->esi)->def_frame_ix = *(_int_*)(c->ebp - 8);
    }
    
    
    // Следующее время ожидания.
    DrawingWaitTime += (*(_int_*)(c->ebp - 48))*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
    
    // Изменяем координаты изображния.
    *(_float_*)(c->ebp - 20) += (*(_float_*)(c->ebp - 36))/(*(_float_*)(c->ebp - 12))*add_frames;
    *(_float_*)(c->ebp - 16) += (*(_float_*)(c->ebp - 40))/(*(_float_*)(c->ebp - 12))*add_frames;
    
    
     
    // Добавляем будущее изображение стека в границы перерисовки.
    o_BattleMgr->Field<_bool8_>(81196) = TRUE;
    o_BattleMgr->Field<_bool32_>(81204) = TRUE;
    if (((_BattleStack_*)c->esi)->def_frame_ix == *(_int_*)(c->ebp - 8)) 
    {
      ((_BattleStack_*)c->esi)->def_frame_ix = 0;
      CALL_4(void, __thiscall, 0x43DE60, c->esi, *(_float_*)(c->ebp - 20), *(_float_*)(c->ebp - 16), FALSE);
      ((_BattleStack_*)c->esi)->def_frame_ix = *(_int_*)(c->ebp - 8);
    }
    else
    {
      CALL_4(void, __thiscall, 0x43DE60, c->esi, *(_float_*)(c->ebp - 20), *(_float_*)(c->ebp - 16), FALSE);
    }
    o_BattleMgr->Field<_bool32_>(81204) = FALSE;
    o_BattleMgr->Field<_bool8_>(81196) = FALSE;
  }
  
    
  //Отрисовка без обновления.
  o_BattleMgr->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
  
  return EXEC_DEFAULT;
}











// При сбросе анимации стека при открытии моста при движении...
int __stdcall HookOn_StackAnim_MoveBridgeOpenReset(LoHook* h, HookContext* c)
{
  // Стек.
  _BattleStack_* Stack = (_BattleStack_*)(c->esi);
  
  // Если стек не стоит, не кривляется и может анимировать, сбрасываем его анимацию.
  if (Stack->def_group_ix != DG_STAY && Stack->def_group_ix != DG_RANDANIM && CanStackAnim(Stack))
  {
    return EXEC_DEFAULT;
  }
  else
  {
    c->return_address = 0x441F3B;
    
    return NO_EXEC_DEFAULT;
  }
}



// При сбросе анимации стека после заклинания...
int __stdcall HookOn_StackAnim_AfterSpellReset(LoHook* h, HookContext* c)
{
  // Стек.
  _BattleStack_* Stack = (_BattleStack_*)(c->edi);
  
  // Если стек не стоит, не кривляется и может анимировать, сбрасываем его анимацию.
  if (Stack->def_group_ix != DG_STAY && Stack->def_group_ix != DG_RANDANIM && CanStackAnim(Stack))
  {
    return EXEC_DEFAULT;
  }
  else
  {
    c->return_address = 0x47746F;
    
    return NO_EXEC_DEFAULT;
  }
}




// При сбросе анимации стека перед действием...
int __stdcall HookOn_StackAnim_BeforeActionReset(LoHook* h, HookContext* c)
{
  // Стек.
  _BattleStack_* Stack = (_BattleStack_*)(c->edi);
  
  // Если стек не стоит, не кривляется и может анимировать, сбрасываем его анимацию.
  if (Stack->def_group_ix != DG_STAY && Stack->def_group_ix != DG_RANDANIM && CanStackAnim(Stack))
  {
    return EXEC_DEFAULT;
  }
  else
  {
    c->return_address = 0x477CD3;
    
    return NO_EXEC_DEFAULT;
  }
}




// При сбросе анимации стека после действия...
int __stdcall HookOn_StackAnim_AfterActionReset(LoHook* h, HookContext* c)
{
  // Стек.
  _BattleStack_* Stack = (_BattleStack_*)(c->ebx);
  
  // Если стек не стоит, не кривляется и может анимировать, сбрасываем его анимацию.
  if (Stack->def_group_ix != DG_STAY && Stack->def_group_ix != DG_RANDANIM && CanStackAnim(Stack))
  {
    return EXEC_DEFAULT;
  }
  else
  {
    c->return_address = 0x479635;
    
    return NO_EXEC_DEFAULT;
  }
}





// При сбросе анимации стека при открытии моста перед полётом...
int __stdcall HookOn_StackAnim_BeforeFlightBridgeOpenReset(LoHook* h, HookContext* c)
{
  // Стек.
  _BattleStack_* Stack = (_BattleStack_*)(c->esi);
  
  // Если стек не стоит, не кривляется и может анимировать, сбрасываем его анимацию.
  if (Stack->def_group_ix != DG_STAY && Stack->def_group_ix != DG_RANDANIM && CanStackAnim(Stack))
  {
    return EXEC_DEFAULT;
  }
  else
  {
    c->return_address = 0x4B480B;
    
    return NO_EXEC_DEFAULT;
  }
}



// При общем сбросе анимации стеков...
int __stdcall HookOn_StackAnimReset(LoHook* h, HookContext* c)
{
  // Стек.
  _BattleStack_* Stack = (_BattleStack_*)(c->esi - 60);
  
  // Если стек не стоит, не кривляется и может анимировать, сбрасываем его анимацию.
  if (Stack->def_group_ix != DG_STAY && Stack->def_group_ix != DG_RANDANIM && CanStackAnim(Stack))
  {
    return EXEC_DEFAULT;
  }
  else
  {
    c->return_address = 0x479803;
    
    return NO_EXEC_DEFAULT;
  }
}






// Заменяем способ проигрывания анимации гекса.
void __stdcall HookOn_GexAnim(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast)
{
  // Проигрываем анимацию гекса при помощи боевой фигуры.
  _BattleFigure_::PlayGexAnim(BattleAnimNum, FrameTime, GexNum, TRUE, DontRedrawLast);
}


// Заменяем способ проигрывания анимации гекса там, где надо проиграть на конкретном месте (а не поверх всего).
void __stdcall HookOn_GexAnimNotAboveAll(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast)
{
  // Проигрываем анимацию гекса при помощи боевой фигуры.
  _BattleFigure_::PlayGexAnim(BattleAnimNum, FrameTime, GexNum, FALSE, DontRedrawLast);
}


// Пропускаем анимацию на гексе.
void __stdcall HookOn_GexSkip(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast)
{
  return;
}


// Появление стены огня.
int __stdcall LoHook_FirewallAppearAnim(LoHook* h, HookContext* c)
{
  
  // Проигрываем анимацию гекса при помощи боевой фигуры.
  _BattleFigure_::PlayGexAnim((*(_Spell_**)(c->ebp - 16))->animation_ix, 100, c->ecx, FALSE, TRUE);
  
  return EXEC_DEFAULT;
}




// Инициализация управления временем при отрисовке тряски землетрясения.
int __stdcall HookOn_Earthquake_Effect_InitTime(LoHook* h, HookContext* c)
{
  // Инициализируем время первой смены кадров.
  c->edi = o_GetTime() + IntAt(c->ebp - 20);
  
  return EXEC_DEFAULT;
}


// Перерисовка при отрисовке тряски землетрясения.
int __stdcall HookOn_Earthquake_Effect(LoHook* h, HookContext* c)
{
  // Менеджер боя.
  _BattleMgr_* BattleMgr = (_BattleMgr_*)c->ebx;
  
  // Изображение боя.
  _Pcx16_* BattleSurface = *(_Pcx16_**)((_ptr_)BattleMgr + 21424);
  
  // Текущее время.
  _int_ CurrTime = o_GetTime();
  
  
  while (CurrTime - c->edi < 0)
  {
    // Проигрываем анимацию ожидания и перерисовываем экран, и ожидаем минимальное время без обновления.
    BattleMgr->ClearRedrawFields();
    BattleMgr->PlayWaitAnim();
    (*(_Pcx16_**)((_ptr_)BattleMgr + 21432)) = 0; // Фон был обновлён
    BattleMgr->RedrawBattlefield(FALSE, FALSE, FALSE, 0, TRUE, FALSE);
    
    // Копируем новое изображение, которое будет отрисовываться.
    BattleSurface->CopySurface16(o_WndMgr->screen_pcx16->buffer, HD_Battle_X, HD_Battle_Y, o_WndMgr->screen_pcx16->width,
                                 o_WndMgr->screen_pcx16->height, o_WndMgr->screen_pcx16->scanline_size);
    
    
    // Границы по-умолчанию.
    _int_ img_x = 0;
    _int_ img_y = 0;
    _int_ img_width = BattleSurface->width;
    _int_ img_height = BattleSurface->height;
    _int_ dest_x = *(_int_*)(c->esi);
    _int_ dest_y = *(_int_*)(c->esi + 4);
    
    
    
    
    // Границы отрисовки.
    _RedrawBorders_* brd = &BattleRedraw_Borders;
    
    // Огриначиваем размеры изображения.
    if (dest_x + img_x < brd->Left)
    {
      img_width -= brd->Left - dest_x - img_x;
      img_x = brd->Left - dest_x;
      dest_x = brd->Left;
    }
    if (dest_y + img_y < brd->High)
    {
      img_height -= brd->High - dest_y - img_y;
      img_y = brd->High - dest_y;
      dest_y = brd->High;
    }
    if (dest_x + img_x + img_width > brd->Right)
    {
      img_width = brd->Right - dest_x - img_x;
    }
    if (dest_y + img_y + img_height > brd->Low)
    {
      img_height = brd->Low - dest_y - img_y;
    }
    
    
    
    
    // Для совместимости с HD.
    if (dest_x < 0)
    {
      img_x -= dest_x;
      img_width += dest_x;
      dest_x = 0;
    }
    if ((dest_x + img_width) > 800)
      img_width = 800 - dest_x;
    
    if (dest_y < 0)
    {
      img_y -= dest_y;
      img_height += dest_y;
      dest_y = 0;
    }
    if ((dest_y + img_height) > 600)
      img_height = 600 - dest_y;
    
    dest_x += HD_Battle_X;
    dest_y += HD_Battle_Y;
    
    
    // Отрисовываем новое изображение.
    BattleSurface->DrawSurface16(img_x, img_y, img_width, img_height, o_WndMgr->screen_pcx16->buffer,
                                 dest_x, dest_y, o_WndMgr->screen_pcx16->width, o_WndMgr->screen_pcx16->height,
                                 o_WndMgr->screen_pcx16->scanline_size, R_NO_COLOR);
    
    // Обновляем экран боя.
    BattleMgr->FlipBattlefield();
    
    // Ожидание.
    CALL_1(void, __fastcall, 0x4F8980, t_min(CurrTime + MIN_FRAME_PERIOD, c->edi));
    
    c->edi += Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
    // Определяем теущее время.
    CurrTime = o_GetTime();
  }
  
  // Следкющее время ожидания.
  c->edi += IntAt(c->ebp - 20);
  
  
  // Пропускаем стандартное ожидание.
  c->return_address = 0x5A805B;
  
  return NO_EXEC_DEFAULT;
}



// Границы перерисовки взрывов землетрясения.
_RedrawBorders_ Earthquake_Expl_RedrawRect;

// Был ли изменён кадр взрыва землетрясения.
_bool_ Earthquake_Expl_Frame_Was_changed;


// Инициализация времени взрыва землетрясения.
int __stdcall HookOn_Earthquake_Expl_InitTime(LoHook* h, HookContext* c)
{
  *(_int_*)(c->ebp - 32) = o_GetTime() + *(_int_*)(c->ebp - 24);
  
  Earthquake_Expl_Frame_Was_changed = FALSE;
  
  return EXEC_DEFAULT;
}




// Перерисовка для взрыва землетрясения.
int __stdcall HookOn_Earthquake_Expl_Draw(LoHook* h, HookContext* c)
{
  (*(_BattleMgr_**)(c->ebp - 8))->ClearRedrawFields();
  
  // Перерисовка.
  (*(_BattleMgr_**)(c->ebp - 8))->PlayWaitAnim();
  if (Earthquake_Expl_Frame_Was_changed)
  {
    (*(_BattleMgr_**)(c->ebp - 8))->SetRedrawBorders();
    (*(_BattleMgr_**)(c->ebp - 8))->RedrawBattlefield(FALSE, FALSE, FALSE, 0, TRUE, FALSE);
  }
  else
  {
    (*(_BattleMgr_**)(c->ebp - 8))->RedrawBattlefield(FALSE, TRUE, FALSE, 0, TRUE, FALSE);
  }
  
  // Взятие границ отрисовки.
  _RedrawBorders_* brd = (*(_BattleMgr_**)(c->ebp - 8))->PField<_RedrawBorders_>(81208);
  MemCopy(&Earthquake_Expl_RedrawRect, brd, sizeof(Earthquake_Expl_RedrawRect));
  
  return EXEC_DEFAULT;
}


// Добавляем взрывы в границы перерисовки, вместо обновления экрана.
int __stdcall HookOn_Earthquake_Expl_FlipBrd(LoHook* h, HookContext* c)
{
  
  if (Earthquake_Expl_Frame_Was_changed) Earthquake_Expl_RedrawRect.AddRect((_RedrawBorders_*)c->esi);
  
  // Пропускаем обновление экрана.
  c->return_address = 0x5A8311;
  
  return NO_EXEC_DEFAULT; 
}


// Обновляем экран при отрисовке взрыва при землетрясении.
int __stdcall HookOn_Earthquake_Expl_Flip(LoHook* h, HookContext* c)
{
  
  // Границы перерисовки.
  _RedrawBorders_* brd = (*(_BattleMgr_**)(c->ebp - 8))->PField<_RedrawBorders_>(81208);
  MemCopy(brd, &Earthquake_Expl_RedrawRect, sizeof(Earthquake_Expl_RedrawRect));
  
  // Обновление.
  (*(_BattleMgr_**)(c->ebp - 8))->FlipRedrawRect();
  
  // Текущее время.
  _int_ curr_time = o_GetTime();
  
  // Ждём минимальное время.
  WaitTill(t_min(curr_time + MIN_FRAME_PERIOD, *(_int_*)(c->ebp - 32)));
  curr_time += Battle_LastDialog_Time + MIN_FRAME_PERIOD;
  *(_int_*)(c->ebp - 32) += Battle_LastDialog_Time;
  Battle_LastDialog_Time = 0;
  Earthquake_Expl_Frame_Was_changed = FALSE;
  
  // Если пришло время, увеличиваем счётчик кадров.
  if (curr_time - *(_int_*)(c->ebp - 32) >= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (*(_int_*)(c->ebp - 24) > 0)
    {
      add_frames = std::min<_int_>((curr_time - *(_int_*)(c->ebp - 32))/(*(_int_*)(c->ebp - 24)) + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    // Не пропускаем 5 кадров, чтобы разрушить стену.
    if (*(_int_*)(c->ebp + 8) < 5 && *(_int_*)(c->ebp + 8) + add_frames > 5)
    {
      add_frames = 5 - *(_int_*)(c->ebp + 8);
    }
    
    // Следующий кадр.
    *(_int_*)(c->ebp + 8) += add_frames;
    
    
    // Следующее время ожидания.
    *(_int_*)(c->ebp - 32) += *(_int_*)(c->ebp - 24)*add_frames;
    
    // Количество кадров взрыва.
    _int_ frames_count = (((((_Def_*)(c->esi))->groups_count > 0) && ((DWORD*)(((_Def_*)(c->esi))->active_groups))[DG_MAIN]) ? ((_Def_*)(c->esi))->groups[DG_MAIN]->frames_count : 0);
    
    // Игнорируем перескок за конец проигрывания.
    if (*(_int_*)(c->ebp + 8) > frames_count)
    {
      *(_int_*)(c->ebp + 8) = frames_count;
    }
    
    // Обнуляем задержку диалогов для взрывов стен.
    Battle_LastDialog_Time = 0;
    
    Earthquake_Expl_Frame_Was_changed = TRUE;
  }
  
  
  // Пропускаем собственное ожидание.
  c->return_address = 0x5A8345;
  
  return EXEC_DEFAULT;
}










// Инициализация времени огненного шара Магога.
int __stdcall HookOn_MGFireballDraw_InitTime(LoHook* h, HookContext* c)
{
  // Время между кадрами.
  MGFireballFrameTime = Round(50.0*BattleAnimPeriodFactors[Settind_BattleFast]);
  
  // Начальное время следующей смены кадров.
  MGFireballNextTime = o_GetTime() + MGFireballFrameTime;
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}


// При отрисовке огненного шара Магога.
int __stdcall HookOn_MGFireballDraw(LoHook* h, HookContext* c)
{
  // Текущее время.
  _int_ curr_time = o_GetTime();
  
  // Ожидаем.
  WaitTill(t_min(curr_time + MIN_FRAME_PERIOD, MGFireballNextTime));
  curr_time += MIN_FRAME_PERIOD - Battle_LastDialog_Time;
  
  // Если пришло время, увеличиваем счётчик кадров.
  if (curr_time - MGFireballNextTime >= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (MGFireballFrameTime > 0)
    {
      add_frames = std::min<_int_>((curr_time - MGFireballNextTime)/MGFireballFrameTime + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Следующий кадр.
    c->edi += add_frames;
    
    
    // Количество кадров взрыва.
    _int_ frames_count = (((((_Def_*)(c->ebx))->groups_count > 0) && ((DWORD*)(((_Def_*)(c->ebx))->active_groups))[DG_MAIN]) ? ((_Def_*)(c->ebx))->groups[DG_MAIN]->frames_count : 0);
    
    // Игнорируем перескок за конец проигрывания.
    if (c->edi > frames_count)
    {
      add_frames -= c->edi - frames_count;
      c->edi = frames_count;
    }
    
    
    // Время следующего проигрывания.
    MGFireballNextTime += MGFireballFrameTime*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
  }
  
  
  return EXEC_DEFAULT;
}







// Инициализация времени облака смерти Лича и Могущественного лича.
int __stdcall HookOn_LichDClDraw_InitTime(LoHook* h, HookContext* c)
{
  // Время между кадрами.
  LichDClFrameTime = Round(100.0*BattleAnimPeriodFactors[Settind_BattleFast]);
  
  // Начальное время следующей смены кадров.
  LichDClNextTime = o_GetTime() + LichDClFrameTime;
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}


// При отрисовке облака смерти Лича и Могущественного лича.
int __stdcall HookOn_LichDClDraw(LoHook* h, HookContext* c)
{
  // Текущее время.
  _int_ curr_time = o_GetTime();
  
  // Ожидаем.
  WaitTill(t_min(curr_time + MIN_FRAME_PERIOD, LichDClNextTime));
  curr_time += MIN_FRAME_PERIOD - Battle_LastDialog_Time;
  
  // Если пришло время, увеличиваем счётчик кадров.
  if (curr_time - LichDClNextTime >= 0)
  {
    
    // Количество добавляемых кадров за прошедшее время.
    _int_ add_frames;
    if (LichDClFrameTime > 0)
    {
      add_frames = std::min<_int_>((curr_time - LichDClNextTime)/LichDClFrameTime + 1, MAX_FRAMESKIP);
    }
    else
    {
      add_frames = MAX_FRAMESKIP;
    }
    
    
    // Следующий кадр.
    c->edi += add_frames;
    
    
    // Количество кадров взрыва.
    _int_ frames_count = (((((_Def_*)(c->ebx))->groups_count > 0) && ((DWORD*)(((_Def_*)(c->ebx))->active_groups))[DG_MAIN]) ? ((_Def_*)(c->ebx))->groups[DG_MAIN]->frames_count : 0);
    
    // Игнорируем перескок за конец проигрывания.
    if (c->edi > frames_count)
    {
      add_frames -= c->edi - frames_count;
      c->edi = frames_count;
    }
    
    
    // Время следующего проигрывания.
    LichDClNextTime += LichDClFrameTime*add_frames + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
  }
  
  return EXEC_DEFAULT;
}







// Инициализация времени армагеддона.
int __stdcall HookOn_ArmageddonDraw_InitTime(LoHook* h, HookContext* c)
{
  // Время между кадрами.
  ArmageddonFrameTime = Round(100.0*BattleAnimPeriodFactors[Settind_BattleFast]);
  
  // Начальное время следующей смены кадров.
  ArmageddonNextTime = o_GetTime() + ArmageddonFrameTime;
  Battle_LastDialog_Time = 0;
  
  return EXEC_DEFAULT;
}


// При отрисовке армагеддона.
int __stdcall HookOn_ArmageddonDraw(LoHook* h, HookContext* c)
{
  // Текущее время.
  _int_ curr_time = o_GetTime();
  
  // Ожидаем.
  WaitTill(t_min(curr_time + MIN_FRAME_PERIOD, ArmageddonNextTime));
  curr_time += MIN_FRAME_PERIOD - Battle_LastDialog_Time;
  
  // Если пришло время, увеличиваем счётчик кадров.
  if (curr_time - ArmageddonNextTime >= 0)
  {

    // Следующий кадр.
    (*(_int_*)(c->ebp - 8))++;
    
    
    // Время следующего проигрывания.
    ArmageddonNextTime += ArmageddonFrameTime + Battle_LastDialog_Time;
    Battle_LastDialog_Time = 0;
    
    return EXEC_DEFAULT;
  }
  // Не наступил следующий кадр - существа не анимируют.
  else
  {
    c->return_address = 0x5A52B0;
    
    return NO_EXEC_DEFAULT;
  }
  
}



// При действиях при ожидании выбора целей некоторых заклинаний (телепорта и жертвы).
_dword_ __stdcall HookOn_SpellSelectTagetsWaitingDraw(HiHook* h, _int_ Param)
{
  // Проигрываем шаг анимации ожидания.
  o_BattleMgr->PlayWaitAnimOnce();
  
  // Вызываем функцию действий.
  return CALL_1(_dword_, __thiscall, h->GetDefaultFunc(), Param);
}








// Перед боем инициализируем функцию отрисовки нажатия кнопки.
void __stdcall HookOn_Battle_Start_ButtonClickDraw(HiHook* h, _BattleMgr_* this_, _dword_ a2)
{
  // Функция отрисовки - боевая.
  ButtonWhileClicked_Draw_List.Append(ButtonWhileClicked_Battle_Draw);
  
  // Очищаем время диалога битвы.
  Battle_LastDialog_Time = 0;
  
  // Вызываем инициализацию битвы.
  CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, a2);
  
}




// После сообщения о конце боя деинициализируем функцию отрисовки нажатия кнопки.
void __stdcall HookOn_Battle_EndMessage_ButtonClickDraw(HiHook* h, _BattleMgr_* this_, _dword_ a2)
{
  // Возвращаем прошлую функцию отрисовки.
  ButtonWhileClicked_Draw_List.Pop();
  
  // Вызываем сообщение о конце боя.
  CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, a2);
}






// При вызове диалога обнуляем функцию отрисовки удержания нажатия кнопки для него.
_dword_ __stdcall HookOn_Show_Dialog(HiHook* h, _WndMgr_* this_, _dword_ a2, _int_ (__fastcall* DalogFunc)(_DlgMsg_* Msg), _dword_ a4)
{
  
  // Был ли диалог скрытым.
  _int_ was_hidden = Dlg_WasHidden;
  Dlg_WasHidden = FALSE;
  
  
  // Время начала диалога.
  _int_ dlg_begin_time;
  
  // Для диалога битвы считываем время его начала.
  if (!was_hidden && ButtonWhileClicked_Draw_List[-1] == ButtonWhileClicked_Battle_Draw)
  {
    dlg_begin_time = o_GetTime();
  }
  
  
  // Если диалог не скрыт...
  if (!was_hidden)
  {
    // Функция отрисовки - нет.
    ButtonWhileClicked_Draw_List.Append(NULL);
  }
  
  // Вызываем диалог.
  _dword_ Result = CALL_4(_dword_, __thiscall, h->GetDefaultFunc(), this_, a2, DalogFunc, a4);
  
  // Если диалог не скрыт...
  if (!was_hidden)
  {
    // Возвращаем прошлую функцию отрисовки.
    ButtonWhileClicked_Draw_List.Pop();
  }
  
  
  // Для диалога битвы считаем его длительность.
  if (!was_hidden && ButtonWhileClicked_Draw_List[-1] == ButtonWhileClicked_Battle_Draw)
  {
    Battle_LastDialog_Time += o_GetTime() - dlg_begin_time;
  }
  
  
  // Возвращаем результат работы диалога.
  return Result;
}



// При отрисовке скрытого диалога включаем отсутствие учёта его существования.
_dword_ __stdcall HookOn_Show_HiddenDialog(HiHook* h, _WndMgr_* this_, _dword_ a2, _int_ (__fastcall* DalogFunc)(_DlgMsg_* Msg), _dword_ a4)
{
  // Диалог скрыт (как диалоги наложения заклинаний).
  Dlg_WasHidden = TRUE;
  
  // Вызываем диалог.
  return CALL_4(_dword_, __thiscall, h->GetDefaultFunc(), this_, a2, DalogFunc, a4);
}







// Отрисовка при удержании нажатия на кнопку.
int __stdcall HookOn_WhileButtonClickedDraw(LoHook* h, HookContext* c)
{
  // Если функция отрисовки определена - вызываем её.
  if (ButtonWhileClicked_Draw_List.GetItemsCount() && ButtonWhileClicked_Draw_List[-1]) ButtonWhileClicked_Draw_List[-1]();
  
  return EXEC_DEFAULT;
}





// При стёрке окна предпросмотра стека или героя перерисовывам участок поля боя.
int __stdcall LoHook_HidePreviewImage_Redraw(LoHook* h, HookContext* c)
{
  if (CanDrawBattle())
  {
    
    SaveBattleRedraws(o_BattleMgr);
    
    // Границы перерисовки поля боя.
    _RedrawBorders_* brd = o_BattleMgr->PField<_RedrawBorders_>(81208);
    
    // Перерисовываемая область.
    _RedrawBorders_ rect;
    rect.Left = c->edi - HD_Battle_X;
    rect.High = c->ebx - HD_Battle_Y;
    rect.Right = c->edi - HD_Battle_X + ((_Pcx_*)PtrAt(c->esi + 48))->width - 1;;
    rect.Low = c->ebx - HD_Battle_Y + ((_Pcx_*)PtrAt(c->esi + 48))->height - 1;
    MemCopy(brd, &rect, sizeof(rect));
    
    
    // Перерисовываем область.
    o_BattleMgr->Field<_bool32_>(81200) = TRUE;
    o_BattleMgr->RedrawBattlefield(FALSE, FALSE, TRUE, 0, TRUE, FALSE);
    o_BattleMgr->Field<_bool32_>(81200) = FALSE;
    
    RestoreBattleRedraws(o_BattleMgr);
    
  }
  
  return EXEC_DEFAULT;
}






// Перед отрисовкой выстрела забираем информацию об атакующем.
void __stdcall Hook_On_BattleStack_DrawShot_GetInfo(HiHook* h, _BattleStack_* this_, _BattleStack_* target)
{
  // Сохраняем текущего атакующего.
  CurrShot_Attacker = this_;
  
  // Рисуем выстрел.
  CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, target);
}




// Перед отрисовкой выстрела стрелковой башни забираем информацию об атакующем.
int __stdcall HookOn_BattleStack_DrawArrowTowerShot_GetInfo(LoHook* h, HookContext* c)
{
  // Получаем информацию о текущем атакующем.
  CurrShot_Attacker = &((_BattleMgr_*)c->edi)->stack[c->ecx][c->esi];
  
  return EXEC_DEFAULT;
}

//
//
//
//// При отрисовке выстрела добавлям взрыв снаряда.
//void __stdcall Hook_On_BattleStack_DrawShot_Expl(HiHook* h, _BattleMgr_* this_, _int_ anim_id, _bool8_ need_clear_redraws)
//{
//  // Если нужно отрисовывать, иной анимации нет и есть стрелок...
//  if (!this_->ShouldNotRenderBattle() && anim_id == -1 && CurrShot_Attacker)
//  {
//    
//    // Получаем тип существа-стрелка.
//    _int_ creature_id = CurrShot_Attacker->creature_id;
//    if (creature_id == CID_ARROW_TOWER)
//    {
//      creature_id = ((_ArrowTower_*)((DWORD)o_BattleMgr + 81272 + 36*(CurrShot_Attacker->Get_ArrowTowerNum())))->CreatureType;
//    }
//    
//    
//    // Если нужно, загружаем и начинаем звук.
//    _Sample_ sound;
//    if (BulletExpl_SoundName[creature_id])
//    {
//      sound = CALL_1(_Sample_, __fastcall, 0x59A770, BulletExpl_SoundName[creature_id]);
//    }
//    else
//    {
//      sound.Wav = 0;
//    }
//    
//    
//    // Если у существа есть анимация...
//    if (BulletExplAnim_ID[creature_id] != -1)
//    {
//      // Настраиваем проигрывание анимации для всех стеков.
//      for (_int_ side_ix = ATTACKER; side_ix <= DEFENDER; side_ix++)
//      {
//        for (_int_ stack_ix = 0; stack_ix < this_->stacks_count[side_ix]; stack_ix++)
//        {
//          // Если стек получал повреждения - на нём будет играть анимация.
//          if (this_->stack[side_ix][stack_ix].is_1_killed || this_->stack[side_ix][stack_ix].isAllKilled)
//          {
//            this_->stack[side_ix][stack_ix].fireshield = TRUE;
//          }
//        }
//      }
//    }
//    
//    // Проигрываем анимации.
//    CALL_3(void, __thiscall, h->GetDefaultFunc(), this_, BulletExplAnim_ID[creature_id], need_clear_redraws);
//    
//    
//    // Если звук был загружен, ожидаем (если нет распараллеливания) и завершаем его.
//    if (sound.Wav)
//    {
//      CALL_3(void, __thiscall, 0x59A7C0, -1, sound.Wav, sound.PlayingInd);
//    }
//    
//  }
//  // Иначе всё стандартно.
//  else
//  {
//    CALL_3(void, __thiscall, h->GetDefaultFunc(), this_, anim_id, need_clear_redraws);
//  }
//}










// При настройке анимации смерти стека учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_DrawActionPlay_SetSpecDeath(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью, настраиваем её ему.
  if ((c->ecx & 255) == 2)
  {
    // Секция анимации - особая смерть, если она есть, иначе - обычная.
    if (((_BattleStack_*)(c->eax - 3))->def->groups_count > DG_SPEC_DEATH
        && ((DWORD*)(((_BattleStack_*)(c->eax - 3))->def->active_groups))[DG_SPEC_DEATH])
    {
      DwordAt(c->eax) = DG_SPEC_DEATH;
    }
    else
    {
      DwordAt(c->eax) = DG_DEATH;
    }
    
    // Пропускаем стандартные настройки.
    c->return_address = 0x46867C;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе - всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}


// При настройке количества кадров анимации смерти стека учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_DrawActionPlay_SpecDeath_FramesCount(LoHook* h, HookContext* c)
{
  // Настраиваем стеку количество кадров анимации смерти.
  if ((*(_Def_**)(c->edx))->groups_count > ByteAt(c->edx - 353) && ((DWORD*)((*(_Def_**)(c->edx))->active_groups))[ByteAt(c->edx - 353)])
  {
    c->eax = (*(_Def_**)(c->edx))->groups[ByteAt(c->edx - 353)]->frames_count;
  }
  else
  {
    c->eax = 0;
  }
  
  // Пропускаем стандартный код.
  c->return_address = 0x46881D;
  
  return NO_EXEC_DEFAULT;
}


// При проверке анимации стека учитываем то, что он мог умереть особой смертью (1).
int __stdcall HookOn_DrawActionPlay_SpecDeath_Check1(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью...
  if (c->eax == DG_SPEC_DEATH)
  {
    // Условный переход.
    c->return_address = 0x468BD0;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе - всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}


// При проверке анимации стека учитываем то, что он мог умереть особой смертью (2).
int __stdcall HookOn_DrawActionPlay_SpecDeath_Check2(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью...
  if ((c->ebx & 255) == DG_SPEC_DEATH)
  {
    // Условный переход.
    c->return_address = 0x468B62;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе - всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}


// При проверке анимации стека учитываем то, что он мог умереть особой смертью (3).
int __stdcall HookOn_DrawActionPlay_SpecDeath_Check3(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью...
  if (c->ecx == DG_SPEC_DEATH)
  {
    // Условный переход.
    c->return_address = 0x468E02;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе - всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}




// При настройке количества кадров анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_Armageddon_SpecDeath_FramesCount(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью - настраиваем ему её количество кадров.
  if (ByteAt(c->edi - 122) == 2)
  {
    // Секция анимации смерти.
    _int_ death_dg;
    
    // Если у стека есть особая смерть, будет играться она.
    if (((_Def_*)(c->eax))->groups_count > DG_SPEC_DEATH
        && ((DWORD*)(((_Def_*)(c->eax))->active_groups))[DG_SPEC_DEATH])
    {
      death_dg = DG_SPEC_DEATH;
    }
    else
    {
      death_dg = DG_DEATH;
    }
    
    // Настраиваем стеку количество кадров анимации смерти.
    if (((_Def_*)(c->eax))->groups_count > death_dg && ((DWORD*)(((_Def_*)(c->eax))->active_groups))[death_dg])
    {
      c->ecx = ((_Def_*)(c->eax))->groups[death_dg]->frames_count;
    }
    else
    {
      c->ecx = 0;
    }
    
    // Пропускаем стандартный код.
    c->return_address = 0x5A50E9;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}



// При настройке анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью (1).
int __stdcall HookOn_Armageddon_SetSpecDeath1(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью - настраиваем ему её количество кадров.
  if (ByteAt(c->edi - 122) == 2)
  {
    // Если у стека есть особая смерть, будет играться она.
    if (((_BattleStack_*)(c->edi - 356))->def->groups_count > DG_SPEC_DEATH
        && ((DWORD*)(((_BattleStack_*)(c->edi - 356))->def->active_groups))[DG_SPEC_DEATH])
    {
      IntAt(c->edi - 296) = DG_SPEC_DEATH;
    }
    else
    {
      IntAt(c->edi - 296) = DG_DEATH;
    }
    
    // Пропускаем стандартный код.
    c->return_address = 0x5A5110;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}


// При настройке анимации смерти стека (армагеддон) учитываем то, что он мог умереть особой смертью (2).
int __stdcall HookOn_Armageddon_SetSpecDeath2(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью - настраиваем ему её количество кадров.
  if (ByteAt(c->edi - 122) == 2)
  {
    // Если у стека есть особая смерть, будет играться она.
    if (((_BattleStack_*)(c->edi - 356))->def->groups_count > DG_SPEC_DEATH
        && ((DWORD*)(((_BattleStack_*)(c->edi - 356))->def->active_groups))[DG_SPEC_DEATH])
    {
      IntAt(c->edi - 296) = DG_SPEC_DEATH;
    }
    else
    {
      IntAt(c->edi - 296) = DG_DEATH;
    }
    
    // Пропускаем стандартный код.
    c->return_address = 0x5A511E;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}






// При настройке анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью.
int __stdcall HookOn_MassSpell_SetSpecDeath(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью - настраиваем ему её количество кадров.
  if (ByteAt(c->eax + 174) == 2)
  {
    // Если у стека есть особая смерть, будет играться она.
    if (((_BattleStack_*)(c->eax - 60))->def->groups_count > DG_SPEC_DEATH
        && ((DWORD*)(((_BattleStack_*)(c->eax - 60))->def->active_groups))[DG_SPEC_DEATH])
    {
      IntAt(c->eax) = DG_SPEC_DEATH;
    }
    else
    {
      IntAt(c->eax) = DG_DEATH;
    }
    
    // Пропускаем стандартный код.
    c->return_address = 0x5A6CFB;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}


// При настройке количества кадров анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью (1).
int __stdcall HookOn_MassSpell_SpecDeath_FramesCount1(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью - настраиваем ему её количество кадров.
  if (ByteAt(c->esi - 122) == 2)
  {
    // Секция анимации смерти.
    _int_ death_dg;
    
    // Если у стека есть особая смерть, будет играться она.
    if (((_Def_*)(c->eax))->groups_count > DG_SPEC_DEATH
        && ((DWORD*)(((_Def_*)(c->eax))->active_groups))[DG_SPEC_DEATH])
    {
      death_dg = DG_SPEC_DEATH;
    }
    else
    {
      death_dg = DG_DEATH;
    }
    
    // Настраиваем стеку количество кадров анимации смерти.
    if (((_Def_*)(c->eax))->groups_count > death_dg && ((DWORD*)(((_Def_*)(c->eax))->active_groups))[death_dg])
    {
      c->ecx = ((_Def_*)(c->eax))->groups[death_dg]->frames_count;
    }
    else
    {
      c->ecx = 0;
    }
    
    // Пропускаем стандартный код.
    c->return_address = 0x5A6BF5;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}



// При настройке количества кадров анимации смерти стека (массовое колдовство) учитываем то, что он мог умереть особой смертью (2).
int __stdcall HookOn_MassSpell_SpecDeath_FramesCount2(LoHook* h, HookContext* c)
{
  // Если стек должен умереть особой смертью - настраиваем ему её количество кадров.
  if (ByteAt(c->esi - 122) == 2)
  {
    // Секция анимации смерти.
    _int_ death_dg;
    
    // Если у стека есть особая смерть, будет играться она.
    if (((_Def_*)(c->eax))->groups_count > DG_SPEC_DEATH
        && ((DWORD*)(((_Def_*)(c->eax))->active_groups))[DG_SPEC_DEATH])
    {
      death_dg = DG_SPEC_DEATH;
    }
    else
    {
      death_dg = DG_DEATH;
    }
    
    // Настраиваем стеку количество кадров анимации смерти.
    if (((_Def_*)(c->eax))->groups_count > death_dg && ((DWORD*)(((_Def_*)(c->eax))->active_groups))[death_dg])
    {
      IntAt(c->ebp - 4) = ((_Def_*)(c->eax))->groups[death_dg]->frames_count;
    }
    else
    {
      IntAt(c->ebp - 4) = 0;
    }
    
    // Пропускаем стандартный код.
    c->return_address = 0x5A6C20;
  
    return NO_EXEC_DEFAULT;
  }
  // Иначе всё стандартно.
  else
  {
    return EXEC_DEFAULT;
  }
}




// Учитываем особую смерть при воскрешении.
int __stdcall LoHook_SpecDeath_Resurrect(LoHook* h, HookContext* c)
{
  // Особая смерть - реакция как на обычную.
  if (c->eax == DG_SPEC_DEATH)
  {
    return NO_EXEC_DEFAULT;
  }
  else
  {
    return EXEC_DEFAULT;
  }
}






// Во время нанесения урона (для магической атаки).
int __stdcall HookOn_MakeMagicDamage(HiHook* h, _BattleStack_* this_, _int_ damage)
{
  // Наносим урон.
  _int_ dead = CALL_2(_int_, __thiscall, h->GetDefaultFunc(), this_, damage);
  
  // Изменяем тип смерти стека.
  if (this_->isAllKilled)
  {
    this_->isAllKilled = 2;
  }
  
  return dead;
}






// При уничтожении препятсвия убираем его время жизни, дабы избежать его повтороного удаления (баг SoD).
void __stdcall HookOn_RemoveObstackle(HiHook* h, _BattleMgr_* this_, _int_ obstackle_ix)
{
  // Убираем препятсвие.
  CALL_2(void, __thiscall, h->GetDefaultFunc(), this_, obstackle_ix);
  
  // Делаем препятствие неисчезающим.
  IntAt(PtrAt((_ptr_)this_ + 81244) + 24*obstackle_ix + 16) = -1;
}





// При заклинании уничтожения препятствия запоминаем его гекс.
int __stdcall LoHook_RemoveObstackle_GetHex(LoHook* h, HookContext* c)
{
  
  // Берём гекс текущего препятсвия.
  CurrDelObst_Hex_IX = ByteAt(c->ecx + 24*c->esi + 8);
  
  return EXEC_DEFAULT;
}




// Заменяем способ проигрывания анимации уничтожения магического препятствия.
void __stdcall HookOn_RemoveObstackleMagic_Anim(HiHook* h, _BattleMgr_* this_, _int_ BattleAnimNum, _int_ GexNum, _int_ FrameTime, _bool_ DontRedrawLast)
{
  // Заменяем гекс.
  CALL_5(void, __thiscall, h->GetDefaultFunc(), this_, BattleAnimNum, CurrDelObst_Hex_IX, FrameTime, DontRedrawLast);
}










// При постановке стека на гекс после полёта предварительно убираем его с предыдущих гексов, если надо.
void __stdcall HiHook_AfterFly_ChangeStackPosition(HiHook* h, _BattleMgr_* this_, _BattleStack_* stack, _int32_ tar_hex_ix)
{
  // Если битва не отрисовывается, убираем стек с гексов.
  if (this_->ShouldNotRenderBattle())
  {
    CALL_2(void, __thiscall, 0x468310, this_, stack);
  }
  
  // Ставим стек на гекс.
  CALL_3(void, __thiscall, h->GetDefaultFunc(), this_, stack, tar_hex_ix);
}






// Звук взрыва волшебной стрелы.
_Sample_ MagicArrowEx;


// Добавляем звук взрыва волшебной стреле.
int __stdcall LoHook_MagicArrow_ExplSound(LoHook* h, HookContext* c)
{
  if (!o_BattleMgr->ShouldNotRenderBattle())
  {
    // Стартуем звук взрыва волшебной стрелы.
    MagicArrowEx = CALL_1(_Sample_, __fastcall, 0x59A770, "MAGCBLTH.wav");
  }
  
  return EXEC_DEFAULT;
}


// Добавляем звук взрыва волшебной стреле - конец.
int __stdcall LoHook_MagicArrow_ExplSoundEnd(LoHook* h, HookContext* c)
{
  if (!o_BattleMgr->ShouldNotRenderBattle())
  {
    // Завершаем звук взрыва волшебной стрелы.
    CALL_3(void, __thiscall, 0x59A7C0, -1, MagicArrowEx.Wav, MagicArrowEx.PlayingInd);
  }
  
  return EXEC_DEFAULT;
}
