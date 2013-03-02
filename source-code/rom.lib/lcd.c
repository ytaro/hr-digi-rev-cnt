/* ----------------------------------------------------------------------
    lcd.c
    液晶モジュール

	Modify at 2009/06/15
		H8/OS 3664 コマンドライン使用時に ROM から使えるように
		オリジナルは lcd_bench2-2.fix
	Modify at 2009/05/15
	Create:   2009/05/12

	液晶モジュール: SC1602
	インタフェース: 4bit
	今回は，16桁2行のLCDを使用している
        液晶のアドレスは下表
	|-+-------------------------------------------------
	| | 01 02 03 04 05 06 07 08 09 00 00 02 03 04 05 06 
	|-+-------------------------------------------------
	|1| 00 00 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 
	|2| 11 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 
	|行

#original http://www.mouiyada.jp/node/89

---------------------------------------------------------------------- */
//#include "iodefine.h"
#include "lcd.h"
#define HPMR5 (*(volatile unsigned char *)0xffe1)
#define HPCR5 (*(volatile unsigned char *)0xffe8)
#define HPDR5 (*(volatile unsigned char *)0xffd8)

char	LcdRam[LCD_ROW_SIZE*LCD_COL_SIZE];
char	lastLcdRam[LCD_ROW_SIZE*LCD_COL_SIZE];

// lcd IOポート					LCD pin		CPU-PIN
#define LCD_E  IO.PDR5.BIT.B6	//  6		CN-1  3
#define LCD_RS IO.PDR5.BIT.B5	//  4		CN-2 19
#define LCD_RW IO.PDR5.BIT.B4	//	5		CN-2 18

#define LCD_D7 IO.PDR5.BIT.B3	//	14		CN-2 17
#define LCD_D6 IO.PDR5.BIT.B2	//	13		CN-2 16
#define LCD_D5 IO.PDR5.BIT.B1	//	12		CN-2 15
#define LCD_D4 IO.PDR5.BIT.B0	//	11		CN-2 14
#define LCD_DT IO.PDR5.BYTE

// lcd IOポートコントロール
#define LCD_PCR_E  IO.PCR5.BIT.B6
#define LCD_PCR_RS IO.PCR5.BIT.B5
#define LCD_PCR_RW IO.PCR5.BIT.B4

#define LCD_PCR_D7 IO.PCR5.BIT.B3
#define LCD_PCR_D6 IO.PCR5.BIT.B2
#define LCD_PCR_D5 IO.PCR5.BIT.B1
#define LCD_PCR_D4 IO.PCR5.BIT.B0

/* ----------------------------------------------------------------------
    lcd_delay
    ディレイルーチン(待ち時間は計算必要)
---------------------------------------------------------------------- */
static	void lcd_delay(unsigned int cnt) 
{
	volatile unsigned int j;
	for(j = 0; j < cnt; j++) {
	}
}

/* ----------------------------------------------------------------------
    lcd_busy
    ビジーフラグのチェック
---------------------------------------------------------------------- */
static	void lcd_busy(void) {
register volatile unsigned char *IPDR5 = ((volatile unsigned char *)0xffd8);
	unsigned char busy;

	// LCD データポートを入力にする
	HPCR5 = 0x70;

	// RW=1, RS=0 で BUSY 読み込み開始
	*IPDR5 = 0x10;

	do {
		// 上位4ビットを入力
		*IPDR5 = 0x50;	// LCD_E = 1;
		busy = *IPDR5;	// busy = data;
		*IPDR5 = 0x10;	// LCD_E = 0;
		// 下位4ビットを入力
		*IPDR5 = 0x50;	// LCD_E = 1;
		*IPDR5 = 0x10;	// LCD_E = 0;
	} while(busy & 0x08);  // BUSY == 0 までループ

	// LCD データポートを出力に戻す
	HPCR5 = 0x7f;
}

/* ----------------------------------------------------------------------
	lcd_write
    LCDへデータ送信する。
	コマンド/データを識別する RS 信号は，この関数を呼び出す前にセット

#    影響は無いがデータの書き込みにバイトアクセスしている。
#    最終的にはポートを移動するかロジック修正が必要
---------------------------------------------------------------------- */
static	void lcd_write(unsigned int ch) 
{
	register volatile unsigned char
		*IPDR5 = ((volatile unsigned char *)0xffd8);	
	register unsigned char buf;

	buf = ch >> 8;	// set RS bit 
	buf |= 0x40;
	// データのうち上位4ビットを送る
	buf |= (ch >> 4);
	*IPDR5 = buf;
	buf &= ~0x40;
	*IPDR5 = buf;

	// LCD_E = 1;
	// データの下位4ビットを送る
	buf &= ~0x0f;
	buf |= 0x40;
	buf |= (ch & 0x0f);
	*IPDR5 = buf;
	buf &= ~0x40;
	*IPDR5 = buf;
	lcd_busy();
}

/* ----------------------------------------------------------------------
	lcd_putch
    液晶へ1文字送信
---------------------------------------------------------------------- */
static	void lcd_putch(char ch) 
{
	lcd_write(ch | 0x2000);
}

/* ----------------------------------------------------------------------
	lcd_putcmd
    液晶へコマンド送信
---------------------------------------------------------------------- */
static	void lcd_putcmd(char cmd) 
{
	lcd_write(cmd);
}

