/****************************************************************************/
/*				CONFIGURACION MODULO A/D PIC 18F4620						*/
/*..........................................................................*/
/*			Revisión:				1.00									*/
/*			PIC:					PIC18F4620								*/
/*			Compilador:				MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.	*/
/*			Autor:					Mariano Ariel Deville					*/
/****************************************************************************/
/*	PCFG3-PCFG0																*/
/*		PCFG	AN12 AN11 AN10  AN9 AN8 AN7 AN6 AN5 AN4 AN3 AN2 AN1 AN0		*/
/*		0000	 A	  A		A	 A	 A	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		0001	 A	  A		A	 A	 A	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		0010	 A	  A		A	 A	 A	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		0011	 D	  A		A	 A	 A	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		0100	 D	  D		A	 A	 A	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		0101	 D	  D		D	 A	 A	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		0110	 D	  D		D	 D	 A	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		0111	 D	  D		D	 D	 D	 A	 A	 A	 A	 A	 A	 A	 A		*/
/*		1000	 D	  D		D	 D	 D	 D	 A	 A	 A	 A	 A	 A	 A		*/
/*		1001	 D	  D		D	 D	 D	 D	 D	 A	 A	 A	 A	 A	 A		*/
/*		1010	 D	  D		D	 D	 D	 D	 D	 D	 A	 A	 A	 A	 A		*/
/*		1011	 D	  D		D	 D	 D	 D	 D	 D	 D	 A	 A	 A	 A		*/
/*		1100	 D	  D		D	 D	 D	 D	 D	 D	 D	 D	 A	 A	 A		*/
/*		1101	 D	  D		D	 D	 D	 D	 D	 D	 D	 D	 D	 A	 A		*/
/*		1110	 D	  D		D	 D	 D	 D	 D	 D	 D	 D	 D	 D	 A		*/
/*		1111	 D	  D		D	 D	 D	 D	 D	 D	 D	 D	 D	 D	 D		*/
/*--------------------------------------------------------------------------*/
/*		Configuración del PORTA												*/
/*			justifico a la derecha.											*/
/*			Todas las entradas como entradas analógicas						*/
/*			Vref+ ------> Vdd												*/
/*			Vref- ------> Vss												*/
/*--------------------------------------------------------------------------*/
/*		ADCS2	ADCS1	ADCS0	Clock Conversion							*/
/*		 0		 0		 0			Fosc/2									*/
/*		 0		 0		 1			Fosc/8									*/
/*		 0		 1		 0			Fosc/32									*/
/*		 0		 1		 1			FRC										*/
/*		 1		 0		 0			Fosc/4									*/
/*		 1		 0		 1			Fosc/16									*/
/*		 1		 1		 0			Fosc/64									*/
/*		 1		 1		 0			FRC										*/
/****************************************************************************/
/*				Conversión Analógica / Digital								*/
/*..........................................................................*/
unsigned int ReadADC(unsigned char canal)
{
	ADCON0=0b00000000;			// Reseteo el módulo ADC.
	DelayMs(1);					// Para los casos en que realizo lectuas seguidas.
	ADCON1=0b00000111;			// Seteo los puertos a usar (hasta AN7 analógico).
	ADCON2=0b10111110;			// Configuro tiempo de adquisicion y reloj de conversion.
 	ADCON0=(canal<<2)+1;		// Arranco el ADC y configuro el canal.
	DelayUs(20);				// Espero que termine la adquisicion.
 	GODONE=1;					// Comienza el proceso de conversión.
 	while(GODONE)				// Espero que termine la conversion.
		continue;
	ADCON0=0b00000000;			// Apago el módulo ADC.
	return (ADRES);				// Devuelvo el valor obtenido.
}
