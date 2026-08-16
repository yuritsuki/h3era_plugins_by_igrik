// pch.cpp: файл исходного кода, соответствующий предварительно скомпилированному заголовочному файлу

#include "pch.h"



//RunTemplatesListDlg 011533F0

//адрес для чека инфы 00586921, см EAX
//
//itemId справа :
//кадр дефа размера выбранной карты : 189
//текст "название миссии" : 100 (похоже что не так)
//текст имени карты : 100 - без открытых вкладок, 101 - сценарий, 102 - дополнительные опции, 103 - случайная
//0057BCCC
//
//доступные сценарии : 128
//дополнительные опции : 129
//случайная карта : 130
//hw опции: 131
//текст "описание миссии" : 105 (точно ли ? )
//текст описания миссии : FFFFFFFF
//сложность с пешки до короля : 107 108 109 110 111

//еще опции : ? ? ? (не триггерит LeftClickProc - в хуке хд - мода до этого следует своя логика)
//начать : 186
//выйти : 188
//
//
//itemId в random map :
//текст template слева от hd вкладки : 7003
//сам скролл вкладки с шаблонами??: 7001
//hd вкладка с шаблонами : 7002 (не триггерит LeftClickProc - в хуке хд - мода до этого следует своя логика)
//размер карты с S по Undergroung : 281 282 283 284 3003 3004 3005 285
//игроки(люди или ии) с 1 по рандом : 287 288 289 290 291 292 293 294 295
//игроки(только ии) с 0 по рандом : 307 308 309 310 311 312 313 314 315
//командные соглашения : ? ? ? (не триггерит LeftClickProc - в хуке хд - мода до этого следует своя логика)
//тип дорог : 7007 7008 7009 (hd id - шники ? ? ? )
//доля воды с без воды по рандом : 326 327 328 329
//сила монстров со слабые по рандом : 331 332 333 334
//сгенерированные карты : 335
//
//
//itemId в available scenarios :
//фильтры по размеру с S по все : 137 138 139 140 3000 3001 3002 141
//фильтры по игрокам до условий поражения : 190 191 192 193 194 195
//выбранная карта с верхней по нижнюю : 142 - 159
//слайдер карт вниз - вверх : 151 (в обе стороны)
//
//
//itemId в show advanced options :
//флаги с красного по розовый : 263 264 265 266 267 268 269 270
//человек или ии : 102 (? ? ? )
//стрелка влево смены города : 215 216 217 218 219 220 221 222
//стрелка вправо смены города : 223 224 225 226 227 228 229 230
//стрелка влево смены героя : 231 232 233 234 235 236 237 238
//стрелка вправо смены героя : 239 240 241 242 243 244 245 246
//стрелка влево смены бонуса : 247 248 249 250 251 252 253 254
//стрелка вправо смены бонуса : 255 256 257 258 259 260 261 262
//таймер(вкладка) : 2705 (не триггерит LeftClickProc - в хуке хд - мода до этого следует своя логика)
//слайдер таймер классический : 338 (в обе стороны)



//char byte_104A779C = 0;
//char byte_104A7440 = 0;

LPCSTR BTN_RMC_HINT_KEY = "XXL.rmcHint.%d";
LPCSTR COLUMN_MAP_SIZE_KEY = "XXL.mapSizeColumn.%d";


// Размеры карт (одна сторона)
#define MAPSIZE_S   36
#define MAPSIZE_M   72
#define MAPSIZE_L  108
#define MAPSIZE_XL 144
#define MAPSIZE_H  180
#define MAPSIZE_XH 216
#define MAPSIZE_G  252


// Compatibility helpers for the public H3API_JS header.  The original XXL
// source was written against Igrik's private header fork, which exposed these
// fields/globals under different names.
static BOOL8& SaveGameMode(H3SelectScenarioDialog* dlg)
{
    return *reinterpret_cast<BOOL8*>(reinterpret_cast<_byte_*>(dlg) + 0x66);
}

static BOOL8& InScenarioOptions(H3SelectScenarioDialog* dlg)
{
    return *reinterpret_cast<BOOL8*>(reinterpret_cast<_byte_*>(dlg) + 0x37D);
}

static BOOL8& RandomMapGeneration(H3SelectScenarioDialog* dlg)
{
    return *reinterpret_cast<BOOL8*>(reinterpret_cast<_byte_*>(dlg) + 0x37F);
}

static H3Vector<H3ScenarioMapInformation>& CurrentMapsList(H3SelectScenarioDialog* dlg)
{
    return *reinterpret_cast<H3Vector<H3ScenarioMapInformation>*>(
        reinterpret_cast<_byte_*>(dlg) + 0x1050);
}

static void SendMouseMsgCompat(H3SelectScenarioDialog* dlg, H3Msg* msg)
{
    THISCALL_2(void, 0x5FF3A0, dlg, msg);
}

static _ptr_& DirectPlayCom()
{
    return *reinterpret_cast<_ptr_*>(0x69D858);
}

static int& MapWidth()
{
    return IntAt(0x6783C8);
}

static int& MapHeight()
{
    return IntAt(0x6783CC);
}


// itemId для сценариев

// Выбор карты.
// Номер элемента диалоге - просмотр H-карт.
#define HMAP_DLGITEM_ID 3000
// Номер элемента диалоге - просмотр XH-карт.
#define XHMAP_DLGITEM_ID 3001
// Номер элемента диалоге - просмотр G-карт.
#define GMAP_DLGITEM_ID 3002


// itemId для случайной карты

// Номер элемента диалоге - просмотр H случайных карт.
#define HRMAP_DLGITEM_ID 3003
// Номер элемента диалоге - просмотр XH случайных карт.
#define XHRMAP_DLGITEM_ID 3004
// Номер элемента диалоге - просмотр G случайных карт.
#define GRMAP_DLGITEM_ID 3005