/* LCD内部アドレス取得 */
//
//	-パラメータ
//		iAdr : 座標データ(0～31)
//	-戻り値
//		LCD内部アドレス
//
static	unsigned char getLcdAdr(unsigned char iAdr)
{
	if (iAdr >= 16) iAdr += 0x30;	// 2行目は要変換
	
	return iAdr;
}

int	RewriteLcd(int dummy)
{
	unsigned char	i,adr;
	for (i = 0;i < LCD_ROW_SIZE * LCD_COL_SIZE;i++) {
		if (lastLcdRam[i] != LcdRam[i]) {
			lastLcdRam[i] = LcdRam[i];
			adr = getLcdAdr(i);
			lcd_putcmd(0x80 + adr);	// 0x80 = DDRAM Address Set
			lcd_putch(lastLcdRam[i]);
		}
	}
	return 0;
}

int	SetLcdRam(int adrDat)
{
	LcdRam[adrDat>>8] = adrDat & 0xff;
	return 0;
}

#if 1
void lcd_putram(unsigned char code,const unsigned char *dat)
{
	unsigned char i;

	code &= 7;//CGRAMは0から7まで8個
	lcd_putcmd(0x40+(code<<3));//CGRAM Addressセット（以降CGRAMがデータ送受）

	for(i=0;i<8;i++){
		lcd_putch(*dat);
		dat++;
	}
}
#else
RAM のコードから呼び出し用
extern	int	SetLcdFont(int ofs)
{
  struct sLcdFont *ptr;
  unsigned char i;

	ptr = (struct sLcdFont *)ofs;
  ptr->code &= 7;//CGRAMは0から7まで8個
  lcd_putcmd(0x40+(ptr->code<<3));//CGRAM Addressセット（以降CGRAMがデータ送受）

  for(i=0;i<8;i++){
    lcd_putch(*ptr->dat);
    ptr->dat++;
  }
	return 0;
}
#endif

static	const unsigned char font[][8] ={
{0x1f,0x00,0x00,0x00,0x00,0x00,0x00,0x1f},
{0x1f,0x10,0x10,0x10,0x10,0x10,0x10,0x1f},
{0x1f,0x18,0x18,0x18,0x18,0x18,0x18,0x1f},
{0x1f,0x1c,0x1c,0x1c,0x1c,0x1c,0x1c,0x1f},
{0x1f,0x1e,0x1e,0x1e,0x1e,0x1e,0x1e,0x1f},
{0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f},
};

static	void	lcd_font_set_up(void)
{
	int	i;

	for (i = 0;i < 6;i++) {
		lcd_putram(i,font[i]);
	}
}

/* ----------------------------------------------------------------------
	InitLcd
    液晶モジュール初期化処理
---------------------------------------------------------------------- */
int InitLcd(int dummy)
{
register volatile unsigned char *IPDR5 = ((volatile unsigned char *)0xffd8);
	int	i;
	for (i = 0;i < LCD_ROW_SIZE * LCD_COL_SIZE;i++) {
		LcdRam[i] = 0x20;
		lastLcdRam[i] = 0x20;
	}

	// データ用ポートを出力にする
	// 制御線用ポートを出力にする
	HPMR5 = 0x00;	
	HPCR5 = 0x7f;

	// 初期化開始
	// RS=0,RW=0
	*IPDR5 = 0;
	lcd_delay(15000); // about 15ms

	*IPDR5 = 0x43;
	*IPDR5 = 0x03;
	lcd_delay(10000); // about 10ms

	*IPDR5 = 0x43;
	*IPDR5 = 0x03;
	lcd_delay(1000); // about 1ms

	*IPDR5 = 0x43;
	*IPDR5 = 0x03;
	lcd_delay(1000); // about 1ms

	*IPDR5 = 0x42;
	*IPDR5 = 0x02;
	lcd_delay(1000); // about 1ms

	lcd_putcmd(0x28); // DL=1, N=1, F=0
	lcd_putcmd(0x08); // display off, cursor off, blink off
	lcd_putcmd(0x01); // 表示クリア
	lcd_putcmd(0x06); // アドレス+1,表示シフトなし
	lcd_putcmd(0x0c); // 移動モード

	// 初期化完了したので，表示 ON にする
	lcd_putcmd(0x0c); // display on, cursor off, blink off

	lcd_font_set_up();

	return (int)LcdRam;
}
#if 0
void	disp_bar(unsigned short j)
{
	short i;
	short k;

	j = j / 100;	
	k = 0;
	for (i = 0;i <	(j / 5);i++) {
		func_tbl[SetLcdRam](k | 5);
		k += 0x100;
	}
	func_tbl[SetLcdRam](k | (j % 5));

	if (j == 0) {
		i--;
		k -= 0x100;
	}
	for (i = i + 1;i < 16;i++) {
		k += 0x100;
		func_tbl[SetLcdRam](k);
	}
}
#endif
int	DispLcdBar(int j)
{
	short i;

	j = j / 100;	
	for (i = 0;i <	(j / 5);i++) {
		LcdRam[i] = 5;
	}
	LcdRam[i] = (j % 5);
	if (j == 0) {
		i--;
	}
	for (i = i + 1;i < 16;i++) {
		LcdRam[i] = 0;
	}
	return 0;
}
