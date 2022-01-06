/********************************************************************************/
/*				COMUNICACION RS232 - RUTINAS DE ENVIO DE DATOS					*/
/*..............................................................................*/
/*			Revisión:				1.00										*/
/*			PIC:					PIC18F4620									*/
/*			Compilador:				MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.		*/
/*			Autor:					Mariano Ariel Deville						*/
/********************************************************************************/
/*						PROTOTIPO DE FUNCIONES									*/
/*..............................................................................*/
void Serial_Setup(long unsigned int velocidad);
void PutStr(register const char *str);
void PutCh(unsigned char c);
/********************************************************************************/
/*						SETEO EL PUERTO SERIE									*/
/*..............................................................................*/
void Serial_Setup(long unsigned int velocidad)
{
	unsigned int calc_spbrg;
	TX9=0;						// 8-bit transmission.
	SYNC=0;						// Asynchronous.
	SENDB=0;					// Send Break Character.
	TX9D=0;						// 9th Bit of Transmit Data.
	SPEN=1;						// Enable serial port pins.
	RX9=0;						// 8-bit reception.
	SREN=0;						// No effect.
	CREN=0;						// Disable continuous reception.
	ADDEN=0;					// No effect.
	BRG16=1;					// 16-Bit Baud Rate Register Enable bit.
	BRGH=1;						// Data rate for sending low speed.
	WUE=0;						// Wake-up Enable bit.
	TXIE=0;						// Disable tx interrupts.
	RCIE=0;						// Disable rx interrupts.
	TXIP=0;
	SENDB=0;
	CLRWDT();
	if(velocidad)
	{
		ABDEN=0;				// Auto-Baud Detect Enable bit.
		ABDOVF=0;				// Auto-Baud Acquisition Rollover disable.
		calc_spbrg=((PIC_CLK/velocidad)/4-1);	// Calculo el coeficiente.
		SPBRG=calc_spbrg;		// Paso la parte baja del calculo.
		SPBRGH=calc_spbrg>>8;	// Paso la parte alta del calculo.
	}
	else
	{
		ABDEN=1;				// Auto-Baud Detect Enable bit.
		ABDOVF=1;				// Auto-Baud Acquisition Rollover disable.
	}
	TXEN=0;						// Disable transmitter.
	TXEN=1;						// Enable the transmitter.
	return;
}
/********************************************************************************/
/*			MANDA UNA CADENA DE CARACTERES POR EL PUERTO SERIE.					*/
/*..............................................................................*/
void PutStr(register const char *str)
{
	while(*str)				// Mientras tenga algo para mandar.
		PutCh(*str++);		// Escribo el caracter en el puerto.
	return;
}
/********************************************************************************/
/*			MANDA SOLO UN CARACTER POR EL PUERTO SERIE							*/
/*..............................................................................*/
void PutCh(unsigned char c)
{
	CLRWDT();
	if(c=='ñ')
		c=164;
	if(c=='Ñ')
		c=165;
	if(c=='á')
		c=160;
	if(c=='é')
		c=130;
	if(c=='í')
		c=161;
	if(c=='ó')
		c=162;
	if(c=='ú')
		c=163;
	while(!TXIF)			// En caso de estar ocupado en otra escritura
		continue;
	TXREG=c;				// Escribo el caracter en el puerto.
	return;
}
