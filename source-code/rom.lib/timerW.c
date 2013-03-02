#include	"../h8os/syscall.h"
#include	"../h8os/reg3664.h"

#include	"timerW.h"

static	volatile unsigned long upperBits;  // 最上位 8bit
static	volatile unsigned long lastPeriod; // 最新の周期
static	volatile unsigned char fBefore;
static	volatile unsigned char fDone;
static	volatile int	state;

// 周期を更新したとき 1
static	volatile unsigned char fRenew;

void	handle_overflow(void)
{
	if (fDone == 0) {
		fDone = 1;
		upperBits += 0x10000;
	}
	TSRW &= ~0x80;
}

void	handle_input_capture(void)
{
static	unsigned long lastCount;// 前回のキャプチャ値
	unsigned long nowCount;	// 今回のキャプチャ値
	if ((fBefore == 1) && (fDone == 0)) {
		if (GRA < 0x8000) {
			// ovef flow 割込みが未処理
			fDone = 1;
			upperBits += 0x10000;
		}
	}
	switch(state) {
	case	0:
		state = 1;
		lastCount = upperBits + GRA;
		break;
	case	1:
		nowCount = upperBits + GRA;
		lastPeriod = nowCount - lastCount;
		lastCount = nowCount;
		fRenew = 1;
		break;
	}
	TSRW &= ~0x01;
}

void	handle_before_point(void)
{
	fBefore = 1;
	TSRW &= ~0x04;
}

void	handle_after_point(void)
{
	fBefore = 0;
	fDone = 0;
	TSRW &= ~0x08;
}

void	handle(void)
{
	unsigned char flag = TSRW;
	if (flag & 0x80) handle_overflow();
	if (flag & 0x01) handle_input_capture();
	if (flag & 0x04) handle_before_point();
	if (flag & 0x08) handle_after_point();
}

int	GetPeriodW(int dummy)
{
	return lastPeriod;
}

// 2009/06/15 追加：読み出したあとクリアする
int	GetRenewStateW(int dummy)
{
	int	tmp;
	tmp = fRenew;
	fRenew = 0;
	return tmp;
}

int	InitTimerW(int dummy)
{
	int_regist(21,handle);

	lastPeriod = 0;
	fBefore = 0;
	fDone = 0;
	upperBits = 0;
	state = 0;
	fRenew = 0;

	TIOR0 = 0x04;	// GRA is input capture mode (IOA2)
			// GRB is output capture
	TIOR1 = 0;	// GRC,GRD is output capture mode
	GRC = 0xe700;	// before point count (1.6ms)
	GRD = 0x1900;	// after point;count  (1.6ms)

	TCRW = 0x20;	// clk = φ / 4
	TIERW = 0x8D;	// OVIE | IMIED | IMIEC | IMIEA
	TMRW = 0x80;	// start counter W
	return 0;
}

#if 0
int main()			/*  C1-19 C1-18 C2-03 C1:17 C1:16 C1:15 C1:14	*/
{				/*  RS:55 RW:54  E:56 D7:53 D6:52 D5:51 D5:50	*/
	char	lcdport[] = {0x20, 0x10, 0x40, 0x08, 0x04, 0x02, 0x01};
	int i = 0;

	PMR5 = 0x00;
	PCR5 = 0x7F;

	lcd_setup(2, 16, (volatile char*)&PDR5, lcdport);
	lcd_clear();
	write_mode(LCD);
		/*    1234567890123456	*/
	write_string("CPU is H8/3664\n");
	write_string("H8OS is ready!");
	sleep(3);
	write_string("\n");

//	init_handle();
	InitTimerA();

	for(i = 0;i < 1000;i++) {
		print("%02d:%03d:%08x\n",i,4000000 / lastPeriod,lastPeriod);
//		while(fRenewLCD == 0);
//		fRenewLCD = 0;
//		sleep(5);
	}
	return 0;
}
#endif