void SetRmcHint(H3DlgItem* item, BOOL8 alloc_mem)
{
    libc::sprintf(h3_TextBuffer, BTN_RMC_HINT_KEY, item->GetID());
    LPCSTR rmcHint = EraJS::read(h3_TextBuffer);
    item->SetHints(NULL, rmcHint, alloc_mem);
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
// хд миникарта
#if 0
// This renderer was never installed: all five hook registrations in
// MapSize_Init are commented out.  It also depends on private H3API symbols.
// Keep it as historical reference while the compatible renderer is integrated.
H3LoadedPcx* pcx8_RadarLineH = nullptr;
H3LoadedPcx* pcx8_RadarLineV = nullptr;
H3LoadedPcx* pcx8_Radar = nullptr;
H3LoadedPcx16* pcx16_Minimap = nullptr;



void SetPaletteFrom(H3LoadedPcx* this_, H3LoadedPcx* src)
{
    //THISCALL_2(void, 0x522E00, &this_->palette565, &src->palette565);
    //MemCopy(this_->palette888.color, src->palette888.color, 0x300u);
    MemCopy(this_->palette565.color, src->palette565.color, 256 * sizeof(_word_));
    MemCopy(this_->palette888.color, src->palette888.color, 256 * sizeof(H3RGB888));
}

void DrawPcx16Resized(H3LoadedPcx16* this_pcx, H3LoadedPcx16* src_pcx, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h)
{
    d_x = std::max<int>(0, d_x);
    d_y = std::max<int>(0, d_y);
    int src_w = std::min<int>(s_w, src_pcx->width);
    int src_h = std::min<int>(s_h, src_pcx->height);

    int src_x;
    int src_y;
    int dst_y;
    int dst_x;

    if (globalPatcher->VarGetValue<INT32>("HD.BPP", 16) == 32)
    {
        _dword_* src_bits0 = (_dword_*)(src_pcx->buffer);
        _dword_* dst_bits0 = (_dword_*)(this_pcx->buffer);
        int src_scan = src_pcx->scanlineSize >> 2;
        int dst_scan = this_pcx->scanlineSize >> 2;


        for (dst_y = 0; dst_y < d_h; dst_y++)
            for (dst_x = 0; dst_x < d_w; dst_x++)
            {
                src_x = dst_x * src_w / d_w;
                src_y = dst_y * src_h / d_h;
                dst_bits0[d_x + dst_x + dst_scan * (d_y + dst_y)] = src_bits0[src_x + src_scan * src_y];
            }
    }
    else // 16
    {
        _word_* src_bits0 = (_word_*)(src_pcx->buffer);
        _word_* dst_bits0 = (_word_*)(this_pcx->buffer);
        int src_scan = src_pcx->scanlineSize >> 1;
        int dst_scan = this_pcx->scanlineSize >> 1;


        for (dst_y = 0; dst_y < d_h; dst_y++)
            for (dst_x = 0; dst_x < d_w; dst_x++)
            {
                src_x = dst_x * src_w / d_w;
                src_y = dst_y * src_h / d_h;
                dst_bits0[d_x + dst_x + dst_scan * (d_y + dst_y)] = src_bits0[src_x + src_scan * src_y];
            }
    }

}

void _Pcx8_Fill(H3LoadedPcx* this_, _byte_ color_index)
{
    int size = this_->scanlineSize * this_->height;
    _byte_* p_pixel = (_byte_*)(this_->buffer);
    for (int i = 0; i < size; i++)
        *(p_pixel++) = color_index;
}

void _Pcx8_DrawPcx8(H3LoadedPcx* this_pcx, H3LoadedPcx* src_pcx, int dst_x, int dst_y, int src_w, int src_h)
{
    int src_x = std::max<int>(0, -dst_x);
    int src_y = std::max<int>(0, -dst_y);
    dst_x = std::max<int>(0, dst_x);
    dst_y = std::max<int>(0, dst_y);
    src_w = std::min<int>(this_pcx->width - dst_x, src_w - src_x);
    src_h = std::min<int>(this_pcx->height - dst_y, src_h - src_y);
    _byte_* src_pixel_bits = (_byte_*)(src_pcx->buffer);
    _byte_* dst_pixel_bits = (_byte_*)(this_pcx->buffer);

    for (int y = 0; y < src_h; y++)
        for (int x = 0; x < src_w; x++)
        {
            if (src_pixel_bits[src_x + x + (src_y + y) * src_pcx->scanlineSize] != 0x00)
                dst_pixel_bits[dst_x + x + (dst_y + y) * this_pcx->scanlineSize] = src_pixel_bits[src_x + x + (src_y + y) * src_pcx->scanlineSize];
        }

}

void ConstructMinimapSelectionRect(int width, int height)
{
    _Pcx8_Fill(pcx8_Radar, 0);

    if ((width > 512) || (height > 512))
        return;

    _Pcx8_DrawPcx8(pcx8_Radar, pcx8_RadarLineH, 0, 0, width, 1);
    _Pcx8_DrawPcx8(pcx8_Radar, pcx8_RadarLineV, 0, 0, 1, height);
    _Pcx8_DrawPcx8(pcx8_Radar, pcx8_RadarLineH, 0, height - 1, width, 1);
    _Pcx8_DrawPcx8(pcx8_Radar, pcx8_RadarLineV, width - 1, 0, 1, height);
}


void __stdcall HiHook_AdvMgr_DrawMinimap(
    HiHook* h, H3AdventureManager* this_, H3Position radar_xyz, INT8 redraw_screen, int a4, INT8 a5, INT8 a6, INT8 a7)
{
    if (pcx8_RadarLineH == nullptr)
        pcx8_RadarLineH = H3LoadedPcx::Load("radar_h.pcx");
    if (pcx8_RadarLineV == nullptr)
        pcx8_RadarLineV = H3LoadedPcx::Load("radar_v.pcx");
    if (pcx8_Radar == nullptr)
    {
        pcx8_Radar = H3LoadedPcx::Create("", 288, 288);
        SetPaletteFrom(pcx8_Radar, pcx8_RadarLineH);
    }
    if (pcx16_Minimap == nullptr)
        pcx16_Minimap = H3LoadedPcx16::Create("", 255, 255);


    //H3DlgItem* minimap = *(H3DlgItem**)(PtrAt(this_ + 68) + 76);
    H3DlgItem* minimap = this_->dlg->minimapTransparentOverlay;
    int mm_x = minimap->xPos;
    int mm_y = minimap->yPos;
    int mm_w = minimap->widthItem;
    int mm_h = minimap->heightItem;
    H3Player* me;
    int h_x, h_y;
    H3Hero* hero;
    H3MapItem* map_item;
    int x, y;
    //int z = b_unpack_z(radar_xyz);
    int z = radar_xyz.GetZ();
    _word_ color;
    int world_view_width = globalPatcher->VarGetValue<INT32>("HD.Rez.X", 800) - (800 - 608);
    int world_view_height = globalPatcher->VarGetValue<INT32>("HD.Rez.Y", 600) - (600 - 560);

    if (!P_NetworkGame ||
        P_ActivePlayer->IsHuman() ||
        PtrAt(P_DirectPlayCom + 0xf0) ||
        ((PtrAt(P_DirectPlayCom + 0xf0)) ? (!ByteAt(PtrAt(P_DirectPlayCom + 0xf0) + 4)) : 0) ||
        ByteAt(0x696A54) ||
        DwordAt(0x6AACA4))
    {
        me = P_Game->GetPlayer();

        if (!P_ActivePlayer->IsHuman())
        {
            //if (!DwordAt(this_ + 908))
            if (!this_->bHeroLogoShowing)
            {
                if (!P_NetworkGame || P_AutoSolo)
                {
                    CDECL_0(void, 0x4CA440); //draw shield on minimap
                }
            }

        }
        if ((P_NetworkGame || P_ActivePlayer->IsHuman() || P_AutoSolo)
            && (!this_->bHeroLogoShowing || P_ActivePlayer->IsHuman()))
        {
            this_->bHeroLogoShowing = 0;
            h_x = -1;
            h_y = -1;
            hero = NULL;
            if (me->currentHero > -1)
            {
                hero = P_Game->GetHero(me->currentHero);
                if (hero->z == z)
                {
                    h_x = hero->x;
                    h_y = hero->y;
                }
            }

            _word_* minimap_ppixel = (_word_*)(pcx16_Minimap->buffer);
            int minimap_scan = pcx16_Minimap->scanlineSize >> 1;


            INT8 v25;
            for (y = 0; y < P_MapHeight; y++)
            {
                for (x = 0; x < P_MapWidth; x++)
                {
                    color = 0;
                    map_item = P_AdventureManager->GetMapItem(x, y, z);

                    if ((!DwordAt(0x699588)) && (H3MapItem::GetMapCellVisability(x, y, z) & ((_word_)ByteAt(0x69CD08))))
                        v25 = 1;
                    else
                        v25 = 0;
                    if (a5)
                    {
                        if (map_item->objectType == 53)
                            v25 = 1;
                    }
                    if (a6)
                    {
                        if (map_item->objectType == 34/*MAPOBJECT_HERO*/)
                            v25 = 1;
                    }
                    if (a7 && map_item->objectType == 98/*MAPOBJECT_TOWN*/ || v25)
                    {
                        //color = WordAt(PtrAt(PtrAt(this_ + 4 * (map_item->land) + 96) + 32) + 44);
                        color = this_->terrainDef[map_item->land]->palette565->color[22];

                        if (x != h_x || y != h_y)
                        {
                            if (map_item->objectType == 34/*MAPOBJECT_HERO*/ && map_item->access & 0x10)
                            {
                                color = P_GameMgrPalette16->color[P_Game->GetHero(map_item->setup)->owner + 64];
                            }
                            else
                            {
                                H3MapItem* mi;
                                _dword_ rot = map_item->GetRealObjectType();

                                //if (rot == OBJ_DECORATIVE)
                                //{
                                //    if (DecorObjects[map_item->os_type].ShowOnMinimap())
                                //    {
                                //        if (!(map_item->mirror & 0x40))
                                //            color = WordAt(PtrAt(PtrAt(this_ + 4 * (map_item->land) + 96) + 32) + 46);
                                //    }
                                //}

                                //if (rot == OBJ_PUZZLE)
                                //{
                                //    if (PuzzleObjects[map_item->os_type].ShowOnMinimap())
                                //    {
                                //        if (!(map_item->mirror & 0x40))
                                //            color = WordAt(PtrAt(PtrAt(this_ + 4 * (map_item->land) + 96) + 32) + 46);
                                //    }
                                //}

                                switch (rot)
                                {
                                case 116:
                                case 118:
                                case 119:
                                case 121:
                                case 123:
                                case 126:
                                case 128:
                                case 131:
                                case 133:
                                case 134:
                                case 135:
                                case 137:
                                case 148:
                                case 149:
                                case 152:
                                case 153:
                                case 154:
                                case 155:
                                case 158:
                                case 159:
                                case 160:
                                    if (!(map_item->mirror & 0x40))
                                        //color = WordAt(PtrAt(PtrAt(this_ + 4 * (map_item->land) + 96) + 32) + 46);
                                        color = this_->terrainDef[map_item->land]->palette565->color[23];
                                    break;
                                case 98/*MAPOBJECT_TOWN*/:
                                    if (!(map_item->mirror & 0x40) || map_item->access & 0x10)
                                    {
                                        mi = map_item->GetEntrance();
                                        if (mi)
                                            color = P_GameMgrPalette16->color[64 + P_Game->GetTown(mi->GetRealSetup())->owner];
                                    }
                                    break;
                                case 42:
                                case 53:
                                    if (!(map_item->mirror & 0x40) || map_item->access & 0x10)
                                    {
                                        mi = map_item->GetEntrance();
                                        if (mi)
                                            //color = P_GameMgrPalette16->color[64 + *(INT8*)(P_Game->Field<_ptr_>(0x4E38C)
                                            //    + (mi->GetRealSetup() << 6))];
                                            color = P_GameMgrPalette16->color[64 + *(INT8*)(P_Game->minesLighthouses.First()
                                                + (mi->GetRealSetup() << 6))];
                                    }
                                    break;
                                case 20:
                                case 17:
                                    if (!(map_item->mirror & 0x40) || map_item->access & 0x10)
                                    {
                                        mi = map_item->GetEntrance();
                                        if (mi)
                                            /*color = P_GameMgrPalette16->color[64 + *(INT8*)(P_Game->Field<_ptr_>(0x4E39C)
                                                + 92 * mi->GetRealSetup() + 87)];*/
                                            color = P_GameMgrPalette16->color[64 + *(INT8*)(P_Game->dwellings.First()
                                                + 92 * mi->GetRealSetup() + 87)];
                                    }
                                    break;
                                case 33:
                                    if (!(map_item->mirror & 0x40) || map_item->access & 0x10)
                                    {
                                        mi = map_item->GetEntrance();
                                        if (mi)
                                            /*color = P_GameMgrPalette16->color[64 + *(INT8*)(P_Game->Field<_ptr_>(0x4E3AC)
                                                + (mi->GetRealSetup() << 6))];*/
                                            color = P_GameMgrPalette16->color[64 + *(INT8*)(P_Game->garrisons.First()
                                                + (mi->GetRealSetup() << 6))];
                                    }
                                    break;
                                case 87:
                                    if (!(map_item->mirror & 0x40) || map_item->access & 0x10)
                                    {
                                        mi = map_item->GetEntrance();
                                        if (mi)
                                            color = P_GameMgrPalette16->color[64 + ((INT32)mi->GetRealSetup() << 24 >> 24)];
                                    }
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                        else
                        {
                            color = P_GameMgrPalette16->color[64 + hero->owner];
                        }
                    }
                    else
                    {
                        color = 0;
                    }

                    *minimap_ppixel = color;
                    minimap_ppixel++;
                } // for x
                minimap_ppixel += minimap_scan - P_MapWidth;
            } // for y

            DrawPcx16Resized(P_WindowManager->screenPcx16, pcx16_Minimap, P_MapWidth, P_MapHeight,
                mm_x, mm_y, mm_w, mm_h);

            //draw selection rect

            int radar_w, radar_h;
            float cell_sz;

            if (P_ViewingWorldEarthAirNow)
                cell_sz = P_ViewWorldEarthAirMapCellSize;
            else
                cell_sz = 32;

            float cs_w, cs_h;

            cs_w = (float)world_view_width / (float)(int)cell_sz;
            cs_h = (float)world_view_height / (float)(int)cell_sz;
            radar_w = (int)((cs_w * 144.0) / (float)P_MapWidth + 0.5);
            radar_h = (int)((cs_h * 144.0) / (float)P_MapHeight + 0.5);

            if ((radar_w < 288) || (radar_h < 288))
            {
                radar_w = std::min<int>(radar_w, 288);
                radar_h = std::min<int>(radar_h, 288);

                if (!P_ViewingWorldEarthAirNow)
                    ConstructMinimapSelectionRect(radar_w, radar_h);

                //int radar_pos_x = b_unpack_x(radar_xyz);
                //int radar_pos_y = b_unpack_y(radar_xyz);
                int radar_pos_x = radar_xyz.GetX();
                int radar_pos_y = radar_xyz.GetY();
                int dst_x;
                int dst_y;
                int src_x;
                int src_y;

                if (radar_pos_x >= 0)
                {
                    dst_x = radar_pos_x * 144 / P_MapWidth;
                    src_x = 0;
                }
                else
                {
                    dst_x = 0;
                    src_x = -radar_pos_x * 144 / P_MapWidth;
                    radar_w -= src_x;
                }

                if (radar_pos_y >= 0)
                {
                    dst_y = radar_pos_y * 144 / P_MapHeight;
                    src_y = 0;
                }
                else
                {
                    dst_y = 0;
                    src_y = -radar_pos_y * 144 / P_MapHeight;
                    radar_h -= src_y;
                }

                radar_w = std::min<int>(radar_w, 144 - dst_x);
                radar_h = std::min<int>(radar_h, 144 - dst_y);

                if (P_ViewingWorldEarthAirNow)
                {
                    radar_w = std::min<int>(radar_w, 144);
                    radar_h = std::min<int>(radar_h, 144);
                    //if ((radar_w < 144) || (radar_h < 144))
                    ConstructMinimapSelectionRect(radar_w, radar_h);
                }

                dst_x += mm_x;
                dst_y += mm_y;

                pcx8_Radar->DrawToPcx16(src_x, src_y, radar_w, radar_h, P_WindowManager->screenPcx16, dst_x, dst_y, 1);
            }
        }
    }
    if (redraw_screen)
        P_WindowManager->H3Redraw(mm_x, mm_y, mm_w, mm_h);

}






_LHF_(LoHook_CalcPixelsPerMinimapCell_40A077)
{
    *(float*)(c->ebp - 8) = 144.0 / (double)P_MapHeight;
    c->return_address = 0x40A0AD;
    return NO_EXEC_DEFAULT;
}

_LHF_(LoHook_CalcPixelsPerMinimapCell_5FD156)
{
    *(float*)(c->ebp + 8) = 144.0 / (double)P_MapHeight;
    c->return_address = 0x5FD187;
    return NO_EXEC_DEFAULT;
}

_LHF_(LoHook_ViewWorldMinimapPart_5FCCD6)
{
    c->eax = 144;
    return NO_EXEC_DEFAULT;
}

_LHF_(LoHook_ViewWorldMinimapPart_5FCCEE)
{
    c->edi = 144;
    return NO_EXEC_DEFAULT;
}
#endif




//void __stdcall HiHook_00412BA0(HiHook* h, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
//{
//    H3RGB565* v8; // edi
//    void(__thiscall * v9)(int); // eax
//    char Src[20]; // [esp+1Ch] [ebp-18h] BYREF
//
//    v8 = &P_GameMgrPalette16->color[63];
//    MemCopy(Src, &P_GameMgrPalette16->color[63], 0x12u);
//    MemCopy(v8, &dword_104A6964, 0x12u);
//    v9 = (void(__thiscall*)(int))dword_104A747C->vTable->GetDefaultFunc(dword_104A747C, a3, a4, a5, a6, a7, a8);
//    v9(a2);
//    MemCopy(v8, Src, 0x12u);
//}









// Обработка кнопок выбора размера в создании диалога выбора карты.
_LHF_(LoHook_MapSelectDlg_Create_SizeButtons)
{
    // Список элементов диалога.
    H3Vector<H3DlgItem*>* items_lst = reinterpret_cast<H3Vector<H3DlgItem*>*>(c->edi);

    // H-карты.
    items_lst->Append(H3DlgDefButton::Create(211, 52, 41, 33, HMAP_DLGITEM_ID, "SCHGBUT.def", 0, 1, 0, 0));

    // XH-карты.
    items_lst->Append(H3DlgDefButton::Create(258, 52, 41, 33, XHMAP_DLGITEM_ID, "SCXHBUT.def", 0, 1, 0, 0));

    // G-карты.
    items_lst->Append(H3DlgDefButton::Create(305, 52, 41, 33, GMAP_DLGITEM_ID, "SCGTBUT.def", 0, 1, 0, 0));

    return EXEC_DEFAULT;
}

// Обработка кнопок выбора размера в создании части для случайных карт диалога выбора карты.
void __stdcall HiHook_MapSelectDlg_RandMapDlgCreate(HiHook* h, H3SelectScenarioDialog* this_)
{
    // HD_WOG is loaded after ERA's initialization events in current HD builds.
    // Install XXL's HD size compatibility before the downstream dialog code
    // restores and reconciles the saved random-map size.
    XXLRuntimeFix_EnsureHdCompatibility();

    // Оригинальные элементы.
    THISCALL_1(void, h->GetDefaultFunc(), this_);

    // Диалог.
    H3Vector<H3DlgItem*>* items_lst = &this_->GetList();

    // Генерация H-карты.
    items_lst->Append(H3DlgDefButton::Create(230, 81, 41, 33, HRMAP_DLGITEM_ID, "RanSizHG.def", 0, 1, 0, 0));

    // Генерация XH-карты.
    items_lst->Append(H3DlgDefButton::Create(273, 81, 41, 33, XHRMAP_DLGITEM_ID, "RanSizXH.def", 0, 1, 0, 0));

    // Генерация G-карты.
    items_lst->Append(H3DlgDefButton::Create(316, 81, 41, 33, GRMAP_DLGITEM_ID, "RanSizGT.def", 0, 1, 0, 0));

    // Match the available sizes to the active RMG template.  This runs after
    // the three XXL controls exist, while HD's own validator (when present)
    // has already restored a valid saved selection.
    XXLRuntimeFix_RefreshTemplateSizes(this_, true);

    //byte_104A779C = 0;
    //byte_104A7440 = 0;
}

// Refresh template-dependent availability only when inputs to the mask can
// change. Size and Underground clicks keep the same union mask; native code
// already redraws their selected/highlight frames.
int __stdcall HiHook_MapSelectDlg_Proc_TemplateSizes(HiHook* h,
    H3SelectScenarioDialog* this_, H3Msg* msg)
{
    const bool refreshAfter = msg && msg->command == eMsgCommand::MOUSE_BUTTON &&
        msg->subtype == eMsgSubtype::LBUTTON_CLICK &&
        (msg->itemId == 130 ||
         (msg->itemId >= 326 && msg->itemId <= 329));
    XXLRuntimeFix_BeforeScenarioDialogMessage(this_, msg);
    const int result = THISCALL_2(int, h->GetDefaultFunc(), this_, msg);
    if (refreshAfter)
        XXLRuntimeFix_AfterScenarioDialogMessage(this_);
    return result;
}

// Обработка кнопок выбора размера в показе спика карт.
_LHF_(LoHook_MapSelectDlg_MapListShow_SizeButtons)
{
    // Диалог.
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->esi);

    // Элемент диалога.
    H3DlgItem* item;

    // Должны ли элементы быть доступными.
    bool need_enabled = (!P_NetworkGame 
        || !DirectPlayCom()
        || THISCALL_1(INT8, *(_ptr_*)(*(_ptr_*)DirectPlayCom() + 144), DirectPlayCom())
        || SaveGameMode(dlg));

    // H-карты.
    item = dlg->GetH3DlgItem(HMAP_DLGITEM_ID);
    if (item)
    {
        // Показываем и устанавливаем доступность элементу.
        item->SendCommand(5, 6);
        item->EnableItem(need_enabled);
    }

    // XH-карты.
    item = dlg->GetH3DlgItem(XHMAP_DLGITEM_ID);
    if (item)
    {
        // Показываем и устанавливаем доступность элементу.
        item->SendCommand(5, 6);
        item->EnableItem(need_enabled);
    }

    // G-карты.
    item = dlg->GetH3DlgItem(GMAP_DLGITEM_ID);
    if (item)
    {
        // Показываем и устанавливаем доступность элементу.
        item->SendCommand(5, 6);
        item->EnableItem(need_enabled);
    }


    return EXEC_DEFAULT;
}

// Обработка новых элементов в скрытии спика карт.
void __stdcall LoHook_MapSelectDlg_MapListHide(HiHook* h, H3SelectScenarioDialog* this_)
{
    this_->RemoveControlState(HMAP_DLGITEM_ID, eControlState(6));
    this_->RemoveControlState(XHMAP_DLGITEM_ID, eControlState(6));
    this_->RemoveControlState(GMAP_DLGITEM_ID, eControlState(6));

    // Скрываем оригинальные элементы.
    THISCALL_1(void, h->GetDefaultFunc(), this_);
}


// Обработка кнопок выбора размера в показе меню настроек ГСК.
_LHF_(LoHook_MapSelectDlg_RMGShow_SizeButtons)
{
    // Диалог.
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->esi);

    dlg->AddControlState(HRMAP_DLGITEM_ID, eControlState(6));
    dlg->AddControlState(XHRMAP_DLGITEM_ID, eControlState(6));
    dlg->AddControlState(GRMAP_DLGITEM_ID, eControlState(6));

    return EXEC_DEFAULT;
}


// Обработка кнопок выбора размера в скрытии меню настроек ГСК.
_LHF_(LoHook_MapSelectDlg_RMGHide_SizeButtons)
{
    // Диалог.
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->esi);

    dlg->RemoveControlState(HRMAP_DLGITEM_ID, eControlState(6));
    dlg->RemoveControlState(XHRMAP_DLGITEM_ID, eControlState(6));
    dlg->RemoveControlState(GRMAP_DLGITEM_ID, eControlState(6));

    return EXEC_DEFAULT;
}


// Перерисовка новых кнопок выбора размера карты вместе со всеми.
_LHF_(LoHook_MapSelectDlg_SizeButtonsRedraw)
{
    // Перерисовываем новые кнопки.
    THISCALL_4(void, PtrAt(PtrAt(c->ebx) + 20), c->ebx, 0, HMAP_DLGITEM_ID, GMAP_DLGITEM_ID);

    return EXEC_DEFAULT;
}


// Обработка нажатия кнопок в меню выбора карты.
_LHF_(LoHook_MapSelectDlg_ButtonClick)
{
    // Диалог.
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);

    // Сообщение на обновление дефа размера карты в правом верхнем углу
    //H3Msg* msg = reinterpret_cast<H3Msg*>(c->esi);
    H3Msg msg;
    msg.command = eMsgCommand::MOUSE_BUTTON;
    msg.subtype = eMsgSubtype::SET_FRAME;
    msg.itemId = 189;
    int itemId = c->eax;
    int mapDimension = 0;

    if (itemId >= 3000)
    {
        // Номер элемента.
        switch (itemId)
        {
            // Фильтр H-карт.
        case HMAP_DLGITEM_ID:
            THISCALL_2(void, 0x586060, dlg, 180);
            c->return_address = 0x5879B0;
            return NO_EXEC_DEFAULT;
            break;

            // Фильтр XH-карт.
        case XHMAP_DLGITEM_ID:
            THISCALL_2(void, 0x586060, dlg, 216);
            c->return_address = 0x5879B0;
            return NO_EXEC_DEFAULT;
            break;

            // Фильтр G-карт.
        case GMAP_DLGITEM_ID:
            THISCALL_2(void, 0x586060, dlg, 252);
            c->return_address = 0x5879B0;
            return NO_EXEC_DEFAULT;
            break;

            // Генерация H-карты.
        case HRMAP_DLGITEM_ID:
            // Устанавливаем размер.
            mapDimension = 180;
            dlg->mapDimension = mapDimension;
            THISCALL_1(void, 0x57F240, dlg);
            dlg->mapInfo.mapDimension = mapDimension;
            P_Game->mapInfo.mapDimension = mapDimension;
            msg.parameter = reinterpret_cast<void*>(4);
            SendMouseMsgCompat(dlg, &msg);
            // Выход с перерисовкой.
            c->return_address = 0x587277;
            return NO_EXEC_DEFAULT;
            break;

            // Генерация XH-карты.
        case XHRMAP_DLGITEM_ID:
            // Устанавливаем размер.
            mapDimension = 216;
            dlg->mapDimension = mapDimension;
            THISCALL_1(void, 0x57F240, dlg);
            dlg->mapInfo.mapDimension = mapDimension;
            P_Game->mapInfo.mapDimension = mapDimension;
            msg.parameter = reinterpret_cast<void*>(5);
            SendMouseMsgCompat(dlg, &msg);
            // Выход с перерисовкой.
            c->return_address = 0x587277;
            return NO_EXEC_DEFAULT;
            break;

            // Генерация G-карты.
        case GRMAP_DLGITEM_ID:
            // Устанавливаем размер.
            mapDimension = 252;
            dlg->mapDimension = mapDimension;
            THISCALL_1(void, 0x57F240, dlg);
            dlg->mapInfo.mapDimension = mapDimension;
            P_Game->mapInfo.mapDimension = mapDimension;
            msg.parameter = reinterpret_cast<void*>(6);
            SendMouseMsgCompat(dlg, &msg);
            // Выход с перерисовкой.
            c->return_address = 0x587277;
            return NO_EXEC_DEFAULT;
            break;

        default:
            return EXEC_DEFAULT;
            break;
        }
    }

    else
    {
        if (itemId < 281)
        {
            return EXEC_DEFAULT;
        }   
        if (itemId > 284)
        {
            if (itemId == 285)
            {
                bool v4 = dlg->mapInfo.hasUnderground == 0;
                dlg->mapInfo.hasUnderground = v4;
                P_Game->mapInfo.hasUnderground = v4;
            }
            return EXEC_DEFAULT;
        }
        dlg->mapInfo.mapDimension = 4 * (9 * itemId - 2520);
        P_Game->mapInfo.mapDimension = 36 * (c->eax - 280);
        msg.parameter = reinterpret_cast<void*>(c->eax - 281);
        SendMouseMsgCompat(dlg, &msg);
        return EXEC_DEFAULT;
    }

    return EXEC_DEFAULT;
}


