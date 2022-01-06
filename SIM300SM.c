/********************************************************************************/
/*	MANEJO DEL MODEM SIM340C MEDIANTE COMANDOS AT ENVIADOS POR PUERTO SERIE.	*/
/********************************************************************************/
unsigned char Modem_Power(unsigned char accion,unsigned char banda);
unsigned char Proveedor(void);
void Modem_Setup(void);
void Consulta_Modem(const char *comando_at,unsigned char *respuesta);
unsigned char Espero_Ok(void);
unsigned char Modem_Banda(unsigned char banda);
void Leer_SMS(unsigned char *respuesta,unsigned char dir);
unsigned char Borrar_SMS(unsigned char dir);
void Enviar_SMS(const char *sms,const char *destino);
void Remitente_SMS(unsigned char *respuesta, unsigned char dir);
unsigned char SMS_Recibido(void);
/************************************************************************/
/*		Manejo la alimentacion del MODEM y controlo que arranque.		*/
/************************************************************************/
unsigned char Modem_Power(unsigned char accion,unsigned char banda)
{
	static bit esperando,c_banda;
	const static unsigned char comando[]="Call Ready";
	unsigned char tiempo;
	MODO_MODEM;						// Habilito el RS232 para el MODEM.
	c_banda=0;
	esperando=0;
	CLRWDT();
	if(accion)						// Debo prender el MODEM??
	{
		POWER_MODEM=0;				// Enciendo el modem
		tiempo=0;
		while(!NET_STATE)			// Espero que el MODEM encienda.
		{
			if(tiempo++>60)			// Espero 3 segundos.
			{
				POWER_MODEM=1;		// 
				LED=1;				// Dejo el LED prendido.
				return 0;
			}
			DelayMs(50);			// Junto con el contador tiempo doy el timeout.
			LED=!LED;				// Parpadea el LED mientras espera que arranque.
		}
		LED=1;						// Dejo el LED prendido.
		POWER_MODEM=1;				// 
		while(!esperando)
		{
			CLRWDT();
			pos=0;					// Reseteo la cadena.
			tiempo=0;				// Reseteo el contador para timeout.
			CREN=1;					// Habilito la recepcion continua en RS232.
			RCIE=1;					// Habilito la interrupción por RS232.
			while(RCIE)				// Espero que termine la recepción RS232.
			{
				DelayMs(100);
				if(++tiempo>100)	// tiempo * DelayMs = 10 segundos.
					return 0;		// Excedió el tiempo de respuesta.
			}
			if(!c_banda)
			{
				Modem_Banda(banda);	// Configuro en que banda quiero que se conecte.
				c_banda=1;			// Solo entro una vez.
			}
			esperando=1;			// Los caracteres coinciden.
			for(pos=0;pos<11;pos++)	// Compruebo el comando recibido.
			{
				if(rs_str[pos]!=comando[pos])	// Algun caracter es distinto?
					esperando=0;	// Bajo la bandera.
			}
		}
		return 1;
	}
	else							// Entonces lo apago.
	{
		PutStr("AT+CPOWD=1");		// Apago el MODEM.
		PutCh(13);
		DelayS(3);					// Le doy tiempo para que se apague.
	}
	return 1;						// Apagado de MODEM correcto.
}
/************************************************************************/
/*		Espero a tener señal e identificar el proveedor de telefonia.	*/
/************************************************************************/
unsigned char Proveedor(void)
{
	static bit encontrado;
	volatile unsigned char temp=0,temp_1;
	static const unsigned char per[]="PERSONAL",cti[]="CTI Movil",uni[]="UNIFON";
	CLRWDT();
	MODO_MODEM;
	while(++temp<10)					// Intento durante 60 segundos.
	{
		temp_1=0;						// Inicializo las variables.
		pos=0;
		PutStr("AT+COPS?");				// Buscando proveedor. 
		CREN=1;							// Habilito la recepcion continua en RS232.
		RCIE=1;							// Habilito la interrupcion por RS232.
		PutCh(13);						// Le doy el enter al comando.
		while(RCIE)						// Espero a terminar de cargar la respuesta.
		{
			DelayMs(20);
			if(++temp_1>100)			// tiempo * DelayMs = 2 segundos.
				break;					// Excedió el tiempo de respuesta.
		}
		pos=0;							// Inicializo las variables.
		temp_1=0;
		while(rs_str[pos++]!='"'&&pos<20)	// Busco el nombre del proveedor.
			CLRWDT();
		encontrado=1;
		if(rs_str[pos]=='P')			// Personal???
		{
			while(rs_str[pos]!='"')		// Comparo la respuesta.
			{
				CLRWDT();
				if(rs_str[pos++]!=per[temp_1++])
					encontrado=0;		// No coincide la empresa.
			}
			if(encontrado)				// Coincidio la cadena de respuesta?
				return 'P';				// confirmado, es personal.
		}
		if(rs_str[pos]=='C')			// CTI???
		{
			while(rs_str[pos]!='"')		// Comparo la respuesta.
			{
				CLRWDT();
				if(rs_str[pos++]!=cti[temp_1++])
					encontrado=0;		// No coincide la empresa.
			}
			if(encontrado)				// Coincidio la cadena de respuesta?
				return 'C';				// confirmado, es cti.
		}
		if(rs_str[pos]=='U')			// Unifon???
		{
			while(rs_str[pos]!='"')		// Comparo la respuesta.
			{
				CLRWDT();
				if(rs_str[pos++]!=uni[temp_1++])
					encontrado=0;		// No coincide la empresa.
			}
			if(encontrado)				// Coincidio la cadena de respuesta?
				return 'U';				// confirmado, es unifon.
		}
		DelayS(3);						// Cada tres segundo intento de nuevo.
	}
	return 0;							// Proveedor no reconocido.
}
/************************************************************************/
/*	CONFIGURACION INICIAL DEL MODEM, SE ACCEDE DESDE MENU DE SERVICIO.	*/
/************************************************************************/
void Modem_Setup(void)
{
	CLRWDT();
	PutStr("AT");					// Abro consola.
	PutCh(13);
	DelayMs(100);
	PutStr("AT");					// 
	Espero_Ok();
	PutStr("ATE0");					// Desactivo el eco de los comandos.
	Espero_Ok();
	PutStr("AT+IPR=9600");			// Seteo el puerto del modem a 9600 bps.
	Espero_Ok();
	PutStr("AT+CMGF=1");			// Seteo los SMS en formato texto.
	Espero_Ok();
	PutStr("AT&D0");				// Configuro el pin DTR del modem.
	Espero_Ok();
	PutStr("AT+CSCS=\"GSM\"");		// Configuro para enviar msn.
	Espero_Ok();					// Espero la respuesta del MODEM.
	PutStr("AT&W");					// Guardo la configuracion en el MODEM.
	Espero_Ok();
}
/************************************************************************/
/*		CONSULTO EL MODEM Y GUARDO LA RESPUESTA EN UNA CADENA.			*/
/************************************************************************/
void Consulta_Modem(const char *comando_at,unsigned char *respuesta)
{
	unsigned char tiempo=0;
	MODO_MODEM;
	pos=0;							// Reseteo la cadena.
	PutStr(comando_at);				// Envio el comando AT al modem
	PutCh(13);						// Enter para que se ejecute el comando.
	CREN=1;							// Habilito la recepcion continua en RS232.
	RCIE=1;							// Habilito la interrupcion por RS232.
	while(RCIE)						// Espero que cargue la cadena recibida.
	{
		DelayMs(10);
		if(++tiempo>250)			// tiempo * DelayMs = 2.5 segundos.
		{
			*respuesta=0;			// Marco el final de la cadena.
			return;					// Excedió el tiempo de respuesta.
		}
	}
	pos=0;							// Reseteo el contador para barrer la cadena desde el principio.
	while(rs_str[pos])				// Mientras no sea un final de cadena.
		*respuesta++=rs_str[pos++];	// Copio la respuesta en la cadena de destino.
	*respuesta=0;					// Marco el fin de la cadena.
	return;
}

