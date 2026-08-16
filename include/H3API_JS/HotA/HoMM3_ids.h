#pragma once



//// Копирование памяти с автоматическим приведением типов.
//#define MemCopy(dst, src, size) memcpy((void*)(dst), (void*)(src), (size_t)(size))
//// Обнуление памяти с автоматическим приведением типов.
//#define MemZero(ptr, size) MemSet(ptr, 0, size)
//// Выделение памяти с автоприведением и возвращением встроенного типа.
//#define MemAlloc(size) ((_ptr_)malloc((size_t)(size)))
//
//
//
//#define nCastles 9










//// Town types
//#define TID_COVE 9

//// Creatures ids
//#define CID_CANNON      150 // РџСѓС€РєР°
//#define CID_SEA_DOG     151 // РњРѕСЂСЃРєРѕР№ РІРѕР»Рє
//#define CID_NYMPH       153 // РќРёРјС„Р°
//#define CID_OCEANID     154 // РћРєРµР°РЅРёРґР°
//#define CID_CREW_MATE   155 // РњР°С‚СЂРѕСЃ
//#define CID_SEAMAN      156 // Р“РѕР»РѕРІРѕСЂРµР·
//#define CID_PIRATE      157 // РџРёСЂР°С‚
//#define CID_CORSAIR     158 // РљРѕСЂСЃР°СЂ
//#define CID_STORMBIRD   159 // Р”СѓС… РѕРєРµР°РЅР°
//#define CID_ASSID       160 // РђСЃСЃРёРґР°
//#define CID_SEA_WITCH   161 // Р–СЂРёС†Р° РјРѕСЂСЏ
//#define CID_SORCERESS   162 // Р—Р°РєР»РёРЅР°С‚РµР»СЊРЅРёС†Р°
//#define CID_NIX         163 // РќРёРєСЃ
//#define CID_NIX_WARRIOR 164 // РќРёРєСЃ-РІРѕРёРЅ
//#define CID_SEA_SERPENT 165 // РњРѕСЂСЃРєРѕР№ Р·РјРµР№ 
//#define CID_HASPID      166 // РђСЃРїРёРґ

//// Heroes ids
//#define HID_CORKES     156 // РљРѕСЂРєРµСЃ
//#define HID_JEREMY     157 // Р”Р¶РµСЂРµРјРё
//#define HID_ILLOR      158 // РР»Р»РѕСЂ
//#define HID_DEREK      159 // Р”РµСЂРµРє
//#define HID_LEENA      160 // Р›РёРЅР°
//#define HID_ANABEL     161 // РђРЅР°Р±РµР»СЊ
//#define HID_CASSIOPEIA 162 // РљР°СЃСЃРёРѕРїРµСЏ
//#define HID_MIRIAM     163 // РњРёСЂРёР°Рј
//#define HID_KASMETRA   164 // РљР°СЃРјРµС‚СЂР°
//#define HID_EOVACIUS   165 // Р­РѕРІР°С†РёР№
//#define HID_SPINT      166 // РЎРїРёРЅС‚
//#define HID_ANDAL      167 // РђРЅРґР°Р»
//#define HID_MANFRED    168 // РњР°РЅС„СЂРµРґ
//#define HID_ZILAR      169 // Р—РёР»Р°СЂ
//#define HID_ASTRA      170 // РђСЃС‚СЂР°
//#define HID_DARGEM     171 // Р”Р°СЂРіРµРј
//#define HID_BIDLEY     172 // Р‘РёРґР»Рё
//#define HID_TARK       173 // РўР°СЂРє
//#define HID_ELMORE     174 // Р­Р»РјРѕСЂ
//#define HID_BEATRICE   175 // Р‘РµР°С‚СЂРёСЃ
//#define HID_KINKERIA   176 // РљРёРЅРєРµСЂРёСЏ
//#define HID_RANLOO     177 // Р Р°РЅР»Сѓ