// Установка подсказок в диалоге выбора карты для новых элементов.
void __stdcall HiHook_MapSelectDlg_SetTips(HiHook* h, H3SelectScenarioDialog* this_, DWORD* tips, INT32 start_item_id, INT32 end_item_id, BOOL8 alloc_mem)
{
    // Устанавливаем подсказки стандартным элементам.
    THISCALL_5(void, h->GetDefaultFunc(), this_, tips, start_item_id, end_item_id, alloc_mem);

    // Элемент диалога.
    H3DlgItem* item;

    // Деф размера карты в правом верхнем углу
    item = this_->GetH3DlgItem(189);
    if (item) SetRmcHint(item, alloc_mem);

    // H-карты.
    item = this_->GetH3DlgItem(HMAP_DLGITEM_ID);
    if (item) SetRmcHint(item, alloc_mem);

    // XH-карты.
    item = this_->GetH3DlgItem(XHMAP_DLGITEM_ID);
    if (item) SetRmcHint(item, alloc_mem);

    // G-карты.
    item = this_->GetH3DlgItem(GMAP_DLGITEM_ID);
    if (item) SetRmcHint(item, alloc_mem);

    // Генерация H-карты.
    item = this_->GetH3DlgItem(HRMAP_DLGITEM_ID);
    if (item) SetRmcHint(item, alloc_mem);

    // Генерация XH-карты.
    item = this_->GetH3DlgItem(XHRMAP_DLGITEM_ID);
    if (item) SetRmcHint(item, alloc_mem);

    // Генерация G-карты.
    item = this_->GetH3DlgItem(GRMAP_DLGITEM_ID);
    if (item) SetRmcHint(item, alloc_mem);
}


