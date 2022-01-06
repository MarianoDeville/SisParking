#ifndef __DELAY_H

#if PIC_CLK == 4000000
	#define DelayDivisor 4
	#define WaitFor1Us asm("nop")
	#define Jumpback asm("goto $ - 3")
#elif PIC_CLK == 8000000
	#define DelayDivisor 2
	#define WaitFor1Us asm("nop")
	#define Jumpback asm("goto $ - 3")
#elif ( (PIC_CLK == 16000000) || (PIC_CLK==16257000) )
	#define DelayDivisor 1
	#define WaitFor1Us asm("nop")
	#define Jumpback asm("goto $ - 3")
#elif PIC_CLK == 20000000
	#define DelayDivisor 1
	#define WaitFor1Us asm("nop"); asm("nop")
	#define Jumpback asm("goto $ - 5")
#elif PIC_CLK == 32000000
	#define DelayDivisor 1
	#define WaitFor1Us asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop")
	#define Jumpback asm("goto $ - 6")
#elif PIC_CLK == 40000000
	#define DelayDivisor 1
	#define WaitFor1Us asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop")
	#define Jumpback asm("goto $ - 8")
#else
	#error delay.h - please define pic_clk correctly
#endif

#define DelayUs(x) { \
			delayus_variable=(unsigned char)(x/DelayDivisor); \
			WaitFor1Us; } \
			asm("decfsz _delayus_variable,f"); \
			Jumpback;

//function prototypes
void DelayMs(unsigned char cnt);
void DelayS(unsigned char cnt);

#endif
