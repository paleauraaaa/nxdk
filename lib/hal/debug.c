// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2004 Craig Edwards
// SPDX-FileCopyrightText: 2005 Robin Mulloy
// SPDX-FileCopyrightText: 2006 Richard Osborne
// SPDX-FileCopyrightText: 2019-2021 Stefan Schmidt
// SPDX-FileCopyrightText: 2019-2020 Jannik Vogel
// SPDX-FileCopyrightText: 2020 Bennet Blischke
// SPDX-FileCopyrightText: 2020 Matthew Lawson
// SPDX-FileCopyrightText: 2021 Ryan Wendland


//
//	Debug output scrolling code submitted by Robin Mulloy
//
//
#include <stdarg.h>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <hal/xbox.h>
#include <hal/video.h>

#include "debug.h"

#define MARGIN         25
#define MARGINS        50 // MARGIN*2

unsigned char *SCREEN_FB = NULL;
int SCREEN_WIDTH	= 0;
int SCREEN_HEIGHT	= 0;
int SCREEN_BPP = 0;

int nextRow = MARGIN;
int nextCol = MARGIN; 

static const unsigned char systemFont[] =
{
#include "font_unscii_16.h"
};

static void synchronizeFramebuffer(void)
{
	VIDEO_MODE vm = XVideoGetMode();
	SCREEN_WIDTH = vm.width;
	SCREEN_HEIGHT = vm.height;
	SCREEN_BPP = vm.bpp;
	SCREEN_FB = XVideoGetFB();
}

static void drawChar(unsigned char c, int x, int y, int fgColour, int bgColour)
{
	unsigned char *videoBuffer = SCREEN_FB;
	videoBuffer += (y * SCREEN_WIDTH + x) * ((SCREEN_BPP+7)/8);

	unsigned char mask;
	const unsigned char *font = systemFont + (c * ((FONT_WIDTH+7)/8) * FONT_HEIGHT);
	int colourToDraw;

	for (int h = 0; h < FONT_HEIGHT; h++)
	{
#if FONT_VMIRROR
		mask = 0x01;
#else
		mask = 0x80;
#endif

		// This loop draws up to 1 byte (8 bit) per line; so 8 pixels at most
#if FONT_WIDTH > 8
#error Font can't be wider than 8 pixels
#endif
		for (int w = 0; w < FONT_WIDTH; w++)
		{
			if ((*font) & mask)
			{
				colourToDraw = fgColour;
			}
			else
			{
				colourToDraw = bgColour;
			}
			switch (SCREEN_BPP)
			{
				case 32:
					*((int*)videoBuffer) = colourToDraw;
					videoBuffer += sizeof(int);
					break;
				case 16:
				case 15:
					*((short*)videoBuffer) = colourToDraw & 0xFFFF;
					videoBuffer += sizeof(short);
					break;
			}
#if FONT_VMIRROR
			mask <<= 1;
#else
			mask >>= 1;
#endif
		}
		
		videoBuffer += (SCREEN_WIDTH-FONT_WIDTH)  * ((SCREEN_BPP+7)/8);
		font++;
	}
}

// given an unsigned nibble (4 bits) between 0x00 to 0x0F, return '0' to 'F'
static char n2c(int byte)
{
	switch(byte)
	{
		case 0x00: return '0';
		case 0x01: return '1';
		case 0x02: return '2';
		case 0x03: return '3';
		case 0x04: return '4';
		case 0x05: return '5';
		case 0x06: return '6';
		case 0x07: return '7';
		case 0x08: return '8';
		case 0x09: return '9';
		case 0x0A: return 'A';
		case 0x0B: return 'B';
		case 0x0C: return 'C';
		case 0x0D: return 'D';
		case 0x0E: return 'E';
		case 0x0F: return 'F';
		default: return '?';
	}
}

void debugPrintNum(int i)
{
	char num[11];
	num[0] = '0';
	num[1] = 'x';
	num[2] = n2c((i >> 28) & 0x0F);
	num[3] = n2c((i >> 24) & 0x0F);
	num[4] = n2c((i >> 20) & 0x0F);
	num[5] = n2c((i >> 16) & 0x0F);
	num[6] = n2c((i >> 12) & 0x0F);
	num[7] = n2c((i >> 8) & 0x0F);
	num[8] = n2c((i >> 4) & 0x0F);
	num[9] = n2c((i >> 0) & 0x0F);
	num[10] = 0;
	debugPrint("%s", num);
}

void debugPrintBinary( int num )
{
   int x = 0;
   char binNum[50] = {0};
   for( int i=31;i>=0;i-- )
   {
	  binNum[x++] = 0x30 + ((num & (0x01 << i))?1:0);
	  if( (i%4) == 0 ) 
		 binNum[x++] = ' ';
   }
   binNum[x] = 0;
   debugPrint("%s", binNum);
}

