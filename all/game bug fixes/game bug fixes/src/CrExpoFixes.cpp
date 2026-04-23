namespace CrExpo
{

// © daemon_n
// отключить выдачу опыта существа при старте игры, есои опыт даётся герою вне битвы
_ERH_(OnGameEnter)
{
    // сбрасываем тип источника выдаваемого опыта на "не битва"
    o_CreExpoCombatFlag = 0;
}

// © daemon_n
// Исправлению получения опыта существами, когда максимальный уровень опыта героем достигнут
int crexpoBonusExp = 0;
_LHF_(Hero__AtGiveExperienceAfterLimit)
{
    // если опыт из битвы и мы превысили лимит
    if (o_CreExpoCombatFlag && c->edx > c->esi)
    {
        crexpoBonusExp = IntAt(c->ebp + 0x8); // добавить псевдоопыт
    }

    return EXEC_DEFAULT;
}
void __stdcall CrExpoSet_AddExpo(HiHook *h, _Hero_ *hero, int expToAdd, const int oldExp)
{
    if (crexpoBonusExp)
    {
        expToAdd += crexpoBonusExp; // добавить недостающий опыт
        crexpoBonusExp = 0;         // сбросить бонусный опыт
    }
    CALL_3(void, __cdecl, h->GetDefaultFunc(), hero, expToAdd, oldExp);
}

_LHF_(WoG_CrExpo_Recalc)
{
    _CrExpo_ *exp = *reinterpret_cast<_CrExpo_ **>(c->ebp - 0x4);
    const int creatureNum = IntAt(c->ebp + 0x8);
    exp->experience = static_cast<int>(static_cast<INT64>(exp->experience) * exp->number / creatureNum);
    exp->number = creatureNum;

    c->return_address = 0x07184D5;
    return NO_EXEC_DEFAULT;
}
_LHF_(WoG_SetNewExp_AtExpPush)
{
    const int sourceExpoOffset = IntAt(c->ebp - 0x4);
    const int dstExpoOffset = IntAt(c->ebp - 0x10);
    const int sourceNumber = IntAt(c->ebp + 0x20);
    const int dstNumber = IntAt(c->ebp + 0x24);

    auto *expTable = reinterpret_cast<_CrExpo_ *>(0x0860550);

    INT64 totalNewExp = static_cast<INT64>(expTable[sourceExpoOffset].experience) * sourceNumber;
    totalNewExp += static_cast<INT64>(expTable[dstExpoOffset].experience) * dstNumber;
    totalNewExp /= static_cast<INT64>(sourceNumber + dstNumber);

    c->eax = static_cast<int>(totalNewExp);

    c->return_address = 0x0719BEA;
    return NO_EXEC_DEFAULT;
}

_LHF_(WoG_CreatureSplitDlg_AtSourceExp)
{
    auto &srcExpo = *reinterpret_cast<_CrExpo_ *>(0x28602AC);
    auto &dstExpo = *reinterpret_cast<_CrExpo_ *>(0x2860294);

    const int srcNum = IntAt(c->ebp - 0x10);

    const int baseNum = IntAt(0x282A77C);
    INT64 totalNewExp = (static_cast<INT64>((srcNum - baseNum)) * srcExpo.experience +
                         static_cast<INT64>(baseNum) * dstExpo.experience) /
                        srcNum;
    IntAt(c->ebp - 0x20) = static_cast<int>(totalNewExp);
    c->return_address = 0x07662D9;
    return NO_EXEC_DEFAULT;
}
_LHF_(WoG_CreatureSplitDlg_AtDestExp)
{
    auto &srcExpo = *reinterpret_cast<_CrExpo_ *>(0x28602AC);
    auto &dstExpo = *reinterpret_cast<_CrExpo_ *>(0x2860294);

    const int baseNum = IntAt(0x282A35C);

    const int dstNum = IntAt(c->ebp - 0x68);

    INT64 totalNewExp = (static_cast<INT64>((dstNum - baseNum)) * dstExpo.experience +
                         static_cast<INT64>(baseNum) * srcExpo.experience) /
                        dstNum;
    IntAt(c->ebp - 0x3C) = static_cast<int>(totalNewExp);
    c->return_address = 0x0766341;
    return NO_EXEC_DEFAULT;
}
struct ArmySlotExperience
{
    int number;
    int experience;
} armySlots[14];

void DebugArmy(_Army_ *army, const char *armyName)
{
    std::string msg = armyName;
    msg += "\n\n\n";
    for (int i = 0; i < 7; i++)
    {
        // msg += "Slot ";
        if (army->type[i] > -1)
        {
            msg += std::to_string(i);
            msg += "{~>CPRSMALL.def:0:";
            msg += std::to_string(army->type[i] + 2) + " valign=bottom}";
            msg += " Count=";
            msg += std::to_string(army->count[i]);
            msg += "\n\n";
        }
    }
    o_MsgBox((char *)msg.c_str());
}

bool isDebugMode = false;
// корректировка опыта при управлении армией ИИ в городе
void __stdcall AI_Player_Hero_ManageArmyInTown(HiHook *h, DWORD *aiData, _Hero_ *hero, _Town_ *town)
{

    const BOOL stackExpisEnabled = IntAt(0x2772730);
    if (stackExpisEnabled)
    {
        isDebugMode = 1;
        auto townArmy = town->GetUpArmy();
        for (size_t i = 0; i < 7; i++)
        {
            townArmy->AddStack(CID_GOLD_DRAGON, 11, 0);
        }
        DebugArmy(town->GetUpArmy(), "town AI_Player_Hero_ManageArmyInTown: army before");
        DebugArmy(&hero->army, "hero AI_Player_Hero_ManageArmyInTown: army before");
    }

    CALL_3(void, __thiscall, h->GetDefaultFunc(), aiData, hero, town);

    if (stackExpisEnabled)
    {
        isDebugMode = 0;
        DebugArmy(town->GetUpArmy(), "town AI_Player_Hero_ManageArmyInTown: army after");
        DebugArmy(&hero->army, "hero AI_Player_Hero_ManageArmyInTown: army after");
    }
}
char __stdcall AI_MoveHeroToTown(HiHook *h, _Player_ *_this, _Town_ *town)
{
    if (_this->id == 1)
    {
        isDebugMode = 1;
    }

    return CALL_2(char, __thiscall, h->GetDefaultFunc(), _this, town);
}

_LHF_(WoG_InTowmArmyMerge_Before)
{

    if (isDebugMode)
    {
        //  o_MsgBox("2");
        //   DebugArmy(reinterpret_cast<_Army_ *>(c->ecx), "hero army before capture");
        //  DebugArmy(reinterpret_cast<_Army_ *>(c->eax), "town army before capture");
    }

    //  isDebugMode = true;
    return EXEC_DEFAULT;
}

char __stdcall H3Army__Merge(HiHook *h, _Army_ *_this, _Army_ *army)
{
    if (isDebugMode)
    {

        //    o_MsgBox("1");

        DebugArmy(army, "townArmy before merge");
        DebugArmy(_this, "heroArmy before merge");
    }
    char result = CALL_2(char, __thiscall, h->GetDefaultFunc(), _this, army);

    if (isDebugMode)
    {
        //  o_MsgBox("3");

        //   DebugArmy(army, "townArmy after merge");
        //   DebugArmy(_this, "heroArmy after merge");
        isDebugMode = false;
    }
    return result;
}

void __stdcall H3Army__Arrange(HiHook *h, _Army_ *_this)
{
}

_LHF_(WoG_InTowmArmyMerge_GuardIterator)
{
    const int slotId = c->eax;

    // сохраняем значения опыта и количества существ для армии защитника
    armySlots[slotId].experience = IntAt(c->ebp - 0x10C);
    armySlots[slotId].number = reinterpret_cast<_Army_ *>(0x2846BF0)->count[slotId];

    // заменить значения опыта существа на уникальный индекс для сложения в битсет
    c->ecx = 1 << slotId;

    return EXEC_DEFAULT;
}

_LHF_(WoG_InTowmArmyMerge_VisitorIterator)
{
    const int slotId = c->ecx + 7;

    // сохраняем значения опыта и количества существ для армии визитёра
    armySlots[slotId].experience = IntAt(c->ebp - 0x10C);
    armySlots[slotId].number = reinterpret_cast<_Army_ *>(0x2846C28)->count[slotId - 7];

    // заменить значения опыта существа на уникальный индекс для сложения в битсет
    c->edx = 1 << slotId;

    return EXEC_DEFAULT;
}
_LHF_(WoG_InTowmArmyMerge_ExperienceCreatorIterator)
{
    INT64 resultExp = 0;

    _Army_ &expArmy = *reinterpret_cast<_Army_ *>(c->ebp - 0xC8);

    const int slotId = IntAt(c->ebp - 0x18);
    const int slotsMerged = expArmy.count[slotId];

    for (size_t i = 0; i < 14; i++)
    {
        if (slotsMerged & (1 << i))
        {
            // сложение общего опыта, основываясь на начальных значениях
            resultExp += static_cast<INT64>(armySlots[i].experience) * armySlots[i].number;
        }
    }

    // установка итогового опыта делением на итоговое количество существ
    const int totalCreatures = IntAt(c->ebp - 0x14);
    c->eax = totalCreatures ? static_cast<int>(resultExp / totalCreatures) : 0;

    c->return_address = 0x0759A2E;
    return NO_EXEC_DEFAULT;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

void CrExpoFixes(PatcherInstance *_PI)
{
    // ОПТИМИЗАЦИИ

    // © daemon_n
    // Оптимизация: убрать повторное вычисление максимального опыта существа при ограничении
    _PI->WriteCodePatch(0x07176D1, "%n", 15); // NOPs вместо повтороного вычисления максимального опыта существа

    // ИСПРАВЛЕНИЯ

    // © daemon_n
    // Ошибка бонуса опыта существ для члена "модификатор", где имеем некорректный тип данных (знаковый) для структуры:
    // Суть в подмене типа копирования символов со знакового на беззнаковое ( MOVSX -> MOVZX )
    _PI->WriteByte(0x71C7A7 + 1, 0xB6);
    _PI->WriteByte(0x71C7B3 + 1, 0xB6);
    _PI->WriteByte(0x71C7D2 + 1, 0xB6);
    // © daemon_n
    // отключить выдачу опыта существа при старте игры, если опыт даётся герою вне битвы
    Era::RegisterHandler(OnGameEnter, "OnGameEnter");
    // © daemon_n
    // Исправлению получения опыта существами, когда максимальный уровень опыта героем достигнут
    _PI->WriteLoHook(0x04E36CE,
                     Hero__AtGiveExperienceAfterLimit); // сохранить опыт героя, который он должен был получить
    _PI->WriteHiHook(0x076B46D, CALL_, EXTENDED_, CDECL_, CrExpoSet_AddExpo); // добавить опыт существам

    // © daemon_n
    // исправление перерасчёта опыта существа при изменении их количества в стеке (при покупке или тп)
    _PI->WriteLoHook(0x07184B0, WoG_CrExpo_Recalc);

    // © daemon_n
    // исправление перерасчёта опыта существа при соединении двух отрядов
    _PI->WriteLoHook(0x0719BE3, WoG_SetNewExp_AtExpPush);

    // © daemon_n
    // исправление расчёта опыта при передаче отрядов через диалог опыта
    _PI->WriteLoHook(0x07662B2, WoG_CreatureSplitDlg_AtSourceExp);
    _PI->WriteLoHook(0x076631A, WoG_CreatureSplitDlg_AtDestExp);

    // © daemon_n
    // исправление расчёта опыта при перемещении героя в город
    _PI->WriteLoHook(0x0759749, WoG_InTowmArmyMerge_GuardIterator);
    _PI->WriteLoHook(0x0759870, WoG_InTowmArmyMerge_VisitorIterator);
    _PI->WriteLoHook(0x0759A24, WoG_InTowmArmyMerge_ExperienceCreatorIterator);

    // !BUG!
    // тестируем отключение сортировки существ в ИИ Армиях, чтобы не ломать опыт
    if (false)
    {
        _PI->WriteLoHook(0x0759A70, WoG_InTowmArmyMerge_Before);
        // _PI->WriteLoHook(0x0525985, Town_BeforeDestroyCapitol);
        // _PI->WriteLoHook(0x052599D, Town_DestroyCapitol);

        _PI->WriteHiHook(0x04B9CE0, CALL_, EXTENDED_, THISCALL_, H3Army__Merge);
        _PI->WriteHiHook(0x0526BE0, CALL_, EXTENDED_, THISCALL_, AI_MoveHeroToTown);
        _PI->WriteHiHook(0x0525985, CALL_, EXTENDED_, THISCALL_, AI_Player_Hero_ManageArmyInTown);
        // _PI->WriteHiHook(0x042D8E0, SPLICE_, EXTENDED_, THISCALL_, H3Army__Arrange);
    }

    // © daemon_n
    // исправление расчёта опыта при перемещении отряда существ из слота в слот
    char *srcExpPatch = "8DB0 50058600 8B06 F76D 20 F77E 04 8906 8B4D 20 894E 04"; // слот-источник
    /*
    lea     esi, [WOG__CrExpo__Table + eax]   ; esi = &Table[src]
    mov     eax, [esi]                        ; eax = Expo
    imul    dword ptr [ebp+source_mon_number] ; edx:eax = Expo * src (64-bit)
    idiv    dword ptr [esi+4]                 ; eax = (Expo*src)/Num

    mov     [esi], eax                        ; Table[src].Expo = eax

    mov     ecx, [ebp+source_mon_number]
    mov     [esi+4], ecx                     ; Table[src].Num = src
    */

    _PI->WriteHexPatch(0x0719993, srcExpPatch); // patch experience
    _PI->WriteCodePatch(0x07199A9, "%n", 28);   // nop extra code

    char *dstExpPatch = "8DB1 50058600 8B06 F76D 24 F77E 04 8906 8B4D 24 894E 04"; // слот-назначение
    /*
    lea     esi, [WOG__CrExpo__Table + ecx]   ; esi = &Table[dst]
    mov     eax, [esi]                        ; eax = Expo
    imul    dword ptr [ebp+dst_mon_number] ; edx:eax = Expo * dst (64-bit)
    idiv    dword ptr [esi+4]                 ; eax = (Expo*dst)/Num

    mov     [esi], eax                        ; Table[dst].Expo = eax

    mov     ecx, [ebp+dst_mon_number]
    mov     [esi+4], ecx                     ; Table[dst].Num = dst
    */

    _PI->WriteHexPatch(0x0719A55, dstExpPatch); // patch experience
    _PI->WriteCodePatch(0x0719A6B, "%n", 28);   // nop extra code
}
} // namespace CrExpo
