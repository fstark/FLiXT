#include "dos.h"
#include "herc.h"

/*
	HGC.COM FULL HALF or DIAG
*/

#define IndexRegister 0x3b4
#define DataRegister 0x3b5
#define DisplayModeControlPort 0x3b8
#define 	GraphicsMode 0x01
#define 	EnableDisplay 0x08
#define 	EnableTextBlinker 0x20
#define 	Page1 0x80
#define DisplayStatusPort 0x3ba
#define 	HSync 0x01
#define 	DotsOn 0x08
#define 	IDmask 0x30
#define 	IDcode 0x10
#define 	VertRetrace 0x80
#define FULL 3

#define xModeReg 0x14
#define 	kRamFontOn 0x1
#define 	k9BitsWide 0x0
#define 	k8BitsWide 0x2
#define 	k48kFont 0x4
#define ScoreReg 0x15
#define StrikeReg 0x16

#define ConfigurationSwitch 0x3bf
#define 	AllowGraphics 0x01
#define		EnablePage1 0x02


#define HREG 0
#define VREG 4

int horiz8[] = { 109, 90, 92, 15 };
int horiz9[] = { 97, 80, 82, 15 };
int vert8[]  = { 45, 2, 43, 44, 2, 7 };
int vert14[] = { 25, 6, 25, 25, 2, 13 };

int trail[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static void write_regs( int start_reg, int *values, int count )
{
	int i;
	for( i = 0; i < count; i++ )
	{
		outportb( IndexRegister, i+start_reg );
		outportb( DataRegister, values[i] );
	}
}

static void set_mode( int *horiz, int *vert )
{
	write_regs( HREG, horiz, 4 );
	write_regs( VREG, vert, 6 );
	write_regs( 10, trail, sizeof( trail ) / sizeof( *trail ) );
}

void HerculesRamfont48K()
{
	outportb( ConfigurationSwitch, AllowGraphics|EnablePage1 );
	set_mode( horiz8, vert8 );
	outportb( IndexRegister, 0x14 );
	outportb( DataRegister, kRamFontOn|k8BitsWide|k48kFont );
}

void HerculesTextMode()
{
	outportb( ConfigurationSwitch, 0 );
	set_mode( horiz9, vert14 );
	outportb( IndexRegister, 0x14 );
	outportb( DataRegister, 0 );

	/* We have to, as the content of the attributes bytes may have rendered anything already on screen unreadable (black-on-black) */
	/* This also means that anything that intent to print a message *before* exit needs to switch back to text before */
	clrscr();
}