/************************************************************************/
/*			ESPERO LA RESPUESTA DEL MODEM								*/
/************************************************************************/
unsigned char Espero_Ok(void)
{
	unsigned char tiempo_ok=0;
	pos=0;							// Reseteo la cadena.
	bandera=1;
	CREN=1;							// Habilito la recepcion continua en RS232.
	RCIE=1;							// Habilito la interrupción por RS232.
	PutCh(13);						// Le doy enter al comando.
	while(RCIE)
	{
		DelayMs(100);
		if(++tiempo_ok>250)			// tiempo * DelayMs = 25 segundos.
			return 0;				// Excedió el tiempo de respuesta.
	}
	return 1;
}
/************************************************************************/
/*				HAGO UN CAMBIO DE BANDA.								*/
/************************************************************************/
unsigned char Modem_Banda(unsigned char banda)
{
	if(banda>=5)
		return 0;
	PutStr("AT+CBAND=\"");
	if(!banda)
		PutStr("PGSM_MODE\"");
	if(banda==1)
		PutStr("DCS_MODE\"");
	if(banda==2)
		PutStr("PCS_MODE\"");
	if(banda==3)
		PutStr("EGSM_DCS_MODE\"");
	if(banda==4)
		PutStr("GSM850_PCS_MODE\"");
	Espero_Ok();						// Espero la respuesta del cambio de banda.
	return 1;
}
/********************************************************************************/
/*				LEO UN SMS ALMACENADO EN MEMORIA								*/
/********************************************************************************/
void Leer_SMS(unsigned char *respuesta,unsigned char dir)
{
	unsigned char tiempo;
	if(dir>19)
		return;
	MODO_MODEM;
	DelayMs(250);
	PutStr("AT+CMGR=");					// Comando para leer el SMS numero:
	if(dir<=9)
		PutCh(dir+48);					// número de mensaje a leer si es menor a 10
	if(dir>9&&dir<=19)
	{
		PutCh('1');
		PutCh(dir+38);					// número de mensaje a leer si es menor a 20 y mayor a 10.
	}
	tiempo=0;
	pos=0;
	bandera=0;							// Leo la segunda cadena.
	CREN=1;								// Habilito la recepción continua.
	RCIE=1;								// Habilito la interrupción por RS232.
	PutCh(13);							// Envio el enter para el comando.
	while(RCIE)
	{
		DelayMs(20);
		if(tiempo++>250)				// Timeout??
		{								// tiempo * DelayMs = 5 segundos.
			RCIE=0;						// Deshabilito la interrupción por RS232.
			CREN=0;						// Deshabilito la recepción continua en RS232.
			*respuesta=0;				// Marco el final de la cadena.
			return;						// Excedió el tiempo de respuesta, salgo.
		}
	}
	pos=0;								// Reseteo el contador para barrer la cadena desde el principio.
	while(rs_str[pos])					// Mientras no sea un final de cadena.
		*respuesta++=rs_str[pos++];		// Copio la respuesta en la cadena de destino.
	*respuesta=0;						// Marco el fin de la cadena.
	return;
}
/********************************************************************************/
/*														*/
/********************************************************************************/
unsigned char Borrar_SMS(unsigned char dir)
{
	if(dir>16)
		return 0;
	MODO_MODEM;
	DelayMs(25);
	PutStr("AT+CMGD=");					// Comando para leer el SMS numero 1.
	if(dir<=9)
		PutCh(dir+48);
	if(dir>9&&dir<=19)
	{
		PutCh('1');
		PutCh(dir+38);
	}
	Espero_Ok();
	return 1;
}
/********************************************************************************/
/*														*/
/********************************************************************************/
void Enviar_SMS(const char *sms,const char *destino)
{
	MODO_MODEM;
	DelayMs(2);
	PutStr("AT+CMGS=\"");				// Comando para definir el destinatario.
	PutStr(destino);					// Número de destino.
	PutStr("\"");
	Espero_Ok();						// Espero la respuesta del MODEM.
	PutStr(sms);
	PutCh(26);							// CTRL Z fin del mensaje.
	Espero_Ok();						// Espero la respuesta del MODEM.
	return;
}
/********************************************************************************/
/*		OBTENGO EL NUMERO DE TELEFONO DE QUIEN ENVIO EL SMS.					*/
/********************************************************************************/
void Remitente_SMS(unsigned char *respuesta, unsigned char dir)
{
	unsigned char tiempo;
	if(dir>16)
		return;
	MODO_MODEM;
	DelayMs(25);
	PutStr("AT+CMGR=");					// Comando para leer el SMS numero:
	if(dir<=9)
		PutCh(dir+48);					// número de mensaje a leer si es menor a 10
	if(dir>9&&dir<=19)
	{
		PutCh('1');
		PutCh(dir+38);					// número de mensaje a leer si es menor a 20 y mayor a 10.
	}
	tiempo=0;
	pos=0;
	bandera=1;							// Leo la primera cadena.
	CREN=1;								// Habilito la recepción continua.
	RCIE=1;								// Habilito la interrupción por RS232.
	PutCh(13);							// Envio el comando.
	while(RCIE)
	{
		DelayMs(20);
		if(tiempo++>250)				// tiempo * DelayMs = 5 segundos.
		{
			RCIE=0;
			CREN=0;
			*respuesta=0;				// Marco el fin de la cadena.
			return;						// Excedió el tiempo de respuesta.
		}
	}
	pos=0;								// Reseteo el contador para barrer la cadena desde el principio.
	tiempo=0;
	while(rs_str[pos])					// Mientras no sea un final de cadena.
	{
		CLRWDT();
		if((rs_str[pos]=='+'||tiempo)&&pos)
		{
			tiempo=1;					// Lo uso como bandera para seguir el llenado.
			if(rs_str[pos]=='"')		// Termino de llenarse el numero?
			{
				*respuesta=0;			// Agrego el NULL al final de la cadena.
				return;					// salgo de la funcion.
			}
			*respuesta++=rs_str[pos];	// Copio la respuesta en la cadena de destino.
		}
		pos++;
	}
	*respuesta=0;						// Marco el final de la cadena.
	return;
}
/********************************************************************************/
/*	EN CASO DE LLEGAR UN SMS OBTENGO LA POSICION EN LA QUE ESTA EN MEMORIA.		*/
/********************************************************************************/
unsigned char SMS_Recibido(void)
{
	unsigned char tiempo;
	MODO_MODEM;
	DelayMs(100);
	PutStr("AT+CMGL=\"ALL\"");			// Comando para listar los sms.
	tiempo=0;
	pos=0;
	bandera=1;							// Leo la primera cadena.
	CREN=1;								// Habilito la recepción continua.
	RCIE=1;								// Habilito la interrupción por RS232.
	PutCh(13);							// Envio el comando.
	while(RCIE)
	{
		DelayMs(10);
		if(tiempo++>100)				// tiempo * DelayMs = 1 segundos.
		{
			RCIE=0;
			CREN=0;
			return 0;					// Excedió el tiempo de respuesta.
		}
	}
	if(Comparo_Cadenas(rs_str,"+CMGL: ",7))
		return (rs_str[7]-48);
	return 0;
}
