/********************************************************************************/
/*				UTILIZACION DEL MODULO I2C DEL PIC 18F4620						*/
/*..............................................................................*/
/*			Revisión:				1.00										*/
/*			PIC:					PIC18F4620									*/
/*			Compilador:				MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.		*/
/*			Autor:					Mariano Ariel Deville						*/
/********************************************************************************/
/*						PROTOTIPO DE FUNCIONES									*/
/*------------------------------------------------------------------------------*/
void I2C_Setup(void);
void I2C_Wait_Idle(void);
void I2C_Start(void);
void I2C_RepStart(void);
void I2C_Stop(void);
unsigned char I2C_Read(unsigned char ack);
unsigned char I2C_Write(unsigned char i2cWriteData);
/********************************************************************************/
/*				CONFIGURACION E INICIALIZACION DEL MODULO						*/
/*------------------------------------------------------------------------------*/
void I2C_Setup(void)
{
	CLRWDT();
	TRISC3=1;			// Set SCL and SDA pins as inputs.
	TRISC4=1;
	SMP=1;
	SSPCON1=0x38;		// Set I2C master mode.
	SSPCON2=0x00;
	SSPADD=48;			// clock=Osc/(4*(sspad+1)).
	GCEN=0;
	CKE=1;				// Use I2C levels worked also with '0'.
	SMP=1;				// Disable slew rate control  worked also with '0'.
	PSPIF=0;			// Clear SSPIF interrupt flag.
	BCLIF=0;			// Clear bus collision flag.
}
/********************************************************************************/
/*						*/
/*------------------------------------------------------------------------------*/
void I2C_Wait_Idle(void)
{
	while((SSPCON2&0x1F)|RW)
		CLRWDT();
}
/********************************************************************************/
/*						COMIENZO LA TRANSMISION									*/
/*------------------------------------------------------------------------------*/
void I2C_Start(void)
{
	I2C_Wait_Idle();
	SEN=1;
}
/********************************************************************************/
/*						REINICIALIZO LA TRANSMISION								*/
/*------------------------------------------------------------------------------*/
void I2C_RepStart(void)
{
	I2C_Wait_Idle();
	RSEN=1;
}
/********************************************************************************/
/*						TERMINO LA TRANSMISION									*/
/*------------------------------------------------------------------------------*/
void I2C_Stop(void)
{
	I2C_Wait_Idle();
	PEN=1;
}
/********************************************************************************/
/*						LEO UN CARACTER 										*/
/*------------------------------------------------------------------------------*/
unsigned char I2C_Read(unsigned char ack)
{
	unsigned char i2cReadData;
	I2C_Wait_Idle();
	RCEN=1;
	I2C_Wait_Idle();
	i2cReadData=SSPBUF;
	I2C_Wait_Idle();
	if(ack)
		ACKDT=0;
	else
		ACKDT=1;
	ACKEN=1;               // send acknowledge sequence
	return(i2cReadData);
}
/********************************************************************************/
/*						ESCRIBO UN CARACTER										*/
/*------------------------------------------------------------------------------*/
unsigned char I2C_Write(unsigned char i2cWriteData)
{
	I2C_Wait_Idle();
	SSPBUF=i2cWriteData;
	return(!ACKSTAT);		// function returns '1' if transmission is acknowledged.
}
