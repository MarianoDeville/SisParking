/********************************************************************************/
/*					MANEJO DE DISPLAY CON INTERFACE DE 4 BITS.					*/
/*..............................................................................*/
/*			Revisión:				1.00										*/
/*			PIC:					PIC18F4620									*/
/*			Display:				LCD 20 caracteres por 2 lineas.				*/
/*			Compilador:				MPLAB IDE 8.7 - HI-TECH PIC18 9.50 PL3.		*/
/*			Autor:					Mariano Ariel Deville						*/
/********************************************************************************/
/*						PROTOTIPO DE FUNCIONES									*/
/*..............................................................................*/
void Lcd_Write(unsigned char);
void Lcd_Clear(void);
void Lcd_Puts(const char *s);
void Lcd_Goto(unsigned char pos);
void Lcd_Setup(void);
void Lcd_Putch(char c);
void Imprimir_Lcd(const char *l_1,const char *l_2,unsigned char borrar);
void Linea_Lcd(const char *linea,unsigned char borrar);

#define	LCD_EN		RA1				// LCD E clock
#define	LCD_RS		RA0				// LCD register 
#define	LCD_STROBE	(LCD_EN=1);(LCD_EN=0)

/********************************************************************************/
/*			CONFIGURACION DEL LCD EN MODO 4 BITS.								*/
/*------------------------------------------------------------------------------*/
void Lcd_Setup(void)
{
	DelayMs(15);
	LCD_RS=0;				// write control bytes
	PORTA &=0xC3;			// Limpio el puerto.
	PORTA |=0B00001100;		// Inicializo.
	LCD_STROBE;
	DelayMs(5);
	LCD_STROBE;
	DelayUs(100);
	PORTA &=0xC3;			// Limpio el puerto.
	PORTA |=0B00001000;
	LCD_STROBE;
	DelayUs(40);
	Lcd_Write(0x28);		// 4 bit mode, 1/16 duty, 5x8 font
	Lcd_Write(0b00001100);	// configuro la visualización.
	Lcd_Write(0x1);			// Limpio el LCD.
	return;
}
/********************************************************************************/
/*			ESCRIBE UN BYTE EN EL DISPLAY.										*/
/*------------------------------------------------------------------------------*/
void Lcd_Write(unsigned char c)
{
	CLRWDT();
	if(c==241||c==209)		// Corrección para que imprima la ñ.
		c=238;
	PORTA &=0xC3;
	PORTA |=(c>>2)&0x3C;
	LCD_STROBE;
	PORTA &=0xC3;
	PORTA |=(c<<2)&0x3C;
	LCD_STROBE;
	DelayMs(1);
	return;
}
/********************************************************************************/
/*			LIMPIO EL LCD.														*/
/*------------------------------------------------------------------------------*/
void Lcd_Clear(void)
{
	LCD_RS=0;
	Lcd_Write(0x1);
	return;
}
/********************************************************************************/
/*			VOY A UNA POSICION DEL LCD.											*/
/*------------------------------------------------------------------------------*/
void Lcd_Goto(unsigned char pos)
{
	LCD_RS=0;
	Lcd_Write(0x80+pos);
	return;
}
/********************************************************************************/
/*			ESCRIBO UN CARACTER EN EL DISPLAY.									*/
/*------------------------------------------------------------------------------*/
void Lcd_Putch(char c)
{
	LCD_RS=1;				// write characters
	CLRWDT();
	Lcd_Write(c);
	return;
}
/********************************************************************************/
/*			ESCRIBO UNA CADENA DE CARACTERES EM EL DISPLAY.						*/
/*------------------------------------------------------------------------------*/
void Lcd_Puts(const char *s)
{
	unsigned char largo=0;
	LCD_RS=1;				// write characters
	CLRWDT();
	while(*s)
	{
		Lcd_Write(*s++);
		if(largo++>30)
			return;
	}
	return;
}
/********************************************************************************/
/*					*/
/*------------------------------------------------------------------------------*/
void Imprimir_Lcd(const char *l_1,const char *l_2,unsigned char borrar)
{
	if(borrar)
		Lcd_Clear();
	Lcd_Goto(0x00);
	Lcd_Puts(l_1);
	Lcd_Goto(0x40);
	Lcd_Puts(l_2);
	return;
}
/********************************************************************************/
/*					*/
/*------------------------------------------------------------------------------*/
void Linea_Lcd(const char *linea,unsigned char borrar)
{
	unsigned char largo=0;
	if(borrar)
	{
		Lcd_Clear();
		Lcd_Goto(0x00);
	}
	while(*linea)
	{
		Lcd_Putch(*linea++);
		largo++;
		if(largo==20)
			Lcd_Goto(0x40);
		if(largo>40)
			return;
	}
	return;
}
