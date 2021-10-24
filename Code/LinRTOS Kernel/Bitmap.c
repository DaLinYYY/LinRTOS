/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-21
 * @LastEditTime: 2021-07-26 22:38:50
 * @Description: 
 */

#include "Bitmap.h"
#include "Tasks.h"



void vBitmapInit(Bitmap_t * bitmap)
{
    bitmap->bitmapValue.Val = 0;
}


void vBitmapSet(Bitmap_t * bitmap, uint32_t pos)
{
    bitmap->bitmapValue.Val |= 1 << pos;
}

uint32_t uBitmapGetCount(void)
{
    return 32;
}


void vBitmapClear(Bitmap_t * bitmap, uint32_t pos)
{
    bitmap->bitmapValue.Val &= ~(1 << pos);
}


uint32_t bBitmapGet(Bitmap_t * bitmap, uint32_t pos)
{
    if (bitmap->bitmapValue.Val & (1 << pos) == 0)
        return 1;
    else 
        return 0;
}

/**
 * @Author: YangSL
 * @Description: 
 * @param {Bitmap_t *} bitmap
 */
uint32_t uBitmapGetFirstSet(Bitmap_t * bitmap)
{
#if 0
    uint32_t index;
    for( index = 0; index < 32 && (!(bitmap->bitmapVlaue & (0x01 << index))); index++);
    return index;
#else 
    static const uint8_t quickFindTable[] =     
	{
	    /* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	    /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
	};

    uint8_t index;
    for (index = 0; index < 4 && ( bitmap->bitmapValue.v[index] == 0); index++);
	
	return quickFindTable[bitmap->bitmapValue.v[index]] + (index << 3);

//	 if (bitmap->bitmapValue.Val & 0xff)
//	 {
//		 return quickFindTable[bitmap->bitmapValue.Val & 0xff];         
//	 }
//	 else if (bitmap->bitmapValue.Val & 0xff00)
//	 {
//		 return quickFindTable[(bitmap->bitmapValue.Val >> 8) & 0xff] + 8;        
//	 }
//	 else if (bitmap->bitmapValue.Val & 0xff0000)
//	 {
//		 return quickFindTable[(bitmap->bitmapValue.Val >> 16) & 0xff] + 16;        
//	 }
//	 else if (bitmap->bitmapValue.Val & 0xFF000000)
//	 {
//		 return quickFindTable[(bitmap->bitmapValue.Val >> 24) & 0xFF] + 24;
//	 }
//	 else
//	 {
//		 return uBitmapGetCount();
//     }

#endif 
}