// Настройка состояния кнопок размера настроек случайной карты.
_LHF_(LoHook_MapSelectDlg_RmgSetSizeButtonsState)
{
    // Диалог выбора карты.
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->esi);

    // Делаем кнопки новых размеров ненажатыми.
    dlg->RemoveControlState(HRMAP_DLGITEM_ID, eControlState::SELECTED);
    dlg->RemoveControlState(XHRMAP_DLGITEM_ID, eControlState::SELECTED);
    dlg->RemoveControlState(GRMAP_DLGITEM_ID, eControlState::SELECTED);

    switch (c->eax)
    {
        // H-карта.
    case 180:
        // Выделяем кнопку как нажатую.
        dlg->AddControlState(HRMAP_DLGITEM_ID, eControlState::SELECTED);
        // Пропускаем стандартное выделение.
        c->return_address = 0x57F2B6;
        return NO_EXEC_DEFAULT;
        break;

        // XH-карта.
    case 216:
        // Выделяем кнопку как нажатую.
        dlg->AddControlState(XHRMAP_DLGITEM_ID, eControlState::SELECTED);
        // Пропускаем стандартное выделение.
        c->return_address = 0x57F2B6;
        return NO_EXEC_DEFAULT;
        break;

        // G-карта.
    case 252:
        // Выделяем кнопку как нажатую.
        dlg->AddControlState(GRMAP_DLGITEM_ID, eControlState::SELECTED);
        // Пропускаем стандартное выделение.
        c->return_address = 0x57F2B6;
        return NO_EXEC_DEFAULT;
        break;

    default:
        return EXEC_DEFAULT;
        break;
    }

    return EXEC_DEFAULT;
}


// Настройка иконки размера карты: выбор сценария кампании.
_LHF_(LoHook_MapSelectDlg_SizeIcon_CampScenario)
{
    // Размер карты.
    switch (c->esi)
    {
        // S-карта.
    case 36:
        IntAt(c->ebp - 20) = 0;
        break;

        // M-карта.
    case 72:
        IntAt(c->ebp - 20) = 1;
        break;

        // L-карта.
    case 108:
        IntAt(c->ebp - 20) = 2;
        break;

        // XL-карта.
    case 144:
        IntAt(c->ebp - 20) = 3;
        break;

        // H-карта.
    case 180:
        IntAt(c->ebp - 20) = 4;
        break;

        // XH-карта.
    case 216:
        IntAt(c->ebp - 20) = 5;
        break;

        // G-карта.
    case 252:
        IntAt(c->ebp - 20) = 6;
        break;

    default:
        IntAt(c->ebp - 20) = 7;
        break;
    }

    // Пропускаем стандартную настройку.
    c->return_address = 0x457895;
    return NO_EXEC_DEFAULT;
}


