//#pragma once
//
//
//#define NOALIGN __declspec(align(1))
//
//
//////////////////////////////////////// IDA TYPES ///////////////////////////////////////
//
//
////typedef          __int64 ll;
////typedef unsigned __int64 ull;
////
////
////typedef          char   int8;
////typedef   signed char   sint8;
////typedef unsigned char   uint8;
////typedef          short  int16;
////typedef   signed short  sint16;
////typedef unsigned short  uint16;
////typedef          int    int32;
////typedef   signed int    sint32;
////typedef unsigned int    uint32;
////typedef ll              int64;
////typedef ll              sint64;
////typedef ull             uint64;
//
//// Partially defined types.
//#define _BYTE  UINT8
//#define _WORD  UINT16
//#define _DWORD UINT32
//#define _QWORD UINT64
//// Some convenience macros to make partial accesses nicer
//#define LAST_IND(x,part_type)    (sizeof(x)/sizeof(part_type) - 1)
//#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN
//#  define LOW_IND(x,part_type)   LAST_IND(x,part_type)
//#  define HIGH_IND(x,part_type)  0
//#else
//#  define HIGH_IND(x,part_type)  LAST_IND(x,part_type)
//#  define LOW_IND(x,part_type)   0
//#endif
//// first unsigned macros:
//#define BYTEn(x, n)   (*((_BYTE*)&(x)+n))
//#define WORDn(x, n)   (*((_WORD*)&(x)+n))
//#define DWORDn(x, n)  (*((_DWORD*)&(x)+n))
//
//#define LOBYTE(x)  BYTEn(x,LOW_IND(x,_BYTE))
//#define LOWORD(x)  WORDn(x,LOW_IND(x,_WORD))
//#define LODWORD(x) DWORDn(x,LOW_IND(x,_DWORD))
//#define HIBYTE(x)  BYTEn(x,HIGH_IND(x,_BYTE))
//#define HIWORD(x)  WORDn(x,HIGH_IND(x,_WORD))
//#define HIDWORD(x) DWORDn(x,HIGH_IND(x,_DWORD))
//#define BYTE1(x)   BYTEn(x,  1)         // byte 1 (counting from 0)
//
//
//// Generate a pair of operands. S stands for 'signed'
////#define __SPAIR16__(high, low)  (((INT16)  (high) <<  8) | (UINT8) (low))
////#define __SPAIR32__(high, low)  (((INT32)  (high) << 16) | (UINT16)(low))
//#define __SPAIR64__(high, low)  (((INT64)  (high) << 32) | (UINT32)(low))
////#define __SPAIR128__(high, low) (((int128) (high) << 64) | (UINT64)(low))
////#define __PAIR16__(high, low)   (((uint16) (high) <<  8) | (UINT8) (low))
////#define __PAIR32__(high, low)   (((uint32) (high) << 16) | (UINT16)(low))
//#define __PAIR64__(high, low)   (((UINT64) (high) << 32) | (UINT32)(low))
////#define __PAIR128__(high, low)  (((uint128)(high) << 64) | (UINT64)(low))
//#define SWORDn(x, n)   (*((INT16*)&(x)+n))
//#define SHIWORD(x)  SWORDn(x,HIGH_IND(x,INT16))
//
//inline UINT32  abs32(INT32   x) { return x >= 0 ? x : -x; }
