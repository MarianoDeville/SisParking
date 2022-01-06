/****************************************************************************/
/*	MENU AUXILIAR PARA APOYO AL INSTALADOR - NECESITA DISPLAY Y TECLADO PS2	*/
/*..........................................................................*/
/*			Revisión:			1.00										*/
/*			PIC:				PIC18F4620									*/
/*			MODEM:				SIM340CZ									*/
/*			RTC:				DS1307										*/
/*			Memoria:			24LC256.									*/
/*			Display:			LCD 20 caracteres por 2 lineas.				*/
/*			Teclado:			PS2											*/
/*			Impresora:			EPSON TM-T88 I.								*/
/*			Comunicación:		RS232 - I2C.								*/
/*			Compilador:			MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.		*/
/*			Autor:				Mariano Ariel Deville						*/
/****************************************************************************/
/*						PROTOTIPOS DE FUNCIONES								*/
/*..........................................................................*/
void Menu_Teclado(unsigned char log);
void Comandos(unsigned char log);
void Log(unsigned char log);
void Visualizar_Hora(void);
void Cambio_Pass(unsigned char log);
void Imprimir_Estadisticas(void);
void Borrado_Completo(void);
void Borrado_Planilla(void);
void Imprimir_Movimientos_Cobrados(void);
void Imprimir_Entero(unsigned int lugar,unsigned char coma);
void Imprimir_Char(unsigned int lugar);
void Cambio_Hora(void);
void Tarifas(unsigned char log);
void Prompt(unsigned char log,const unsigned char *mensaje);
void Descargo_Memoria(void);
void Capturo_Guardo(unsigned int lugar,const unsigned char *mensaje);
unsigned char Anular_Ticket(unsigned char atorizacion);
/****************************************************************************/
/*						VARIABLES GLOBALES									*/
/*..........................................................................*/
bank2 bit consola;						// Bandera.							*/
/****************************************************************************/
/*		CAPTURA DE TECLADO Y VISUALIZACION EN DISPLAY						*/
/*..........................................................................*/
void Menu_Teclado(unsigned char log)
{
	consola=1;
	CLRWDT();
	Reseteo_Teclado();
	while(consola)						// Mientras este el jumper puesto sigo en este menu.
	{
		while(INT0IE)					// Espero un enter del teclado
		{								// para procesar el comando.
			if(pos_str_ps2<vis-1)		// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)		// Actualizo la pantalla?
			{
				Log(log);
				Lcd_Puts(ps2_str);		// Muestro lo que se va escribiendo.
				vis++;					// 
			}
			CLRWDT();
		}
		Comandos(log);					// Busco y ejecuto el comando ingresado.
		if(consola)
		{
			Reseteo_Teclado();
			while(!pos_str_ps2&&INT0IE)	// Espero a que se teclee algo.
				CLRWDT();
			vis=pos_str_ps2;
		}
		CLRWDT();
	}
	return;
}
/****************************************************************************/
/*		COMANDOS PARA CONFIGURAR GPS, MODEM, MEMORIA Y SENSORES				*/
/*--------------------------------------------------------------------------*/
/*			CAMBIAR CONTRASEÑA												*/
/*			CAMBIAR HORA													*/
/*			AUTOS?															*/
/*			HORA?				F3											*/
/*			CAJA?				F4											*/
/*			AUTOS EN PLAYA		F5											*/
/*			PLANILLA			F6											*/
/*			IMPRIMIR CAJAS		F7											*/
/*			ESTADISTICAS													*/
/*			TELEFONO?			F9											*/
/*			BORRAR PLANILLA													*/
/*			BORRAR TELEFONO													*/
/*			BORRAR AUTO														*/
/*			BANDAS HORARIAS													*/
/*			TARIFAS															*/
/*			PROMOCIONES														*/
/*			ESTADIA															*/
/*			TOLERANCIAS														*/
/*			BORRAR TELEFONO													*/
/*			VALORES DEFINIDOS												*/
/*			BORRAR CAJAS													*/
/*			COMANDOS			F10											*/
/*			?					F10											*/
/*--------------------------------------------------------------------------*/
/*		COMANDOS DEL ROOT PARA MANTENIMIENTO								*/
/*--------------------------------------------------------------------------*/
/*			DESCARGAR EEPROM												*/
/*			DESCARGAR MEMORIA												*/
/*			BORRAR CONTRASEÑA												*/
/*			BORRAR EEPROM													*/
/*			BORRADO LENTO													*/
/*			RELOJ SETUP														*/
/*			SMS?															*/
/*			MANDAR SMS														*/
/*			MODEM SETUP														*/
/*			PRENDER MODEM													*/
/*			APAGAR MODEM													*/
/*			COMANDOS AT														*/
/****************************************************************************/
void Comandos(unsigned char log)
{
	static bit bis;
	volatile unsigned int pag,e;
	unsigned char i,cad_temp[40];
	CLRWDT();
	bis=0;
	if(!pos_str_ps2)
	{
		Prompt(log,"");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"CAMBIAR CONTRASEÑA",0))
	{
		Cambio_Pass(log);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"RELOJ SETUP",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		DS1307_Setup();
		Imprimir_Lcd("R:","Reloj configurado.",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"CAMBIAR HORA",0))
	{
		Cambio_Hora();
		bis=1;
	}
	CLRWDT();
	if(Comparo_Cadenas(ps2_str,"HORA?",0)||bis)
	{
		Prompt(log,"");
		Visualizar_Hora();
		return;
	}
	if(Comparo_Cadenas(ps2_str,"CAJA?",0))
	{
		Prompt(log,"");
		Visualizar_Caja();
		return;
	}
	if(Comparo_Cadenas(ps2_str,"AUTOS?",0))
	{
		Prompt(log,"");
		Contar_Autos();
		return;
	}
	if(Comparo_Cadenas(ps2_str,"PLANILLA",0))
	{
		Imprimir_Lcd("","Imprimiendo...",0);
		Inicializo_Impresion(1,0);
		Obtener_Hora(rs_str,1);					// Cargo la hora en formato estandar
		PutStr("Movimientos:\n");
		PutStr(rs_str);							// Imprimimo la fecha y hora.
		PutStr("\n\n");
		Inicializo_Impresion(1,1);
		e=1;
		for(pag=0;pag<=2047;pag++)				// Barro las 2048 paginas de las memorias.
		{
			i=Read_24LC(pag*32);				// Cargo en memoria la página.
			if(i&&i!='*')						// La página tiene algún contenido?
			{
				Rec_Paquete_24LC(pag,rs_str);	// Cargo en memoria la página.
				IntToStr(e*10,cad_temp);
				PutStr(cad_temp);
				PutCh(' ');
				i=0;
				while(rs_str[i])
				{
					if(rs_str[i-3]==' ')
						PutCh(':');				// Imprimo los dos puntos en la hora de cobro.
					if(rs_str[i-5]==' ')
						PutStr(" $");			// Imprimo el espacio que separa el monto.
					PutCh(rs_str[i++]);			// La mando por RS232.
					if(i==4||i==6)
						PutCh('/');
					if(i==2||i==8||i==14)
						PutCh(' ');
					if(i==10||i==12)
						PutCh(':');
				}
				e++;
				PutStr("\n");
			}
		}
		PutStr("\nA - Anulado.                        a - Auto.\n");
		PutStr("B - Borrado por el administrador.   m - Moto.\n");
		PutStr("C - Cobrado.                        u - Utilitario.\n");
#if defined(ESTADIA)
		PutStr("E - Estadia diaria.\n");
#endif
		PutStr("P - Vehículo en playa.\n");
		Corto_Papel();
		Prompt(log,"");							// Promp del sistema.
		return;
	}
	if(Comparo_Cadenas(ps2_str,"AUTOS EN PLAYA",0))
	{
		Imprimir_Lcd("","Imprimiendo...",0);
		Inicializo_Impresion(1,1);
		PutStr("Autos en la playa:\n");
		e=1;
		for(pag=0;pag<2048;pag++)				// Barro las 1024 paginas de las memorias.
		{
			i=Read_24LC(pag*32);				// Cargo en memoria la página.
			if(i=='P')							// La página tiene algún contenido?
			{
				IntToStr(e++*10,cad_temp);
				PutStr(cad_temp);
				PutCh(' ');
				Rec_Paquete_24LC(pag,rs_str);	// Cargo en memoria la página.
				i=1;
				while(rs_str[i])
				{
					if(i==2)
						PutCh(' ');
					PutCh(rs_str[i++]);			// La mando por RS232.
					if(i==4||i==6)
						PutCh('/');
					if(i==8||i==14)
						PutCh(' ');
					if(i==10||i==12)
						PutCh(':');
				}
				PutStr("\n");
			}
		}
		Corto_Papel();
		Prompt(log,"");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"BORRAR PLANILLA",0))
	{
		Imprimir_Lcd("","borrando...",0);
		Borrado_Planilla();
		Caja_Borrar();
		Prompt(log,"Planilla borrada.");
		return;
	}
	CLRWDT();
	if(Comparo_Cadenas(ps2_str,"DESCARGAR EEPROM",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		Linea_Lcd("Descargando...",1);
		MODO_PC;
		Vaciar_Memoria();						// Mando los datos de la EEPROM por puerto serie.
		PutCh(13);
		Prompt(log,"Memoria descargada.");
		MODO_MODEM;
		return;
	}
	if(Comparo_Cadenas(ps2_str,"BORRAR EEPROM",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		Borrar_Memoria();						// Borro todo el contenido de la EEPROM del PIC.
		Prompt(log,"Memoria borrada.");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"BANDAS HORARIAS",0))
	{
		Capturo_Guardo(150,"COMIENZO 1 TURNO:");
		Capturo_Guardo(151,"COMIENZO 2 TURNO:");
		Capturo_Guardo(152,"COMIENZO 3 TURNO:");
		Imprimir_Lcd("FRACCIONAR LA HORA?","S/N",1);
		Reseteo_Teclado();							// Reinicio la cauptura por teclado PS2.
		ps2_str[0]=0;
		while(1)									// Espero un S/N del teclado.
		{
			if(ps2_str[0]=='N'||ps2_str[0]=='S')
			{
				INT0IE=0;							// Ya no necesito el teclado.
				break;
			}
			pos_str_ps2=0;							// Solo necesito obtener un caracter.
			INT0IE=1;								// Si apretaron un ENTER pero no ingresaron S/N vuelvo a levantar la interrupción.
			CLRWDT();
		}
		Eeprom_Write(153,ps2_str[0]);
		Prompt(log,"Bandas configuradas.");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"TARIFAS",0))
	{
		Tarifas(log);
		return;
	}
	CLRWDT();
	if(Comparo_Cadenas(ps2_str,"PROMOCIONES",0))
	{
		Capturo_Guardo(200,"HORA INICIO:");
		Capturo_Guardo(201,"HORA FINALIZACION:");
		Capturo_Guardo(202,"TARIFA 1/2 Hs AUTO:");
		Capturo_Guardo(203,"TARIFA 1/2 Hs MOTO:");
		Capturo_Guardo(204,"TARIFA 1/2 Hs UTILITARIO:");
		Prompt(log,"Promos cargadas.");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"TOLERANCIAS",0))
	{
		Capturo_Guardo(220,"TOLERANCIA HORA:");
		Capturo_Guardo(221,"TOLERANCIA 1/2 HORA:");
		Capturo_Guardo(222,"ANULACION TICKET:");
		Prompt(log,"Tolerancias cargadas.");
		return;
	}
