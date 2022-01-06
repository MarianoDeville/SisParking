/****************************************************************************/
/*			MANEJO DE LA IMPRESORA EPSON TM-T88 POR PUERTO SERIE.			*/
/*..........................................................................*/
/*				PIC:				PIC18F4620								*/
/*				Compilador:			MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.	*/
/*				Autor:				Mariano Ariel Deville					*/
/****************************************************************************/
/*						PROTOTIPOS DE FUNCIONES								*/
/*..........................................................................*/
void Corto_Papel(void);
void Inicializo_Impresion(unsigned char letra,unsigned char jus);
void Imprimir(register const char *cadena,unsigned char barras);
void Imprimir_Ticket(unsigned char *patente,unsigned char *hora);
void Retorno_Carro(void);
/****************************************************************************/
/*			Impresion utilizando impresora EPSON TM88						*/
/*..........................................................................*/
void Inicializo_Impresion(unsigned char letra,unsigned char jus)
{
	CLRWDT();
	MODO_IMP;
	DelayMs(5);
	PutCh(0x1b);		// esc @ = inicializo printer.
	PutCh('@');
	PutCh(0x1b);		// esc R = tabla de caracteres spain
	PutCh('R');
	PutCh(0x07);		
	if(!jus)
	{
		PutCh(0x1b);	// esc a = jstifica al centro.
		PutCh('a');
		PutCh(0x01);
	}
	else
	{
		PutCh(0x1b);	// esc a = justifica a la izquierda.
		PutCh('a');
		PutCh(0x00);
	}
	if(letra)
	{
		PutCh(0x1b);	// Esc ! = cambio tamaño letra.
		PutCh(0x21);
		PutCh(0x01);
	}
	return;
}
/****************************************************************************/
/*			Impresion utilizando impresora EPSON TM88						*/
/*..........................................................................*/
void Imprimir(register const char *cadena,unsigned char barras)
{
	Inicializo_Impresion(0,0);
	if(barras)
	{
		PutCh(0x1d);	// gs = 1d - h - numero de dots de altura del codigo
		PutCh('h');
		PutCh(0x40);	// altura del codigo = 50 dots
		PutCh(0x1d);	// gs H = HRI = posicision del # del codigo
		PutCh('H');		// 0 = sin numero - 1 = arriba - 2 = abajo - 3 = arriba y abajo
		PutCh(0x00);
		PutCh(0x1d);
		PutCh('k');		// gs k = imprimir codigo con formato 0x04 = codigo 39
		PutCh(0x04);
		PutStr("C ");
	}
	PutStr(cadena);
	PutCh(0x00);		// Enter
	PutCh(0x0a);
	return;
}
/****************************************************************************/
/*			Secuencia para cortar el papel de la impresora EPSON			*/
/*..........................................................................*/
void Corto_Papel(void)
{
	MODO_IMP;
	PutCh(0x1b);
	PutCh('@');
	PutCh(0x1b);
	PutCh('a');
	PutStr("\n");
	PutCh(0x1d);
	PutCh('V');
	PutCh(0x42);
	PutCh(0x80);
	DelayMs(2);
	return;
}
/****************************************************************************/
/*			Secuencia para cortar el papel de la impresora EPSON			*/
/*..........................................................................*/
void Imprimir_Ticket(unsigned char *patente,unsigned char *hora)
{
	volatile unsigned char i;
	Inicializo_Impresion(0,0);
	PutStr("PLAYA DE ESTACIONAMIENTO \"LA PLAYITA\"\n");
	PutStr("CUIT: 20-35893820-6\n");
	PutStr("IIB: 280-494381\n");
	PutStr("MUN: 20-358933820-6\n");
 	PutStr("Inicio 01/08/2011\n");
	PutStr("Lisandro de la Torre 88 - V.C.PAZ\n");
	PutStr("\n");
	PutStr("Fecha: ");
	for(i=0;i<8;i++)
		PutCh(ps2_str[i]);		// Imprimo la patente del vehiculo.
	PutStr("  Hora: ");
	for(i=9;i<18;i++)
		PutCh(ps2_str[i]);		// Imprimo la patente del vehiculo.
	PutStr("\n");
	PutStr("Patente: ");
	PutStr(patente);			// Imprimo la patente del vehiculo.
	PutStr("\n");
	Imprimir(patente,1);		// Imprimo la patente en codigo de barras.
	Inicializo_Impresion(1,1);
	PutStr("Esta empresa no se responsabiliza por el robo, hurto,\n");
	PutStr("desperfectos y/o daños producidos por cualquier causa\n");
	PutStr("en su automovil, mientras se halle estacionado o en\n");
	PutStr("circulación en el interior de la playa. No se cobra por\n");
	PutStr("la guarda del vehículo sino por el lugar que ocupa.\n");
	Corto_Papel();
	return;
}

