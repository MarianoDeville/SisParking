/********************************************************************************/
/*	MENU AUXILIAR PARA APOYO AL INSTALADOR - NECESITA DISPLAY Y TECLADO PS2		*/
/*..............................................................................*/
/*			Revisión:		1.00												*/
/*			PIC:			PIC18F4620											*/
/*			Compilador:		MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.				*/
/*			Autor:			Mariano Ariel Deville								*/
/********************************************************************************/
/*						PROTOTIPOS DE FUNCIONES									*/
/*------------------------------------------------------------------------------*/
unsigned char Comparo_Cadenas(const char *cadena_1,const char *cadena_2,unsigned char largo);
void Reseteo_Teclado(void);
/********************************************************************************/
/*				COMPARO DOS CADENAS DE CARACTERES								*/
/*------------------------------------------------------------------------------*/
unsigned char Comparo_Cadenas(const char *cadena_1,const char *cadena_2,unsigned char largo)
{
	CLRWDT();
	if(!largo)								// Comparo la cadena completa??
	{
		while(*cadena_1||*cadena_2)			// Comparo hasta terminar las dos cadenas.
		{
			if(*cadena_1++!=*cadena_2++)	// Comparo byte a byte de la cadena.
				return 0;					// Hay diferencia asi que salgo y aviso.
		}
	}
	else
	{
		while((*cadena_1||*cadena_2)&&largo)	// Comparo hasta terminar las dos cadenas.
		{
			if(*cadena_1++!=*cadena_2++)	// Comparo byte a byte de la cadena.
				return 0;					// Hay diferencia asi que salgo y aviso.
			largo--;
		}
	}
	return 1;								// Las cadenas son iguales.
}
/********************************************************************************/
/*										*/
/*------------------------------------------------------------------------------*/
void Reseteo_Teclado(void)
{
	DelayMs(20);	// Anti rebote.
	ps2_str[0]=0;	// Reseteo la cadena.
	pos_str_ps2=0;	// Posicion inicial de la cadena de caracteres.
	vis=0;			// Variable relacionada con la visualización.
	lect_ps2=0;		// Vacío la variable lectura del teclado PS2.
	pos_ps2=11;		// Necesario para la correcta captura de teclas.
	mayus_ps2=1;	// Comienzo escribiendo con mayusculas.
	shift_ps2=0;	// Reseteo el SHIFT.
	INT0IF=0;		// Bajo la bandera de la interrupcion.
	INT0IE=1;		// Habilito la interrupción del teclado.
	return;
}