#if defined(ESTADIA)
	if(Comparo_Cadenas(ps2_str,"ESTADIA",0))
	{
		Capturo_Guardo(230,"ESTADIA AUTO:");
		Capturo_Guardo(231,"ESTADIA MOTO:");
		Capturo_Guardo(232,"ESTADIA UTILITARIO:");
		Prompt(log,"Estadias cargadas.");
		return;
	}
#endif
	if(Comparo_Cadenas(ps2_str,"VALORES DEFINIDOS",0))
	{
		if(log)
		{
			Imprimir_Lcd("","Imprimiendo...",0);
			Inicializo_Impresion(1,1);
			PutStr("VALORES DEFINIDOS\n\n");
			PutStr("BANDAS HORARIAS:\n");
			PutStr("COMIENZO 1 TURNO: ");
			Imprimir_Char(150);
			PutStr(" Hs.\n");
			PutStr("COMIENZO 2 TURNO: ");
			Imprimir_Char(151);
			PutStr(" Hs.\n");
			PutStr("COMIENZO 3 TURNO: ");
			Imprimir_Char(152);
			PutStr(" Hs.\n");
			PutStr("FRACCIONAR LA HORA: ");
			i=Eeprom_Read(153);
			if(i=='S')
				PutStr("SI");
			else
				PutStr("NO");
			PutStr("\n\n");
			PutStr("TARIFAS\n");
			PutStr("PRIMER TURNO:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(160);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(161);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(162);
			PutStr("\n");
			PutStr("1/2 HORA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(169);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(170);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(171);
			PutStr("\n\n");
			PutStr("SEGUNDO TURNO:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(163);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(164);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(165);
			PutStr("\n");
			PutStr("1/2 HORA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(172);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(173);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(174);
			PutStr("\n\n");
			PutStr("TERCER TURNO:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(166);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(167);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(168);
			PutStr("\n");
			PutStr("1/2 HORA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(175);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(176);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(177);
			PutStr("\n\n");
			PutStr("PROMOCIONES:\n");
			PutStr("COMIENZO:     ");
			Imprimir_Char(200);
			PutStr(" Hs.\n");
			PutStr("FINALIZACION: ");
			Imprimir_Char(201);
			PutStr(" Hs.\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(202);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(203);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(204);
			PutStr("\n\n");
			PutStr("TOLERANCIAS:\n");
			PutStr("HORA:             ");
			Imprimir_Char(220);
			PutStr(" min.\n");
			PutStr("1/2 HORA:         ");
			Imprimir_Char(221);
			PutStr(" min.\n");
			PutStr("ANULACION TICKET: ");
			Imprimir_Char(222);
			PutStr(" min.\n\n");
#if defined(ESTADIA)
			PutStr("ESTADIA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(230);
			PutStr("\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(231);
			PutStr("\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(232);
			PutStr("\n");
#endif
			Corto_Papel();
			Prompt(log,"");
			return;
		}
		else
		{
			Imprimir_Lcd("","Imprimiendo...",0);
			Inicializo_Impresion(1,1);
			PutStr("VALORES DEFINIDOS\n\n");
			PutStr("BANDAS HORARIAS:                         Dir.\n");
			PutStr("COMIENZO 1 TURNO: ");
			Imprimir_Char(150);
			PutStr(" Hs.                 150\n");
			PutStr("COMIENZO 2 TURNO: ");
			Imprimir_Char(151);
			PutStr(" Hs.                 151\n");
			PutStr("COMIENZO 3 TURNO: ");
			Imprimir_Char(152);
			PutStr(" Hs.                 152\n");
			PutStr("FRACCIONAR LA HORA: ");
			i=Eeprom_Read(153);
			if(i=='S')
				PutStr("SI                   153");
			else
				PutStr("NO                   153");
			PutStr("\n\n");
			PutStr("TARIFAS\n");
			PutStr("PRIMER TURNO:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(160);
			PutStr("                        160\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(161);
			PutStr("                        161\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(162);
			PutStr("                        162\n");
			PutStr("1/2 HORA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(169);
			PutStr("                        169\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(170);
			PutStr("                        170\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(171);
			PutStr("                        171\n\n");
			PutStr("SEGUNDO TURNO:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(163);
			PutStr("                        163\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(164);
			PutStr("                        164\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(165);
			PutStr("                        165\n");
			PutStr("1/2 HORA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(172);
			PutStr("                        172\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(173);
			PutStr("                        173\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(174);
			PutStr("                        174\n\n");
			PutStr("TERCER TURNO:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(166);
			PutStr("                        166\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(167);
			PutStr("                        167\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(168);
			PutStr("                        168\n");
			PutStr("1/2 HORA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(175);
			PutStr("                        175\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(176);
			PutStr("                        176\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(177);
			PutStr("                        177\n\n");
			PutStr("PROMOCIONES:\n");
			PutStr("COMIENZO:     ");
			Imprimir_Char(200);
			PutStr(" Hs.                     200\n");
			PutStr("FINALIZACION: ");
			Imprimir_Char(201);
			PutStr(" Hs.                     201\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(202);
			PutStr("                        202\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(203);
			PutStr("                        203\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(204);
			PutStr("                        204\n\n");
			PutStr("TOLERANCIAS:\n");
			PutStr("HORA:             ");
			Imprimir_Char(220);
			PutStr(" min.                220\n");
			PutStr("1/2 HORA:         ");
			Imprimir_Char(221);
			PutStr(" min.                221\n");
			PutStr("ANULACION TICKET: ");
			Imprimir_Char(222);
			PutStr(" min.                222\n\n");
#if defined(ESTADIA)
			PutStr("ESTADIA:\n");
			PutStr("AUTO:        $ ");
			Imprimir_Char(230);
			PutStr("                        230\n");
			PutStr("MOTO:        $ ");
			Imprimir_Char(231);
			PutStr("                        231\n");
			PutStr("UTILITARIO:  $ ");
			Imprimir_Char(232);
			PutStr("                        232\n");
#endif
			Corto_Papel();
			Prompt(log,"");
			return;
		}
	}
	if(Comparo_Cadenas(ps2_str,"BORRAR CAJAS",0))	// Solo el ROOt lo puede hacer.
	{
		Prompt(log,"Borrando las cajas...");
		Caja_Borrar();
		Prompt(log,"Cajas borradas.");
		return;
	}
	CLRWDT();
	if(Comparo_Cadenas(ps2_str,"DESCARGAR MEMORIA",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		MODO_PC;
		Descargo_Memoria();
		Linea_Lcd("R:",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"BORRADO LENTO",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		Linea_Lcd("R:",1);
		Borrado_Completo();
		Imprimir_Lcd("R:","Memoria borrada",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"ESTADISTICAS",0))
	{
		Imprimir_Lcd("","Imprimiendo...",0);
		Imprimir_Estadisticas();
		Prompt(log,"");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"IMPRIMIR CAJAS",0))
	{
		Imprimir_Lcd("","Imprimiendo...",0);
		Inicializo_Impresion(1,1);
		Obtener_Hora(rs_str,1);					// Cargo la hora en formato estandar
		PutStr(rs_str);							// Imprimimo la fecha y hora.
		PutStr("\nResumen de cierres:\n\n");
		for(pag=250;pag<1023;pag++)
		{
			CLRWDT();
			i=Eeprom_Read(pag);
			if(i=='#')
			{
				PutStr("Inicio: ");
				Imprimir_Char(pag+8);			// Fecha de inicio del truno.
				PutCh('/');
				Imprimir_Char(pag+9);
				PutCh('/');
				Imprimir_Char(pag+10);
				PutCh(' ');
				Imprimir_Char(pag+11);			// Hora de inicio del turno.
				PutCh(':');
				Imprimir_Char(pag+12);
				PutStr("   Cierre: ");
				Imprimir_Char(pag+3);			// Fecha de cierre del turno.
				PutCh('/');
				Imprimir_Char(pag+4);
				PutCh('/');
				Imprimir_Char(pag+5);
				PutCh(' ');
				Imprimir_Char(pag+6);			// Hora de cierre del turno.
				PutCh(':');
				Imprimir_Char(pag+7);
				PutCh(' ');				
				PutStr("\nCobrado: $");
				Imprimir_Entero(pag+1,1);
				pag=pag+13;						// Lugar en donde comienzan las estadisticas.
				PutStr("\nEstadisticas del cierre:\n");
				PutStr("Anulación de tickets: ");
				Imprimir_Entero(pag+24,0);
				PutStr("\n");
#if defined(ESTADIA)
				PutStr("Estadia diaria: ");
				Imprimir_Char(pag+27);
#endif
				PutStr("\n");
				PutStr("Vehículos con promoción:\n");
				PutStr("Autos:       ");
				Imprimir_Entero(pag+18,0);
				PutStr("\n");
				PutStr("Motos:       ");
				Imprimir_Entero(pag+20,0);
				PutStr("\n");
				PutStr("Utilitarios: ");
				Imprimir_Entero(pag+22,0);
				PutStr("\n");
				PutStr("Vehículos con tarifa 1:\n");
				PutStr("Autos:       ");
				Imprimir_Entero(pag,0);
				PutStr("\n");
				PutStr("Motos:       ");
				Imprimir_Entero(pag+2,0);
				PutStr("\n");
				PutStr("Utilitarios: ");
				Imprimir_Entero(pag+4,0);
				PutStr("\n");
				PutStr("Vehículos con tarifa 2:\n");
				PutStr("Autos:       ");
				Imprimir_Entero(pag+6,0);
				PutStr("\n");
				PutStr("Motos:       ");
				Imprimir_Entero(pag+8,0);
				PutStr("\n");
				PutStr("Utilitarios: ");
				Imprimir_Entero(pag+10,0);
				PutStr("\n");
				PutStr("Vehículos con tarifa 3:\n");
				PutStr("Autos:       ");
				Imprimir_Entero(pag+12,0);
				PutStr("\n");
				PutStr("Motos:       ");
				Imprimir_Entero(pag+14,0);
				PutStr("\n");
				PutStr("Utilitarios: ");
				Imprimir_Entero(pag+16,0);
				PutStr("\n");
				PutStr(" . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n");
			}
		}
		Corto_Papel();
		Prompt(log,"");
		return;
	}
#if defined(USO_MODEM)
	if(Comparo_Cadenas(ps2_str,"TELEFONO?",0))	// El administrador borra su numero.
	{
		i=0;
		e=50;
		do
		{
			ps2_str[i]=Eeprom_Read(e);			// Obtengo el valor almacenado en la mem.
			e++;
		}while(ps2_str[i++]);
		ps2_str[i]=0;
		if(i>5)
			Prompt(log,ps2_str);
		else
			Prompt(log,"Nro. no definido.");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"BORRAR TELEFONO",0))	// El administrador borra su numero.
	{
		Borrar_Telefono();
		Imprimir_Lcd("","Telefono borrado.",0);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"MODEM SETUP",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		i=0;
		MODO_MODEM;								// Habilito el RS232 para el MODEM.
		POWER_MODEM=0;							// Prendo el MODEM.
		while(!NET_STATE)						// Espero que el MODEM me de una señal.
		{
			if(i++>100)							// Timeout???
			{
				LED=1;							// Dejo el led fijo prendido.
				MODO_PC;
				POWER_MODEM=1;
				Imprimir_Lcd("R:","No se puede prender el MODEM",1);
				return;
			}
			DelayMs(30);
			LED=!LED;							// Parpadea el led mientras se prende el MODEM.
		}
		LED=1;									// Dejo el led fijo prendido.
		POWER_MODEM=1;							// Prendo el MODEM.
		Imprimir_Lcd("","Configurando MODEM...",0);
		DelayS(1);
		Modem_Setup();							// Configuracion inicial del MODEM.
		Prompt(log,"MODEM configurado");
		Modem_Power(0,0);						// Apago el MODEM.
		return;
	}
	if(Comparo_Cadenas(ps2_str,"PRENDER MODEM",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		Imprimir_Lcd("","inicializando.....",0);
		i=0;
		MODO_MODEM;								// Habilito el RS232 para el MODEM.
		POWER_MODEM=0;							// Prendo el MODEM.
		while(!NET_STATE)						// Espero que el MODEM me de una señal.
		{
			if(i++>100)							// Timeout???
			{
				LED=1;							// Dejo el led fijo prendido.
				MODO_PC;
				POWER_MODEM=1;
				Imprimir_Lcd("R:","No se puede prender el MODEM",1);
				return;
			}
			DelayMs(30);
			LED=!LED;							// Parpadea el led mientras se prende el MODEM.
		}
		LED=1;									// Dejo el led fijo prendido.
		POWER_MODEM=1;							// Prendo el MODEM.
		Imprimir_Lcd("R:","MODEM encendido.",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"APAGAR MODEM",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		if(Modem_Power(0,0))
			Imprimir_Lcd("R:","MODEM apagado",1);
		else
			Imprimir_Lcd("error al apagar","el MODEM",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"COMANDOS AT",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		MODO_MODEM;								// Habilito el RS232 para el MODEM.
		Reseteo_Teclado();
		while(1)								// Espero comando de salida.
		{
			while(INT0IE)						// Espero un enter del teclado
			{									// para procesar el comando.
				if(pos_str_ps2<vis-1)			// Se perdió la variable vis?
					vis=pos_str_ps2;
				if(pos_str_ps2==vis)			// Actualiazo la pantalla?
				{
					Linea_Lcd(ps2_str,1);		// Muestro lo que se va escribiendo.
					i=0;
					vis++;						// 
				}
				CLRWDT();
			}
			if(Comparo_Cadenas(ps2_str,"SALIR",0))	// Comando para salir.
			{
				Imprimir_Lcd("R:","Sesion cerrada.",1);
				return;
			}
			Consulta_Modem(ps2_str,rs_str);		// Envio el comando escrito por teclado PS2.
			Linea_Lcd(rs_str,1);				// Imprimo la respuesta del MODEM.
			Reseteo_Teclado();
			while(!pos_str_ps2&&INT0IE)			// Espero a que se teclee algo.
				CLRWDT();
			vis=pos_str_ps2;
		}
	}
	if(Comparo_Cadenas(ps2_str,"SMS?",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		MODO_MODEM;								// Habilito el RS232 para el MODEM.
		bandera=1;
		Consulta_Modem("AT+CMGL=\"ALL\"",cad_temp);
		Linea_Lcd(cad_temp,1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"MANDAR SMS",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		Reseteo_Teclado();						// Reinicio la cauptura por teclado PS2.
		while(INT0IE)							// Espero un enter del teclado
		{										// para procesar el comando.
			if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)				// Actualiazo la pantalla?
			{
				Imprimir_Lcd("Ingrese nro telefono:",ps2_str,1);
				vis++;							// 
			}
			CLRWDT();
		}
		pos_str_ps2=0;
		do
		{
			cad_temp[pos_str_ps2]=ps2_str[pos_str_ps2];
		}while(ps2_str[pos_str_ps2++]);
		cad_temp[pos_str_ps2]=0;				// Marco el fin de cadena.
		Reseteo_Teclado();						// Reinicio la cauptura por teclado PS2.
		while(INT0IE)							// Espero un enter del teclado
		{										// para procesar el comando.
			if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)				// Actualiazo la pantalla?
			{
				Linea_Lcd("SMS:",1);
				Lcd_Goto(0x04);
				i=0;
				while(i<pos_str_ps2)
				{
					Lcd_Putch(ps2_str[i]);
					if(i++==15)
						Lcd_Goto(0x40);
				}
				vis++;							// 
			}
			CLRWDT();
		}
		Enviar_SMS(ps2_str,cad_temp);
		Imprimir_Lcd("R:","Mensaje enviado.",1);
		return;
	}
#endif
	if(Comparo_Cadenas(ps2_str,"BORRAR CONTRASEÑA",0)&&!log)	// Solo el ROOt lo puede hacer.
	{
		Imprimir_Lcd("","Borrando...",0);
		Borrar_Pass();
		return;
	}
	CLRWDT();
	if(Comparo_Cadenas(ps2_str,"BORRAR AUTO",0))	// Solo el ROOt lo puede hacer.
	{
		Reseteo_Teclado();						// Reinicio la cauptura por teclado PS2.
		while(INT0IE)							// Espero un enter del teclado
		{										// para procesar el comando.
			if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
				vis=pos_str_ps2;
			if(pos_str_ps2==vis)				// Actualiazo la pantalla?
			{
				Log(log);
				Imprimir_Lcd("Ingrese patente:",ps2_str,0);
				vis++;							// 
			}
			CLRWDT();
		}
		for(pagina=0;pagina<2049;pagina++)
		{
			Rec_Paquete_24LC(pagina,rs_str);
			if(Anular_Ticket(1))
				break;
		}
		if(pagina>=2048)						// Termine sin encontrar nada.
			Prompt(log,"Auto no encontrado.");
		else
			Prompt(log,"Vehiculo eliminado.");
		return;
	}
	if(Comparo_Cadenas(ps2_str,"SALIR",0))		// 
	{
		consola=0;
		Imprimir_Lcd(":","",1);
		return;
	}
	if(Comparo_Cadenas(ps2_str,"COMANDOS",0)||Comparo_Cadenas(ps2_str,"?",0))	// Solo el ROOt lo puede hacer.
	{
		Imprimir_Lcd("","Imprimiendo...",0);
		Inicializo_Impresion(1,1);
		PutStr("Comandos del cajero:\n\n");
		PutStr("I                  F1\n");
		PutStr("C                  F2\n");
		PutStr("HORA?              F3\n");
		PutStr("CAJA?              F4\n");
		PutStr("AUTOS EN PLAYA     F5\n");
#if defined(ESTADIA)
		PutStr("ESTADIA\n");
#endif
		PutStr("A \n");
		PutStr("CERRAR CAJA\n");
		PutStr("APAGAR\n");
		if(log)
			PutStr("\nComandos del ADMIN:\n\n");
		if(!log)
		{
			PutStr("\nComandos del ROOT:\n\n");
			PutStr("BORRAR CONTRASEÑA\n");
			PutStr("BORRAR EEPROM\n");
			PutStr("BORRADO LENTO\n");
			PutStr("DESCARGAR EEPROM\n");
			PutStr("DESCARGAR MEMORIA\n");
			PutStr("RELOJ SETUP\n");
#if defined(USO_MODEM)
			PutStr("PRENDER MODEM\n");
			PutStr("MODEM SETUP\n");
			PutStr("COMANDOS AT\n");
			PutStr("MANDAR SMS\n");
			PutStr("SMS?\n");
			PutStr("APAGAR MODEM\n");
#endif
		}
		PutStr("CAMBIAR HORA\n");
		PutStr("CAMBIAR CONTRASEÑA\n");
		PutStr("BANDAS HORARIAS\n");
		PutStr("TARIFAS\n");
		PutStr("PROMOCIONES\n");
		PutStr("TOLERANCIAS\n");
		PutStr("VALORES DEFINIDOS\n");
		PutStr("PLANILLA           F6\n");
		PutStr("IMPRIMIR CAJAS     F7\n");
		PutStr("ESTADISTICAS\n");
		PutStr("BORRAR AUTO\n");
		PutStr("BORRAR CAJAS\n");
		PutStr("BORRAR PLANILLA\n");
#if defined(USO_MODEM)
		PutStr("TELEFONO?          F9\n");
		PutStr("BORRAR TELEFONO\n");
#endif
#if defined(ESTADIA)
		PutStr("ESTADIA\n");
#endif
		PutStr("SALIR\n");
		Corto_Papel();
		Prompt(log,"");
		return;
	}
	Prompt(log,"Comando inexistente.");		// Si llegue hasta aca es porque no existe el comando.
	return;
}
/****************************************************************************/
/*				PROMP del sistema según quien este logueado.				*/
/*--------------------------------------------------------------------------*/
void Log(unsigned char log)
{
	Lcd_Clear();
	Lcd_Goto(0);
	if(!log)
		Lcd_Putch('R');		// Identifico al ROOT.
	if(log==1)
		Lcd_Putch('A');		// Identifico al administrador.
	Lcd_Putch(':');
	return;
}
/********************************************************************************/
/*	Anulación de ticket dentro del tiempo de tolerancia o por el administrador. */
/*------------------------------------------------------------------------------*/
unsigned char Anular_Ticket(unsigned char autorizacion)
{
	unsigned char hora_t,hora_c,hora_f[5];
	unsigned char i,horas,minutos,dia;
	unsigned int lugar;
	CLRWDT();
	if(rs_str[0]!='P')								// El ticket es para cobrar??
		return 0;
	if(autorizacion)
	{
		i=0;
		while(ps2_str[i++])							// Comparo si es el ticket que debo cobrar.
		{
			if(ps2_str[i]>96&&ps2_str[i]<123)
				ps2_str[i]=ps2_str[i]-32;			// Si hay alguna minúscula la conviento en mayuscula.
			if(ps2_str[i]!=rs_str[i+14])			// Comparo
				return 0;							// Si son distintos salgo y sigo con otro.
		}
		lugar=i+13+(pagina*32);						// Direccion necesaria luego.
	}
	else
	{
		i=1;
		while(ps2_str[i++])							// Comparo si es el ticket que debo anular.
		{
			if(ps2_str[i]>96&&ps2_str[i]<123)
				ps2_str[i]=ps2_str[i]-32;			// Si hay alguna minúscula la conviento en mayuscula.
			if(ps2_str[i]!=rs_str[i+12])			// Comparo
				return 0;							// Si son distintos salgo y sigo con otro.
		}
		lugar=i+11+(pagina*32);						// Direccion necesaria luego.
	}
	if(i<4)
		return 0;									// No se ingreso ninguna patente.
	Obtener_Hora(ps2_str,0);
	i=0;
	do
	{
		hora_f[i]=ps2_str[i+6];
	}while(i++<4);
	hora_f[4]=0;									// Marco final de cadena.
	hora_c=(ps2_str[9]-48)+((ps2_str[8]-48)*10);	// Transformo la hora en num.
	hora_t=(rs_str[11]-48)+((rs_str[10]-48)*10);	// Transformo el char en num y corrijo en caso de que salga al dia siguiente.
	if(hora_c<hora_t)								// Minutos de entrada mayor a los de salida.
		minutos=(hora_c+60)-hora_t;					// Obtengo los minutos a cobrar.
	else
		minutos=hora_c-hora_t;						// Obtengo los minutos a cobrar.
	i=0;
	horas=0;
	do
	{
		if(rs_str[i+2]!=ps2_str[i])
			horas=1;
	}while(i++<7);
	dia=Eeprom_Read(222);							// Tolerancia de anulación.
	if((minutos>dia||horas)&&!autorizacion)
	{
		Imprimir_Lcd(":","Tiempo excedido.",1);
		return 3;
	}
	Imprimir_Lcd("Anular ticket? S/N","",1);
	Reseteo_Teclado();								// Reinicio la cauptura por teclado PS2.
	while(INT0IE)									// Por defecto anula.
	{
		CLRWDT();
		pos_str_ps2=0;
		Imprimir_Lcd("",ps2_str,0);
		if(ps2_str[0]=='N')							// Salgo a la pantalla principal.
		{
			Imprimir_Lcd(":","Anulacion cancelada.",1);
			return 2;								// Salgo y aviso de la anulacion.
		}
		if(ps2_str[0]=='S')							// Imprimo el ticket y lo guardo en memoria.
			break;
	}
	if(autorizacion)
		Write_24LC((pagina*32),'B');				// Marco la entrada como eliminado por el administrador.
	else
		Write_24LC((pagina*32),'A');				// Marco la entrada como borrada.
	Guardo_Estadisticas(12);
	Write_24LC(lugar++,' ');
	i=0;
	do												// Cargo la hora de anulacion.
	{
		Write_24LC(lugar++,hora_f[i]);
	}while(hora_f[i++]&&(lugar<((pagina*32)+31)));	// No escribo mas alla de la pagina en la que estoy.
	Write_24LC(lugar++,0);							// Marco el final de la cadena guardada en memoria.
	Imprimir_Lcd(":","Ticket anulado.",1);
	return 1;
}
/********************************************************************************/
/*	Imprimo por impresora un valor numérico de un byte leido de la memoria.		*/
/*------------------------------------------------------------------------------*/
void Imprimir_Char(unsigned int lugar)
{
	unsigned char cad_imp[4],a;
	cad_imp[3]=Eeprom_Read(lugar);
	cad_imp[0]=cad_imp[3]/100;					// Calculo la centena.
	cad_imp[1]=(cad_imp[3]-cad_imp[0]*100)/10;	// Calculo la decena.
	cad_imp[2]=(cad_imp[3]-cad_imp[0]*100-cad_imp[1]*10);	// Calculo la unidad.
	a=0;
	if(cad_imp[0])
		cad_imp[a++]=cad_imp[0]+48;				// Centena.
	cad_imp[a++]=cad_imp[1]+48;					// Decena.
	cad_imp[a++]=cad_imp[2]+48;					// Unidad.
	cad_imp[a++]=0;								// Final 3na.
	PutStr(cad_imp);
	return;
}
/********************************************************************************/
/*				Muestro la hora actual en el display							*/
/*------------------------------------------------------------------------------*/
void Visualizar_Hora(void)
{
	Obtener_Hora(ps2_str,1);
	Imprimir_Lcd("",ps2_str,0);
	return;
}
/********************************************************************************/
/*		Imprimo por impresora las estadísticas del turno.						*/
/*------------------------------------------------------------------------------*/
void Imprimir_Estadisticas(void)
{
	unsigned char cad_temp[10];
	volatile unsigned int i;
	Obtener_Hora(rs_str,1);			// Cargo la hora en formato estandar
	Inicializo_Impresion(1,1);
	PutStr("Fecha y Hora del inicio:\n");
	for(i=0;i<5;i++)
	{
		cad_temp[8]=Eeprom_Read(109+i);
		if(cad_temp[8]<10)
		{
			cad_temp[0]=48;
			cad_temp[1]=cad_temp[8]+48;
		}
		else
		{
			cad_temp[0]=(cad_temp[8]/10)+48;
			cad_temp[1]=(cad_temp[8]-((cad_temp[0]-48)*10))+48;
		}
		cad_temp[2]=0;
		PutStr(cad_temp);
		if(!i||i==1)
			PutCh('/');
		if(i==2)
			PutCh(' ');
		if(i==3)
			PutCh(':');
	}
	PutStr("\nFecha y Hora del cierre:\n");
	PutStr(rs_str);					// Imprimimo la fecha y hora del cierre.
	PutStr("\n\nEstadísticas:");
	PutStr("\n\nAnulación de tickets: ");
	Recupero_Estadisticas(12,cad_temp);
	PutStr(cad_temp);
#if defined(ESTADIA)
	PutStr("\nEstadias diarias: ");
	Recupero_Estadisticas(13,cad_temp);
	PutStr(cad_temp);
#endif
	PutStr("\nVehículos con promoción:\n");
	PutStr("Autos:       ");
	Recupero_Estadisticas(9,cad_temp);
	PutStr(cad_temp);
	PutStr("\nMotos:       ");
	Recupero_Estadisticas(10,cad_temp);
	PutStr(cad_temp);
	PutStr("\nUtilitarios: ");
	Recupero_Estadisticas(11,cad_temp);
	PutStr(cad_temp);
	PutStr("\nVehículos con tarifa 1:\n");
	PutStr("Autos:       ");
	Recupero_Estadisticas(0,cad_temp);
	PutStr(cad_temp);
	PutStr("\nMotos:       ");
	Recupero_Estadisticas(1,cad_temp);
	PutStr(cad_temp);
	PutStr("\nUtilitarios: ");
	Recupero_Estadisticas(2,cad_temp);
	PutStr(cad_temp);
	PutStr("\nVehículos con tarifa 2:\n");
	PutStr("Autos:       ");
	Recupero_Estadisticas(3,cad_temp);
	PutStr(cad_temp);
	PutStr("\nMotos:       ");
	Recupero_Estadisticas(4,cad_temp);
	PutStr(cad_temp);
	PutStr("\nUtilitarios: ");
	Recupero_Estadisticas(5,cad_temp);
	PutStr(cad_temp);
	PutStr("\nVehículos con tarifa 3:\n");
	PutStr("Autos:       ");
	Recupero_Estadisticas(6,cad_temp);
	PutStr(cad_temp);
	PutStr("\nMotos:       ");
	Recupero_Estadisticas(7,cad_temp);
	PutStr(cad_temp);
	PutStr("\nUtilitarios: ");
	Recupero_Estadisticas(8,cad_temp);
	PutStr(cad_temp);
	PutStr("\n\nTotal cobrado: $");
	i=Caja_Read();
	IntToStr(i,ps2_str);					// Transformo el entero en una cadena de caracteres.
	PutStr(ps2_str);
	PutStr("\n\n");
	Corto_Papel();
	return;
}
/********************************************************************************/
/*		Borrado rápido, solo elimino el primer caracter de cada página.			*/
/*------------------------------------------------------------------------------*/
void Borrado_Planilla(void)
{
	volatile unsigned int i,porcent,comparar;
	unsigned char cadena[3];
	Imprimir_Lcd("Borrando memoria","   %",1);
	for(i=0;i<2048;i++)
	{
		Borrar_24LC256(i);				// Borro pagina por pagina.
		porcent=(i*10)/205;
		if(comparar!=porcent)
		{
			comparar=porcent;
			if(porcent<10)
			{
				cadena[1]=porcent+48;
				cadena[0]=48;
			}
			else
			{
				cadena[0]=((porcent/10)+48);
				cadena[1]=((porcent-((cadena[0]-48)*10))+48);
			}
			cadena[2]=0;
			Lcd_Goto(0x40);
			Lcd_Puts(cadena);
		}
	}
	return;
}
/********************************************************************************/
/*								*/
/*------------------------------------------------------------------------------*/
void Imprimir_Movimientos_Cobrados(void)
{
	volatile unsigned int pag,e;
	unsigned char i,cad_temp[5];
	Inicializo_Impresion(0,1);
	PutStr("Movimientos cobrados:\n");
	PutStr(rs_str);							// Imprimimo la fecha y hora.
	PutStr("\n\n");
	Inicializo_Impresion(1,1);
	e=1;
	for(pag=0;pag<=2047;pag++)				// Barro las 2048 paginas de las memorias.
	{
		i=Read_24LC(pag*32);				// Cargo en memoria la página.
		if(i&&i!='P'&&i!='*')				// La página tiene algún contenido?
		{
			Rec_Paquete_24LC(pag,rs_str);	// Cargo en memoria la página.
			IntToStr(e*10,cad_temp);
			PutStr(cad_temp);
			PutCh(' ');
			i=0;
			while(rs_str[i])
			{
				if(rs_str[i-3]==' ')
					PutCh(':');				// Imprimo los dos puntos en la hora de cobro.
				if(rs_str[i-5]==' ')
					PutStr(" $");			// Imprimo el espacio que separa el monto.
				PutCh(rs_str[i++]);			// La mando por RS232.
				if(i==4||i==6)
					PutCh('/');
				if(i==2||i==8||i==14)
					PutCh(' ');
				if(i==10||i==12)
					PutCh(':');
			}
			e++;
			PutStr("\n");
			Write_24LC(pag*32,0);			// Borro los registros que voy imprimiendo.
		}
	}
	PutStr("\nA - Anulado.                        a - Auto.\n");
	PutStr("B - Borrado por el administrador.   m - Moto.\n");
	PutStr("C - Cobrado.                        u - Utilitario.\n");
#if defined(ESTADIA)
	PutStr("E - Estadia diaria.\n");
#endif
	PutStr("P - Vehículo en playa.\n");
	Corto_Papel();
	return;
}
/********************************************************************************/
/*	Imprimo por impresora un valor numérico de dos bytes leido de la memoria.	*/
/*------------------------------------------------------------------------------*/
void Imprimir_Entero(unsigned int lugar,unsigned char coma)
{
	unsigned char a,b,cad_imp[5];
	unsigned int a1,a2,a3,a4;
	CLRWDT();
	a=Eeprom_Read(lugar);
	b=Eeprom_Read(lugar+1);
	lugar=a<<8|b;
	if(!coma)
		lugar*=10;
	IntToStr(lugar,cad_imp);
	PutStr(cad_imp);
	return;
}
/********************************************************************************/
/*		Cambio de contraseñas para el ROOT y el ADMINISTRADOR.					*/
/*------------------------------------------------------------------------------*/
void Cambio_Pass(unsigned char log)
{
	unsigned char i,cad_temp[20];
	CLRWDT();
	Reseteo_Teclado();
	while(INT0IE)							// Espero un enter del teclado
	{										// para procesar el comando.
		if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
			vis=pos_str_ps2;
		if(pos_str_ps2==vis)				// Actulizo la pantalla?
		{
			Linea_Lcd("NUEVA CONTRASEÑA?",1);	// Muestro el mensaje.
			Lcd_Goto(0x40);
			i=0;
			while(i<pos_str_ps2&&pos_str_ps2)	// Escondo lo que se escribe.
			{
				i++;
				Lcd_Putch('*');				// Muestro en display *.
			}
			if(pos_str_ps2>20)				// Largo maximo de la contraseña.
			{
				pos_str_ps2=20;
				vis=20;
			}
			vis++;							// 
		}
		CLRWDT();
	}
	pos_str_ps2=0;
	do
	{
		cad_temp[pos_str_ps2]=ps2_str[pos_str_ps2];
	}while(ps2_str[pos_str_ps2++]);
	cad_temp[pos_str_ps2]=0;				// Marco el fin de cadena.
	Reseteo_Teclado();
	while(INT0IE)							// Espero un enter del teclado
	{										// para procesar el comando.
		if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
			vis=pos_str_ps2;
		if(pos_str_ps2==vis)				// Actulizo la pantalla?
		{
			Log(log);
			Linea_Lcd("REPETIR CONTRASEÑA",0);	// Muestro lo que se va escribiendo.
			Lcd_Goto(0x40);
			i=0;
			while(i<pos_str_ps2&&pos_str_ps2)	// Escondo lo que se escribe.
			{
				i++;
				Lcd_Putch('*');				// Muestro en display *.
			}
			if(pos_str_ps2>20)				// Largo maximo de la contraseña.
			{
				pos_str_ps2=20;
				vis=20;
			}
			vis++;							// 
		}
		CLRWDT();
	}
	if(Comparo_Cadenas(ps2_str,cad_temp,0))
	{
		Almacenar_Pass(ps2_str,log);
		Prompt(log,"Contraseña cambiada.");
		return;
	}
	else
		Prompt(log,"ERROR");
	DelayMs(150);
	return;
}
/********************************************************************************/
/*			Cambio de la hora del sistema.										*/
/*------------------------------------------------------------------------------*/
void Cambio_Hora(void)
{
	Reseteo_Teclado();
	while(INT0IE)							// Espero un enter del teclado
	{										// para procesar el comando.
		if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
			vis=pos_str_ps2;
		if(pos_str_ps2==vis)				// Actualiazo la pantalla?
		{
			Linea_Lcd("FECHA:",1);
			Lcd_Goto(0x40);
			if(pos_str_ps2>0)
				Lcd_Putch(ps2_str[0]);
			if(pos_str_ps2>1)
				Lcd_Putch(ps2_str[1]);
			Lcd_Goto(0x42);
			Lcd_Putch('/');
			if(pos_str_ps2>2)
				Lcd_Putch(ps2_str[2]);
			if(pos_str_ps2>3)
				Lcd_Putch(ps2_str[3]);
			Lcd_Goto(0x45);
			Lcd_Putch('/');
			if(pos_str_ps2>4)
				Lcd_Putch(ps2_str[4]);
			if(pos_str_ps2>5)
				Lcd_Putch(ps2_str[5]);
			if(pos_str_ps2>6)
			{
				pos_str_ps2=6;
				vis=6;
			}
			vis++;							// 
		}
		CLRWDT();
	}
		pos_str_ps2=6;
	vis=6;
	shift_ps2=0;
	lect_ps2=0;								// Vacío la variable lectura del teclado PS2.
	pos_ps2=11;								// Necesario para la correcta captura de teclas.
	mayus_ps2=1;							// Comienzo escribiendo con mayusculas.
	INT0IF=0;								// Bajo la bandera de la interrupcion.
	INT0IE=1;								// Habilito la interrupción del teclado
	while(INT0IE)							// Espero un enter del teclado
	{										// para procesar el comando.
		if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
			vis=pos_str_ps2;
		if(pos_str_ps2==vis)				// Actualiazo la pantalla?
		{
			Linea_Lcd("HORA:",1);
			vis++;							// 
			Lcd_Goto(0x40);
			if(pos_str_ps2>6)
				Lcd_Putch(ps2_str[6]);
			if(pos_str_ps2>7)
				Lcd_Putch(ps2_str[7]);
			Lcd_Goto(0x42);
			Lcd_Putch(':');
			if(pos_str_ps2>8)
				Lcd_Putch(ps2_str[8]);
			if(pos_str_ps2>9)
				Lcd_Putch(ps2_str[9]);
		}
		CLRWDT();
	}
	Seteo_Hora(ps2_str);
	return;
}
/********************************************************************************/
/*			Capturo por teclado y guardo en la memoria EEPROM.					*/
/*------------------------------------------------------------------------------*/
void Capturo_Guardo(unsigned int lugar,const unsigned char *mensaje)
{
	unsigned char i;
	Reseteo_Teclado();
	while(INT0IE)							// Espero un enter del teclado
	{										// para procesar el comando.
		if(pos_str_ps2<vis-1)				// Se perdió la variable vis?
			vis=pos_str_ps2;
		if(pos_str_ps2==vis)				// Actulizo la pantalla?
		{
			Imprimir_Lcd(mensaje,ps2_str,1);	// Muestro lo que se va escribiendo.
			vis++;								// 
		}
		CLRWDT();
	}
	if(pos_str_ps2>1)
		i=(ps2_str[1]-48)+((ps2_str[0]-48)*10);
	else
	{
		if(ps2_str[0])
			i=ps2_str[0]-48;
	}
	Eeprom_Write(lugar,i);
	return;
}
/********************************************************************************/
/*			Cambio de las tarifas.												*/
/*------------------------------------------------------------------------------*/
void Tarifas(unsigned char log)
{
	Capturo_Guardo(160,"1 TURNO AUTO:");
	Capturo_Guardo(169,"1/2 1 TURNO AUTO:");
	Capturo_Guardo(161,"1 TURNO MOTO:");
	Capturo_Guardo(170,"1/2 1 TURNO MOTO:");
	Capturo_Guardo(162,"1 TURNO UTILITARIO:");
	Capturo_Guardo(171,"1/2 1 TURNO UTILITARIO:");
	Capturo_Guardo(163,"2 TURNO AUTO:");
	Capturo_Guardo(172,"1/2 2 TURNO AUTO:");
	Capturo_Guardo(164,"2 TURNO MOTO:");
	Capturo_Guardo(173,"1/2 2 TURNO MOTO:");
	Capturo_Guardo(165,"2 TURNO UTILITARIO:");
	Capturo_Guardo(174,"1/2 2 TURNO UTILITARIO:");
	Capturo_Guardo(166,"3 TURNO AUTO:");
	Capturo_Guardo(175,"1/2 3 TURNO AUTO:");
	Capturo_Guardo(167,"3 TURNO MOTO:");
	Capturo_Guardo(176,"1/2 3 TURNO MOTO:");
	Capturo_Guardo(168,"3 TURNO UTIL.:");
	Capturo_Guardo(177,"1/2 3 TURNO UTIL.:");
#if defined(ESTADIA)
	Capturo_Guardo(230,"ESTADIA AUTO:");
	Capturo_Guardo(231,"ESTADIA MOTO:");
	Capturo_Guardo(232,"ESTADIA UTIL.:");
#endif
	Prompt(log,"Tarifas configuradas.");
	return;
}
/****************************************************************************/
/*	Muestro en display el promp, ya sea para el ROOT o el ADMINISTRADOR.	*/
/*--------------------------------------------------------------------------*/
void Prompt(unsigned char log,const unsigned char *mensaje)
{
	Lcd_Clear();
	Lcd_Goto(0);
	if(!log)
		Lcd_Putch('R');		// Identifico al ROOT.
	if(log==1)
		Lcd_Putch('A');		// Identifico al administrador.
	Lcd_Putch(':');
	if(*mensaje)
	{
		Lcd_Goto(0x40);
		Lcd_Puts(mensaje);
	}
	return;
}
/****************************************************************************/
/*			Envío pr RS232 todo el contenido de la memoria externa.			*/
/*--------------------------------------------------------------------------*/
void Descargo_Memoria(void)
{
	volatile unsigned int i,e,porcent,comparar=0;
	unsigned char cadena[3]={'0','0',0};
	Imprimir_Lcd("Descargando memoria","   %",1);
	Lcd_Goto(0x40);
	Lcd_Puts(cadena);
	for(i=0;i<65535;i++)
	{
		e=Read_24LC(i);							// Obtengo el valor almacenado en la mem.
		PutCh(e);
		porcent=i/655;
		if(comparar!=porcent)
		{
			comparar=porcent;
			if(porcent<10)
			{
				cadena[1]=porcent+48;
				cadena[0]=48;
			}
			else
			{
				cadena[0]=((porcent/10)+48);
				cadena[1]=((porcent-((cadena[0]-48)*10))+48);
			}
			cadena[2]=0;
			Lcd_Goto(0x40);
			Lcd_Puts(cadena);
		}
	}
	return;
}
/********************************************************************************/
/*				Elimino todo el contenido de la memoria externa.				*/
/*------------------------------------------------------------------------------*/
void Borrado_Completo(void)
{
	volatile unsigned int i,porcent,comparar;
	unsigned char cadena[3]={'0','0',0};
	Imprimir_Lcd("Borrando memoria","   %",1);
	Lcd_Goto(0x40);
	Lcd_Puts(cadena);
	for(i=0;i<65535;i++)
	{
		Write_24LC(i,0);
		porcent=i/655;
		if(comparar!=porcent)
		{
			comparar=porcent;
			if(porcent<10)
			{
				cadena[1]=porcent+48;
				cadena[0]=48;
			}
			else
			{
				cadena[0]=((porcent/10)+48);
				cadena[1]=((porcent-((cadena[0]-48)*10))+48);
			}
			cadena[2]=0;
			Lcd_Goto(0x40);
			Lcd_Puts(cadena);
		}
	}
	return;
}
