#pragma once

//https://github.com/reactos/reactos/blob/master/sdk/include/reactos/drivers/bootvid/display.h
//https://github.com/reactos/reactos/blob/master/sdk/include/reactos/drivers/bootvid/bootvid.h

#include <wdm.h>

#define BV_COLOR_BLACK          0
#define BV_COLOR_RED            1
#define BV_COLOR_GREEN          2
#define BV_COLOR_BROWN          3
#define BV_COLOR_BLUE           4
#define BV_COLOR_MAGENTA        5
#define BV_COLOR_CYAN           6
#define BV_COLOR_DARK_GRAY      7
#define BV_COLOR_LIGHT_GRAY     8
#define BV_COLOR_LIGHT_RED      9
#define BV_COLOR_LIGHT_GREEN    10
#define BV_COLOR_YELLOW         11
#define BV_COLOR_LIGHT_BLUE     12
#define BV_COLOR_LIGHT_MAGENTA  13
#define BV_COLOR_LIGHT_CYAN     14
#define BV_COLOR_WHITE          15
#define BV_COLOR_NONE           16
#define BV_MAX_COLORS           16

NTKERNELAPI BOOLEAN VidInitialize(BOOLEAN SetMode);
NTKERNELAPI VOID  	VidResetDisplay(BOOLEAN HalReset);
NTKERNELAPI ULONG   VidSetTextColor(ULONG Color);
NTKERNELAPI VOID  	VidDisplayStringXY(PUCHAR String, ULONG Left, ULONG Top, BOOLEAN Transparent);
NTKERNELAPI VOID  	VidSetScrollRegion(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom);
NTKERNELAPI VOID  	VidCleanUp(VOID);
NTKERNELAPI VOID  	VidBufferToScreenBlt(PUCHAR Buffer, ULONG Left, ULONG Top, ULONG Width, ULONG Height, ULONG Delta);
NTKERNELAPI VOID  	VidDisplayString(PUCHAR String);
NTKERNELAPI ULONG   VidBitBlt(PVOID pbuff, ULONG x, ULONG y);
NTKERNELAPI VOID  	VidScreenToBufferBlt(PUCHAR Buffer, ULONG Left, ULONG Top, ULONG Width, ULONG Height, ULONG Delta);
NTKERNELAPI VOID  	VidSolidColorFill(ULONG Left, ULONG Top, ULONG Right, ULONG Bottom, UCHAR Color);
