/*
 * @FileName: 
 * @Author: YangSL
 * @Date: 2021-07-21
 * @LastEditTime: 2021-07-28 18:08:33
 * @Description: 
 */

#ifndef _BIT_MAP_H
#define _BIT_MAP_H

#include <stdint.h>

typedef union
{
	uint32_t Val;
	uint16_t w[2];
    uint8_t  v[4];
    struct
    {
    	uint16_t LW;
    	uint16_t HW;
    } word;
    struct
    {
    	uint8_t LB;
    	uint8_t HB;
    	uint8_t UB;
    	uint8_t MB;
    } byte;
    struct
    {
    	uint16_t low;
    	uint16_t high;
    }wordUnion;
    struct
    {
         uint8_t b0:1;
         uint8_t b1:1;
         uint8_t b2:1;
         uint8_t b3:1;
         uint8_t b4:1;
         uint8_t b5:1;
         uint8_t b6:1;
         uint8_t b7:1;
         uint8_t b8:1;
         uint8_t b9:1;
         uint8_t b10:1;
         uint8_t b11:1;
         uint8_t b12:1;
         uint8_t b13:1;
         uint8_t b14:1;
         uint8_t b15:1;
         uint8_t b16:1;
         uint8_t b17:1;
         uint8_t b18:1;
         uint8_t b19:1;
         uint8_t b20:1;
         uint8_t b21:1;
         uint8_t b22:1;
         uint8_t b23:1;
         uint8_t b24:1;
         uint8_t b25:1;
         uint8_t b26:1;
         uint8_t b27:1;
         uint8_t b28:1;
         uint8_t b29:1;
         uint8_t b30:1;
         uint8_t b31:1;
    } bits;
} UINT32_VAL;

typedef struct  
{
    UINT32_VAL bitmapValue;
    uint8_t *bits;
}Bitmap_t;


void vBitmapInit(Bitmap_t * bitmap);
void vBitmapSet(Bitmap_t * bitmap, uint32_t pos);
void vBitmapClear(Bitmap_t * bitmap, uint32_t pos);
uint32_t uBitmapGetCount(void);
uint32_t bBitmapGet(Bitmap_t * bitmap, uint32_t pos);
uint32_t uBitmapGetFirstSet(Bitmap_t * bitmap);


#endif /* _BIT_MAP_H */
