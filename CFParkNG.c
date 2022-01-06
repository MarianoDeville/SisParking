/********************************************************************************/
/*						CFPARK ECONOMICO										*/
/*..............................................................................*/
/*			Revisión:				1.00										*/
/*			PIC:					PIC18F4620									*/
/*			MODEM:					SIM340CZ									*/
/*			RTC:					DS1307										*/
/*			Memoria:				24LC256.									*/
/*			Display:				LCD 20 caracteres por 2 lineas.				*/
/*			Teclado:				PS2											*/
/*			Impresora:				EPSON TM-T88.								*/
/*			Comunicación:			RS232 - I2C.								*/
/*			Compilador:				MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.		*/
/*			Fecha de creación:		14/10/2009									*/
/*			Fecha de modificación:	15/10/2012									*/
/*			Autor:					Mariano Ariel Deville						*/
/********************************************************************************/
/*								MACROS											*/
/*..............................................................................*/
#define		PIC_CLK			20000000	// 20Mhz.								*/
#define		ENTRADA			1			//										*/
#define		SALIDA			0			//										*/
//#define		ESTADIA						// Habilita el uso de estadia diaria.	*/
//#define		USO_MODEM					// Habilito el uso del modem.			*/
#define 	MODO_IMP		RD2=0;RC5=1	// Sal./ent. RS232 a la impresora.		*/
#define		MODO_MODEM		RD2=1;RC5=0	// Sal./ent. RS232 al MODEM.			*/
#define		MODO_PC			RD2=1;RC5=1	// Sal./ent. RS232 exclusiva a PC.		*/
/*------------------------------------------------------------------------------*/
/*				Defino los nombres de los pines de E/S							*/
/*..............................................................................*/
#define		DATO_PS2		RD7			// Datos desde un teclado PS2.			*/
#define		CLOCK_PS2		RB0			// Clock desde un teclado PS2.			*/
#define		DCD				RB2			// Entrada DCD desde el MODEM.			*/
#define		POWER_MODEM		RB4			// Manejo el POWER del MODEM.			*/
#define		LED				RB1			// LED DEBUG.							*/
#define		NET_STATE		RB5			// Estado de conección del MODEM.		*/
/********************************************************************************/
/*						VARIABLES GLOBALES										*/
/*..............................................................................*/
bank1 unsigned char rs_str[70];			// Cadena para la recepción RS232.		*/
bank1 unsigned int pagina;				//										*/
bank2 unsigned char pos,vis;			// 										*/
bank2 bit bandera;						// Banderas.							*/
/********************************************************************************/
/*						PROTOTIPO DE FUNCIONES									*/
/*..............................................................................*/
void Interpreto_Comando(void);
unsigned char Cobro_Ticket(void);
void Cierre_Caja(void);
void Procesar_SMS(unsigned char mensaje);
void Visualizar_Caja(void);
void Contar_Autos(void);
void Ayuda(void);
void Limpiar_Cadena(unsigned char *str,unsigned char longitud);
void IntToStr(unsigned int origen,register unsigned char *destino);
unsigned char Estadia_Diaria(void);
void Guardo_Arranque_Turno(unsigned char guardar);
/********************************************************************************/
/*							LIBRERIAS											*/
/*..............................................................................*/
#include	"htc.h"				// Necesario para el compilador.				*/
#include	"Delay.c"			// Rutinas de demoras.							*/
#include	"RS232.c"			// Configuración y comunicación puerto serie.	*/
#include	"Lcd.c"				// Rutina de manejo de un display LCD.			*/
#include	"EEPROM.c"			// Manejo memoria EEPROM interna del PIC.		*/
#include	"I2C.c"				// Manejo del módulo I2C interno del PIC.		*/
#include	"24LC256.c"			// Manejo de la memoria externa I2C 24LC256.	*/
#include	"DS1307.c"			// Manejo del Reloj de Tiempo Real DS1307.		*/
#include	"TecladoPS2.c"		// Rutina de interpretación PS2.				*/
#include	"Utilidades.c"		// Funciones generales varias.					*/
#include	"TM-T88.c"			// Driver de la impresora Epson TM-T88.			*/
#if defined(USO_MODEM)			//												*/
#include	"SIM300SM.c"		// Driver del MODEM GSM/GPRS.					*/
#endif							//												*/
#include	"Interrup.c"		// Manejo de interrupciones.					*/
#include	"MenuAdmin.c"		// Menu administrativo y de mantenimiento.		*/
/********************************************************************************/
__CONFIG(1,IESOEN & FCMEN & HS);					//							*/
__CONFIG(2,BOREN & WDTEN & PWRTEN);					//							*/
__CONFIG(3,MCLREN & LPT1DIS & PBDIGITAL);			//							*/
__CONFIG(4,XINSTEN & DEBUGDIS & LVPDIS & STVREN);	//							*/
//__CONFIG(5,UNPROTECT);								//							*/
__CONFIG(5,CPALL);									//							*/
__CONFIG(6,WRTEN);									//							*/
/********************************************************************************/
void main(void)
{
	unsigned char mensajes,remitente[20];
/********************************************************************************/
/*			Configuración de los puertos										*/
/*..............................................................................*/
	ADCON1=0b00001111;		// Sin entradas analógicas.							*/
	PORTA=0;				// Reseteo el puerto.								*/
	PORTB=0b00010000;		//													*/
	PORTC=0;				//													*/
	PORTD=0;				//													*/
	PORTE=0;				//													*/
/*------------------------------------------------------------------------------*/
	TRISA0=SALIDA;	   		// Salida para el LCD RS.							*/
	TRISA1=SALIDA;   		// Salida para el LCD E.							*/
	TRISA2=SALIDA; 	  		// Salida para el LCD AD4.							*/
	TRISA3=SALIDA;   		// Salida para el LCD AD5.							*/
	TRISA4=SALIDA;   		// Salida para el LCD AD6.							*/
	TRISA5=SALIDA;   		// Salida para el LCD AD7.							*/
/*------------------------------------------------------------------------------*/
	TRISB0=ENTRADA;			// PS2 - CLOCK.										*/
	TRISB1=SALIDA;			// Salida al LED de ESTADO.							*/
	TRISB2=ENTRADA;			// DCD del MODEM.									*/
	TRISB3=ENTRADA;			// ICSP.											*/
	TRISB4=SALIDA;			// Manejo POWER del MODEM							*/
	TRISB5=ENTRADA;			// Estado del MODEM (NET STATE).					*/
	TRISB6=ENTRADA;			// ICSP.											*/
	TRISB7=ENTRADA;			// ICSP.											*/
/*------------------------------------------------------------------------------*/
	TRISC0=ENTRADA;			// Entrada auxiliar C 1.							*/
	TRISC1=ENTRADA;			// Entrada auxiliar C 2.							*/
	TRISC2=ENTRADA;			// Entrada auxiliar C 3.							*/
	TRISC3=ENTRADA;			// I2C - SCL serial clock.							*/
	TRISC4=ENTRADA;			// I2C - SDA serial data.							*/
	TRISC5=SALIDA;			// Habilitación RS232 para el MODEM.				*/
	TRISC6=SALIDA;			// RS232 - Salida TX.								*/
	TRISC7=ENTRADA;			// RS232 - Entrada RX.								*/
/*------------------------------------------------------------------------------*/
	TRISD0=ENTRADA;			// Entrada auxiliar D 1.							*/
	TRISD1=ENTRADA;			// Entrada auxiliar D 2.							*/
	TRISD2=SALIDA;			// Habilitación RS232 para la impresora.			*/
	TRISD3=SALIDA;			//													*/
	TRISD4=SALIDA;			//													*/
	TRISD5=SALIDA;			//													*/
	TRISD6=SALIDA;			//													*/
	TRISD7=ENTRADA;			// PS2 - DATOS.										*/
/*------------------------------------------------------------------------------*/
	TRISE0=SALIDA;			//													*/
	TRISE1=SALIDA;			//													*/
	TRISE2=SALIDA;			//													*/
/********************************************************************************/
/*			TIMER 0 - NO UTILIZADO												*/
/*..............................................................................*/
	T0CS=0;					// Oscilador interno.								*/
	T0SE=0;					// Flanco ascendente.								*/
	PSA=1;					// Asigno el preescaler a WDT.						*/
	TMR0IF=0;				// Bajo la bandera de la interrupción.				*/
/********************************************************************************/
/*			TIMER 1 - Lo utilizo para calcular los 10 min entre envio y envio.	*/
/*..............................................................................*/
	T1CKPS0=1; 				// Preescaler TMR1 a 1:8.							*/
	T1CKPS1=1; 				//													*/
	T1SYNC=1;				// No sincronizo con clock externo.					*/
	T1OSCEN=0;				// Oscilador deshabilitado.							*/
	TMR1CS=0;  				// Reloj interno Fosc/4.							*/
	TMR1IF=0;				// Bajo la bandera de la interrupción.				*/
	TMR1ON=0;				// Apago el TMR1.									*/
	TMR1H=0;				// Configuro el tiempo que tarda en generar			*/
	TMR1L=0;				// la interrupcion.									*/
/********************************************************************************/
/*			TIMER 2 - NO UTILIZADO												*/
/*..............................................................................*/
	TMR2ON=0;				// Timer 2 apagado.									*/
	T2CKPS0=0;				// Configuro el Preescaler.							*/
	T2CKPS1=0;				// 													*/
	TMR2IF=0;				// Bajo la bandera de la interrupción.				*/
/********************************************************************************/
/*			Configuración de las interrupciones									*/
/*..............................................................................*/
	IPEN=0;					// Deshabilito las prioridades para las int.		*/
	GIE=1;					// Utilizo interrupciones.							*/
	PEIE=1;					// Interrupcion externa habilitada.					*/
	INT0IE=0;				// Interrupcion RB0/INT deshabilitada.				*/
	INT1IE=0;				// Interrupcion RB1/INT deshabilitada.				*/
	INT2IE=0;				// Interrupcion RB2/INT deshabilitada.				*/
	TMR0IE=0;				// Interrupcion desborde TMR0 deshabilitada.		*/
	TMR1IE=0;				// Interrupcion desborde TMR1 deshabilitada.		*/
	TMR2IE=0;				// Interrupcion desborde TMR2 deshabilitada.		*/
	CCP1IE=0;				// CCP1 Interrupt disable.							*/
	CCP2IE=0;				// CCP2 Interrupt disable.							*/
	CMIE=0;					// Comparator Interrupt disable.					*/
	EEIE=0;					// EEPROM Write Operation Interrupt disable.		*/
	SSPIE=0;				// Interrupcion por comunicacion I2C.				*/
	PSPIE=0;				// Slave Port Read/Write Interrupt disable.			*/
	BCLIE=0;				// Bus Collision Interrupt disable.					*/
	ADIE=0;					// Interrupcion del conversor AD deshabilitada.		*/
	RBIE=0;					// Interrupcion por RB deshabilitada.				*/
	RCIE=0;					// Interrupcion recepcion USART habilitada.			*/
 	INTEDG0=0;				// Interrupcion en el flanco descendente de RB0.	*/
	RBPU=1;					// RB pull-ups estan deshabilitadas.				*/
/********************************************************************************/
	POWER_MODEM=1;						// MODEM apagado hasta necesitarlo.
	Lcd_Setup();						// Inicializo el LCD.
	Imprimir_Lcd("PUESTO DE CAJA","Inicializando...",1);
	I2C_Setup();						// Configuro la comunicacion I2C.
	Serial_Setup(9600);					// Setea el puerto serie.
#if defined(USO_MODEM)
	Modem_Power(1,4);					// Prendo el MODEM.
#endif
	LED=1;								// Led de POWER encendido.
	DelayS(1);							// Tiempo para que arranque el teclado.
	bandera=1;							// Modo de lectura del RS232.
	Guardo_Arranque_Turno(0);			// Guardo la hora de arranque del turno.
	Reseteo_Teclado();					// Reinicio la cauptura por teclado PS2.
	for(;;)
	{
		while(INT0IE)					// Espero un enter del teclado
		{								// para procesar el comando.
			if(pos_str_ps2<vis-1)		// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)		// Actualiazo la pantalla?
			{
				Lcd_Clear();
				Lcd_Goto(0);
				Lcd_Putch(':');			// Promp del sistema.
				Lcd_Puts(ps2_str);		// Muestro lo que se va escribiendo.
				vis++;					// 
			}
			if(pos_str_ps2>18)			// Elimino el refresco.
			{
				pos_str_ps2=18;
				vis=19;
			}
			CLRWDT();
		}
		Interpreto_Comando();			// 
		Reseteo_Teclado();				// Reinicio la cauptura por teclado PS2.
		while(!pos_str_ps2&&INT0IE)		// Espero a que se teclee algo.
		{
			DelayMs(1);
#if defined(USO_MODEM)
			if(pagina++>30000)			// Mientras espero controlo si entro un SMS.
			{
				pagina=0;
				mensajes=SMS_Recibido();	// Recupero la cantidad de SMS en cola.
				if(mensajes)			// Hay SMS en cola?
				{
					INT0IE=0;			// Deshabilito el teclado.
					INT0IF=0;
					Limpiar_Cadena(rs_str,70);
					Procesar_SMS(mensajes);
					Reseteo_Teclado();	// Reinicio la cauptura por teclado PS2.					
				}
			}
#endif
		}
		vis=pos_str_ps2;
	}
}
/********************************************************************************/
/*				Interpreto el comando leido por teclado.						*/
/*..............................................................................*/
void Interpreto_Comando(void)
{
	unsigned int i,vehiculo,e;
	unsigned char alma;
	CREN=0;										// Deshabilito la recepcion continua en RS232.
	RCIE=0;										// Deshabilito la interrupcion por RS232.
	Limpiar_Cadena(rs_str,70);					// Borro todo el contenido de la cadena.
	i=0;
	if(!pos_str_ps2)
	{
		Linea_Lcd(":",1);
		return;
	}
	while(ps2_str[i])
	{
		if(ps2_str[i]>96&&ps2_str[i]<123)
			ps2_str[i]=ps2_str[i]-32;			// Convierto todo a mayusculas.
		i++;
	}
	if(Comparo_Cadenas(ps2_str,"I ",2))			// Debo imprimir un ticket de entrada??
	{
		if(!ps2_str[2])
		{
			Imprimir_Lcd(":","Error.",1);
			return;
		}
		i=2;
		e=0;
		do
		{
			if(e>7)
			{
				Imprimir_Lcd(":","Patente muy larga.",1);
				return;
			}
			if((ps2_str[i]>64&&ps2_str[i]<91)||(ps2_str[i]>47&&ps2_str[i]<58))
				rs_str[e++]=ps2_str[i];
		}while(ps2_str[i++]);					// Depuro la patende.
		rs_str[e]=0;							// Marco el final de la cadena.
		Reseteo_Teclado();						// Reinicio la cauptura por teclado PS2.
		while(1)								// Espero que defina el vehículo.
		{
			if(pos_str_ps2==vis)				// Actualiazo la pantalla?
			{
				Imprimir_Lcd("Auto Moto Utilitario",ps2_str,1);
				if(ps2_str[0]=='A'||ps2_str[0]=='M'||ps2_str[0]=='U'||ps2_str[0]=='C')
				{
					INT0IE=0;					// Deshabilito la interrupción del teclado.
					break;
				}
				pos_str_ps2=0;
				vis=1;							// 
			}
			CLRWDT();
			INT0IE=1;							// Deshabilito la interrupción del teclado.
		}
		vehiculo=ps2_str[0]+32;					// Lo transformo en minusculas.
		if(ps2_str[0]=='C')						// Tarifa cancelo la operación?
		{
			Imprimir_Lcd(":","Operacion cancelada.",1);
			return;
		}
		Obtener_Hora(ps2_str,1);
		Imprimir_Ticket(rs_str,ps2_str);
		i=0;
		do
		{
			rs_str[i+14]=rs_str[i];
		}while(rs_str[i++]);
		rs_str[i+14]=0;							// Me aseguro de marcar el final de cadena.
		rs_str[0]='P';							// Marco como vehículo en playa.
		rs_str[1]=vehiculo;						// Indico que vehículo es.
		i=0;
		pos_str_ps2=2;
		do
		{
			if(ps2_str[i]!=':'&&ps2_str[i]!=' '&&ps2_str[i]!='/')
				rs_str[pos_str_ps2++]=ps2_str[i];
			i++;
		}while(ps2_str[i]);
		Alma_Paquete_24LC(rs_str);				// Almaceno el ticket en memoria.
		Linea_Lcd(":",1);						// Promp del sistema.
		return;
	}
	if(Comparo_Cadenas(ps2_str,"C ",2))			// Debo cobrar un ticket??
	{
		for(pagina=0;pagina<2048;pagina++)
		{
			Rec_Paquete_24LC(pagina,rs_str);
			if(Cobro_Ticket())
				break;
		}
		if(pagina>=2048)						// Termine sin encontrar nada.
			Imprimir_Lcd(":","Vehiculo no encontrado.",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"A ",2))			// Debo anular un ticket??
	{
		for(pagina=0;pagina<2048;pagina++)
		{
			Rec_Paquete_24LC(pagina,rs_str);
			if(Anular_Ticket(0))
				break;
		}
		if(pagina>=2048)						// Termine sin encontrar nada.
			Imprimir_Lcd(":","Vehiculo no encontrado.",1);
		return;
	}
#if defined(ESTADIA)
	if(Comparo_Cadenas(ps2_str,"ESTADIA",6))	// Debo anular un ticket??
	{
		Reseteo_Teclado();						// Reinicio la cauptura por teclado PS2.
		while(INT0IE)							// Espero un enter del teclado
		{										// para procesar el comando.
			if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)				// Actualiazo la pantalla?
			{
				Imprimir_Lcd("Ingrese patente:",ps2_str,1);
				vis++;							// 
			}
			CLRWDT();
		}
		for(pagina=0;pagina<2049;pagina++)
		{
			Rec_Paquete_24LC(pagina,rs_str);
			alma=Estadia_Diaria();
			if(alma)
				break;
		}
		if(pagina>=2048&&!alma)					// Termine sin encontrar nada.
			Imprimir_Lcd(":","Auto no encontrado.",1);
		if(alma==1)
			Imprimir_Lcd(":","Estadia cobrada.",1);
		if(alma==2)
			Imprimir_Lcd(":","Operacion cancelada.",1);
		if(alma==3)
			Imprimir_Lcd(":","Tarifa no definida.",1);
		return;
	}