// Настройка иконки размера карты: информация о сценарии.
_LHF_(LoHook_MapSelectDlg_SizeIcon_ScenarioInfo)
{
    // Размер карты.
    switch (c->eax)
    {
        // S-карта.
    case 36:
        c->eax = 0;
        break;

        // M-карта.
    case 72:
        c->eax = 1;
        break;

        // L-карта.
    case 108:
        c->eax = 2;
        break;

        // XL-карта.
    case 144:
        c->eax = 3;
        break;

        // H-карта.
    case 180:
        c->eax = 4;
        break;

        // XH-карта.
    case 216:
        c->eax = 5;
        break;

        // G-карта.
    case 252:
        c->eax = 6;
        break;

    default:
        c->eax = 7;
        break;
    }

    // Пропускаем стандартную настройку.
    c->return_address = 0x56950F;
    return NO_EXEC_DEFAULT;
}


// Настройка иконки размера карты: выбор карты.
_LHF_(LoHook_MapSelectDlg_SizeIcon_SelectMap)
{
    // Размер карты.
    switch (c->eax)
    {
        // S-карта.
    case 36:
        IntAt(c->ebp - 8) = 0;
        break;

        // M-карта.
    case 72:
        IntAt(c->ebp - 8) = 1;
        break;

        // L-карта.
    case 108:
        IntAt(c->ebp - 8) = 2;
        break;

        // XL-карта.
    case 144:
        IntAt(c->ebp - 8) = 3;
        break;

        // H-карта.
    case 180:
        IntAt(c->ebp - 8) = 4;
        break;

        // XH-карта.
    case 216:
        IntAt(c->ebp - 8) = 5;
        break;

        // G-карта.
    case 252:
        IntAt(c->ebp - 8) = 6;
        break;

    default:
        IntAt(c->ebp - 8) = 7;
        break;
    }

    // Пропускаем стандартную настройку.
    c->return_address = 0x5858D5;
    return NO_EXEC_DEFAULT;
}


// Настройка иконки размера карты: выбор карты - сеть.
_LHF_(LoHook_MapSelectDlg_SizeIcon_SelectMapOnline)
{
    // Размер карты.
    switch (c->eax)
    {
        // S-карта.
    case 36:
        IntAt(c->ebp - 8) = 0;
        break;

        // M-карта.
    case 72:
        IntAt(c->ebp - 8) = 1;
        break;

        // L-карта.
    case 108:
        IntAt(c->ebp - 8) = 2;
        break;

        // XL-карта.
    case 144:
        IntAt(c->ebp - 8) = 3;
        break;

        // H-карта.
    case 180:
        IntAt(c->ebp - 8) = 4;
        break;

        // XH-карта.
    case 216:
        IntAt(c->ebp - 8) = 5;
        break;

        // G-карта.
    case 252:
        IntAt(c->ebp - 8) = 6;
        break;

    default:
        IntAt(c->ebp - 8) = 7;
        break;
    }

    // Пропускаем стандартную настройку.
    c->return_address = 0x585AF6;
    return NO_EXEC_DEFAULT;
}


// Настройка иконки размера карты: фильтр карт.
_LHF_(LoHook_MapSelectDlg_SizeIcon_FilterMaps)
{
    // Восстанавливаем затёртые команды.
    IntAt(c->ebp - 56) = c->ecx;
    IntAt(c->ebp - 52) = c->ecx;
    IntAt(c->ebp - 48) = c->ecx;
    IntAt(c->ebp - 40) = c->ecx;
    IntAt(c->ebp - 68) = 0x200;
    IntAt(c->ebp - 60) = 189;
    IntAt(c->ebp - 64) = c->edx;

    // Размер карты.
    switch (c->eax)
    {
        // S-карта.
    case 36:
        IntAt(c->ebp - 44) = 0;
        break;

        // M-карта.
    case 72:
        IntAt(c->ebp - 44) = 1;
        break;

        // L-карта.
    case 108:
        IntAt(c->ebp - 44) = 2;
        break;

        // XL-карта.
    case 144:
        IntAt(c->ebp - 44) = 3;
        break;

        // H-карта.
    case 180:
        IntAt(c->ebp - 44) = 4;
        break;

        // XH-карта.
    case 216:
        IntAt(c->ebp - 44) = 5;
        break;

        // G-карта.
    case 252:
        IntAt(c->ebp - 44) = 6;
        break;

    default:
        IntAt(c->ebp - 44) = 7;
        break;
    }

    // Пропускаем стандартную настройку.
    c->return_address = 0x5861C5;
    return NO_EXEC_DEFAULT;
}


_LHF_(LoHook_MapSelectDlg_SizeAbbr)
{
    int mapSize = c->esi;

    if (mapSize == MAPSIZE_H || mapSize == MAPSIZE_XH || mapSize == MAPSIZE_G)
    {
        libc::sprintf(h3_TextBuffer, COLUMN_MAP_SIZE_KEY, mapSize);
        LPCSTR rmcHint = EraJS::read(h3_TextBuffer);
        c->edi = (_ptr_)rmcHint;
        c->return_address = 0x584ACC;
        return NO_EXEC_DEFAULT;
    }

    return EXEC_DEFAULT;
}


_LHF_(LoHook_00584ac6)
{
    sprintf_s(h3_TextBuffer, 0x300u, "%d", c->esi);
    c->edi = (_ptr_)h3_TextBuffer;
    return 0;
}


_LHF_(LoHook_00584b0d)
{
    char x = ByteAt(c->ebp - 0x7C);
    if (x < 48 || x > 57)
    {
        return 1;
    }
        
    c->ecx = 0x698A54;
    return 0;
}


// При вводе чита открываем всю карту.
void __stdcall HiHook_Cheat_OpenMap(HiHook* h, H3Game* this_, INT32 x, INT32 y, INT32 z, INT32 player_id, INT32 radius, BOOL8 unk)
{
    // Открываем всю карту.
    THISCALL_7(void, h->GetDefaultFunc(),
        this_,
        MapWidth() / 2,
        MapHeight() / 2,
        z,
        player_id,
        IntAt(4 * (MapWidth() < MapHeight()) + 0x6783C8), //std::max<INT32>(P_MapWidth, P_MapHeight),
        unk);
}


void __stdcall HiHook_0040275b(HiHook* h, H3Game* this_, int x, int y, int z, int player_owner, int radius)
{
    THISCALL_6(void, h->GetDefaultFunc(),
        this_,
        MapWidth() / 2,
        MapHeight() / 2,
        z,
        player_owner,
        IntAt(4 * (MapWidth() < MapHeight()) + 0x6783C8)); //std::max<INT32>(P_MapWidth, P_MapHeight)
}


// Отправляем сообщение при нажатии на изменение размера карты в настройках ГСК.
_LHF_(LoHook_MapSelectDlg_RmgSizeButtonClick_OnlineMsg)
{
    // Размер карты.
    switch (c->eax)
    {
        // ГСК-размеры карт.
    case 281:
    case 282:
    case 283:
    case 284:
    case 285:
    case HRMAP_DLGITEM_ID:
    case XHRMAP_DLGITEM_ID:
    case GRMAP_DLGITEM_ID:
        // Отправляем сообщение.
        c->return_address = 0x5868E6;

        return NO_EXEC_DEFAULT;
        break;

        // Иначе по-умолчанию.
    default:
        return EXEC_DEFAULT;
        break;
    }
}


_LHF_(LoHook_005806d9)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->esi);
    int param = 0;

    // Размер карты.
    switch (dlg->mapDimension)
    {
        // S-карта.
    case 36:
        param = 0;
        break;

        // M-карта.
    case 72:
        param = 1;
        break;

        // L-карта.
    case 108:
        param = 2;
        break;

        // XL-карта.
    case 144:
        param = 3;
        break;

        // H-карта.
    case 180:
        param = 4;
        break;

        // XH-карта.
    case 216:
        param = 5;
        break;

        // G-карта.
    case 252:
        param = 6;
        break;

    default:
        param = 7;
        break;
    }

    H3Msg msg;
    msg.command = eMsgCommand::MOUSE_BUTTON;
    msg.subtype = eMsgSubtype::SET_FRAME;
    msg.itemId = 189;
    msg.parameter = reinterpret_cast<void*>(param);
    SendMouseMsgCompat(dlg, &msg);

    return EXEC_DEFAULT;
}


#if 0
// Experimental network/UI hooks below were not installed by MapSize_Init and
// rely heavily on the missing private header.  Excluding them is behavior-neutral.
_LHF_(LoHook_0057fa9d)
{
    H3Msg msg;
    msg.command = eMsgCommand::MOUSE_BUTTON;
    msg.subtype = eMsgSubtype::SET_FRAME;
    msg.itemId = 189;  // кадр дефа размера выбранной карты
    msg.parameter = 7; // все карты

    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    dlg->SendMouseMsg(&msg);

    return EXEC_DEFAULT;
}


// Встроен в HD_WOG
//_LHF_(LoHook_005844a9)
//{
//    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
//
//    if (dlg->showRandomMaps)
//    {
//        c->eax = (int)&dlg->randomMapsList[0];
//    }   
//    else
//    {
//        c->eax = (int)&dlg->mapsList[0];
//    }
//    return NO_EXEC_DEFAULT;
//}


// Встроен в HD_WOG
//_LHF_(LoHook_0058447f)
//{
//    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
//    
//    if (!dlg->randomMapGeneration)
//    {
//        return EXEC_DEFAULT;
//    }
//        
//    c->return_address = 0x584540;
//    return NO_EXEC_DEFAULT;
//}















