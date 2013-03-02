/* ----------------------------------------------------------------------
    lcd.c
    液晶モジュール

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
void lcd_delay(unsigned int cnt) 
{
#if 0
	volatile unsigned int i,j;
	for(i = 0; i < cnt; i++) {
		for(j = 0; j < 100; j++) {
		}
	}
#endif
	volatile unsigned int j;
	for(j = 0; j < cnt; j++) {
	}
}

/* ----------------------------------------------------------------------
    lcd_busy
    ビジーフラグのチェック
---------------------------------------------------------------------- */
void lcd_busy(void) {
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
#if 1
void lcd_write(unsigned int ch) 
{
	register volatile unsigned char
		*IPDR5 = ((volatile unsigned char *)0xffd8);	
	register unsigned char buf;

//	LCD_RW = 0;
//	LCD_E = 1;
//	buf = *IPDR5;
	buf = ch >> 8;	// set RS bit 
	buf |= 0x40;
	// データのうち上位4ビットを送る
	buf |= (ch >> 4);
	*IPDR5 = buf;
//	LCD_E = 0;
	buf &= ~0x40;
	*IPDR5 = buf;

	// LCD_E = 1;
	// データの下位4ビットを送る
	buf &= ~0x0f;
	buf |= 0x40;
	buf |= (ch & 0x0f);
	*IPDR5 = buf;
//	LCD_E = 0;
	buf &= ~0x40;
	*IPDR5 = buf;
	lcd_busy();
}
#else
void lcd_write(char ch) {
register volatile unsigned char *IPDR5 = ((volatile unsigned char *)0xffd8);
	unsigned char buf;

	LCD_RW = 0;
	LCD_E = 1;
	// データのうち上位4ビットを送る
	buf = LCD_DT & 0xf0;
	LCD_DT = ((ch >> 4) & 0x0f) | buf;
	LCD_E = 0;
	LCD_E = 1;
	// データの下位4ビットを送る
	buf = LCD_DT & 0xf0;
	LCD_DT = (ch & 0x0f) | buf;
	LCD_E = 0;
	lcd_busy();
}
#endif
/* ----------------------------------------------------------------------
	lcd_putch
    液晶へ1文字送信
---------------------------------------------------------------------- */
void lcd_putch(char ch) {

//	LCD_RS = 1;
//	HPDR5 |= 0x20; 
	lcd_write(ch | 0x2000);
}

/* ----------------------------------------------------------------------
	lcd_putstr
    液晶へ文字列送信
---------------------------------------------------------------------- */
#if 0
void lcd_putstr(char *ch) {

	while(*ch) {
		lcd_putch(*ch++);
	}
}
#endif
/* ----------------------------------------------------------------------
	lcd_putcmd
    液晶へコマンド送信
---------------------------------------------------------------------- */
void lcd_putcmd(char cmd) {

//	LCD_RS = 0;
	lcd_write(cmd);
}

/* LCD内部アドレス取得 */
//
//	-パラメータ
//		iAdr : 座標データ(0～31)
//	-戻り値
//		LCD内部アドレス
//
unsigned char getLcdAdr(unsigned char iAdr)
{
	if (iAdr >= 16) iAdr += 0x30;	// 2行目は要変換
	
	return iAdr;
}

void	RewriteLcd(void)
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
}

void lcd_putram(unsigned char code,unsigned char *dat)
{
  unsigned char i;

  code &= 7;//CGRAMは0から7まで8個
  lcd_putcmd(0x40+(code<<3));//CGRAM Addressセット（以降CGRAMがデータ送受）

  for(i=0;i<8;i++){
    lcd_putch(*dat);
    dat++;
  }
}

/* ----------------------------------------------------------------------
	lcd_init
    液晶モジュール初期化処理
---------------------------------------------------------------------- */
//#define OMOD
void lcd_init(void)
{
register volatile unsigned char *IPDR5 = ((volatile unsigned char *)0xffd8);
	int	i;
	for (i = 0;i < LCD_ROW_SIZE * LCD_COL_SIZE;i++) {
		LcdRam[i] = 0x20;
		lastLcdRam[i] = 0x20;
	}

	// データ用ポートを出力にする
//    IO.PCR5.BYTE = 0x0f;
//    IO.PMR5.BYTE = 0x00;
//    IO.PCR5 = 0x7f;
	HPMR5 = 0x00;	
	HPCR5 = 0x7f;
	// 制御線用ポートを出力にする
//    IO.PCR7.BYTE = 0x70;

	// 初期化開始
	// RS=0,RW=0
#ifdef OMOD
	LCD_RS = 0;
	LCD_RW = 0;
	LCD_E = 0;
#else
	*IPDR5 = 0;
#endif
	lcd_delay(15000); // about 15ms

#ifdef OMOD
	LCD_E = 1;
	LCD_DT = 0x03;
	for(i = 0;i < 10;i++);
	LCD_E = 0;
#else
	*IPDR5 = 0x43;
//	for(i = 0;i < 10;i++);
//	lcd_wait2(10);
	*IPDR5 = 0x03;
#endif
	lcd_delay(10000); // about 10ms

#ifdef OMOD
	LCD_E = 1;
	LCD_DT = 0x03;
//	for(i = 0;i < 10;i++);
//	lcd_wait2(10);
	LCD_E = 0;
#else
	*IPDR5 = 0x43;
//	for(i = 0;i < 10;i++);
//	lcd_wait2(10);
	*IPDR5 = 0x03;
#endif
	lcd_delay(1000); // about 1ms

#ifdef OMOD
	LCD_E = 1;
	LCD_DT = 0x03;
	for(i = 0;i < 10;i++);
	LCD_E = 0;
#else
	*IPDR5 = 0x43;
//	for(i = 0;i < 10;i++);
//	lcd_wait2(10);
	*IPDR5 = 0x03;
#endif
	lcd_delay(1000); // about 1ms

#ifdef OMOD
	LCD_E = 1;
	LCD_DT = 0x02;
	for(i = 0;i < 10;i++);
	lcd_wait2(1000);
	LCD_E = 0;
#else
	*IPDR5 = 0x42;
//	for(i = 0;i < 10;i++);
//	lcd_wait2(10);
	*IPDR5 = 0x02;
#endif
	lcd_delay(1000); // about 1ms

	lcd_putcmd(0x28); // DL=1, N=1, F=0
	lcd_putcmd(0x08); // display off, cursor off, blink off
	lcd_putcmd(0x01); // 表示クリア
	lcd_putcmd(0x06); // アドレス+1,表示シフトなし
	lcd_putcmd(0x0c); // 移動モード

	// 初期化完了したので，表示 ON にする
	lcd_putcmd(0x0c); // display on, cursor off, blink off
}
