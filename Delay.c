/********************************************************************************/
/*						For Microchip 18F and Hi-Tech C							*/
/********************************************************************************/
#ifndef __DELAY_C
#include "Delay.h"

unsigned char delayus_variable;
/********************************************************************************/
/*					PAUSA DE HASTA 255 MILISEGUNDOS								*/
/********************************************************************************/
void DelayMs(unsigned char cnt)
{
	unsigned char i;
	do
	{
		i=4;
		do
		{
			DelayUs(250);
			CLRWDT();
		}while(--i);
	}while(--cnt);
}
/********************************************************************************/
/*					PAUSA DE HASTA 255 SEGUNDOS									*/
/********************************************************************************/
void DelayS(unsigned char cnt)
{
	unsigned char i;
	do
	{
		i=4;
		do
		{
			DelayMs(250);
			CLRWDT();
		}while(--i);
	}while(--cnt);
}
#endif