////////////////////////////////////////////////////////////


_LHF_(LoHook_005843f3)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    if (!dlg->randomMapGeneration)            // H3SelectScenarioDialog->randomMapGeneration
    {
        return EXEC_DEFAULT;
    }
      
    c->return_address = 0x5843FE;
    return NO_EXEC_DEFAULT;
}


_LHF_(LoHook_00585914)
{
    H3Msg msg;
    msg.command = eMsgCommand::MOUSE_BUTTON;
    msg.subtype = eMsgSubtype::SET_FRAME;
    msg.itemId = 189;  // кадр дефа размера выбранной карты
    msg.parameter = 7; // все карты
    
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    dlg->SendMouseMsg(&msg);

    for (int i = 0; i < 8; ++i)
    {
        P_Game->playersInfo.playerType[i] = -1;
        H3DlgItem* dlgItem = dlg->GetH3DlgItem(i + 112);
        dlgItem->SendCommand(6, 6);
        H3DlgItem* dlgItem1 = dlg->GetH3DlgItem(i + 120);
        dlgItem1->SendCommand(6, 6);
    }
    P_Game->playersInfo.difficulty = DifficultyLevel;

    return EXEC_DEFAULT;
}


_LHF_(LoHook_0058580a)
{
    if (c->eax >= c->edx)          // eax - mapIndex edx - size
    {
        H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
        if (dlg->randomMapGeneration)
        {
            c->return_address = 0x585812;
            return NO_EXEC_DEFAULT;
        }
        THISCALL_2(void, 0x5BAA90, dlg->mapDescScroll, 0x691260);
        H3Msg msg;
        msg.command = eMsgCommand::MOUSE_BUTTON;
        msg.subtype = eMsgSubtype::SET_FRAME;
        msg.itemId = 189;
        msg.parameter = 7;
        dlg->SendMouseMsg(&msg);

        for (int i = 0; i < 8; ++i)
        {
            P_Game->playersInfo.playerType[i] = -1;
            H3DlgItem* dlgItem = dlg->GetH3DlgItem(i + 112);
            dlgItem->SendCommand(6, 6);
            H3DlgItem* dlgItem1 = dlg->GetH3DlgItem(i + 120);
            dlgItem1->SendCommand(6, 6);
        }
        P_Game->playersInfo.difficulty = DifficultyLevel;
        H3DlgItem* v7 = dlg->GetH3DlgItem(186);
        v7->EnableItem(0);
    }

    return EXEC_DEFAULT;
}


_LHF_(LoHook_00583afc)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    if (!P_Game->mapInfo.mapVersion)
    {
        P_Game->playersInfo.playerType[0] = -1;
        P_Game->playersInfo.playerType[1] = -1;
        P_Game->playersInfo.playerType[2] = -1;
        P_Game->playersInfo.playerType[3] = -1;
        P_Game->playersInfo.playerType[4] = -1;
        P_Game->playersInfo.playerType[5] = -1;
        P_Game->playersInfo.playerType[6] = -1;
        P_Game->playersInfo.playerType[7] = -1;
        THISCALL_2(void, 0x5BAA90, dlg->mapDescScroll, 0x691260);
    }

    return EXEC_DEFAULT;
}

_LHF_(LoHook_0058396c)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    if (!dlg->saveGameMode)
    {
        P_Game->playersInfo.playerType[0] = -1;
        P_Game->playersInfo.playerType[1] = -1;
        P_Game->playersInfo.playerType[2] = -1;
        P_Game->playersInfo.playerType[3] = -1;
        P_Game->playersInfo.playerType[4] = -1;
        P_Game->playersInfo.playerType[5] = -1;
        P_Game->playersInfo.playerType[6] = -1;
        P_Game->playersInfo.playerType[7] = -1;
        THISCALL_2(void, 0x5BAA90, dlg->mapDescScroll, 0x691260);
    }
    return EXEC_DEFAULT;
}

void __stdcall HiHook_005806d4(HiHook* h, H3SelectScenarioDialog* this_)
{
    if (P_NetworkGame
        && P_DirectPlayCom
        && !THISCALL_1(INT8, *(_ptr_*)(*(_ptr_*)P_DirectPlayCom + 144), P_DirectPlayCom))
    {
        this_->randomMapGeneration = 1;
    }
    THISCALL_1(void, h->GetDefaultFunc(), this_);
}


_LHF_(LoHook_00578e79)
{
    if (!c->edx) // numSent
    {
        H3NewPlayerUpdateProc* proc = reinterpret_cast<H3NewPlayerUpdateProc*>(c->ebx);
        
        if (!proc->numSent)
        {
            proc->numSent = 1;
            c->return_address = 0x57912C;
        }
    }
    return EXEC_DEFAULT;
}

_LHF_(LoHook_005781b0)
{
    if (c->edx)  // numSent
    {
        return EXEC_DEFAULT;
    }
        
    H3NewPlayerUpdateProc* proc = reinterpret_cast<H3NewPlayerUpdateProc*>(c->ebx);
    if (proc->numSent)
    {
        return EXEC_DEFAULT;
    }
        
    proc->numSent = 1;
    c->return_address = 0x57833E;
    return NO_EXEC_DEFAULT;
}


_LHF_(LoHook_00583861)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    if (!dlg->randomMapGeneration)
    {
        return EXEC_DEFAULT;
    }
        
    c->return_address = 0x583896;
    return NO_EXEC_DEFAULT;
}













/////////////////////////////////////////////////////////////


void __stdcall HiHook_005843C0(HiHook* h, H3SelectScenarioDialog* this_)
{
    if (this_->randomMapGeneration || this_->saveGameMode)
    {
        THISCALL_1(void, h->GetDefaultFunc(), this_);
    }
    else
    {
        int selectedMapIndex = this_->selectedMapIndex;
        int size = this_->currentMapsList.Size();
        if (selectedMapIndex >= 0 && selectedMapIndex < size)
        {
            if (this_->currentMapsList[selectedMapIndex].mapVersion)
            {
                THISCALL_1(void, h->GetDefaultFunc(), this_);
            }
        }
    }
}



/////////////////////////////////////////////////////////////

_LHF_(LoHook_00583403)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    int v4 = dlg->currentMapsList.Size();
    int v5 = dlg->selectedMapIndex;

    if (v5 >= 0 && v5 < v4)
    {
        return EXEC_DEFAULT;
    }
        
    c->return_address = 0x583424;
    return NO_EXEC_DEFAULT;
}




















//void __stdcall HiHook_00583e10(HiHook* h, H3SelectScenarioDialog* this_)
//{
//    mapsListPcx->vTable->VDeref(mapsListPcx);
//    mapsListPcx = 0;
//    folderPcx->vTable->VDeref(folderPcx);
//    folderPcx = 0;
//    THISCALL_1(void, h->GetDefaultFunc(), this_);
//}


_LHF_(LoHook_00583023)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->edi);
    
    if (!dlg->loadGameMode && !dlg->saveGameMode)
    {
        return EXEC_DEFAULT;
    }
    
    H3GameSelectionHeadersStruct* header = *reinterpret_cast<H3GameSelectionHeadersStruct**>(c->ebp - 0xCBC);
    int cnt = 0;
    
    if (!header->header.deadPlayer[0] && header->playerAttributes[0].humanPlayable)
    {
        cnt = 1;
    }  
    if (!header->header.deadPlayer[1] && header->playerAttributes[1].humanPlayable)
    {
        ++cnt;
    }
    if (!header->header.deadPlayer[2] && header->playerAttributes[2].humanPlayable)
    {
        ++cnt;
    }
    if (!header->header.deadPlayer[3] && header->playerAttributes[3].humanPlayable)
    {
        ++cnt;
    }
    if (!header->header.deadPlayer[4] && header->playerAttributes[4].humanPlayable)
    {
        ++cnt;
    }
    if (!header->header.deadPlayer[5] && header->playerAttributes[5].humanPlayable)
    {
        ++cnt;
    }
    if (!header->header.deadPlayer[6] && header->playerAttributes[6].humanPlayable)
    {
        ++cnt;
    }
    if (!header->header.deadPlayer[7])
    {
        if (header->playerAttributes[7].humanPlayable)
        {
            ++cnt;
        }   
    }
    if (cnt >= 2)
    {
        return EXEC_DEFAULT;
    }
        
    c->return_address = 0x583040;
    return NO_EXEC_DEFAULT;
}


_LHF_(LoHook_00580138)
{
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->edi);
    
    int size = dlg->currentMapsList.Size();
    int mapIndex = dlg->selectedMapIndex;
    
    if (mapIndex >= 0 && mapIndex < size || dlg->saveGameMode)
    {
        c->return_address = 0x580179;
        return 0;
    }
    else
    {
        c->return_address = 0x580164;
        return 0;
    }
}

#endif


_LHF_(LoHook_00584470)
{
    UINT32 mapIndex = -1;
    int mapDimension = 0;
    bool emptyMapSize = true;
    H3SelectScenarioDialog* dlg = reinterpret_cast<H3SelectScenarioDialog*>(c->ebx);
    H3Vector<H3ScenarioMapInformation>& maps = CurrentMapsList(dlg);
    int size = maps.Size();

    if (RandomMapGeneration(dlg))
    {
        mapDimension = dlg->mapDimension;
    LABEL_10:
        emptyMapSize = mapDimension <= 0;
        goto LABEL_11;
    }
    mapIndex = dlg->selectedMapIndex;

    if (mapIndex < 0
        || size == 0
        || mapIndex >= (size-1)
        || !maps[mapIndex].mapVersion
        || (mapDimension = maps[mapIndex].mapDimension, emptyMapSize = mapDimension <= 0, !mapDimension))
    {
        if (!SaveGameMode(dlg))
        {
            return 1;
        }
            
        mapDimension = P_Game->mapInfo.mapDimension;
        goto LABEL_10;
    }
LABEL_11:
    if (!emptyMapSize)
    {
        sprintf_s(h3_TextBuffer, 0x300u, "%dx%d", mapDimension, mapDimension);
        P_TinyFont->TextDraw(
            P_WindowManager->screenPcx16, 
            h3_TextBuffer, 
            dlg->GetX() + 712,
            dlg->GetY() + 55, 35, 16, eTextColor(4), eTextAlignment(5));
    }

    return EXEC_DEFAULT;
}


