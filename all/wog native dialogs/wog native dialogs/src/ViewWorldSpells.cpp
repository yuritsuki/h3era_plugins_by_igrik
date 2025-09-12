// возвращаемые значения сделаны на 1 больше, чтобы не грузить игру переменными
const char *viewAirFormat = "wnd_player_air_power_%d";
const char *viewEarthFormat = "wnd_player_earth_power_%d";

inline int GetPlayerAirPower()
{
    sprintf(MyString1, viewAirFormat, o_ActivePlayerID);
    return Era::GetAssocVarIntValue(MyString1) - 1;
}

inline int GetPlayerEarthPower()
{
    sprintf(MyString1, viewEarthFormat, o_ActivePlayerID);
    return Era::GetAssocVarIntValue(MyString1) - 1;
}
inline void SetPlayerAirPower(const int power)
{
    sprintf(MyString1, viewAirFormat, o_ActivePlayerID);
    return Era::SetAssocVarIntValue(MyString1, power + 1);
}

inline void SetPlayerEarthPower(const int power)
{
    sprintf(MyString1, viewEarthFormat, o_ActivePlayerID);
    return Era::SetAssocVarIntValue(MyString1, power + 1);
}

void __stdcall HiHook_0041C80F(HiHook *h, _AdvMgr_ *this_, const int spell, const int power)
{
    if (power > GetPlayerAirPower())
    {
        SetPlayerAirPower(power);
    }

    CALL_3(void, __thiscall, h->GetDefaultFunc(), this_, spell, power);

    // H3AdventureManager::ForceNewMouseOver
    // THISCALL_1(void, 0x4194D0, this_);
}

void __stdcall HiHook_0041C75E(HiHook *h, _AdvMgr_ *this_, const int spell, const int power)
{
    if (power > GetPlayerEarthPower())
    {
        SetPlayerEarthPower(power);
    }

    CALL_3(void, __thiscall, h->GetDefaultFunc(), this_, spell, power);

    // H3AdventureManager::ForceNewMouseOver
    // THISCALL_1(void, 0x4194D0, this_);
}

_LHF_(LoHook_005FC394)
{
    // Если каст заклинания (а не просмотр мира) - выходим
    if (IntAt(c->ebp + 0x8) > 0)
    {
        return EXEC_DEFAULT;
    }

    ByteAt(0x6AABD0) = 0;

    // Если игрок кастовал просмотр земли
    const int viewEarthPlayerPower = GetPlayerEarthPower();
    if (viewEarthPlayerPower >= 0)
    {
        ByteAt(0x6AABE1) = 1;
        // Если продвинутый или экспертный навык магии земли
        if (viewEarthPlayerPower > 1)
        {
            ByteAt(0x6AABD0) = 1;
            // Если экспертный навык магии земли
            if (viewEarthPlayerPower > 2)
            {
                ByteAt(0x6AABE0) = 1;
            }
        }
    }
    // Если игрок кастовал просмотр воздуха
    const int viewAirPlayerPower = GetPlayerAirPower();

    if (viewAirPlayerPower >= 0)
    {
        ByteAt(0x6AAC70) = 1;
        // Если продвинутый или экспертный навык магии земли
        if (viewAirPlayerPower > 1)
        {
            ByteAt(0x6AAC98) = 1;
            // Если экспертный навык магии земли
            if (viewAirPlayerPower > 2)
            {
                ByteAt(0x6AAC7C) = 1;
            }
        }
    }
    c->return_address = 0x5FC3EC;
    return NO_EXEC_DEFAULT;
}

_LHF_(LoHook_005FC459)
{
    DwordAt(c->ebp - 0x14) = o_AdvMgr->xyz.xyz;
    return EXEC_DEFAULT;
}

_ERH_(OnEveryDay)
{
    SetPlayerAirPower(-1);
    SetPlayerEarthPower(-1);
}

////////////////////////////////////////////////////////////////////////

_LHF_(LoHook_0041D474)
{

    if (o_ActivePlayer->selected_hero_id != -1)
    {
        // _Hero_ *hero = reinterpret_cast<_Hero_ *>(c->esi);
        int tempRef = 0;

        // Триггер битвы - H3AdventureManager::EndMoveHero
        CALL_8(_MapItem_ *, __thiscall, 0x47F9B0, o_AdvMgr, c->esi, 0, 0, -1, -1, 1, &tempRef);
    }

    return EXEC_DEFAULT;
}

void Dlg_ViewWorldScreen(PatcherInstance *_PI)
{
    // Сохраненный эффект просмотра воздуха/земли на 1 день
    // H3AdventureManager::CastSpell - CASE SPELL_VIEW_EARTH
    _PI->WriteHiHook(0x41C75E, CALL_, EXTENDED_, THISCALL_, HiHook_0041C75E);

    // H3AdventureManager::CastSpell - CASE SPELL_VIEW_AIR
    _PI->WriteHiHook(0x41C80F, CALL_, EXTENDED_, THISCALL_, HiHook_0041C80F);

    // H3AdventureManager::CastViewEarthOrAir - проверка на уже сделанные касты
    _PI->WriteLoHook(0x5FC394, LoHook_005FC394);

    // H3AdventureManager::CastViewEarthOrAir - фикс позиции экрана
    _PI->WriteLoHook(0x5FC459, LoHook_005FC459);

    // Сбрасываем глобалки просмотра земли/воздуха
    Era::RegisterHandler(OnEveryDay, "OnEveryDay");
}
