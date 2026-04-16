/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
namespace AI
{
// © Raistlin
// Исправляем косяк, из-за которого ИИ не мог использовать торговцев артефактами в Сопряжении
int __stdcall R_FixAI_ConfluxNerchant(LoHook *h, HookContext *c)
{
    if ((c->eax & 0xFF) == TOWN_CONFLUX) // al
    {
        *(int *)(c->ebp - 36) = c->ecx;
        c->return_address = 0x525EE0;
        return NO_EXEC_DEFAULT;
    }
    return EXEC_DEFAULT;
}
// Фикс бесконечного цикла ИИ: когда он оценивает ценность получения стека обнуляем отрицательное значение, чтобы
// избежать переполнения.
int __stdcall Fix_AI_GetBestStackExchVal(HiHook *h, DWORD this_, int crId, __int16 qty, __int16 *slotPtr,
                                         char mustReplaceCreature)
{
    const int result = CALL_5(int, __thiscall, h->GetDefaultFunc(), this_, crId, qty, slotPtr, mustReplaceCreature);
    return result < 0 ? 0 : result;
}

// фикс вылета: АИ битва (просчёт)
// проверка на скорость монстра и когда он дойдет до защиты стрелка.
// Убираем из проверки существ с нулевой скоростью и боевые машины
_int_ __stdcall Y_AIMgr_Stack_MinRoundToReachHex(HiHook *hook, _dword_ this_, _BattleStack_ *stack, _int_ a3)
{
    if (stack->creature.flags == BCF_CANT_MOVE || stack->creature.speed < 1)
    {
        return 99; // 99 раундов необходимо, чтобы добраться до стрелка
    }

    return CALL_3(_int_, __thiscall, hook->GetDefaultFunc(), this_, stack, a3);
}

// © daemon_n
// фикс бага WoG -- для ИИ не был добавлен рассчёт кавалерийского бонуса для новых существ и SE:
_LHF_(FixAI_CavalryBonus) // 0x0436200
{
    const int atkStack = c->ecx; // поместим нападающий стек в ebx для ф-ции ниже

    int result = reinterpret_cast<_BattleStack_ *>(c->ecx)->creature_id;
    __asm {
        mov ebx, atkStack // attacker
        mov edi, 0 // второй аргумент
        mov eax, 0x075D7F5 // вызов ф-ции вог, на случай, если кто хочет её хукнуть
        call eax
        mov result, eax
    }
    c->ecx = result;               // во время возврата должен быть уже id существа, а не stack*
    c->return_address = 0x0436208; // возвращаем "Чемпиона" для последующей проверки
    return NO_EXEC_DEFAULT;
}

// Исправляем плохой учёт ИИ нейтралов при рассчёте боевого духа с учётом Альянса Ангелов (1).
int __stdcall LoHook_FixAngelicAllianceAI1(LoHook *h, HookContext *c)
{
    // Если существо - нейтрал, пропускаем взятие бита из маски альянса.
    if (c->esi == -1)
    {
        // Восстанавливаем затёртую команду.
        c->edi = c->eax;

        // Стек не подвержен альянсу.
        c->return_address = 0x42C7C3;

        return NO_EXEC_DEFAULT;
    }
    else
    {
        return EXEC_DEFAULT;
    }
}

// Исправляем плохой учёт ИИ нейтралов при рассчёте боевого духа с учётом Альянса Ангелов (2).
int __stdcall LoHook_FixAngelicAllianceAI2(LoHook *h, HookContext *c)
{
    // Если существо - нейтрал, пропускаем взятие бита из маски альянса.
    if (c->edi == -1)
    {
        // Восстанавливаем затёртую команду.
        c->ebx = c->eax;

        // Стек не подвержен альянсу.
        c->return_address = 0x42C8F9;

        return NO_EXEC_DEFAULT;
    }
    else
    {
        return EXEC_DEFAULT;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

void AIFixes(PatcherInstance *_PI)
{
    // © Raistlin
    // Исправляем косяк, из-за которого ИИ не мог использовать торговцев артефактами в Сопряжении
    _PI->WriteLoHook(0x525ED1, R_FixAI_ConfluxNerchant);
    // Фикс бесконечного цикла ИИ: когда он оценивает ценность получения стека обнуляем отрицательное значение, чтобы
    // избежать переполнения.
    _PI->WriteHiHook(0x42c830, SPLICE_, EXTENDED_, THISCALL_, Fix_AI_GetBestStackExchVal);

    // AI битва (просчёт)
    // проверка на скорость монстра и когда он дойдет до защиты стрелка.
    // Убираем из проверки существ с нулевой скоростью и боевые машины
    _PI->WriteHiHook(0x4B3C80, SPLICE_, EXTENDED_, THISCALL_, Y_AIMgr_Stack_MinRoundToReachHex);

    // AI БИТВА
    // AI всегда будет получать артефакты и опыт за побеждённого врага
    _PI->WriteByte(0x426F41, 0);

    // Исправление бага SoD с передачей артефактов побеждённому компьютеру.
    _PI->WriteHexPatch(0x426FA1 + 1, "CA"); // (mov ecx,) edx
    _PI->WriteHexPatch(0x426FA3, "53");     // push ebx

    // © JackSlater
    // Фикс бага WoG - Драколичи не имели флаг сплеша для ИИ
    o_CreatureInfo[CID_DRACOLICH].fireballAttack = true; // !#DC(MON_FLAG_SPLASH_SHOOTER) = 1048576;
    // Исправляем баг SoD: ИИ считал, что облако личей не задевает только нежить.
    _PI->WriteByte(0x41EFA6 + 2, 4);
    _PI->WriteHexPatch(0x41EFAC, "75 16"); // jnz 0x41EFC4

    // © daemon_n
    // фикс бага WoG -- для ИИ не был добавлен рассчёт кавалерийского бонуса для новых существ и SE:
    _PI->WriteLoHook(0x0436200, FixAI_CavalryBonus);

    return;
    //// + Исправляем плохой учёт ИИ нейтралов при рассчёте боевого духа с учётом Альянса Ангелов (баг SoD).
    //_PI->WriteLoHook(0x42C778, LoHook_FixAngelicAllianceAI1);
    //_PI->WriteLoHook(0x42C8AD, LoHook_FixAngelicAllianceAI2);
}

} // namespace AI
