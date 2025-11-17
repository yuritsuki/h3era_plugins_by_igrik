
#define NDI_HERO_PORTRAIT_ID 400
DWORD __stdcall BlackMarketDlg__Ctor(HiHook *h, _Dlg_ *dlg, const int x, const int y)
{
    DWORD retAddr = CALL_3(DWORD, __thiscall, h->GetDefaultFunc(), dlg, x, y);
    if (dlg)
    {
        auto heroPortraitFrame = _DlgStaticPcx8_::Create(126, 55, NDI_HERO_PORTRAIT_ID - 1, "TpTavSel.pcx");
        dlg->AddItem(heroPortraitFrame);

        auto heroPortrait =
            _DlgStaticPcx8_::Create(128, 57, NDI_HERO_PORTRAIT_ID, (char *)o_HeroInfo[o_Market_Hero->id].hpl_name);
        dlg->AddItem(heroPortrait);
    }
    return retAddr;
}
int __stdcall BlackMarketDlg__Proc(HiHook *h, const _Dlg_ *dlg, const _DlgMsg_ *msg)
{
    const int result = CALL_2(int, __thiscall, h->GetDefaultFunc(), dlg, msg);
    if (result != 2 && msg->type == MT_MOUSEBUTTON && msg->item_id == NDI_HERO_PORTRAIT_ID &&
        msg->subtype != MST_LBUTTONCLICK)
    {
        o_Market_Hero->ShowHeroScreen(true, true, msg->subtype == MST_RBUTTONDOWN);
    }
    return result;
}
_LHF_(BlackMarketDlg__SetHint)
{
    if (c->eax == NDI_HERO_PORTRAIT_ID)
    {
        sprintf(o_TextBuffer, o_GENRLTXT_TXT->GetString(17), o_Market_Hero->name, o_Market_Hero->Get_className());
        c->Push(reinterpret_cast<int>(o_TextBuffer));
        c->return_address = 0x05EE308;
        return SKIP_DEFAULT;
    }
    return EXEC_DEFAULT;
}

void NativeDlgsImprovements(PatcherInstance *_PI)
{
    // отображение фунционального портрета активного героя в диалоге черного рынка и торговца артефактами
    _PI->WriteHiHook(0x05EA785, CALL_, EXTENDED_, THISCALL_, BlackMarketDlg__Ctor);
    _PI->WriteHiHook(0x05EDDE0, SPLICE_, EXTENDED_, THISCALL_, BlackMarketDlg__Proc);
    _PI->WriteLoHook(0x05EE2B4, BlackMarketDlg__SetHint);
}
