/* ----------------------------------------------------------------------
    lcd.c
    �t�����W���[��

	Modify at 2009/05/15
    Create:   2009/05/12

	�t�����W���[��: SC1602
	�C���^�t�F�[�X: 4bit
	����́C16��2�s��LCD���g�p���Ă���
        �t���̃A�h���X�͉��\
	|-+-------------------------------------------------
	| | 01 02 03 04 05 06 07 08 09 00 00 02 03 04 05 06 
	|-+-------------------------------------------------
	|1| 00 00 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 
	|2| 11 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 
	|�s

#original http://www.mouiyada.jp/node/89

---------------------------------------------------------------------- */
//#include "iodefine.h"
#include "lcd.h"
#define HPMR5 (*(volatile unsigned char *)0xffe1)
#define HPCR5 (*(volatile unsigned char *)0xffe8)
#define HPDR5 (*(volatile unsigned char *)0xffd8)

char	LcdRam[LCD_ROW_SIZE*LCD_COL_SIZE];
char	lastLcdRam[LCD_ROW_SIZE*LCD_COL_SIZE];

// lcd IO�|�[�g					LCD pin		CPU-PIN
#define LCD_E  IO.PDR5.BIT.B6	//  6		CN-1  3
#define LCD_RS IO.PDR5.BIT.B5	//  4		CN-2 19
#define LCD_RW IO.PDR5.BIT.B4	//	5		CN-2 18

#define LCD_D7 IO.PDR5.BIT.B3	//	14		CN-2 17
#define LCD_D6 IO.PDR5.BIT.B2	//	13		CN-2 16
#define LCD_D5 IO.PDR5.BIT.B1	//	12		CN-2 15
#define LCD_D4 IO.PDR5.BIT.B0	//	11		CN-2 14
#define LCD_DT IO.PDR5.BYTE

// lcd IO�|�[�g�R���g���[��
#define LCD_PCR_E  IO.PCR5.BIT.B6
#define LCD_PCR_RS IO.PCR5.BIT.B5
#define LCD_PCR_RW IO.PCR5.BIT.B4

#define LCD_PCR_D7 IO.PCR5.BIT.B3
#define LCD_PCR_D6 IO.PCR5.BIT.B2
#define LCD_PCR_D5 IO.PCR5.BIT.B1
#define LCD_PCR_D4 IO.PCR5.BIT.B0

/* ----------------------------------------------------------------------
    lcd_delay
    �f�B���C���[�`��(�҂����Ԃ͌v�Z�K�v)
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
    �r�W�[�t���O�̃`�F�b�N
---------------------------------------------------------------------- */
void lcd_busy(void) {
register volatile unsigned char *IPDR5 = ((volatile unsigned char *)0xffd8);
	unsigned char busy;

	// LCD �f�[�^�|�[�g����͂ɂ���
	HPCR5 = 0x70;

	// RW=1, RS=0 �� BUSY �ǂݍ��݊J�n
	*IPDR5 = 0x10;

	do {
		// ���4�r�b�g�����
		*IPDR5 = 0x50;	// LCD_E = 1;
		busy = *IPDR5;	// busy = data;
		*IPDR5 = 0x10;	// LCD_E = 0;
		// ����4�r�b�g�����
		*IPDR5 = 0x50;	// LCD_E = 1;
		*IPDR5 = 0x10;	// LCD_E = 0;
	} while(busy & 0x08);  // BUSY == 0 �܂Ń��[�v

	// LCD �f�[�^�|�[�g���o�͂ɖ߂�
	HPCR5 = 0x7f;
}

/* ----------------------------------------------------------------------
	lcd_write
    LCD�փf�[�^���M����B
	�R�}���h/�f�[�^�����ʂ��� RS �M���́C���̊֐����Ăяo���O�ɃZ�b�g

#    �e���͖������f�[�^�̏������݂Ƀo�C�g�A�N�Z�X���Ă���B
#    �ŏI�I�ɂ̓|�[�g���ړ����邩���W�b�N�C�����K�v
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
	// �f�[�^�̂������4�r�b�g�𑗂�
	buf |= (ch >> 4);
	*IPDR5 = buf;
//	LCD_E = 0;
	buf &= ~0x40;
	*IPDR5 = buf;

	// LCD_E = 1;
	// �f�[�^�̉���4�r�b�g�𑗂�
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
	// �f�[�^�̂������4�r�b�g�𑗂�
	buf = LCD_DT & 0xf0;
	LCD_DT = ((ch >> 4) & 0x0f) | buf;
	LCD_E = 0;
	LCD_E = 1;
	// �f�[�^�̉���4�r�b�g�𑗂�
	buf = LCD_DT & 0xf0;
	LCD_DT = (ch & 0x0f) | buf;
	LCD_E = 0;
	lcd_busy();
}
#endif
/* ----------------------------------------------------------------------
	lcd_putch
    �t����1�������M
---------------------------------------------------------------------- */
void lcd_putch(char ch) {

//	LCD_RS = 1;
//	HPDR5 |= 0x20; 
	lcd_write(ch | 0x2000);
}

/* ----------------------------------------------------------------------
	lcd_putstr
    �t���֕����񑗐M
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
    �t���փR�}���h���M
---------------------------------------------------------------------- */
void lcd_putcmd(char cmd) {

//	LCD_RS = 0;
	lcd_write(cmd);
}

/* LCD�����A�h���X�擾 */
//
//	-�p�����[�^
//		iAdr : ���W�f�[�^(0�`31)
//	-�߂�l
//		LCD�����A�h���X
//
unsigned char getLcdAdr(unsigned char iAdr)
{
	if (iAdr >= 16) iAdr += 0x30;	// 2�s�ڂ͗v�ϊ�
	
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

  code &= 7;//CGRAM��0����7�܂�8��
  lcd_putcmd(0x40+(code<<3));//CGRAM Address�Z�b�g�i�ȍ~CGRAM���f�[�^����j

  for(i=0;i<8;i++){
    lcd_putch(*dat);
    dat++;
  }
}

/* ----------------------------------------------------------------------
	lcd_init
    �t�����W���[������������
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

	// �f�[�^�p�|�[�g���o�͂ɂ���
//    IO.PCR5.BYTE = 0x0f;
//    IO.PMR5.BYTE = 0x00;
//    IO.PCR5 = 0x7f;
	HPMR5 = 0x00;	
	HPCR5 = 0x7f;
	// ������p�|�[�g���o�͂ɂ���
//    IO.PCR7.BYTE = 0x70;

	// �������J�n
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
	lcd_putcmd(0x01); // �\���N���A
	lcd_putcmd(0x06); // �A�h���X+1,�\���V�t�g�Ȃ�
	lcd_putcmd(0x0c); // �ړ����[�h

	// ���������������̂ŁC�\�� ON �ɂ���
	lcd_putcmd(0x0c); // display on, cursor off, blink off
}
