#ifndef LCD_H
#define	LCD_H

#define	LCD_ROW_SIZE	2
#define	LCD_COL_SIZE	16

#if 0
RAM のコードから呼び出し用
struct sLcdFont {
	unsigned char code;
	unsigned char *dat;
};
extern	int	SetLcdFont(int ofs);
#endif

extern	int	InitLcd(int dummy);
extern	int	RewriteLcd(int dummy);
//extern	void	lcd_putram(unsigned char code,unsigned char *dat);
extern	int	SetLcdRam(int adrDat);
extern	int	DispLcdBar(int j);

#endif
