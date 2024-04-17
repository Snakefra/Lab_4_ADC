/*****************************************************************************
 * Universidad del Valle de Guatemala
 * IE2023: Programación de microcontroladores
 * Autor: Oscar Melchor
 * Proyecto: Sumador de 8 bits
 * Archivo: main.c
 * Hardware : ATMEGA328P
 * Created: 10/04/2024 08:34:02
 *****************************************************************************/ 
#define F_CPU 16000000 //Frecuencia a 16 MHz

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t Contador = 0; //Contador de 8 bits
uint8_t Display = 0; //Valor bit justificado 
uint8_t DisplayL = 0; //Display unidades
uint8_t DisplayH = 0; //Display decenas


void setup(void);

void initADC(void);

int Numeros[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};

//****************************************************************************

int main(void){ //Función principal
	
	setup();
	initADC();
	while (1){	//Loop infinito
	// Iniciando secuencia de ADC
	ADCSRA |= (1<< ADSC);
	_delay_ms(7);
	PORTB = 0b0001;
	PORTD = Contador; //Toogle Puerto D
	_delay_ms(7);
	PORTB = 0b0010;
	PORTD = Numeros[DisplayH];
	_delay_ms(7);
	PORTB = 0b0100;
	PORTD = Numeros[DisplayL];
	if (Display > Contador){
		PORTC |= (1<<PINC2);
	}
	else
	{
		PORTC &= ~(1<<PINC2);
	}
	}	
}
//****************************************************************************
void setup(void){ //Función setup
	
	cli();	//Deshabilita interrupciones
	
	UCSR0B = 0;	//Deshabilita módulo comunicación serial
	
	// Output Puerto D
	DDRD = 0xFF;
	PORTD = 0;
	DDRB = 0x07;
	PORTB = 0;
	DDRC |= (1<<PINC2);
	PORTC = 0; 
	
	// Inputs PC0 y PC1
	DDRC &= ~(1 << PINC0) | ~(1 << PINC1);
	PORTC |= (1 << PINC0) | (1 << PINC1);
	
	// Configuración de interrupciones
	PCICR |= (1<<PCIE1); // Habilita interrupciones en el puerto C
	PCMSK1 |= ((1<<PCINT8)|(1<<PCINT9)); // Habilita la interrupciones en PC0 y PC1
	
	sei(); // Habilita las interrupciones 
}
 //****************************************************************************
ISR(PCINT1_vect){ // Interrupciones de pines
	
	_delay_ms(10);	//Delay antirebote
	
	if (!(PINC & (1 << PINC0))) { // Botón incremento
		Contador++;
		}
	else if (!(PINC & (1 << PINC1))) { // Botón decremento
		Contador--;
	}
	else if((!(PINC & (1 << PINC0))) && (Contador==255))  { // Overflow
		Contador = 0;
	}
	else if((!(PINC & (1 << PINC1))) && (Contador==255))  { // Underflow
		Contador = 254;
	}
	else { // Mantener valor
		Contador = Contador;
	}
}
//****************************************************************************

void initADC(void){
	ADMUX = 0;
	// Vref = AVCC = 5V
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	// Justificando hacia la izquierda
	ADMUX |= (1<<ADLAR);
	// Habilitamos canal ADC7
	ADMUX |= (1<<MUX2) | (1<<MUX1) | (1<<MUX0);

	ADCSRA = 0;
	// Encendiendo ADC
	ADCSRA |= (1<<ADEN);
	//Habilitamos ISR ADC
	ADCSRA |= (1<<ADIE);

	//Prescaler de 128 > 16M / 128 = 125KHz
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}
//****************************************************************************
ISR(ADC_vect){
	Display = ADCH;
	ADCSRA |= (1<< ADIF);
	DisplayH = (Display>>4);
	DisplayL = Display & 0x0F;
}