_LHF_(LoHook_00567a0b)
{
    H3SelectScenarioDialog* dlg = *reinterpret_cast<H3SelectScenarioDialog**>(c->ebp - 0x34);
    sprintf_s(h3_TextBuffer, 0x300u, "%dx%d", P_Game->mapInfo.mapDimension, P_Game->mapInfo.mapDimension);
    dlg->AddItem(H3DlgText::Create(709, 49, 35, 16, h3_TextBuffer, "tiny.fnt", 4, 100, 5, 0), false);
    
    return EXEC_DEFAULT;
}


void __stdcall HiHook_00579CE0(HiHook* h, H3SelectScenarioDialog* this_, int gameMode)
{
    THISCALL_2(void, h->GetDefaultFunc(), this_, gameMode);
    int mapSize = P_Game->mapInfo.mapDimension;
    sprintf_s(h3_TextBuffer, 0x300u, "%dx%d", mapSize, mapSize);
    this_->AddItem(H3DlgText::Create(709, 49, 35, 16, h3_TextBuffer, "tiny.fnt", 4, 100, 5, 0), false);
}


_LHF_(LoHook_00586103)
{
    H3ScenarioMapInformation* mapInfo = (H3ScenarioMapInformation*)(c->ecx + c->ebx);
    int filterSize = c->eax;
    int mapDimension = mapInfo->mapDimension;
    
    if (!mapInfo->mapVersion)
    {
        c->return_address = 0x586109;
        return NO_EXEC_DEFAULT;
    }   
    if (mapDimension > filterSize)
    {
        if (filterSize == 252)
        {
            c->return_address = 0x586109;
            return NO_EXEC_DEFAULT;
        }
    }
    else if (mapDimension > filterSize - 36)
    {
        c->return_address = 0x586109;
        return NO_EXEC_DEFAULT;
    }
    c->return_address = 0x58611C;
    return NO_EXEC_DEFAULT;
}


_LHF_(LoHook_005854be)
{
    H3ScenarioMapInformation* mapInfo = (H3ScenarioMapInformation*)(c->ebx + c->esi);
    int filterSize = c->eax;
    int mapDimension = mapInfo->mapDimension;
    
    if (!mapInfo->mapVersion)
    {
        c->return_address = 0x5854C4;
        return NO_EXEC_DEFAULT;
    }
    if (mapDimension > filterSize)
    {
        if (filterSize == 252)
        {
            c->return_address = 0x5854C4;
            return NO_EXEC_DEFAULT;
        }
    }
    else if (mapDimension > filterSize - 36)
    {
        c->return_address = 0x5854C4;
        return NO_EXEC_DEFAULT;
    }
    c->return_address = 0x5854DC;
    return NO_EXEC_DEFAULT;
}





















// Удаление мусора из имени рандом карты
void __stdcall HiHook_RemoveJunkLettersFromRandomMapHeader(HiHook* h, H3String* this_, LPCSTR mapName, int len)
{
    THISCALL_3(void, h->GetDefaultFunc(), this_, mapName, len);
    this_->Remove("\x12");
    this_->Remove("\x1");
    this_->Remove("\x2");
}

// При показе списка карт обновляем выбранную карту (баг SoD с пропаданием выбора игрока).
void __stdcall HiHook_MapList_UpdateMap(HiHook* h, H3SelectScenarioDialog* this_, char a2)
{
    THISCALL_2(void, h->GetDefaultFunc(), this_, a2);
    THISCALL_3(void, 0x5857D0, this_, this_->selectedMapIndex, false);
}

// Корректно обновляем кадр дефа размера карты при переключении на вкладку случайных карт
void __stdcall HiHook_00580D40(HiHook* h, H3SelectScenarioDialog* this_, int randMaps)
{
    THISCALL_2(void, h->GetDefaultFunc(), this_, randMaps);

    if (InScenarioOptions(this_))
    {
        THISCALL_3(void, 0x5857D0, this_, this_->selectedMapIndex, 0); // UpdateNewGameMap
    }
}








// Отрисовываем в правом верхнем углу имя шаблона вместо "Random Map"
LPCSTR __stdcall HiHook_005844311(HiHook* h, H3SelectScenarioDialog* this_, int selectedMapIndex)
{
    LPCSTR hdModRmgTxtPath = globalPatcher->VarGetValue<LPCSTR>("HD.File.RMG", "Default value");
    return THISCALL_2(LPCSTR, h->GetDefaultFunc(), this_, selectedMapIndex);
}

