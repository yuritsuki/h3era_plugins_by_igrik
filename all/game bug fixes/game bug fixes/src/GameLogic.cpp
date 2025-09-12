/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
// by igrik /////////////////////////////////////////////////////////////////////////////////////////

int __stdcall Y_SetCanselWitchHut(HiHook* hook, _Hero_* hero, _int_ skill, _byte_ skill_lvl)
{
    if (o_WndMgr->result_dlg_item_id == DIID_CANCEL) {
        return 0;
    }

    return CALL_3(int, __thiscall, hook->GetDefaultFunc(), hero, skill, skill_lvl);
}

int __stdcall Y_SetCanselScholarlySS(LoHook* h, HookContext* c)
{
    int sskill = c->edi;
    _Hero_* hero = (_Hero_*)c->ebx;

    if (!b_MsgBoxD(o_ADVEVENT_TXT->GetString(116), 2, 20, hero->second_skill[sskill] +3*sskill +2) )
    {
        if (hero->second_skill[sskill] == 1 ) {
            hero->second_skill_count -= 1;
            hero->second_skill_show[sskill] = 0;
        }
        hero->second_skill[sskill] -= 1;
    }
    c->return_address = 0x4A4B86;
    return NO_EXEC_DEFAULT;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

// исправление созданий WoG'ом корявых пакованых координат
_dword_ __cdecl Y_WoG_MixedPos_Fix(HiHook* hook, int x, int y, int z)
{
    _dword_ xyz = b_pack_xyz(x, y, z);

    return xyz;
}

// исправление созданий WoG'ом корявых разпакованных координат
void __cdecl Y_WoG_UnMixedPos_Fix(HiHook* hook, _dword_ x, _dword_ y, _dword_ z, _dword_ xyz)
{
    *(_dword_*)x = b_unpack_x(xyz);
    *(_dword_*)y = b_unpack_y(xyz);
    *(_dword_*)z = b_unpack_z(xyz);

    return;
}



// фикс вылета: нет проверки на наличие стуктуры целевого стека
// но тут не хватает проверки на c->edi
int __stdcall Y_FixCrash_CastSpell_38(LoHook* h, HookContext* c)
{
   if ( c->edi )
   {
       c->eax = *(int*)(c->edi + 0x38);
       c->ecx = *(int*)(c->ebx + 0x132C0);
       c->return_address = 0x5A1C20;

   } else c->return_address = 0x5A2368;
   return NO_EXEC_DEFAULT;
}

// фикс вылета: при удалении препятствия в битве нет проверки на наличие стуктуры препятствия
// (привет WoG и его стена огня у Огненных Лошадей)
int __stdcall Y_FixCrash_RemoveObstacle(LoHook* h, HookContext* c)
{
   // проверяем на пустую структуру боевого пропятствия
   // чтобы пропустить код обращения к ней и как следствие крит.краш.игры
   if ( !c->ecx || !c->edi )
   {
       c->return_address = 0x466826;
       return NO_EXEC_DEFAULT;

   }
   return EXEC_DEFAULT;
}


// фикс: атакующий летающий стек НЕ пропускает ход при атаке заблоченного со всех сторон целевого стека
_byte_ __stdcall AIMgr_Stack_SetHexes_WayToMoveLength(HiHook* hook, _dword_ this_, _BattleStack_* stack, _int_ side, _int_ gex_id, _int_ isTactics, _int_ speed, _int_ a7)
{
    // результат по умолчанию: клетка не доступна
    _byte_ result = FALSE;

    // если стек существует (защита от вылетов)
    if (stack)
    {
        // пишем сразу структуру менеджера битвы
        _BattleMgr_* bm = o_BattleMgr;

        // проходим по гексам вокруг целевого гекса (gex_id)
        for (int i = 0; i < 6; i++)
        {
            // сначала исключаем случай, если атакующий уже стоит рядом с целью
            _int_ stackGexID = bm->adjacentSquares[gex_id].hexAdjacent[i];

            if ( stackGexID == stack->hex_ix || stackGexID == stack->GetSecondGexID() )
            {
                result = TRUE;
                break;
            }

            // теперь ищем пустой гекс вокруг цели
            _int_ emptyGexID = bm->GetEmptySquareAroundThis(gex_id, i);

            // если в текущем цикле найден пустой доступный гекс вокруг цели
            if ( emptyGexID != -1 )
            {
                // клетка найдена
                result = TRUE;

                // проверяем на доступность широкому атакующему монстру
                if (stack->creature.flags & BCF_2HEX_WIDE)
                {
                    // проверяем справа и слева клетку на под атакующим монстром
                    if (  stack->GetSecondGexID() == (emptyGexID+1) || stack->GetSecondGexID() == (emptyGexID-1) )
                    {
                        result = TRUE;
                        break;
                    }

                    // ищем второй гекс: справа или слева от текущего
                    _byte_ isNotEmptyGexID_Right = bm->IsGexNotFree(emptyGexID +1);
                    _byte_ isNotEmptyGexID_Left = bm->IsGexNotFree(emptyGexID -1);

                    // если оба гекса заняты: убираем флаг найденного и ищем дальше в следующей итерации
                    if (isNotEmptyGexID_Right && isNotEmptyGexID_Left)
                        result = FALSE;
                }

                // если гекс(ы) найден(ы) - выход из цикла
                // если не найдены: ищем в следующей итерации
                if (result)
                    break;
            }
        }
    }

    // если клетка найдена: добавляем её в список
    if (result)
        return CALL_7(_byte_, __thiscall, hook->GetDefaultFunc(), this_, stack, side, gex_id, isTactics, speed, a7);
    else return FALSE;
}

// фикс: двуклеточные монстры в бою могут сделать 1 шаг назад
_LHF_(Y_FixBattle_StackStepBack)
{
    _BattleStack_* stack = (_BattleStack_*)c->edi;
    _int_ target_pos = *(int*)(c->ebp +8);

    c->eax = 5; // стандартный курсор

    if(stack->creature.flags & BCF_2HEX_WIDE && target_pos != stack->GetSecondGexID()) {
        _int_ backGexID = target_pos +1 -2*stack->orientation;
        if (backGexID % 17 > 0 && backGexID % 17 < 16) {
            if(stack->creature.flags & BCF_CAN_FLY) {
                c->eax = 2;
            } else {
                c->eax = 1;
            }
        }
    }
    c->Pop(); // pop edi
    c->Pop(); // pop esi

    return EXEC_DEFAULT;
}

// фикс: командиры, имеющие флаг стрельбы и двойной атаки, в рукопашной бъют один раз.
// Исправим это, ибо они уникальны (03/12/2023)
_LHF_(Y_NPC_FixDoubleAttackOnMelle)
{
    _BattleStack_* stack = (_BattleStack_*)c->edi;
    // CF_DOUBLEATTACK уже проверена к этому моменту
    // т.е. мы гарантированно знаем, что стек имеет двойную атаку
    if (stack->creature.flags & BCF_CAN_SHOOT)
    {
        if (stack->creature_id >= CID_COMMANDER_FIRST_A &&
            stack->creature_id <= CID_COMMANDER_LAST_D )
        {
            // может иметь двойную атаку
            c->return_address = 0x441BB1;
        }
        // НЕ может иметь двойную атаку
        else c->return_address = 0x441C01;
    }
    // может иметь двойную атаку
    else c->return_address = 0x441BB1;

    return NO_EXEC_DEFAULT;
}

// убираем клонов из показа в диалоге результатов битвы
_LHF_(Y_Dlg_BattleResults_IgnoreClones)
{
    _BattleStack_* stack = (_BattleStack_*)(c->eax -96);

    if (stack && stack->creature.flags & BCF_CLONE)
    {
        c->return_address = 0x470958;
        return NO_EXEC_DEFAULT;
    }

    return EXEC_DEFAULT;
}




/**
* Фикс бага встречи одного и того-же героя: игрой в редких случаях перезатирается ecx
* в дополнение: запрещаем встречу, если номера у героев одинаковые
*/
_LHF_(Y_Fix_HeroesInteract)
{
  // восстанавливаем затёртую и важную команду игры
  c->ebx = c->ecx;
  // сохраняем номер героя, чтобы потом восстановить
  _int_ HeroRid = IntAt(DwordAt(c->ebp +0xC));

  // получаем структуру героя правой стороны
  _Hero_* HeroL = *(_Hero_**)(c->ebp +0x8);

  if (!HeroL || HeroRid < 0 || HeroRid > 255 || HeroL->id == HeroRid)
  {
    // запрещаем вызов обмена героями
    c->return_address = 0x4A251F;
    return NO_EXEC_DEFAULT;
  }

  o_AdvMgr->HeroActive_DeMobilize();
  // после выполнения DeMobilize(), в (c->ebp +0xC) в ряде случаев портится указатель на номер героя
  // поэтому мы сначала сохранили номер героя, а щас восстанавливаем
  c->ecx = HeroRid;

  // пропускаем стандартный код игры (мы его выполнили сами)
  c->return_address = 0x4A2486;
  return NO_EXEC_DEFAULT;
}


/**
* Добавляем проверку правого героя на воде (если он не в лодке)
* иначе левый герой в лодке просто "наезжает" на правого героя (который не в лодке)
* и получаем "баг раздвоения" героев
*/
_LHF_(Y_Fix_HeroesOnWaterCheckInteract)
{
  _MapItem_* mapItem = (_MapItem_*)(c->eax);
  _Hero_* heroRigth = o_GameMgr->GetHero(mapItem->setup);

  if ( mapItem && heroRigth && heroRigth->temp_mod_flags & 0x40000 || mapItem->land == 8 )
    c->eax = TRUE;
  else
    c->eax = FALSE;

  c->return_address = 0x4814DD;
  return NO_EXEC_DEFAULT;
}

// Исправление копейщиков в лагерях беженцев (только для клетки с триггером).
_LHF_(LoHook_FixRefugeeCamp_dx)
{
    // Клетка.
    _MapItem_* item = reinterpret_cast<_MapItem_*>(c->edi);

    // Если это загрузка карты или не лагерь беженцев - устанавливаем подтип.
    if (ByteAt(c->ebp + 0xC) || item->object_type != 78)
    {
        item->os_type = LOWORD(c->edx);
    }

    return EXEC_DEFAULT;
}

// решение вылета в городе после битвы, когда её инициируют в городе (например ERM или очарование Суккубов)
void __stdcall Y_CallManager(HiHook* hook, _ExecMgr_* execMgr, _Manager_* callMgr)
{
  if ( execMgr->current == &o_TownMgr->mgr && callMgr == &o_BattleMgr->mgr  )
  {
    _Manager_* mgr = &o_TownMgr->mgr;
    execMgr->RemoveManager(mgr);
    execMgr->AddManager(callMgr);
    execMgr->RunManager();
    execMgr->RemoveManager(callMgr);

    o_TownMgr->town = o_GameMgr->GetTownByMapPoint(o_BattleMgr->mapPoint);
    execMgr->AddManager(mgr);
    execMgr->current = mgr;
  }
  else
  {
    CALL_2(void, __thiscall, hook->GetDefaultFunc(), execMgr, callMgr);
  }
}

// убираем обновление панели ресурсов, если активный менеджер это менеджер города
void __stdcall Y_RedrawResources(HiHook* hook, _dword_ a1, _int_ a2, _int_ a3)
{
  if (!(o_HD_Y >= 660 && o_ExecMgr->current == &o_TownMgr->mgr))
    CALL_3(void, __thiscall, hook->GetDefaultFunc(), a1, a2, a3);
}

// убираем обновление инфо-панели, если активный менеджер не менеджер карты приключений
_LHF_(Y_AdvMgr_RedrawInfoPanel)
{
  if (!o_ExecMgr->current || o_ExecMgr->current == &o_AdvMgr->mgr)
    return EXEC_DEFAULT;

  c->return_address = 0x415E5D;
  return NO_EXEC_DEFAULT;
}

// фикс бага при удалении объектов - не даём удалять разные типы объектов
_LHF_(Y_OnDeleteObjectOnMap)
{
  _MapItem_* currentMapItem = (_MapItem_*)c->edx;
  _MapItem_* deletingMapItem = *(_MapItem_**)(c->ebp +8);

  if (currentMapItem && deletingMapItem)
  {
    if (currentMapItem->object_type!= 62 && currentMapItem->GetReal__object_type() != deletingMapItem->GetReal__object_type())
    {
      c->return_address = 0x4AA984;
      return NO_EXEC_DEFAULT;
    }
  }
  return EXEC_DEFAULT;
}
int globRecord = false;
int __stdcall HiHook_004aa820(HiHook*h, _AdvMgr_* a2, _MapItem_* a3, int XYZ, int record)
{
    globRecord = record;
	return CALL_4(int, __thiscall, h->GetDefaultFunc(), a2, a3, XYZ, record);
}

_LHF_(LoHook_004AA9B3)
{

   NOALIGN struct _Boat_
    {
        INT16 x;
        /** @brief [02]*/
        INT16 y;
        /** @brief [04]*/
        INT16 z;
        /** @brief [06]*/
        INT8 visible;
        /** @brief [07] no clue how to get this offset without align-1, may be substructure*/
        _MapItem_* item;
        char _f_0B;
        /** @brief [0C]*/
        INT32 objectType;
        /** @brief [10]*/
        INT8 objectFlag;
        char _f_11[3];
        /** @brief [14]*/
        INT32 objectSetup;
        /** @brief [18]*/
        INT8 exists;
        /** @brief [19]*/
        INT8 index;
        /** @brief [1A]*/
        INT8 par1;
        /** @brief [1B]*/
        INT8 par2;
        /** @brief [1C]*/
        INT8 owner;
        char _f_1D[3];
        /** @brief [20]*/
        INT32 heroId;
        /** @brief [24]*/
        char hasHero;
        char _f_25[3];
    };
    _MapItem_* cell; // esi
    _Hero_* hero; // ebx
    _Boat_* v4; // edi

    cell = (_MapItem_*)c->edx;
    hero = 0;
    v4 = 0;
    if (cell->object_type == 34)
    {
        hero = o_GameMgr->GetHero( cell->setup);
        hero->Hide();
    }
    if (cell->object_type == 8)
    {
        v4 = &o_GameMgr->Field<_List_<_Boat_>>(0x4E3B8)[cell->setup];
        CALL_3(void, __thiscall, 0x4D7840, v4, 8, v4->index);
    }
    cell->setup = -1;
    if (v4)
		CALL_3(void, __thiscall, 0x4D7840, v4, 8, v4->index);
    if (hero)
        hero->Show(34, hero->id);
    return 0;
}

_LHF_(LoHook_004AA9BF)
{
    return globRecord != 0;
}
// © SadnessPower
// Пожиратель душ ранее воскрешал стеки существ, уровень которых больше 4,
// если их здоровье было меньше 50
_LHF_(BattleStack_AtGettingResurrectionResistance)
{
    // если кастует стек
    if (IntAt(c->ebp + 0x1C))
    {
        if (const auto activeStack = o_BattleMgr->activeStack)
        {
            // если кастует Пожиратель Душ
            if (activeStack->creature_id == CID_SOUL_EATER_A || activeStack->creature_id == CID_SOUL_EATER_D)
            {
                if (reinterpret_cast<_BattleStack_*>(c->edi)->creature.level > 4)
                {
                    // ставим иммунитет к касту
                    c->return_address = 0x05A8824;
                    return NO_EXEC_DEFAULT;
                }
            }
        }
    }

    return EXEC_DEFAULT;
}

// © daemon_n
// фикс бага расчёта защиты с возвратом 0 (чаще всего при атаке медведями существ с низкой защитой)
_LHF_(WoG_BattleStack_GetDefenceAgainst)
{
    // поместить сразу в переменную возврата результат выполнения оригинальной ф-ции
    // вот если бы не Ghost Behemoth, проблема бы решалась в 2 счёта (байта, кек)

    IntAt(0x2832700) = c->eax;
    return EXEC_DEFAULT;

}
// © daemon_n
// Ошибка WoG/ERM -- помещение банка существ не типа объекта 16 не инициализировала его как банк существ
_LHF_(WoG_PlaceObject)
{
    const int objectType = IntAt(c->ebp +0x14);

    int creatureBankType = -1;
    switch (objectType)
    {
    case 24:
        creatureBankType = 8;
		break;
	case 25:
        creatureBankType = 10;
		break;
    case 84:
		creatureBankType = 9;
		break;
	case 85:
		creatureBankType = 7;
		break;
    default:
        break;
    }
    // если это банк существ
    if (creatureBankType != -1)
    {
        // помещаем подтип согласно типу
        IntAt(c->ebp + 0x18) = creatureBankType;
       // и прыгаем в тип 16
        c->return_address = 0x07141B6;
        return NO_EXEC_DEFAULT;
    }

    return EXEC_DEFAULT;

}
// © daemon_n
// фикс SoD бага, который не отмечает посещённосто университа игроком
_LHF_(AdvMgr_EnterToUniversity)
{
    const auto hero = *reinterpret_cast<_Hero_**>(c->ebp +0x8);
    if (hero && hero->owner_id >= 0 && hero->owner_id < 8)
    {
        CALL_2(void, __thiscall, 0x4FC620, c->edi, hero->owner_id);
    }
    return EXEC_DEFAULT;
}

// © JackSlater
// Фикс бага SoD - Невозможно колдовать более сильную версию заклинания "Полёт"
// Если герой уже колдовал его на этот ход
_LHF_(js_Cast_AdventureMagic_BeforeCheckFlyPower)
{
    if (const auto* hero = reinterpret_cast<_Hero_*>(c->esi))
    {
        // cast power is more than current fly power
        const auto& spell = o_Spell[SPL_FLY];
        if (spell.effect[c->edi] > spell.effect[hero->fly_cast_power])
        {
            c->return_address = 0x041C886;
            return NO_EXEC_DEFAULT;
        }
    }
    return EXEC_DEFAULT;
}
// © JackSlater
// Фикс бага SoD - усилители заклинаний от героя не работали при касте существами
_LHF_(js_BattleMgr_CastSpell_BeforeSwitchCase)
{
    // if monster casts spell and it affects enemy stack
    if (IntAt(c->ebp + 0x10) == 1 && o_Spell[IntAt(c->ebp +0x8)].type == -1)
    {
        // set active hero from function return
        *reinterpret_cast<_Hero_**>(c->ebp - 0x14) = CALL_1(_Hero_*, __thiscall, 0x04423B0, o_BattleMgr->GetCurrentStack());
    }
    return EXEC_DEFAULT;
}

// © JackSlater
// Прыжок через ProcessMessagesForTime, если скрытая битва

_LHF_(LoHook_00441B25)
{
    if (!o_BattleMgr->IsHiddenBattle())
    {
        return EXEC_DEFAULT;
    }

    c->return_address = 0x441B43;
    return NO_EXEC_DEFAULT;
}
// © JackSlater
// Прыжок через ProcessMessagesForTime, если скрытая битва
_LHF_(LoHook_00441BD6)
{
    if (!o_BattleMgr->IsHiddenBattle())
    {
        return EXEC_DEFAULT;
    }

    c->return_address = 0x441BF5;
    return NO_EXEC_DEFAULT;
}
// Оковы войны действуют только в битве двух героев (ИИ).
int __stdcall LoHook_ShacklesRest_AI(LoHook* h, HookContext* c)
{
    // Менеджер боя.
    _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;

    // Оковы действуют только если есть
    if (!b_mgr->hero[0] || !b_mgr->hero[1])
    {
        c->return_address = 0x41E77C;

        return NO_EXEC_DEFAULT;
    }
    else
    {
        return EXEC_DEFAULT;
    }
}


// Оковы войны действуют только в битве двух героев (побег).
int __stdcall LoHook_ShacklesRest_Surrend(LoHook* h, HookContext* c)
{
    // Менеджер боя.
    _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;

    // Оковы действуют только если есть
    if (!b_mgr->hero[0] || !b_mgr->hero[1])
    {
        c->return_address = 0x478E1E;

        return NO_EXEC_DEFAULT;
    }
    else
    {
        return EXEC_DEFAULT;
    }
}


// Оковы войны действуют только в битве двух героев (сдача).
int __stdcall LoHook_ShacklesRest_GU(LoHook* h, HookContext* c)
{
    // Менеджер боя.
    _BattleMgr_* b_mgr = (_BattleMgr_*)c->esi;

    // Оковы действуют только если есть
    if (!b_mgr->hero[0] || !b_mgr->hero[1])
    {
        c->return_address = 0x478EE6;

        return NO_EXEC_DEFAULT;
    }
    else
    {
        return EXEC_DEFAULT;
    }
}
// Баг: срабатывание огненного щита по трупу - перепрыгиваем AfterAttackAbilities и GetFireshieldDamage, если
// count_current <=0
_LHF_(LoHook_00441982)
{
    if (reinterpret_cast<_BattleStack_*>(c->esi)->count_current > 0)
    {
        return EXEC_DEFAULT;
    }
    c->return_address = 0x4419A5;
    return NO_EXEC_DEFAULT;
}
// Баг: перед контратакой - проверка на count_current цели
_LHF_(LoHook_00441AFF)
{

    if (reinterpret_cast<_BattleStack_*>(c->edi)->count_current > 0)
    {
        return EXEC_DEFAULT;
    }
    c->return_address = 0x441B85;
    return NO_EXEC_DEFAULT;
}
// исправление активной стороны при контратаке
char __stdcall HiHook_00441b5d(HiHook* h, _BattleStack_* attacker, _BattleStack_* target, int direction)
{
    int currentStackIndex = o_BattleMgr->currentStackIndex;
    int currentStackSide = o_BattleMgr->currentStackSide;
    o_BattleMgr->currentStackSide = attacker->side;
    o_BattleMgr->currentStackIndex = attacker->index_on_side;

    char result = CALL_3(char, __thiscall, h->GetDefaultFunc(), attacker, target, direction);

    o_BattleMgr->currentStackSide = currentStackSide;
    o_BattleMgr->currentStackIndex = currentStackIndex;
    return result;
}

// ##############################################################################################################################
// ##############################################################################################################################
// ##############################################################################################################################

void GameLogic(PatcherInstance* _PI)
{

    // фикс: двуклеточные монстры в бою могут сделать 1 шаг назад
    _PI->WriteCodePatch(0x47601E, "%n", 7); // 7 nops
    _PI->WriteLoHook(0x476020, Y_FixBattle_StackStepBack);

    // фикс: атакующий летающий стек НЕ пропускает ход при атаке заблоченного со всех сторон целевого стека
    _PI->WriteHiHook(0x523FE6, CALL_, EXTENDED_, THISCALL_,  AIMgr_Stack_SetHexes_WayToMoveLength);

    // исправление созданий WoG'ом корявых пакованых координат
    _PI->WriteHiHook(0x711E7F, SPLICE_, EXTENDED_, CDECL_, Y_WoG_MixedPos_Fix);
    _PI->WriteHiHook(0x711F49, SPLICE_, SAFE_, CDECL_, Y_WoG_UnMixedPos_Fix);

    // исправление бага с исчезновением стартового героя при переигрывании
    _PI->WriteByte(0x5029C0, 0xEB);

    // Делаем кнопку отмены в Хижине Ведьмы
    _PI->WriteDword(0x4A7E63 +1, 2);
    _PI->WriteHiHook(0x4A7E8A, CALL_, EXTENDED_, THISCALL_, Y_SetCanselWitchHut);

    // Делаем кнопку отмены у ученого, предлагающего втор.навык
    _PI->WriteLoHook(0x4A4AFE, Y_SetCanselScholarlySS);

    // фикс выбора типа атаки при битве ИИ vs человек (человек не мог выбрать тип атаки)
    // суть в том, что была проверка на флаг V997, а должна быть V998
    _PI->WriteByte(0x762601 +3, 0xC5);

    // пропускаем показ всем игрокам захват Двеллинга 8-го уровня существ
    _PI->WriteByte(0x70DB3B +1, 0x37);

    // логическая ошибка SOD:
    // пропускаем показ обновления экрана для ИИ в телепортах:
    // - если мы не видим территорию
    // - если стоит опция "не показывать передвижения противника"
    _PI->WriteByte(0x41DBE8 +1, 0x5C);
    
    // [центрируем текст названия города по вертикали в окне города(id 149)]
    _PI->WriteByte(0x5C5C1B, 4);
    
    // радус открытия всей карты
    _PI->WriteDword(0x4F4B57, 320); // [чит - меню(ориг = 180)]
    _PI->WriteDword(0x4026FA, 360); // [чит wogeyeofsauron(ориг = 200)]
     // радус закрытия всей карты
    _PI->WriteDword(0x402751, 360); //[чит wogeyeofsauron(ориг = 200)]

    // © daemon_n
    // при доступе к рынку в окне союзника без своих собственных (возможно через торговца артефактов)
    // курс делится на 0, что приводит к крашу при клику на ресурсах и артефактах
    _PI->WriteDword(0x678344, 0x3dcccccd); // float 0.1
    _PI->WriteDword(0x678344 + 11*4, 0x3dcccccd); // float 0.1
    _PI->WriteDword(0x678344 + 22*4, 0x3dcccccd); // float 0.1



    // фикс WoG'a
    // командиры, имеющие флаг стрельбы и двойной атаки, в рукопашной бъют один раз. Исправим это, ибо они уникальны
    _PI->WriteLoHook(0x441BAA, Y_NPC_FixDoubleAttackOnMelle);

    // © daemon_n
    // фикс ERM команды CB:M: при проверке/установке типа и количество существ значение ограничивалось 196 (Драколич)
    const int MAX_MON_ID = IntAt(0x4A1657);
    _PI->WriteDword(0x073A847 + 3, MAX_MON_ID + 1); // увечиличить макс id при проверке на выход из границы
    _PI->WriteDword(0x073A850 + 3, MAX_MON_ID); // увеличить макс id при выходе за границы

    // © daemon_n
    // Ошибка бонуса опыта существ для члена "модификатор", где имеем некорректный тип данных (знаковый) для структуры:
    // Суть в подмене типа копирования символов со знакового на беззнаковое ( MOVSX -> MOVZX )
    _PI->WriteByte(0x71C7A7 +1, 0xB6);
    _PI->WriteByte(0x71C7B3 +1, 0xB6);
    _PI->WriteByte(0x71C7D2 +1, 0xB6);

    // © daemon_n
    // WoG баг, при котором, если защита отряда из оригинальной ф-ции = 0
    // и нет нападающего отряда при расчёте, игра возвращала мусорное значение;
    _PI->WriteLoHook(0x075D42C, WoG_BattleStack_GetDefenceAgainst);

    // © daemon_n
    // Ошибка WoG/ERM -- помещение банка существ не типа объекта 16 не инициализировала его как банк существ
    _PI->WriteLoHook(0x0713429, WoG_PlaceObject);

    // © daemon_n
    // фикс SoD бага, который не отмечает посещённосто университа игроком
    _PI->WriteLoHook(0x04AA153, AdvMgr_EnterToUniversity);

    // убираем клонов из показа в диалоге результатов битвы
    _PI->WriteLoHook(0x4708FC, Y_Dlg_BattleResults_IgnoreClones);


    // Темница(5) на поверхности генерируется на родном типе земли(6), а не на грязи(0)
    _PI->WriteByte(0x464044, 0xEB);
    //_PI->WriteDword(0x6408D8 + 5*4, 6);

    // фикс бага при удалении объектов - затираются данные,
    // если жёлтая клетка другого объекта
    // стоит выше на 1 ед. по y-координате
   // _PI->WriteLoHook(0x4AA979, Y_OnDeleteObjectOnMap);
    _PI->WriteHiHook(0x4aa820, SPLICE_, EXTENDED_, THISCALL_, HiHook_004aa820);
    _PI->WriteLoHook(0x04AA9B3, LoHook_004AA9B3);
    _PI->WriteLoHook(0x04AA9BF, LoHook_004AA9BF);
    _PI->WriteLoHook(0x04AA9EA, LoHook_004AA9BF);

    // © JackSlater
    // фикс бага при получении хинта от Магических Святынь
    // ранее использовался массив с заклинаиями от артефактов 0x430 -> 0x3EA
    _PI->WriteWord(0x40D979 +3, 0x3EA);


    // © JackSlater
    // Фикс бага SoD - Невозможно колдовать более сильную версию заклинания "Полёт"
    // Если герой уже колдовал его на этот ход
    _PI->WriteLoHook(0x041C879, js_Cast_AdventureMagic_BeforeCheckFlyPower);

    // © JackSlater
    // Фикс бага SoD - усилители заклинаний от героя не работали при касте существами
    _PI->WriteLoHook(0x05A065D, js_BattleMgr_CastSpell_BeforeSwitchCase);

    // © JackSlater
    // Исправляем баг SoD: сброс посещённости сирен после боя.
    _PI->WriteCodePatch(0x4DA8FC, "%n", 24); // 24 nop

    // Фикс Уланда - герой имеет продвинутую мудрость на старте
    o_HeroInfo[HID_ULAND].second_skill_1_lvl = 1;
    // Фикс Димера - герой имеет продвинутую разведку на старте
    o_HeroInfo[HID_DEEMER].second_skill_2_lvl = 1;

    // © SadnessPower
    // Фикс Бага воскрешения командиром существ со здоровьем <=50, а не макс 5-го уровня
    _PI->WriteLoHook(0x05A881C, BattleStack_AtGettingResurrectionResistance);

	// @ JackSlater
    // Прыжок через ProcessMessagesForTime, если скрытая битва
    _PI->WriteLoHook(0x441B25, LoHook_00441B25);
    _PI->WriteLoHook(0x441BD6, LoHook_00441BD6); // return address ->0x441BF5


    // Оковы войны действуют только в битве двух героев (ИИ).
    _PI->WriteLoHook(0x41E74A, LoHook_ShacklesRest_AI);
    // Оковы войны действуют только в битве двух героев (побег).
    _PI->WriteLoHook(0x478DA2, LoHook_ShacklesRest_Surrend);
    // Оковы войны действуют только в битве двух героев (сдача).
    _PI->WriteLoHook(0x478EBC, LoHook_ShacklesRest_GU);

    // Баг: срабатывание огненного щита по трупу - перепрыгиваем AfterAttackAbilities и GetFireshieldDamage, если
    // count_current <=0
    _PI->WriteLoHook(0x441982, LoHook_00441982);
    // Баг: перед контратакой - проверка на count_current цели
    _PI->WriteLoHook(0x441AFF, LoHook_00441AFF);
    // Баг? _BattleStack_::MeleeAtack (контратака) - фикс сайда
    _PI->WriteHiHook(0x441b5d, CALL_, EXTENDED_, THISCALL_, HiHook_00441b5d);
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////// Фиксы раздвоения героя //////////////////////////

    // фикс бага встречи одного и того-же героя: игрой в редких случаях перезатирается ecx
    // в дополнение: запрещаем встречу, если номера у героев одинаковые
    _PI->WriteLoHook(0x4A2476, Y_Fix_HeroesInteract);

    // проверка взаимодействия героя в лодке с героем в другой лодке
    // фиксим: вносим проверку на воду под героем 2, вдруг он стоит на воде
    // например если остановился на MapEvent
    _PI->WriteLoHook(0x4814C0, Y_Fix_HeroesOnWaterCheckInteract);
    // прописываем возврат регистров
    _PI->WriteByte(0x4814DD +0, 0x90); // nop
    _PI->WriteByte(0x4814DD +1, 0x5F); // pop EDI
    _PI->WriteByte(0x4814DD +2, 0x5E); // pop ESI

    // добавляем проверку на воду: mapItem->land == 8 (и убираем проверку на лодку)
    _PI->WriteCodePatch(0x4806D9, "F743 04 08000000 752A 909090");

	// @ daemon_n
    // фикс: посещение мифрила обновляет экран
	_PI->WriteCodePatch(0x07060AC, "%n", 7); // удалить добавление мифрила костылём
	_PI->WriteJmp(0x0705F42, 0x0705F7D); // пропустить изменение объекта на карте на костыльное решение

    // Исправление копейщиков в лагерях беженцев (только для клетки с триггером).
    _PI->WriteCodePatch(0x505E15,"%n", 4);
    _PI->WriteLoHook(0x505E15, LoHook_FixRefugeeCamp_dx);

    // Теперь в городах игрока при наличии форта всегда будет отстроено 2 уровня существ.
    _PI->WriteCodePatch(0x5C1051, "%n", 20); // 20 nop
    // Убираем случайность количества стеков в начальной армии.
    _PI->WriteCodePatch(0x4C948F, "%n", 5); // 5 nop
    _PI->WriteCodePatch(0x4C9497, "%n", 10); // 10 nop
    _PI->WriteCodePatch(0x4C950F, "%n", 20); // 20 nop

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////// Фиксы крит.крашей игры //////////////////////////


    // фикс вылета: нет проверки на на наличие стуктуры целевого стека
    // но тут не хватает проверки на c->edi
    _PI->WriteLoHook(0x5A1C17, Y_FixCrash_CastSpell_38);

    // фикс вылета: при удалении препятствия в битве, когда его стуктура таблицы равна нуля
    // (привет WoG и его стена огня у Огненных Лошадей)
    _PI->WriteLoHook(0x46681B, Y_FixCrash_RemoveObstacle);

    // решение вылета битвы, когда её инициируют в городе (например ERM или очарование Суккубов)
    _PI->WriteHiHook(0x4B09D0, SPLICE_, EXTENDED_, THISCALL_, Y_CallManager);

    // убираем обновление инфо-панели и панели ресурсов, если активный менеджер не менеджер карты приключений
    _PI->WriteHiHook(0x403F00, SPLICE_, EXTENDED_, THISCALL_, Y_RedrawResources);
    _PI->WriteLoHook(0x415D4D, Y_AdvMgr_RedrawInfoPanel);

    //_PI->WriteLoHook(0x056AE8A,AIAdvMgr_RedrawInfoPanel);

    //// ЧИТ-Меню ////
    // Увеличиваем кол-во заклинаний 999 -> 9999
    _PI->WriteWord(0x4F508F +4, 9999);
    _PI->WriteWord(0x4F50DE +4, 9999);
    _PI->WriteWord(0x4F511A +4, 9999);
    // Увеличиваем кол-во существ 5 -> 100
    _PI->WriteByte(0x4F4ED2 +1, 100);
}
