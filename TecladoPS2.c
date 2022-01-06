/********************************************************************************/
/*			RUTINAS PARA INTERPRETAR UN TECLADO ESTANDAR PS2 DE PC.				*/
/*..............................................................................*/
/*			Revisión:				1.00										*/
/*			PIC:					PIC18F4620									*/
/*			MODEM:					SIM340CZ									*/
/*			Teclado:				PS2											*/
/*			Compilador:				MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.		*/
/*			Autor:					Mariano Ariel Deville						*/
/********************************************************************************/
/*						PROTOTIPOS DE FUNCIONES									*/
/*------------------------------------------------------------------------------*/
#include	"Teclado.h"				// Tabla para interpretar teclado PS2.
void Interpretar_Teclado(void);

/********************************************************************************/
/*		LLENADO DE CARATERES EN UNA CADENA RECIBIDOS DESDE EL TECLADO.			*/
/********************************************************************************/
void Interpretar_Teclado(void)
{											// entonces hago la conversión.
	pos_ps2=11;								// Necesario para la correcta captura de teclas.
	if(lect_ps2==0xf0)						// Solte la tecla que presionaba.
	{
		suelto_tecla_ps2=1;					// Levanto la bandera.
		return;								// Salgo de la interrupción.
	}
	if(suelto_tecla_ps2)					// Omito el caracter de la tecla que se solto.
	{
		suelto_tecla_ps2=0;					// Bajo la bandera.
		if(lect_ps2==0x12||lect_ps2==0x59)
			shift_ps2=0;					// Bajo la bandera del SHIFT.
		if(lect_ps2==0x5a)					// Si es un final de linea proceso el comando.
			INT0IE=0;						// Deshabilito la interrupción.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x66||(lect_ps2==0x71&&ctrl_ps2))	// Se presiono DEL O DELETE???
	{
		if(pos_str_ps2)						// Si 'pos' es distinto de cero
			pos_str_ps2--;					// vuelvo un lugar.
		ps2_str[pos_str_ps2]=0;				// Borro el contenido del lugar.
		ctrl_ps2=0;							// Bajo la bandera de control.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0xe0)						// Presionaron una tecla de control.
	{
		ctrl_ps2=1;							// Levanto la bandera de control.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x58)						// Presionaron la tecla mayusculas.
	{
		mayus_ps2=!mayus_ps2;				// Cambio el balor de la bandera MAYUSCULAS.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x05)						// Presionaron F1.
	{
		pos_str_ps2=2;						// 
		vis=2;								// Fuerzo el refresco de la pantalla.
		ps2_str[0]='I';						//
		ps2_str[1]=' ';						//
		ps2_str[2]=0;						// Marco el final de la cadena.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x06)						// Presionaron F2.
	{
		pos_str_ps2=2;						// 
		vis=2;								// Fuerzo el refresco de la pantalla.
		ps2_str[0]='C';						//
		ps2_str[1]=' ';						//
		ps2_str[2]=0;						// Marco el final de la cadena.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x04)						// Presionaron F3.
	{
		pos_str_ps2=5;						// Apunto al primer lugar de la cadena.
		ps2_str[0]='H';						//
		ps2_str[1]='O';						//
		ps2_str[2]='R';						//
		ps2_str[3]='A';						//
		ps2_str[4]='?';						//
		ps2_str[5]=0;						// Marco el final de la cadena.
		INT0IE=0;							// Deshabilito la interrupción.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x0c)						// Presionaron F4.
	{
		pos_str_ps2=5;						// Apunto al primer lugar de la cadena.
		ps2_str[0]='C';						//
		ps2_str[1]='A';						//
		ps2_str[2]='J';						//
		ps2_str[3]='A';						//
		ps2_str[4]='?';						//
		ps2_str[5]=0;						// Marco el final de la cadena.
		INT0IE=0;							// Deshabilito la interrupción.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x03)						// Presionaron F5.
	{
		pos_str_ps2=5;						// Apunto al primer lugar de la cadena.
		ps2_str[0]='A';						//
		ps2_str[1]='U';						//
		ps2_str[2]='T';						//
		ps2_str[3]='O';						//
		ps2_str[4]='S';						//
		ps2_str[5]=' ';						//
		ps2_str[6]='E';						//
		ps2_str[7]='N';						//
		ps2_str[8]=' ';						//
		ps2_str[9]='P';						//
		ps2_str[10]='L';					//
		ps2_str[11]='A';					//
		ps2_str[12]='Y';					//
		ps2_str[13]='A';					//
		ps2_str[14]=0;						// Marco el final de la cadena.
		INT0IE=0;							// Deshabilito la interrupción.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x0b)						// Presionaron F6.
	{
		pos_str_ps2=5;						// Apunto al primer lugar de la cadena.
		ps2_str[0]='P';						//
		ps2_str[1]='L';						//
		ps2_str[2]='A';						//
		ps2_str[3]='N';						//
		ps2_str[4]='I';						//
		ps2_str[5]='L';						//
		ps2_str[6]='L';						//
		ps2_str[7]='A';						//
		ps2_str[8]=0;						// Marco el final de la cadena.
		INT0IE=0;							// Deshabilito la interrupción.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x83)						// Presionaron F7.
	{
		pos_str_ps2=5;						// Apunto al primer lugar de la cadena.
		ps2_str[0]='I';						//
		ps2_str[1]='M';						//
		ps2_str[2]='P';						//
		ps2_str[3]='R';						//
		ps2_str[4]='I';						//
		ps2_str[5]='M';						//
		ps2_str[6]='I';						//
		ps2_str[7]='R';						//
		ps2_str[8]=' ';						//
		ps2_str[9]='C';						//
		ps2_str[10]='A';					//
		ps2_str[11]='J';					//
		ps2_str[12]='A';					//
		ps2_str[13]='S';					//
		ps2_str[14]=0;						// Marco el final de la cadena.
		INT0IE=0;							// Deshabilito la interrupción.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x0a)						// Presionaron F8.
		return;								// Salgo de la interrupción.
	if(lect_ps2==0x01)						// Presionaron F9.
		return;								// Salgo de la interrupción.
	if(lect_ps2==0x09)						// Presionaron F10.
	{
		pos_str_ps2=2;						// 
		ps2_str[0]='?';						//
		ps2_str[1]=0;						// Marco el final de la cadena.
		INT0IE=0;							// Deshabilito la interrupción.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x78)						// Presionaron F11.
		return;								// Salgo de la interrupción.
	if(lect_ps2==0x07)						// Presionaron F12.
		return;								// Salgo de la interrupción.
	if(lect_ps2==0x76)						// Presionaron la tecla ESC borro la linea.
	{
		pos_str_ps2=0;						// Apunto al primer lugar de la cadena.
		ps2_str[0]=0;						// Marco el final de la cadena.
		vis=0;
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x12||lect_ps2==0x59)		// Presionaron SHIFT
	{
		shift_ps2=1;						// Levanto la bandera del SHIFT.
		return;								// Salgo de la interrupción.
	}
	if(lect_ps2==0x5a)						// Si es un final de linea proceso el comando.
	{
		ps2_str[pos_str_ps2]=0;				// Marco el final de la cadena.
		return;								// Salgo de la interrupción.
	}
	cont_ps2=0;
	do										// Recorro la tabla en busca del ASCII.
	{
		CLRWDT();
		if(lect_ps2==ps2[cont_ps2][0])		// Comparo el codigo leido con la tabla.
		{
			if(!mayus_ps2&&!shift_ps2)		// Estaba presionada la tecla MAYUSCULA???
			{
				ps2_str[pos_str_ps2]=ps2[cont_ps2][1];
				break;
			}
			if(mayus_ps2&&!shift_ps2)		// Estaba presionada la tecla SHIFT???
			{
				ps2_str[pos_str_ps2]=ps2[cont_ps2][2];
				break;
			}
			if(!mayus_ps2&&shift_ps2)		// Estaba presionada la tecla MAYUSCULA y SHIFT???
			{
				ps2_str[pos_str_ps2]=ps2[cont_ps2][3];
				break;
			}
			if(mayus_ps2&&shift_ps2)		// Estaba presionada la tecla MAYUSCULA y SHIFT???
				ps2_str[pos_str_ps2]=ps2[cont_ps2][4];
			break;							// Una vez encontrado salgo del bucle.
		}
	}while(++cont_ps2<64);					// Busco hasta el último caracter de la tabla.
	if(pos_str_ps2<33)						// Limito a 30 caracteres el comando.
		pos_str_ps2++;						// Voy al siguiente lugar en la cadena.
	ps2_str[pos_str_ps2]=0;					// Marco el final de la cadena
	return;									// Salgo de la interrupción.
}