void debugPrint(const char *format, ...)
{
	char buffer[512];
	unsigned short len;
	va_list argList;
	va_start(argList, format);
	vsnprintf(buffer, sizeof(buffer), format, argList);
	va_end(argList);

	synchronizeFramebuffer();

	int fgColour;
	int bgColour;
	switch (SCREEN_BPP) {
	case 32:
		fgColour = WHITE;
		bgColour = BLACK;
		break;
	case 16:
		fgColour = WHITE_16BPP;
		bgColour = BLACK_16BPP;
		break;
	case 15:
		fgColour = WHITE_15BPP;
		bgColour = BLACK_15BPP;
	}

	unsigned char *s = (unsigned char*)	buffer;
	while (*s)
	{
		if( nextRow >= (SCREEN_HEIGHT-MARGINS) ) {
			debugClearScreen();
			// debugAdvanceScreen();
		}
		
		if (*s == '\n')
		{
			nextRow += FONT_HEIGHT+1;
			nextCol = MARGIN;
		}
		else if (*s == '\r')
		{
			nextCol = MARGIN;
		}
		else
		{
			drawChar( *s, nextCol, nextRow, fgColour, bgColour );

			nextCol += FONT_WIDTH+1;
			if( nextCol > (SCREEN_WIDTH-MARGINS))
			{
				nextRow += FONT_HEIGHT+1;
				nextCol  = MARGIN;
			}
		}

		s++;
	}

	XVideoFlushFB();
}

void debugAdvanceScreen( void )
{
	synchronizeFramebuffer();

	int pixelSize = (SCREEN_BPP+7)/8;
	int screenSize  = SCREEN_WIDTH * (SCREEN_HEIGHT - MARGINS)  * pixelSize;
	int lineSize    = SCREEN_WIDTH * (FONT_HEIGHT + 1) * pixelSize;
	
	unsigned char* thisScreen = SCREEN_FB + (SCREEN_WIDTH * MARGIN)  * pixelSize;
	unsigned char* prevScreen = thisScreen+lineSize;
		
	memmove(thisScreen, prevScreen, screenSize);

	nextRow -= (FONT_HEIGHT+1);
	nextCol  = MARGIN; 

	XVideoFlushFB();
}

void debugClearScreen( void )
{
	synchronizeFramebuffer();

	memset( SCREEN_FB, 0, ((SCREEN_BPP+7)/8) * (SCREEN_WIDTH * SCREEN_HEIGHT) );
	nextRow = MARGIN;
	nextCol = MARGIN; 

	XVideoFlushFB();
}

void debugResetCursor ( void )
{
	nextRow = MARGIN;
	nextCol = MARGIN;
}

void debugMoveCursor (int x, int y)
{
	if ( x < MARGIN || x > SCREEN_WIDTH-MARGIN-FONT_WIDTH ) {
		return;
	}
	if ( y < MARGIN || y > SCREEN_HEIGHT-MARGIN-FONT_HEIGHT ) {
		return;
	}
	
	nextRow = y;
	nextCol = x;
}

void debugPrintHex(const char *buffer, int length)
{
	char tmp[10];
	for (int i = 0; i < length; i++)
	{
		sprintf(tmp, "%02x ", buffer[i] & 0xFF);
		debugPrint("%s", tmp);
	}
}

__asm__(
	".globl _dbg_output_isr_stub\n"
	"_dbg_output_isr_stub:\n\t"
	    "pushl %ebp;"
	    "pushl %edi;"
	    "pushl %esi;"
	    "pushl %edx;"
	    "pushl %ecx;"
	    "pushl %ebx;"
	    "pushl %eax;"
	    "call _dbg_output;"
	    "popl %eax;"
	    "popl %ebx;"
	    "popl %ecx;"
	    "popl %edx;"
	    "popl %esi;"
	    "popl %edi;"
	    "popl %ebp;"
	".extern dbg_output_original_isr_addr"
	    "jmp *_dbg_output_original_isr_addr"
);

void __cdecl dbg_output(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx,
	                    uint32_t esi, uint32_t edi, uint32_t ebp) 
{
	if (eax == 1) {
        PANSI_STRING s = (PANSI_STRING)ecx;
        debugPrint("%s\n", s->Buffer);
    }
}

struct idtr_t {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct idt_descriptor_t {
	uint16_t offset_low;
	uint16_t selector;
    uint8_t zero;
	uint8_t type_attr;
	uint16_t offset_high;
} __attribute__((packed));

static void get_idtr(struct idtr_t *idtr) {
    asm ("sidt %0" : "=m" (*idtr));
}

static uint32_t dbg_output_original_isr_addr;
extern char     dbg_output_isr_stub;

void debugEnableOutputDebugString(void) {
	struct idtr_t idtr;
    struct idt_descriptor_t *entries;
    uint32_t new_isr = (uint32_t)&dbg_output_isr_stub;

    get_idtr(&idtr);
    entries = (struct idt_descriptor_t *)idtr.base;

    uint32_t old_isr = (((uint32_t)entries[0x2d].offset_high) << 16) | entries[0x2d].offset_low;
    dbg_output_original_isr_addr = old_isr;
    entries[0x2d].offset_low = ((uint32_t)new_isr) & 0xffff;
    entries[0x2d].offset_high = (((uint32_t)new_isr) >> 16) & 0xffff;
}