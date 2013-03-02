#include	"../h8os/syscall.h"
#include	"../h8os/reg3664.h"
#include	"timerA.h"

unsigned short 	TimerA_Counter = 0;

void	handleTimerAOverflow(void)
{
	TimerA_Counter += 1;
	IRR1 &= ~0x40;
}

int	GetTimerA_Counter(int dummy)
{
	return TimerA_Counter;
}

int	InitTimerA(int x)
{
	int	r;

	TimerA_Counter = 0;
	r = int_regist(19,handleTimerAOverflow);
	TMA = 0x0b;
	IENR1 |= 0x60;
	return r;
}

