#include	"timerA.h"
#include	"timerW.h"
#include	"lcd.h"

extern 	int	main(int);
int (* const func_tbl[])(int) = {main,
	InitTimerA,GetTimerA_Counter,
	InitTimerW,GetPeriodW,GetRenewStateW,
	InitLcd,RewriteLcd,DispLcdBar};

int main(int x)
{
	return 0;
}