#endif
	if(Comparo_Cadenas(ps2_str,"CERRAR CAJA",0))
	{
		Cierre_Caja();
		Linea_Lcd(":",1);						// Promp del sistema.
		return;
	}
	if(Comparo_Cadenas(ps2_str,"HORA?",0))
	{
		Linea_Lcd(":",1);
		Visualizar_Hora();
		return;
	}
	if(Comparo_Cadenas(ps2_str,"CAJA?",0))
	{
		Linea_Lcd(":",1);
		Visualizar_Caja();
		return;
	}
	if(Comparo_Cadenas(ps2_str,"AUTOS EN PLAYA",0))
	{
		Imprimir_Lcd(":","Procesando...",1);
		Contar_Autos();
		return;
	}
	if(Comparo_Cadenas(ps2_str,"ADMIN",0))		//
	{
		Reseteo_Teclado();						// Reinicio la cauptura por teclado PS2.
		while(INT0IE)							// Espero un enter del teclado
		{										// para procesar el comando.
			if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)				// Actulizo la pantalla?
			{
				Linea_Lcd("Ingrese contraseña:",1);	// Muestro lo que se va escribiendo.
				Lcd_Goto(0x40);
				i=0;
				while(i<pos_str_ps2&&pos_str_ps2)	// Escondo lo que se escribe.
				{
					i++;
					Lcd_Putch('*');					// Muestro en display *.
				}
				vis++;								// 
			}
			CLRWDT();
		}
		if(Check_Pass(ps2_str,1))
			Menu_Teclado(1);
		else
			Imprimir_Lcd(":","Contraseña invalida.",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"ROOT",0))			//
	{
		Reseteo_Teclado();							// Reinicio la cauptura por teclado PS2.
		while(INT0IE)								// Espero un enter del teclado
		{											// para procesar el comando.
			if(pos_str_ps2<vis-1)					// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)					// Actulizo la pantalla?
			{
				Linea_Lcd("Ingrese contraseña:",1);	// Muestro lo que se va escribiendo.
				Lcd_Goto(0x40);
				i=0;
				while(i<pos_str_ps2&&pos_str_ps2)	// Escondo lo que se escribe.
				{
					i++;
					Lcd_Putch('*');					// Muestro en display *.
				}
				vis++;								// 
			}
			CLRWDT();
		}
		if(Check_Pass(ps2_str,0))
			Menu_Teclado(0);
		else
			Imprimir_Lcd(":","Contraseña invalida.",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"?",0))				// Imprimo la ayuda.
	{
		Ayuda();
		Linea_Lcd(":",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"IMPRIMIR CAJAS",0))	// Salgo sin hacer nada.
	{
		Imprimir_Lcd(":","",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"PLANILLA",0))		// Salgo sin hacer nada.
	{
		Imprimir_Lcd(":","",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"ESTADISTICAS",0))	// Salgo sin hacer nada.
	{
		Imprimir_Lcd(":","",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"TELEFONO?",0))		// Salgo sin hacer nada.
	{
		Imprimir_Lcd(":","",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"APAGAR",0))			// Apagado del equipo.
	{
		Imprimir_Lcd("Puede apagar","el equipo.",1);
		Eeprom_Write(108,1);						// Para que guarde el arranque.
		while(1)
			CLRWDT();
	}
	Imprimir_Lcd(":","Comando inexistente.",1);
	return;
}
/********************************************************************************/
/*		Calculo monto a cobrar y guardo en memoria estadisticas del cobro.		*/
/*..............................................................................*/
unsigned char Cobro_Ticket(void)
{
	unsigned char hora_t[2],hora_c[2],tiempo_c[6],hora_f[5],monto[6];
	unsigned char rango_hor[3],t_rango[3],promo[2],aux[2],hora_dif_tar[3];
	unsigned char horas,minutos,dia,tol[2],tarifa,vehiculo,frac;
	unsigned int lugar,monto_n,i,calc_monto[4];
	if(rs_str[0]!='P')							// El ticket es para cobrar??
		return 0;
	i=1;
	while(ps2_str[i++])							// Comparo si es el ticket que debo cobrar.
	{
		if(ps2_str[i]!=rs_str[i+12])			// Comparo
			return 0;							// Si son distintos salgo y sigo con otro.
	}
	lugar=i+11+(pagina*32);						// Direccion necesaria luego.
	Obtener_Hora(ps2_str,0);					// Obtengo la hora actual, hora de cobro.
	i=0;
	do
	{
		hora_f[i]=ps2_str[i+6];					// Almaceno la hora de facturacion como cadena.
	}while(i++<4);
	hora_f[4]=0;								// Marco final de cadena.
	horas=0;									// Reseteo variables.
	minutos=0;
	dia=0;
	aux[0]=((ps2_str[0]-48)*10)+(ps2_str[1]-48);		// Paso a numérico el día de cobro.
	aux[1]=((rs_str[2]-48)*10)+(rs_str[3]-48);			// Paso a numérico el día de emisión.
	if(aux[0]<aux[1])									// Estoy en el mes que sigue?
		aux[0]=aux[0]+aux[1];							// Corrijo la cantidad de días.
	dia=(aux[0]-aux[1])*24;								// Calculo la corrección para la cantidad de horas.
	hora_c[0]=(ps2_str[7]-48)+((ps2_str[6]-48)*10)+dia;	// Transformo el char de las horas en num.
	hora_c[1]=(ps2_str[9]-48)+((ps2_str[8]-48)*10);		// Transformo el char de los minutos en num.
	hora_t[0]=(rs_str[9]-48)+((rs_str[8]-48)*10);		// Transformo el char en num y corrijo en caso de que salga al dia siguiente.
	hora_t[1]=(rs_str[11]-48)+((rs_str[10]-48)*10);		// Transformo el char en num.
	horas=hora_c[0]-hora_t[0];					// Obtengo las horas a cobrar.
	if(hora_c[1]<hora_t[1])						// Minutos de entrada mayor a los de salida.
	{
		horas--;								// Resto una hora en la parte horas.
		minutos=60;								// Agrego la hora restada en los minutos.
	}
	minutos=(minutos+hora_c[1])-hora_t[1];		// Obtengo los minutos a cobrar.
// tiempo_c lo uso para imprimir la cantidad de hh:mm cobrados.
	if(horas<10)
	{
		tiempo_c[0]=48;
		tiempo_c[1]=horas+48;
	}
	else
	{
		tiempo_c[0]=(horas/10)+48;
		tiempo_c[1]=(horas-((tiempo_c[0]-48)*10))+48;
	}
	tiempo_c[2]=':';
	if(minutos<10)
	{
		tiempo_c[3]=48;
		tiempo_c[4]=minutos+48;
	}
	else
	{
		tiempo_c[3]=(minutos/10)+48;
		tiempo_c[4]=(minutos-((tiempo_c[3]-48)*10))+48;
	}
	tiempo_c[5]=0;
	if(rs_str[1]=='a')							// Necesario para saber la terifa correspondiente.
		vehiculo=0;
	if(rs_str[1]=='m')
		vehiculo=1;
	if(rs_str[1]=='u')
		vehiculo=2;
	promo[0]=Eeprom_Read(200);					// Cargo el rango para la promocion de media hora.
	promo[1]=Eeprom_Read(201);
	monto_n=0;
// Si entra en la promocion de media hora calculo el monto a pagar.
	if(!horas&&minutos<=30&&promo[0]!=promo[1])		// Condiciones para entrar en la promo
	{
		if(hora_c[0]>promo[0]&&hora_c[0]<promo[1])	// Esta dentro del horario de la promo?
		{
			monto_n=Eeprom_Read(202+vehiculo)*10;	// Calculo el monto a cobrar.
			i=178+vehiculo;
		}
	}
	calc_monto[1]=0;
	calc_monto[3]=0;
// Si no entro en la promoción debo calcular el monto a pagar.
	if(!monto_n)									// En caso de que no entro en la promoción.
	{
		tarifa=0;									// reseteo la tarifa.
		tol[0]=Eeprom_Read(220);					// Cargo la tolerancia horaria.
		tol[1]=Eeprom_Read(221);					// Cargo la tolerancia para la media hora.
		rango_hor[0]=Eeprom_Read(150);				// Obtengo los rangos horarios.
		rango_hor[1]=Eeprom_Read(151);
		rango_hor[2]=Eeprom_Read(152);
		frac=Eeprom_Read(153);						// Cargo la configuración del fraccionamiento.
		t_rango[0]=rango_hor[1]-rango_hor[0];		// Tiempo que dura el primer rango horario.
		t_rango[1]=rango_hor[2]-rango_hor[1];		// Tiempo que dura el segundo rango horario.
		t_rango[2]=(rango_hor[0]+24)-rango_hor[2];	// Tiempo que dura el tercer rango horario.
		if(hora_t[0]>=rango_hor[0]&&hora_t[0]<rango_hor[1])
		{
			i=160;
			aux[0]=0;							// Estoy en el primer rango horario.
		}
		else
		{
			if((hora_t[0]>=rango_hor[1])&&(hora_t[0]<rango_hor[2]))
			{
				i=163;
				aux[0]=1;						// Estoy en el segundo rango horario.
			}
			else
			{
				i=166;
				aux[0]=2;						// Estoy en el tercer rango horario.
			}
		}
		i=i+vehiculo;							// Lugar de memória en donde está la tarifa correspondiente.
		tarifa=Eeprom_Read(i);
		if(minutos>tol[0])						// Pase la tolerancia?
		{
			if(frac=='S'&&minutos<(31+tol[1])&&horas)	// Debo fraccionar la hora?
			{
				calc_monto[3]=Eeprom_Read(i+9)*10;	// Le sume el valor de la media hora.
				if(!calc_monto[3])
					calc_monto[3]=(tarifa*10)/2;
			}
			else
				horas++;						// Incremento la cant. de horas a cobrar.
		}
		else
			minutos=0;
		if(!horas)								// Salgo a la misma hora que entré?
			horas=1;							// Cobro una hora.
		hora_dif_tar[0]=0;
		hora_dif_tar[1]=0;
		if(!aux[0])
		{
			monto[0]=Eeprom_Read(i+3);			// Tarifas de las bandas anteriores.
			monto[1]=Eeprom_Read(i+6);
			aux[1]=rango_hor[1]-hora_t[0];		// Calculo cuantas horas quedan en este rango.
		}
		if(aux[0]==1)
		{
			monto[0]=Eeprom_Read(i+3);			// Tarifas de las bandas anteriores.
			monto[1]=Eeprom_Read(i-3);
			aux[1]=rango_hor[2]-hora_t[0];		// Calculo cuantas horas quedan en este rango.
		}
		if(aux[0]==2)
		{
			monto[0]=Eeprom_Read(i-3);			// Tarifas de las bandas anteriores.
			monto[1]=Eeprom_Read(i-6);
			aux[1]=rango_hor[0]-hora_t[0];		// Calculo cuantas horas quedan en este rango.
		}
		if(aux[1]<horas)						// La cantidad de horas supera el rango?
			hora_dif_tar[0]=horas-aux[1];
		horas=horas-hora_dif_tar[0];
// Calculo del total a pagar.
		calc_monto[0]=tarifa*horas*10;
		calc_monto[1]=hora_dif_tar[0]*monto[0]*10;
		calc_monto[2]=hora_dif_tar[1]*monto[1]*10;
		monto_n=calc_monto[0]+calc_monto[1]+calc_monto[2]+calc_monto[3];
	}
// muestro en display el monto y doy la oportunidad de cancelar el cobro
	IntToStr(monto_n,monto);
	Linea_Lcd("Tiempo: ",1);
	tiempo_c[5]=0;								// Aseguro el final de cadena.
	Lcd_Puts(tiempo_c);
	Lcd_Goto(0x40);
	Lcd_Puts("A pagar: $");
	monto[5]=0;									// Aseguro el final de cadena.
	Lcd_Puts(monto);
	Reseteo_Teclado();							// Reinicio la cauptura por teclado PS2.
	while(INT0IE)								// Espero un enter del teclado
	{
		pos_str_ps2=0;
		CLRWDT();
	}
	Imprimir_Lcd("Cobrar ticket?","S/N",1);
	Reseteo_Teclado();							// Reinicio la cauptura por teclado PS2.
	while(1)									// Espero un S/N del teclado.
	{
		pos_str_ps2=0;							// Solo necesito obtener un caracter.
		if(ps2_str[0]=='N')						// Salgo a la pantalla principal.
		{
			Imprimir_Lcd(":","Cobro cancelado.",1);
			return 2;
		}
		if(ps2_str[0]=='S')						// Seguir con el cobro.
		{
			INT0IE=0;							// Bajo la interrupción y continuo. 
			break;
		}
		INT0IE=1;								// Si apretaron un ENTER pero no ingresaron S/N vuelvo a levantar la interrupción.
		CLRWDT();
	}
// guardo el cobro en la memoria y actualizo las estadisticas.
	monto_n=monto_n+Caja_Read();				// Leo de la Eeprom la caja actual y le sumo el cobro.
	Caja_Write(monto_n);						// Guardo la nueva caja.
	Guardo_Estadisticas(i-160);					// Guardo la estadistica del cobro.
	Write_24LC((pagina*32),'C');				// Marco la entrada como cobrada.
	Write_24LC(lugar++,' ');
	i=0;
	while(hora_f[i])
		Write_24LC(lugar++,hora_f[i++]);
	i=0;
	while(monto[i])
		Write_24LC(lugar++,monto[i++]);
	Write_24LC(lugar,0);						// Marco el final con un NULL.
	Imprimir_Lcd(":","Ticket cobrado.",1);
	return 1;
}
/********************************************************************************/
/*	Cierro la caja y guardo en memoria del PIC las estadísticas del turno.		*/
/*..............................................................................*/
void Cierre_Caja(void)
{
	volatile unsigned char temporal,a;
 	unsigned int lugar;
	volatile unsigned int e;
	lugar=250;									// Lugar donde almaceno la estadisticas viejas.
	do											// Recorro los bytes de memoria superior.
	{
		temporal=Eeprom_Read(lugar);			// Busco un lugar libre.
		if(temporal!='#')
			break;
		lugar=lugar+41;							// Tamaño de la pagina.
	}while(lugar<1020);
	if(lugar>981)
		lugar=250;
	else
		Eeprom_Write(lugar+41,0);				// Dejo libre la siguiente pos.
	Eeprom_Write(lugar++,'#');					// Marco el comienzo.
	Imprimir("Cierre de caja\n",0);
	Imprimir_Estadisticas();					// Imprimo las estadisticas de la playa.
	Imprimir_Movimientos_Cobrados();			// Imprimo los movimientos cobrados y los borro.
	Obtener_Hora(rs_str,0);						// Hora en la que reliazo el cierre de caja.
	temporal=Eeprom_Read(240);					// Leo la parte alta de la caja.
	Eeprom_Write(240,0);
	Eeprom_Write(lugar++,temporal);
	temporal=Eeprom_Read(241);					// Leo la parte bajade la caja.
	Eeprom_Write(241,0);
	Eeprom_Write(lugar++,temporal);
	temporal=((rs_str[0]-48)*10)+(rs_str[1]-48);	// Fecha
	Eeprom_Write(lugar++,temporal);
	temporal=((rs_str[2]-48)*10)+(rs_str[3]-48);	// Mes
	Eeprom_Write(lugar++,temporal);
	temporal=((rs_str[4]-48)*10)+(rs_str[5]-48);	// Año
	Eeprom_Write(lugar++,temporal);
	temporal=((rs_str[6]-48)*10)+(rs_str[7]-48);	// Hora
	Eeprom_Write(lugar++,temporal);
	temporal=((rs_str[8]-48)*10)+(rs_str[9]-48);	// Minutos
	Eeprom_Write(lugar++,temporal);
	for(e=0;e<5;e++)							// Copio la hora de apertura.
	{
		temporal=Eeprom_Read(e+109);			// Cargo el contenido de la memoria.
		Eeprom_Write(e+109,0);					// Borro las estadisticas que ya copie.
		Eeprom_Write(lugar++,temporal);			// Lo almaceno en el nuevo lugar.
	}
	for(e=0;e<29;e++)							// Copio las estadisticas en el cierre.
	{
		temporal=Eeprom_Read(e+80);				// Cargo el contenido de la memoria.
		Eeprom_Write(e+80,0);					// Borro las estadisticas que ya copie.
		Eeprom_Write(lugar++,temporal);			// Lo almaceno en el nuevo lugar.
	}
	Guardo_Arranque_Turno(1);					// Guardo el comienzo del siguiente turno.
	return;
}
/********************************************************************************/
/*					Muestro el monto actual de la caja.							*/
/*..............................................................................*/
void Visualizar_Caja(void)
{
	volatile unsigned int i;
	i=Caja_Read();
	IntToStr(i,rs_str);
	Imprimir_Lcd("","Caja actual: $",0);		// Imprimimo en display.
	Lcd_Puts(rs_str);
	return;
}
/********************************************************************************/
/*				Imprimo por impresora un listado de comandos posibles.			*/
/*..............................................................................*/
void Ayuda(void)
{
	Imprimir_Lcd(":","Imprimiendo ayuda...",1);
	Inicializo_Impresion(1,1);
	PutStr("Comandos del cajero:\n\n");
	PutStr("I               F1  Imprimir ticket.\n");
	PutStr("C               F2  Cobrar ticket.\n");
	PutStr("HORA?           F3  Ver la hora actual.\n");
	PutStr("CAJA?           F4  Ver la caja actual.\n");
	PutStr("AUTOS EN PLAYA  F5  Cantidad de autos en la playa.\n");
#if defined(ESTADIA)
	PutStr("ESTADIA             Estadia diaria.\n");
#endif
	PutStr("A                   Anular ticket.\n");
	PutStr("CERRAR CAJA         Hacer cierre de caja.\n");
	PutStr("APAGAR              Apagar el equipo con seguridad.\n\n");
	Corto_Papel();
	return;
}
/********************************************************************************/
/*			Muestro la cantidad de autos que hay en la playa.					*/
/*..............................................................................*/
void Contar_Autos(void)
{
	volatile unsigned int pag;
	unsigned int e;
	unsigned char i,valor_obtenido[5];
	e=0;
	for(pag=0;pag<2048;pag++)				// Barro las 1024 paginas de las memorias.
	{
		i=Read_24LC(pag*32);				// Cargo en memoria la página.
		if(i=='P')							// La página tiene algún contenido?
			e++;
	}
	IntToStr(e*10,valor_obtenido);
	Imprimir_Lcd("","Cant. de autos: ",0);
	Lcd_Puts(valor_obtenido);
	return;
}
/********************************************************************************/
/*			Lleno una cadena con \0												*/
/*..............................................................................*/
void Limpiar_Cadena(unsigned char *str,unsigned char longitud)
{
	unsigned char i=0;
	while(i<=longitud)
	{
		*str++=0;					// Lleno toda la cadena con NULL.
		i++;
	}
	return;
}
/********************************************************************************/
/*			CONVIERTO LA LECTURA EN UNA CADENA DE CARACTERES.					*/
/*..............................................................................*/
void IntToStr(unsigned int origen,register unsigned char *destino)
{
	unsigned int aux0,aux1,aux2,aux3,aux4;		// Variables auxiliares para la conversion.
	CLRWDT();
	aux0=(origen/10000);						// Calculo la unidad de mil.
	aux1=(origen-aux0*10000)/1000;				// Calculo la centena.
	aux2=(origen-aux0*10000-aux1*1000)/100;				// Calculo la decena.
	aux3=(origen-aux0*10000-aux1*1000-aux2*100)/10;		// Calculo la unidad.
	aux4=(origen-aux0*10000-aux1*1000-aux2*100-aux3*10);	// Calculo la fracción.
	if(aux0)
		(*destino++)=aux0+48;					// Unidad de mil.
	if(aux1||aux0)
		(*destino++)=aux1+48;					// Centena.
	if(aux2||aux1||aux0)
		(*destino++)=aux2+48;					// Decena.
	(*destino++)=aux3+48;						// Unidad.
	if(aux4)
	{
		(*destino++)='.';
		(*destino++)=aux4+48;					// Fracción.
		(*destino++)=48;
	}
	(*destino)=0;								// Final cadena.
	return;
}
/********************************************************************************/
/*			CONVIERTO LA LECTURA EN UNA CADENA DE CARACTERES.					*/
/*..............................................................................*/
unsigned char Estadia_Diaria(void)
{
#if defined(ESTADIA)
	unsigned char hora_f[5],monto[4],vehiculo;
	unsigned char i,horas,dias,tarifa;
	unsigned int lugar,monto_n;
	CLRWDT();
	if(rs_str[0]!='P')							// El ticket es para cobrar??
		return 0;
	i=0;
	while(ps2_str[i++])							// Comparo si es el ticket que debo cobrar.
	{
		if(ps2_str[i]>96&&ps2_str[i]<123)
			ps2_str[i]=ps2_str[i]-32;			// Si hay alguna minúscula la conviento en mayuscula.
		if(ps2_str[i]!=rs_str[i+14])			// Comparo
			return 0;							// Si son distintos salgo y sigo con otro.
	}
	if(i<4)
		return 0;								// No se ingreso ninguna patente.
	if(rs_str[1]=='a')							// Necesario para saber la terifa correspondiente.
		vehiculo=0;
	if(rs_str[1]=='m')
		vehiculo=1;
	if(rs_str[1]=='u')
		vehiculo=2;
	tarifa=Eeprom_Read(230+vehiculo);
	if(!tarifa)
		return 3;
	lugar=i+13+(pagina*32);						// Direccion necesaria luego.
	Obtener_Hora(ps2_str,0);
	i=0;
	do
	{
		hora_f[i]=ps2_str[i+6];
	}while(i++<4);
	hora_f[4]=0;
	i=0;
	Reseteo_Teclado();							// Reinicio la cauptura por teclado PS2.
	while(INT0IE)								// Por defecto anula.
	{
		if(pos_str_ps2<vis-1)					// Se perdió la variable vis?
			vis=pos_str_ps2;
		if(pos_str_ps2==vis)					// Actualiazo la pantalla?
		{
			Imprimir_Lcd("DIAS A COBRAR: 0 - 9",ps2_str,1);
			vis++;								// 
		}
		if(pos_str_ps2>18)						// Elimino el refresco.
		{
			pos_str_ps2=18;
			vis=19;
		}
		CLRWDT();
	}
	if(ps2_str[0]<49||ps2_str[0]>57)			// Si ingreso un valor no válido cancelo la operación.
		return 2;
	dias=ps2_str[0]-48;							// Convierto el ASCII en num.
	monto_n=dias*tarifa;
	IntToStr(monto_n,monto);
	Linea_Lcd("Tiempo: ",1);
	Lcd_Putch(dias+48);
	Lcd_Puts(" dias");
	Lcd_Goto(0x40);
	Lcd_Puts("A pagar: $");
	Lcd_Puts(monto);
	Reseteo_Teclado();							// Reinicio la cauptura por teclado PS2.
	while(INT0IE)								// Espero un enter del teclado
	{
		pos_str_ps2=0;
		CLRWDT();
	}
	Imprimir_Lcd("Cobrar?","S/N",1);
	Reseteo_Teclado();							// Reinicio la cauptura por teclado PS2.
	while(1)									// Espero un S/N del teclado.
	{
		pos_str_ps2=0;							// Solo necesito obtener un caracter.
		if(ps2_str[0]=='N')						// Salgo a la pantalla principal.
		{
			INT0IE=0;							// Bajo la interrupción y continuo.
			return 2;
		}
		if(ps2_str[0]=='S')						// Seguir con el cobro.
		{
			INT0IE=0;							// Bajo la interrupción y continuo.
			break;
		}
		INT0IE=1;								// Si apretaron un ENTER pero no ingresaron S/N vuelvo a levantar la interrupción.
		CLRWDT();
	}
// guardo el cobro en la memoria y actualizo las estadisticas.
	monto_n=monto_n+Caja_Read();				// Leo de la Eeprom la caja actual y le sumo el cobro.
	Caja_Write(monto_n);						// Guardo la nueva caja.
	Guardo_Estadisticas(13);
	Write_24LC((pagina*32),'E');				// Marco la entrada como estadia.
	Write_24LC(lugar++,' ');
	i=0;
	while(hora_f[i])
		Write_24LC(lugar++,hora_f[i++]);
	i=0;
	while(monto[i])
		Write_24LC(lugar++,monto[i++]);
	Write_24LC(lugar,0);						// Marco el final con un NULL.
#endif
	return 1;
}
/********************************************************************************/
/*				Guardo la hora en la que se inició la caja actual.				*/
/*..............................................................................*/
void Guardo_Arranque_Turno(unsigned char guardar)
{
	unsigned char actualizo,temporal;
	actualizo=Eeprom_Read(108);
	if(actualizo||guardar)
	{
		Eeprom_Write(108,0);					// Guardo la bandera.
		Obtener_Hora(rs_str,0);					// Hora en la que reliazó la apertura de la caja.
		temporal=((rs_str[0]-48)*10)+(rs_str[1]-48);	// Fecha
		Eeprom_Write(109,temporal);
		temporal=((rs_str[2]-48)*10)+(rs_str[3]-48);	// Mes
		Eeprom_Write(110,temporal);
		temporal=((rs_str[4]-48)*10)+(rs_str[5]-48);	// Año
		Eeprom_Write(111,temporal);
		temporal=((rs_str[6]-48)*10)+(rs_str[7]-48);	// Hora
		Eeprom_Write(112,temporal);
		temporal=((rs_str[8]-48)*10)+(rs_str[9]-48);	// Minutos
		Eeprom_Write(113,temporal);
	}
	return;
}
/********************************************************************************/
/*					En caso de recibir un SMS lo proceso.						*/
/*..............................................................................*/
void Procesar_SMS(unsigned char orden)
{
#if defined(USO_MODEM)
	static unsigned char temp[160];
	static bit salto;
	unsigned char e,a,u;
	volatile unsigned int i,o;
	e=0;
	i=50;
	Remitente_SMS(ps2_str,orden);				// Obtengo el numero del remitente.
	do
	{
		temp[e]=Eeprom_Read(i);					// Lleno la cadena con el numero guardado em memoria.
		i++;
	}while(temp[e]&&e++<20);					// Repito hasta encontrar un NULL.
	if(Comparo_Cadenas(ps2_str,temp,0)||e<5)	// Esta definido el numero del admin?
	{
		Leer_SMS(temp,orden);					// Leo nuevamente el SMS.
		if(e<5)
		{
			if(Comparo_Cadenas(temp,"ADMIN",0))	// Mensaje para ser agregado como administrador.
			{
				Almacenar_Telefono(ps2_str);	// Agrego el número de telefono.
				Enviar_SMS("Telefono guardado.",ps2_str);
			}
		}
		else
		{
			salto=0;
			if(Comparo_Cadenas(temp,"Caja actual",0))
			{
				i=Caja_Read();
				IntToStr(i,rs_str);
				e=0;
				temp[11]=':';
				temp[12]=' ';
				temp[13]='$';
				while(rs_str[e])
				{
					temp[14+e]=rs_str[e];
					e++;
				}
				temp[14+e]=0;
				Enviar_SMS(temp,ps2_str);
				salto=1;
			}
			if(Comparo_Cadenas(temp,"Autos en playa",0))
			{
				u=0;
				for(i=0;i<2048;i++)					// Barro las 1024 paginas de las memorias.
				{
					e=Read_24LC(i*32);				// Cargo en memoria la página.
					if(e=='P')						// La página tiene algún contenido?
					{
						Rec_Paquete_24LC(i,rs_str);	// Cargo en memoria la página.
						a=13;
						while(rs_str[a])
							temp[u++]=rs_str[a++];	// La mando por RS232.
						temp[u++]=13;
						temp[u++]=10;
					}
				}
				Enviar_SMS(temp,ps2_str);
				salto=1;
			}
			if(Comparo_Cadenas(temp,"Cierres de caja",0))
			{
				a=0;
				u=0;
				i=310;
				for(i=310;i<1023;i++)
				{
					e=Eeprom_Read(i);				// Busco la marca.
					if(e=='#')
					{
						i++;
						e=Eeprom_Read(i+2);			// Leo el día.
						IntToStr(e,rs_str);
						a=0;
						while(rs_str[a])			// Armo el SMS con los datos que recolecto.
							temp[u++]=rs_str[a++];
						temp[u++]='/';
						e=Eeprom_Read(i+3);			// Leo el mes.
						IntToStr(e,rs_str);
						a=0;
						while(rs_str[a])			// Armo el SMS con los datos que recolecto.
							temp[u++]=rs_str[a++];	
						temp[u++]='/';
						e=Eeprom_Read(i+4);			// Leo el año.
						IntToStr(e,rs_str);
						a=0;
						while(rs_str[a])			// Armo el SMS con los datos que recolecto.
							temp[u++]=rs_str[a++];
						temp[u++]=' ';
						e=Eeprom_Read(i+5);			// Leo la hora.
						IntToStr(e,rs_str);
						a=0;
						while(rs_str[a])			// Armo el SMS con los datos que recolecto.
							temp[u++]=rs_str[a++];	
						temp[u++]=':';
						e=Eeprom_Read(i+6);			// Leo los minutos.
						IntToStr(e,rs_str);
						a=0;
						while(rs_str[a])			// Armo el SMS con los datos que recolecto.
							temp[u++]=rs_str[a++];
						temp[u++]=' ';
						temp[u++]='$';
						e=Eeprom_Read(i);			// Leo la caja guardada.
						a=Eeprom_Read(i+1);
						o=e<<8|a;
						IntToStr(o,rs_str);
						a=0;
						while(rs_str[a])			// Armo el SMS con los datos que recolecto.
							temp[u++]=rs_str[a++];
						temp[u++]=13;
						temp[u++]=10;
					}
				}
				temp[u]=0;
				Enviar_SMS(temp,ps2_str);
				salto=1;
			}
			if(Comparo_Cadenas(temp,"Borrar cajas",0))
			{
				Caja_Borrar();
				Enviar_SMS("Cierres borrados.",ps2_str);
				salto=1;
			}
			if(salto)
				Enviar_SMS("Comando no válido.",ps2_str);
		}
	} 
	else
		Enviar_SMS("No posee autorización para esta operación.",ps2_str);
	Borrar_SMS(orden);							// Borro el SMS una vez procesado.
	Limpiar_Cadena(ps2_str,33);
	Limpiar_Cadena(rs_str,70);
#endif
	return;
}
