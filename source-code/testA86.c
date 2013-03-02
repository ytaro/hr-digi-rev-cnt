#include	"../h8os/syscall.h"
#include	"../h8os/reg3664.h"
#include <string.h>
#define func_tbl_ptr (*(int (***)(int))0x5800)

int (**func_tbl)(int);

//#define	BASE_TIME	240000000  @ 4MHz 1puls/cycle
#define	BASE_TIME    120000000	//  @ 4MHz 1puls/cycle
#define	LOWER_LIMIT	240000	//  500rpm @ 4MHz 2puls/cycle
#define	HIGHER_LIMIT	 14999	// 8000rpm @ 4MHz 2puls/cycle

		       /*12345678901234561234567890123456*/
const char * const openingMsg= "OfficeR presents   a rev-counter";

enum {dummy,
	InitTimerA,GetTimerA_Counter,
	InitTimerW,GetPeriodW,GetRenewStateW,
	InitLcd,RewriteLcd,DispLcdBar};
int main()
{
	int i,tmp;
	unsigned short lastCount = 0,sTmp;
	char *lcdRam;
	unsigned long period;
	int blinkState = 0;
	func_tbl = func_tbl_ptr;

	lcdRam = (char *)func_tbl[InitLcd](0);
	memset(lcdRam,32,32);
#if 0
	sTmp = 65534;

	sTmp = lastCount - sTmp;

	sprint(lcdRam,"%i",sTmp);
	func_tbl[RewriteLcd](0);
	sleep(3);
#endif
	for (i = 0;i < 32;i++) {
		if (i < 32) {
			lcdRam[i] = openingMsg[i];
		}
		if (i > 10) {
			lcdRam[i - 11] = ' ';
		}
		func_tbl[RewriteLcd](0);
		sleep(1);
	}
	sleep(3);
	memset(lcdRam,32,32);

	func_tbl[InitTimerA](0);
	func_tbl[InitTimerW](0);

	lastCount = func_tbl[GetTimerA_Counter](0);
	while(1) {
//	for(i = 0;i < 480;i++) {
		ADCSR = 0x20;
		do {
			sTmp = func_tbl[GetTimerA_Counter](0);
		} while((unsigned short)(sTmp - lastCount) < 4); // wait 125ms
		lastCount = sTmp;
		blinkState ++;
		period = func_tbl[GetPeriodW](0);

		if (func_tbl[GetRenewStateW](0) == 0) {
			memset(lcdRam,0,16);
			memcpy((void *)lcdRam + 16,"Stop   ",7);
		} else if (period > LOWER_LIMIT) {
			if (blinkState & 2) {
				tmp = 0x20;
			} else {
				tmp = 0x0;
			}
			memset(lcdRam,tmp,16);
			memcpy((void *)lcdRam + 16,"Under  ",7);
		} else if (period < HIGHER_LIMIT) {
			if (blinkState & 2) {
				tmp = 0x20;
			} else {
				tmp = 0x5;
			}
			memset(lcdRam,tmp,16);
			memcpy((void *)lcdRam + 16,"Over   ",7);
		} else {
			unsigned short x = BASE_TIME/period;
			sprint(lcdRam + 16,"%4drpm",x);
			*(lcdRam + 16 + 7) = 0x20;
			func_tbl[DispLcdBar](x);
		}
		sprint(lcdRam + 26,"%4dV",((ADDRA >> 6) * 196) / 1000);
		lcdRam[31] = lcdRam[30];
		lcdRam[30] = lcdRam[29];
		lcdRam[29] = '.';
		func_tbl[RewriteLcd](0);
	}
}
