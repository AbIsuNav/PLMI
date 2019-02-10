/**
 * Solar Cell Model using PLMI
 * Kennet Garcia, Abegiba Isunza
 *
 */
#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <asf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <avr/pgmspace.h>
#include "PLMI.h"
#include "PLMI.c"

#define DEB_LED 4
#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1
#define VREF 25
#define ICRVALUE 1600

void usartInit(void);
void sendUSART_char( uint8_t data );
void sendUSART_line(uint8_t* data, uint8_t lenght);
void stringToFloat(unsigned char flag);

void timerInit(void);
void modifyPWM(void);

void InitADC(void);
uint16_t ReadADC(uint8_t ADCchannel);

void reverse(char *str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char *res, int afterpoint);
void SendDataChar(char dato[], int size);
void SendFloat(float value, char *text, int decPos, int length);

char sep[] = "\n";
uint8_t Ttext[] = "00000000\n";
uint8_t Gtext[] = "00000000\n";
float testValues[3]={25,1000,7};
unsigned char tipo=0, countText=0;

float maxValueI=3.5;
float maxValueP=50.1;
float adcVoltage;


int main(void){

	//Initializa ports
	DDRB|=(1<<DEB_LED);
	usartInit();
	timerInit();
	InitADC();
	sei();
	
	while(1){
		
		//Read ADC value
		adcVoltage=(float) ReadADC(0);
		testValues[2]= (adcVoltage/1024)*VREF;
		//Run linInterp
		interpValue= linInterp(testValues);
		//Modify PWM
		modifyPWM();
	}
	return 0;
}

//Interruptions
ISR(USART_RX_vect){
	uint8_t recibido;
	recibido = UDR0;
	if(recibido=='T'){
		tipo=0; //Text for T
		//sendUSART_line(tON, sizeof(tON));
	}else if(recibido=='G'){
		tipo=1; //Text for G
	}else if(recibido=='='){
		//Start receiving text
		PORTB&=(0<<DEB_LED);
		countText=0;
	}else if(recibido==','){
		//End transmission
		if(tipo==1){
			PORTB|=(1<<DEB_LED);
			sendUSART_line(Ttext, sizeof(Ttext));
			stringToFloat(0);
			_delay_ms(10);
			sendUSART_line(Gtext, sizeof(Gtext));
			stringToFloat(1);
		}
	}else{
		if(tipo==0){
			Ttext[countText]=recibido;
			countText++;
		}else{
			Gtext[countText]=recibido;
			countText++;
		}
	}
}

void stringToFloat(unsigned char flag){
	float auxVal[9]={1000,100,10,1,0,0.1, 0.01, 0.001, 0.0001};
	float Taux=0, Gaux=0; 
	for(unsigned char i=0; i<9; i++){
		if(flag==0){
			Taux=Taux+(Ttext[i]-0X30)*auxVal[i];
		}else if(flag==1){
			Gaux=Gaux+(Gtext[i]-0X30)*auxVal[i];
		}
	}
	if (flag==0) testValues[0]=Taux;
	if (flag==1) testValues[1]=Gaux;
}

//Methods for ADC
void InitADC(void)
{
	// Select Vref=AVcc
	ADMUX |= (1<<REFS0);
	//set prescaller to 128 and enable ADC
	ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);
}

uint16_t ReadADC(uint8_t ADCchannel)
{
	//select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (ADCchannel & 0x0F);
	//single conversion mode
	ADCSRA |= (1<<ADSC);
	// wait until ADC conversion is complete
	while( ADCSRA & (1<<ADSC) );
	return ADC;
}


//Merhod for PWM
void timerInit(void){
	
	DDRB |= (1 << DDB1)|(1 << DDB2);
	// PB1 and PB2 is now an output

	ICR1 = ICRVALUE;
	// set TOP to 16bit

	OCR1A = 0x3FFF;
	// set PWM for 25% duty cycle @ 16bit

	OCR1B = 0xBFFF;
	// set PWM for 75% duty cycle @ 16bit

	TCCR1A |= (1 << COM1A1)|(1 << COM1B1);
	// set none-inverting mode

	TCCR1A |= (1 << WGM11);
	TCCR1B |= (1 << WGM12)|(1 << WGM13);
	// set Fast PWM mode using ICR1 as TOP
	    
	TCCR1B |= (1 << CS10);
	// START the timer with no prescaler
}

void modifyPWM(void){
	//Modify duty cycle according to interpValue
	int dutyC=round((interpValue*ICRVALUE*1.1)/5);
	OCR1A=dutyC;
	dutyC=round(((interpValue*testValues[2])/maxValueP)*ICRVALUE);
	OCR1B=dutyC;
}

//Methods for USART
void usartInit(void){
	UBRR0H=0;
	UBRR0L=MYUBRR; //9600 bauds
	UCSR0B = (1<<TXEN0)|(1<<RXEN0)|(1<<RXCIE0); //activamos  transmision, recepcion e interrupcio
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00); //8bits
}

void sendUSART_line(uint8_t* data, uint8_t lenght){
	for (int i = 0; i < lenght-1; i++)
	{
		sendUSART_char(*data++);
	}
}

void sendUSART_char( uint8_t data )
{
	while ( !( UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void SendDataChar(char dato[], int size){
	for(int i=0;i<size-1;i++){
		while (!(UCSR0A & (1<<UDRE0)));
		UDR0 = dato[i];
	}
	//sendUSART_char('\');
	//sendUSART_char('n');
}

void SendFloat(float value, char *text, int decPos, int length){
	ftoa(value, text, decPos);
	SendDataChar(text,length);
	_delay_ms(5);
	SendDataChar(sep,2);
}

 // Converts a given integer x to string str[].  d is the number
 // of digits required in output. If d is more than the number
 // of digits in x, then 0s are added at the beginning.
 
 // reverses a string 'str' of length 'len'
 void reverse(char *str, int len)
 {
	 int i=0, j=len-1, temp;
	 while (i<j)
	 {
		 temp = str[i];
		 str[i] = str[j];
		 str[j] = temp;
		 i++; j--;
	 }
 }
 
 int intToStr(int x, char str[], int d)
 {
	 int i = 0;
	 while (x)
	 {
		 str[i++] = (x%10) + '0';
		 x = x/10;
	 }
	 
	 // If number of digits required is more, then
	 // add 0s at the beginning
	 while (i < d)
	 str[i++] = '0';
	 
	 reverse(str, i);
	 str[i] = '\0';
	 return i;
 }
 
// Converts a floating point number to string.
void ftoa(float n, char *res, int afterpoint)
{
	// Extract integer part
	int ipart = (int)n;
	
	// Extract floating part
	float fpart = n - (float)ipart;
	
	// convert integer part to string
	int i = intToStr(ipart, res, 0);
	
	// check for display option after point
	if (afterpoint != 0)
	{
		res[i] = '.';  // add dot
		
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		fpart = fpart * pow(10, afterpoint);
		
		intToStr((int)fpart, res + i + 1, afterpoint);
	}
}

