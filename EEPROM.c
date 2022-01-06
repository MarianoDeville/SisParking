/****************************************************************************/
/*				MANEJO DE LA MEMÓRIA EEPROM DEL PIC							*/
/*..........................................................................*/
/*				PIC:				PIC18F4620								*/
/*				Compilador:			MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.	*/
/*				Autor:				Mariano Ariel Deville					*/
/****************************************************************************/
/*						PROTOTIPOS DE FUNCIONES								*/
/*..........................................................................*/
unsigned char Eeprom_Read(unsigned int address);
void Eeprom_Write(unsigned int address,unsigned char value);
unsigned char Check_Pass(unsigned char pass[],unsigned char orden);
void Almacenar_Pass(const char *pass,unsigned char orden);
void Vaciar_Memoria(void);
void Borrar_Memoria(void);
void Caja_Write(unsigned int valor_caja);
unsigned int Caja_Read(void);
void Caja_Borrar(void);
void Caja_Cerrar(void);
void Guardo_Estadisticas(unsigned int a);
void Recupero_Estadisticas(unsigned char a,unsigned char cadena[]);
void Almacenar_Telefono(const unsigned char *numero);
void Borrar_Pass(void);
/****************************************************************************/
/*			ORGANIZACIÓN DE LA MEMRIA Y DATOS ALMACENADOS.					*/
/*..........................................................................*/
/*		0	- 19		Contraseña del root.								*/
/*		20	- 39		Contraseña del administrador.						*/
/*		40	- 49		Reservado.											*/
/*		50	- 64		Almacenamiento del teléfono.						*/
/*		65	- 99		Reservado.											*/
/*		80	- 107		Estadísticas del turno actual.						*/
/*		108	- 108		Bandera de apertura de caja.						*/
/*		109	- 113		Almaceno la hora de apertura de caja.				*/
/*		114	- 149		Reservado.											*/
/*		150	- 153		Bandas horarias para el cobro.						*/
/*		153	- 159		Reservado.											*/
/*		160	- 177		Almacenamiento de las distintas tarifas.			*/
/*		178	- 199		Reservado.											*/
/*		200	- 204		Promociones, horarios y valores.					*/
/*		205	- 219		Reservado.											*/
/*		220	- 222		Tolerancias.										*/
/*		223	- 229		Reservado.											*/
/*		230	- 232		Valores de las estadías.							*/
/*		233	- 239		Reservado.											*/
/*		240	- 241		Guardo la caja actual.								*/
/*		242	- 249		Reservado.											*/
/*		250	- 1023		Cierres de cajas almacenados.						*/
/****************************************************************************/
/*		DEVUELVO EL BYTE LEIDO EN LA DIRECCIÓN PASADA COMO ARGUMENTO		*/
/*..........................................................................*/
unsigned char Eeprom_Read(unsigned int address)
{
	CLRWDT();
 	while(WR||RD)				// Si hay una escritura o lectura en curso espero.
		continue;				// Si demora mucho actua el WDT.
	FREE=0;
	EEADR=address & 0xFF;		// Paso la dirección en la que voy a leer.
	EEADRH=(address>>8) & 0xFF;	// Paso la dirección en la que voy a leer.
	EEPGD=0;					// Point to EE memory
	CFGS=0;
	RD=1;						// Inicio el ciclo de lectura.
	while(RD)					// Espero a que termine la lectura.
		continue;				// Si demora mucho actua el WDT.
	return EEDATA;				// Devuelvo el valor leido.
}
/****************************************************************************/
/*				ESCRIBO UN BYTE EN LA MEMORIA EEPROM						*/
/*..........................................................................*/
void Eeprom_Write(unsigned int address,unsigned char value)
{
	CLRWDT();
 	while(WR||RD)				// Si hay una escritura o lectura en curso espero.
		continue;				// Si demora mucho actua el WDT.
	FREE=0;
	EEADR=address & 0xFF;		// Paso la dirección en la que voy a leer.
	EEADRH=(address>>8) & 0xFF;	// Paso la dirección en la que voy a leer.
	EEDATA=value; 				// Paso el byte que deseo escribir.
	EEPGD=0;
	CFGS=0;
 	WREN=1;						// EEPROM modo escritura.
	EECON2=0x55;
	EECON2=0xAA;
	WR=1;						// Escribo en memoria.
 	while(WR)					// Espero que termine de escribir.
		continue;				// Si demora mucho actua el WDT.
	WREN=0;						// Deshabilito EEPROM modo escritura.
	return;
}
/****************************************************************************/
/*			Comparo la contraseña del ROOT o del ADMIN con las guardadas.	*/
/*..........................................................................*/
unsigned char Check_Pass(unsigned char pass[],unsigned char orden)
{
	unsigned char dato;
	unsigned int i;
	i=(orden*20)&0xFF;
	orden=0;
	dato=Eeprom_Read(i);
	i++;
	if(dato!=35)				// Hay contraseña guardada?
		return 2;
	do
	{
		dato=Eeprom_Read(i);
		i++;
		if(pass[orden++]!=dato)
			return 0;
	}while(dato!=0);			// Recorro la memoria hasta encontrar un NULL.
	return 1;					// Devuelvo el valor de memoria en donde termina la cadena.
}
/****************************************************************************/
/*			Almaceno la contraseña del ROOT o del ADMIN.					*/
/*..........................................................................*/
void Almacenar_Pass(const char *pass,unsigned char orden)
{
	unsigned int i;
	i=orden*20;
	Eeprom_Write(i,35);			// Marco con un asterisco el comienzo.
	i++;
	do
	{
		Eeprom_Write(i,*pass);
		i++;
	}while(*pass++);			//
	return;						// Devuelvo la dirección final de la cadena.
}
/****************************************************************************/
/*			MANDO TODO EL CONTENIDO DE LA MEMORIA							*/
/*..........................................................................*/
void Vaciar_Memoria(void)
{
	volatile unsigned int i;
	unsigned char dato;
	for(i=0;i<1024;i++)			// Recorro los 256 bytes de memoria.
	{
		dato=Eeprom_Read(i);	// Leo el contenido de la memoria.
		PutCh(dato);			// Mando por puerto serie el caracter leido.
	}
	return;
}
/****************************************************************************/
/*					ESCRIBO EN TODA LA MEMORIA NULL							*/
/*..........................................................................*/
void Borrar_Memoria(void)
{
	volatile unsigned int i,porcent,comparar;
	unsigned char cadena[3];
	Imprimir_Lcd("Borrando EEPROM","   %",1);
	for(i=0;i<1024;i++)			// Recorro los 1024 bytes de memoria.
	{
		Eeprom_Write(i,0);		// Escribo un cero en la memoria.
		porcent=(i*10)/103;
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
/****************************************************************************/
/*			Guardo el valor de la caja actual.								*/
/*..........................................................................*/
void Caja_Write(unsigned int valor_caja)
{
	unsigned char valor_b,valor_a;
	valor_b=valor_caja&0xFF;
	valor_a=(valor_caja>>8)&0xFF;
	Eeprom_Write(240,valor_a);		// Escribo en memoria la parte alta.
	Eeprom_Write(241,valor_b);		// Escribo en memoria la parte baja.
	return;
}
/****************************************************************************/
/*			Leo el valor de la caja actual.									*/
/*..........................................................................*/
unsigned int Caja_Read(void)
{
	unsigned char valor_b,valor_a;
	valor_a=Eeprom_Read(240);		// Leo la parte alta.
	valor_b=Eeprom_Read(241);		// Leo la parte baja.
	return((valor_a<<8)|valor_b);
}
/****************************************************************************/
/*			Borro los cierres de caja almacenados en memoria.				*/
/*..........................................................................*/
void Caja_Borrar(void)
{
	volatile unsigned int i;
	for(i=250;i<1024;i++)			// Recorro los bytes de memoria superior.
		Eeprom_Write(i,0);			// Escribo un cero en la memoria.
	return;
}
/****************************************************************************/
/*			Guardo las estadísticas de la caja actual en memoria.			*/
/*..........................................................................*/
void Guardo_Estadisticas(unsigned int a)
{
	unsigned char valor_a,valor_b;
	unsigned char temp;
	a=a*2;							// Establesco el paso.
	valor_a=Eeprom_Read(a+80);
	valor_b=Eeprom_Read(a+81);
	temp=(valor_a<<8)|valor_b;		// Armo el entero con los dos char de la memoria.
	temp++;							// Incremento el contador.
	valor_b=temp&0xFF;				// Obtengo la parte baja del entero.
	valor_a=(temp>>8)&0xFF;			// Obtengo la parte alta del entero.
	Eeprom_Write(a+80,valor_a);
	Eeprom_Write(a+81,valor_b);
	return;
}
/****************************************************************************/
/*		Leo las estadísticas de la caja actual en memoria.					*/
/*..........................................................................*/
void Recupero_Estadisticas(unsigned char a,unsigned char cadena[])
{
	unsigned char valor_a,valor_b;
	unsigned char i;
	CLRWDT();
	a=a*2;							// Establesco el paso.
	valor_a=Eeprom_Read(a+80);
	valor_b=Eeprom_Read(a+81);
	i=(valor_a<<8)|valor_b;
	IntToStr(i*10,cadena);
	return;
}
/****************************************************************************/
/*			Guardo el número de teléfono del administrador remoto.			*/
/*..........................................................................*/
void Almacenar_Telefono(const unsigned char *numero)
{
	unsigned char i,temp;
	CLRWDT();
	i=50;
	while(*numero)
		Eeprom_Write(i++,*numero++);
	return;
}
/****************************************************************************/
/*			Borro el teléfono del administrador remoto.						*/
/*..........................................................................*/
void Borrar_Telefono(void)
{
	volatile unsigned char i;
	for(i=50;i<65;i++)
		Eeprom_Write(i,0);
	return;
}
/****************************************************************************/
/*			Borro la contraseña del administrador.							*/
/*..........................................................................*/
void Borrar_Pass(void)
{
	volatile unsigned char i;
	for(i=20;i<40;i++)
		Eeprom_Write(i,0);
	return;
}