// Artifact ids - с 171
#define AID_DIPLOMATS_CLOAK        171 // 141 +
#define AID_PEDANT_OF_REFLECTION   172 // 142 +
#define AID_IRONFIST_OF_THE_OGRE   173 // 143 +
//#define AID_CANNON                 146
#define AID_TRIDENT_OF_DOMINION    174 // 147 +
#define AID_SHIELD_OF_NAVAL_GLORY  175 // 148 +
#define AID_ROYAL_ARMOR_OF_NIX     176 // 149 +
#define AID_CROWN_OF_THE_FIVE_SEAS 177 // 150 +
#define AID_WAYFARERS_BOOTS        178 // 151 +
#define AID_RUNES_OF_IMMINENCY     179 // 152 +
#define AID_DEMONS_HORSESHOE       180 // 153 +
#define AID_SHAMANS_PUPPET         181 // 154 +
#define AID_HIDEOUS_MASK           182 // 155 +
#define AID_RING_OF_SUPPRESION     183 // 156 +
#define AID_PEDANT_OF_DOWNFALL     184 // 157 +
#define AID_RING_OF_OBLIVION       185 // 158 +
#define AID_CAPE_OF_SILENCE        186 // 159 +
#define AID_GOLDEN_GOOSE           187 // 160 +
#define AID_HORN_OF_THE_ABYSS      188 // 161 +
#define AID_CHARM_OF_ECLIPSE       189 // 162 +
#define AID_SEAL_OF_SUNSET         190 // 163 +
#define AID_PLATE_OF_DYING_LIGHT   191 // 164 +
#define AID_SLEEPKEEPER            192 // 165 +



// Флаги новых модификаторов героев.
#define HFM_TEMPLE_OF_LOYALITY_BONUS 0x00000001 // 0 - Есть бонус объединения мировоззрений Храма Лояльности
#define HFM_CHURCHYARD_PENALTY       0x00000002 // 1 - Есть штраф -1 к боевому духу от Погоста

#define HFM_WATERING_PLACE_VISITED   0x00000004 // посещен водопой на этот ход

// 0x00000100 - на глобалке
#define HFM_ON_AIRSHIP               0x00000200 // герой в дирижабле


// Флаги существ и стеков в бою.
#define CF_2HEX_WIDE         0x00000001 // Занимает 2 гекса
#define CF_CAN_FLY           0x00000002 // Летает
#define CF_CAN_SHOOT         0x00000004 // Cтреляет
#define CF_2HEX_ATTACK       0x00000008 // Драконье дыхание
#define CF_ALIVE             0x00000010 // Живое существо
#define CF_CAN_ATTACK_WALL   0x00000020 // Может стрелять по стенам
#define CF_SIEGE_WEAPON      0x00000040 // Боевая машина
#define CF_KING_1            0x00000080 // King 1 (базовый Палач и выше)
#define CF_KING_2            0x00000100 // King 2 (продвинутый Палач и выше)
#define CF_KING_3            0x00000200 // King 3 (экспертный Палач)
#define CF_IMMUNE_TO_MIND    0x00000400 // Ииммунитет к магии разума
#define CF_SHOT_BY_RAY       0x00000800 // Вместо анимации выстрела непрерывный прямой луч
#define CF_NO_MELEE_PENALTY  0x00001000 // Нет штрафа при рукопашной
#define CF_NOT_USED          0x00002000 // Не используется
#define CF_IMMUNE_TO_FIRE    0x00004000 // Иммунитет к магии огня
#define CF_ATTACK_TWICE      0x00008000 // Атакует дважды
#define CF_UNREQ_ATTACK      0x00010000 // Враг не отвечает на атаку
#define CF_NO_MORALE_PENALTY 0x00020000 // Нет влияния боевого духа
#define CF_UNDEAD            0x00040000 // Нежить
#define CF_ROUND_ATTACK      0x00080000 // Атакует всех врагов вокруг
#define CF_EXT_SHOT_RANGE    0x00100000 // Расширенный радиус стрельбы (для ИИ)
#define CF_DEAD              0x00200000 // Стек мёртв (для стека в бою)
#define CF_WAS_SUMMONED      0x00400000 // Стек был призван, исчезнет при гибели или в конце боя (для стека в бою)
#define CF_CLONE             0x00800000 // Является клоном (для стека в бою)
#define CF_HAD_GOOD_MORALE   0x01000000 // При текущем действии у стека сработал положительный боевой дух (для стека в бою)
#define CF_WAITED            0x02000000 // Стек уже ожидал в этом раунде (для стека в бою)
#define CF_ACTED             0x04000000 // Стек уже действовал в этом раунде (для стека в бою)
#define CF_DEFENSE_POSITION  0x08000000 // Стоит в обороне (для стека в бою)
#define CF_PERMAMENTLY_DEAD  0x10000000 // Стек безвозвратно мёртв, нет трупа, не может быть воскрешён (для стека в бою)
#define CF_IS_RED            0x20000000 // Окрашен в красный цвет, наложение Жажды Крови (для стека в бою)
#define CF_IS_GRAY           0x40000000 // Окрашен в серый цвет, Окаменение (для стека в бою)
#define CF_IS_DRAGON         0x80000000 // Является драконом


// Стороны в бою.
#define ATTACKER 0 // Атакующий
#define DEFENDER 1 // Защищающийся

