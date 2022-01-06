/********************************************************************************/
/*				MANEJO DEL RELOJ DE TIEMPO REAL DS1307							*/
/*..............................................................................*/
/*			Revisión:				1.00										*/
/*			PIC:					PIC18F4620									*/
/*			RTC:					DS1307										*/
/*			Compilador:				MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.		*/
/*			Autor:					Mariano Ariel Deville						*/
/********************************************************************************/
/*						PROTOTIPO DE FUNCIONES									*/
/*..............................................................................*/
void DS1307_Setup(void);
void Seteo_Hora(const char *hora);
void Obtener_Hora(unsigned char *hora, unsigned char formato);
/************************************************************************/
/*				*/
/*----------------------------------------------------------------------*/
void DS1307_Setup(void)
{
	I2C_Start();			// Inicio comunicación I2C.
	I2C_Write(0b11010000);	// Selecciono dispositivo.
	I2C_Write(0x07);		// Dirección del registro CONTROL.
	I2C_Write(0b00);		// Configuro el registro.
	I2C_Stop();				// Termino con la comunicación I2C.
	return;
}
/************************************************************************/
/*				*/
/*----------------------------------------------------------------------*/
void Seteo_Hora(const char *hora)
{
	unsigned char min,hor,dia,mes,ano,aux;
	aux=(*hora++)-48;
	dia=(aux<<4)|((*hora++)-48);
	aux=(*hora++)-48;
	mes=(aux<<4)|((*hora++)-48);
	aux=*hora++-48;
	ano=(aux<<4)|((*hora++)-48);
	aux=*hora++-48;
	hor=(aux<<4)|((*hora++)-48);
	aux=*hora++-48;
	min=(aux<<4)|((*hora++)-48);
	I2C_Start();				// Inicio comunicación I2C.
	I2C_Write(0b11010000);		// Direccion del DS1307, modo write.
	I2C_Write(0x00);			// Direccion de los segundos.
	I2C_Write(0);				// Segundos.
	I2C_Write(min);				// Minutos.
	I2C_Write(hor);				// Horas.
	I2C_Write(0);				//
	I2C_Write(dia);				// Fecha.
	I2C_Write(mes);				// Mes.
	I2C_Write(ano);				// Año.
	I2C_Stop();					// Termino con la comunicación I2C.
	return;
}
/************************************************************************/
/*	LEO LA HORA Y FECHA DEL RTC Y LO COMVIERTO EN UNA CADENA DE ASCII.	*/
/*----------------------------------------------------------------------*/
void Obtener_Hora(unsigned char *hora, unsigned char formato)
{
	unsigned char seg,min,hor,dia,mes,ano;
	unsigned char aux;
	I2C_Start();
	I2C_Write(0b11010000);	// Selecciono el dispositivo.
	I2C_Write(0x0);
	I2C_Stop();
	I2C_Start();
	I2C_Write(0b11010001);	// Selecciono el dispositivo.
	seg=I2C_Read(1);		// Leo los segundos.
	min=I2C_Read(1);		// Leo los minutos.
	hor=I2C_Read(1);		// Leo la hora.
	I2C_Read(1);			// Leo el día.
	dia=I2C_Read(1);		// Leo la fecha.
	mes=I2C_Read(1);		// Leo el mes.
	ano=I2C_Read(0);		// Leo el año.
	I2C_Stop();
	aux=dia&0xf;
	dia=(dia>>4)&0x3;
	*hora++=dia+48;			// Día.
	*hora++=aux+48;
	if(formato)
		*hora++='/';
	aux=mes&0xf;
	mes=(mes>>4)&0x3;
	*hora++=mes+48;			// Mes.
	*hora++=aux+48;
	if(formato)
		*hora++='/';
	aux=ano&0xf;
	ano=(ano>>4)&0xf;
	*hora++=ano+48;			// Año.
	*hora++=aux+48;
	if(formato)
		*hora++=' ';
	aux=hor&0xf;
	hor=(hor>>4)&0x3;
	*hora++=hor+48;			// Horas.
	*hora++=aux+48;
	if(formato)
		*hora++=':';
	aux=min&0xf;
	min=(min>>4)&0xf;
	*hora++=min+48;			// Minutos.
	*hora++=aux+48;
	if(formato)
		*hora++=':';
	aux=seg&0xf;			// Copio la parte baja.
	seg=(seg>>4)&0xf;
	*hora++=seg+48;			// Segundos.
	*hora++=aux+48;
	*hora=0;				// NULL al final de la cadena.
	return;
}