void MapSize_Init()
{
    // Отрисовываем в правом верхнем углу имя шаблона вместо "Random Map"
    //_PI->WriteHiHook(0x5842E0, SPLICE_, EXTENDED_, THISCALL_, HiHook_005844311);










    //***
    // хд миникарта
    //if (globalPatcher->VarGetValue<int>("HD.BPP", 16) == 16)
    //{
    //    _PI->WriteHiHook(0x412BA0, SPLICE_, EXTENDED_, THISCALL_, HiHook_AdvMgr_DrawMinimap);
    //    _PI->WriteLoHook(0x40A077, LoHook_CalcPixelsPerMinimapCell_40A077);
    //    _PI->WriteLoHook(0x5FD156, LoHook_CalcPixelsPerMinimapCell_5FD156);
    //    _PI->WriteLoHook(0x5FCCD6, LoHook_ViewWorldMinimapPart_5FCCD6);
    //    _PI->WriteLoHook(0x5FCCEE, LoHook_ViewWorldMinimapPart_5FCCEE);
    //}
    //***

    // Палитра цветов игроков?
    //_PI->WriteHiHook(0x412BA0, SPLICE_, EXTENDED_, THISCALL_, HiHook_00412BA0);

    // Обработка кнопок выбора размера в создании диалога выбора карты.
    _PI->WriteLoHook(0x57C734, LoHook_MapSelectDlg_Create_SizeButtons);

    // Обработка кнопок выбора размера в создании части для случайных карт диалога выбора карты.
    _PI->WriteHiHook(0x57D440, SPLICE_, EXTENDED_, THISCALL_, HiHook_MapSelectDlg_RandMapDlgCreate);

    // Обработка кнопок выбора размера в показе спика карт.
    _PI->WriteLoHook(0x5810BB, LoHook_MapSelectDlg_MapListShow_SizeButtons);

    // Обработка новых элементов в скрытии спика карт.
    _PI->WriteHiHook(0x581C70, SPLICE_, EXTENDED_, THISCALL_, LoHook_MapSelectDlg_MapListHide);

    // Обработка кнопок выбора размера в показе меню настроек ГСК.
    _PI->WriteLoHook(0x58023D, LoHook_MapSelectDlg_RMGShow_SizeButtons);

    // Обработка кнопок выбора размера в скрытии меню настроек ГСК.
    _PI->WriteLoHook(0x582102, LoHook_MapSelectDlg_RMGHide_SizeButtons);

    // Перерисовка новых кнопок выбора размера карты вместе со всеми.
    _PI->WriteLoHook(0x585D12, LoHook_MapSelectDlg_SizeButtonsRedraw);

    // Обработка нажатия кнопок в меню выбора карты.
    _PI->WriteLoHook(0x586921, LoHook_MapSelectDlg_ButtonClick);

    // Re-evaluate the no-HD template mask after template/water inputs change.
    _PI->WriteHiHook(0x587FD0, SPLICE_, EXTENDED_, THISCALL_,
        HiHook_MapSelectDlg_Proc_TemplateSizes);

    // Установка подсказок в диалоге выбора карты для новых элементов.
    _PI->WriteHiHook(0x57C99D, CALL_, EXTENDED_, THISCALL_, HiHook_MapSelectDlg_SetTips);
    
    // Настройка состояния кнопок размера настроек случайной карты.
    _PI->WriteLoHook(0x57F276, LoHook_MapSelectDlg_RmgSetSizeButtonsState);

    // Настройка иконки размера карты: выбор сценария кампании.
    _PI->WriteLoHook(0x45785B, LoHook_MapSelectDlg_SizeIcon_CampScenario);

    // Настройка иконки размера карты: информация о сценарии.
    _PI->WriteLoHook(0x5694DB, LoHook_MapSelectDlg_SizeIcon_ScenarioInfo);

    // Настройка иконки размера карты: выбор карты.
    _PI->WriteLoHook(0x58589B, LoHook_MapSelectDlg_SizeIcon_SelectMap);

    // Настройка иконки размера карты: выбор карты - сеть.
    _PI->WriteLoHook(0x585AB8, LoHook_MapSelectDlg_SizeIcon_SelectMapOnline);

    // Настройка иконки размера карты: фильтр карт.
    _PI->WriteLoHook(0x58616E, LoHook_MapSelectDlg_SizeIcon_FilterMaps);

    // Добавление в столбец размера карт новых элементов.
    _PI->WriteLoHook(0x584A7D, LoHook_MapSelectDlg_SizeAbbr);

    // Не нужен, т.к. и так всё без проблем парсится в предыдущем хуке
    //_PI->WriteLoHook(0x584ac6, LoHook_00584ac6);
    
    // Какой-то финт с отрисовкой 
    //_PI->WriteLoHook(0x584b0d, LoHook_00584b0d);

    // Сдвиаем иконку сортировки по количеству игроков на пиксель влево.
    _PI->WriteByte(0x57AFD5 + 1, 25);

    // Меняем координаты и размеры кнопок карт.

    // Фильтр S.
    _PI->WriteDword(0x57AE22 + 1, 23);
    _PI->WriteByte(0x57AE1E + 1, 41);

    // Фильтр M.
    _PI->WriteDword(0x57AE79 + 1, 70);
    _PI->WriteByte(0x57AE75 + 1, 41);

    // Фильтр L.
    _PI->WriteDword(0x57AED0 + 1, 117);
    _PI->WriteByte(0x57AECC + 1, 41);

    // Фильтр XL.
    _PI->WriteDword(0x57AF27 + 1, 164);
    _PI->WriteByte(0x57AF23 + 1, 41);

    // Показать все карты.
    _PI->WriteDword(0x57AF7E + 1, 351);

    // Генерация S.
    _PI->WriteDword(0x57D539 + 1, 58);
    _PI->WriteByte(0x57D535 + 1, 41);

    // Генерация M.
    _PI->WriteDword(0x57D593 + 1, 101);
    _PI->WriteByte(0x57D58F + 1, 41);

    // Генерация L.
    _PI->WriteDword(0x57D5ED + 1, 144);
    _PI->WriteByte(0x57D5E9 + 1, 41);

    // Генерация XL.
    _PI->WriteDword(0x57D647 + 1, 187);
    _PI->WriteByte(0x57D643 + 1, 41);

    // Генерация с подземкой / без неё.
    _PI->WriteDword(0x57D6A1 + 1, 359);
    _PI->WriteByte(0x57D69D + 1, 34);

    // Убираем надпись о размере в меню выбора карты.
    _PI->WriteCodePatch(0x584909, "%n", 54); // 54 nop

    // Убираем надпись о размере в меню случайных карт.

    // Создание элемента.
    _PI->WriteCodePatch(0x57D49B, "%n", 2); // 2 nop
    _PI->WriteCodePatch(0x57D4A6, "%n", 70); // 70 nop
    _PI->WriteCodePatch(0x57D4EF, "%n", 27); // 27 nop

    // Открытие настроек ГСК.
    _PI->WriteCodePatch(0x5801CA, "%n", 23); // 23 nop

    // Скрытие настроек ГСК.
    _PI->WriteCodePatch(0x58208F, "%n", 23); // 23 nop

    // Убираем показ текста о создании случайной карты.
    _PI->WriteCodePatch(0x584E35, "%n", 54); // 54 nop

    // При вводе чита открываем всю карту.
    _PI->WriteHiHook(0x402704, CALL_, EXTENDED_, THISCALL_, HiHook_Cheat_OpenMap); // Чит
    _PI->WriteHiHook(0x4F4B61, CALL_, EXTENDED_, THISCALL_, HiHook_Cheat_OpenMap); // Чит-меню
    _PI->WriteHiHook(0x40275b, CALL_, EXTENDED_, THISCALL_, HiHook_0040275b);

    // Отправляем сообщение при нажатии на изменение размера карты в настройках ГСК.
    _PI->WriteLoHook(0x5868CC, LoHook_MapSelectDlg_RmgSizeButtonClick_OnlineMsg);

    // Корректно обновляем кадр дефа размера карты при переключении на вкладку случайных карт
    _PI->WriteLoHook(0x5806d9, LoHook_005806d9);

    // Корректно обновляем кадр дефа размера карты при нетворке
    //_PI->WriteLoHook(0x57fa9d, LoHook_0057fa9d);
    //_PI->WriteLoHook(0x57f688, LoHook_0057fa9d);

    // Подставляем корректную карту в зависимости от вкладки
    // ВЫКЛ: встроен в HD_WOG
    //_PI->WriteLoHook(0x5844a9, LoHook_005844a9);

    // Подставляем корректную карту в зависимости от вкладки
    // ВЫКЛ: встроен в HD_WOG
    //_PI->WriteLoHook(0x58447f, LoHook_0058447f);

    // Подставляем корректную карту в зависимости от вкладки
    //_PI->WriteLoHook(0x5843f3, LoHook_005843f3);

    // Для нетворка, если в сценарии mapIndex == -1
    //_PI->WriteLoHook(0x585914, LoHook_00585914);

    // Если индекс карты выше сайза вектора
    //_PI->WriteLoHook(0x58580a, LoHook_0058580a);

    // Загрузка сценария - если mapVersion = 0
    //_PI->WriteLoHook(0x583afc, LoHook_00583afc);

    // Если индекс карты -1 и не сохранение
    //_PI->WriteLoHook(0x58396c, LoHook_0058396c);

    // Случайная карта, прелоад инфы
    //_PI->WriteHiHook(0x5806d4, CALL_, EXTENDED_, THISCALL_, HiHook_005806d4);

    // UpdateNewGameMap - убираем проверку на mapchanged для нетворка
    //_PI->WriteCodePatch(0x585e3f, "%n", 14);

    // Вырезаем в конструкторе диалога:
    //  if ( sub_0058EDA0(&v2->currentMapsList.init) || v2->saveGameMode )
    //      sub_00586330(v2);
    //_PI->WriteJmp(0x57c8be, 0x57c8d4);

    // Хз что это, что-то для нетворка
    //_PI->WriteLoHook(0x578e79, LoHook_00578e79);
    //_PI->WriteLoHook(0x5781b0, LoHook_005781b0);

    // MapPreLoadInfo - прыжок для randomMapGeneration
    //_PI->WriteLoHook(0x583861, LoHook_00583861);

    // Перед отправкой нетворк даты рандомных карт?
    // esp и говно какое-то
    //LoHook_00580df3
    //LoHook_00580dd0
    //HiHook_00578cb0
    //LoHook_00579af7
    //LoHook_0058a805

    // Отрисовка списка карт с новыми PCX-ами списка карт и папки
    //mapsListPcx = 0;
    //folderPcx = 0;
    //LoHook_005849BB

    // Страхуемся от выхода за диапазон карт?
    //_PI->WriteHiHook(0x5843C0, SPLICE_, EXTENDED_, THISCALL_, HiHook_005843C0);

    // Что-то с версиями карт и активацией длг итемов
    //_PI->WriteHiHook(0x5857d0, SPLICE_, EXTENDED_, THISCALL_, HiHook_005857d0);
    //HiHook_00579CE0

    // Проверка за выход за рейнж массива с картами
    //_PI->WriteLoHook(0x583403, LoHook_00583403);

    //
    //_PI->WriteJmp(0x583339, 0x583360);

    // Проверки на selectedMapIndex != -1
    //LoHook_0058e2ba
    //LoHook_0058b789
    //LoHook_00585bf2
    
    // Для всяких хоткеев
    //LoHook_00588161
    //0x5881c7 LoHook_00588161
    //LoHook_005882DF
    //0x588344 LoHook_005882DF
    //_PI->WriteHiHook(0x58832c, CALL_, EXTENDED_, THISCALL_, HiHook_0058832c);
    //_PI->WriteHiHook(0x588422, CALL_, EXTENDED_, THISCALL_, HiHook_00588422);
    
    // Подтягивание карт
    //_PI->WriteLoHook(0x585441, LoHook_00585441);

    //
    //_PI->WriteHiHook(0x580dfd, CALL_, EXTENDED_, THISCALL_, HiHook_00580dfd);

    // Что-то с хендлом, ноп стандартного
    //_PI->WriteHiHook(0x582e10, SPLICE_, EXTENDED_, THISCALL_, HiHook_00582e10);
    //_PI->WriteCodePatch(0x57c8b9, "%n", 5);
    //_PI->WriteJmp(0x57c91b, 0x57c950);

    // опять с новыми PCX
    //HiHook_MapSelectDlgCreate






    ////////////////////



    // Деструктор PCX-ов списка карт и папки
    //_PI->WriteHiHook(0x583e10, SPLICE_, EXTENDED_, THISCALL_, HiHook_00583e10);

    // Проверяем доступных для игры на карте игроков
    //_PI->WriteLoHook(0x583023, LoHook_00583023);

    // Вырезаем кнопку "Начать", если вектор карт пуст
    //_PI->WriteLoHook(0x580138, LoHook_00580138);

    // Отрисовка текста про размер карты под иконкой размера - при отрисовке общих (на правой стороне экрана) атрибутов карты
    _PI->WriteLoHook(0x584470, LoHook_00584470);

    // Отрисовка текста про размер карты под иконкой размера - клик в игре на "просмотр информации о сценарии"
    _PI->WriteLoHook(0x567a0b, LoHook_00567a0b);

    // Фильтрование карт по размеру по кнопке размера
    _PI->WriteLoHook(0x586103, LoHook_00586103);
    // Фильтрование карт кнопками столбцов по игрокам/размеру/версии игры/названии/целям при активном фильтре по кнопке размера
    _PI->WriteLoHook(0x5854be, LoHook_005854be);

    // Меняем формат даты и времени
    _PI->WriteDword(0x584503, "%02d.%02d.%d, %d:%02d");

    // Говно какое-то
    //LoHook_005844fc








    //////////////
    //////////////
    // Мои хуки

    // В баг-фиксах hd-мода - Удаление мусора из имени рандом карты
    //_PI->WriteHiHook(0x54A264, CALL_, EXTENDED_, THISCALL_, HiHook_RemoveJunkLettersFromRandomMapHeader);

    // В баг-фиксах - При показе списка карт обновляем выбранную карту (баг SoD с пропаданием выбора игрока).
    //_PI->WriteHiHook(0x587272, CALL_, EXTENDED_, THISCALL_, HiHook_MapList_UpdateMap);

    // Корректно обновляем кадр дефа размера карты при переключении на вкладку случайных карт
    _PI->WriteHiHook(0x580D40, SPLICE_, EXTENDED_, THISCALL_, HiHook_00580D40);

}
