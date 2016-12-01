/*
 * Syntezator.c
 *
 * Created: 2016-11-02 12:12:18
 * Author : Madzia
 */ 

#define F_CPU 16000000												// Ustawienia baudrate
#define BAUD 31														// fosc/(16*baud)-1
#define PI 3.14159265359
#define ROZMIAR 50
#define ZNAK ^

#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned char MIDIsignal[3];
volatile unsigned char nuta, tryb_a, tryb_b, pusty;
int counter = 0;

volatile unsigned int sinus[ROZMIAR], trojkat[ROZMIAR], prostokat[ROZMIAR], pila[ROZMIAR], sindwa[ROZMIAR], sintrzy[ROZMIAR], sincztery[ROZMIAR];

unsigned int sample, indeks_sampla, potencjometr_jeden = 0, potencjometr_dwa = 0, potencjometr_cztery = 0, potencjometr_trzy = 0;
int fine_tuning;

void USARTStart(unsigned int ubr){									// USART init
	
	UBRRH = (unsigned char)(ubr >> 8);
	UBRRL = (unsigned char)ubr;
	UCSRB |= (1 << RXEN);
	UCSRC |=  (1 << URSEL) | (1 << UCSZ1) |(1 << UCSZ0);
	
}

void TIMERStart(void){												// TIMER init
	
	TIMSK = (1 << OCIE1A);
	
}

void ADCInit(void){													// ADC Init
	
	ADMUX = (1 << REFS0) | (1 << ADLAR);
	ADCSRA = (1 << ADEN) | (1 << ADPS2);
	
}

unsigned int ADCpobierz(unsigned char nozka){						// Pobranie stanów ADC
	
	ADMUX &= 0xF0;
	ADMUX |= nozka;
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADCH/6;
	
}


void getMIDI(){												// Pobierz sygna³ MIDI
	
	while (!(UCSRA & (1 << RXC)));
	MIDIsignal[0] = UDR;
		while (!(UCSRA & (1 << RXC)));
	MIDIsignal[1] = UDR;
		while (!(UCSRA & (1 << RXC)));
	MIDIsignal[2] = UDR;
}

void playsample(unsigned int sample){
	PORTB = sample;
	PORTC = (sample >> 8);
}


int main(void)
{																	// MAIN -----------------------------------------
	cli();
	
	PORTD ^= (1 << PORTD7);
	_delay_ms(200);
	PORTD ^= (1 << PORTD7);
	
	
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0b10000000;
	PORTD = (1 << PORTD6) | (1 << PORTD5);
	
	USARTStart(BAUD);
	TIMERStart();
	ADCInit();
	
	PORTD ^= (1 << PORTD7);
	_delay_ms(200);
	PORTD ^= (1 << PORTD7);
	
	for (int i=0; i < ROZMIAR; i++){										// DEKLARACJA SINUSÓW
		double wartosc = (127 + (127 * sin(i*((2*PI)/ROZMIAR))));
		sinus[i] = round(wartosc);
	}
	
	for (int i=0; i < ROZMIAR; i++){
		double wartosc = (127 + (127 * sin(3*i*((2*PI)/ROZMIAR))));
		sindwa[i] = round(wartosc);
	}
	
	for (int i=0; i < ROZMIAR; i++){
		double wartosc = (127 + (127 * sin(5 * i*((2*PI)/ROZMIAR))));
		sintrzy[i] = round(wartosc);
	}
	
	for (int i=0; i < ROZMIAR; i++){
		double wartosc = (127 + (127 * sin(8 * i*((2*PI)/ROZMIAR))));
		sincztery[i] = round(wartosc);
	}
	
	PORTD ^= (1 << PORTD7);
	_delay_ms(200);
	PORTD ^= (1 << PORTD7);
	
	  for (int i=0; i < ROZMIAR; i++){										// DEKLARACJA TRÓJK¥TA
		  if (i>(ROZMIAR/2)){
			  trojkat[i] = round(i*(500/ROZMIAR));
			  }else{
			  trojkat[i] = round((ROZMIAR - i)*(500/ROZMIAR));
		  }
	  }
	  
	  PORTD ^= (1 << PORTD7);
	  _delay_ms(100);
	  PORTD ^= (1 << PORTD7);
	  
	  for (int i=0; i < ROZMIAR; i++){										// DEKLARACJA PROSTOK¥TA
		  if ((i>(ROZMIAR/4)) && (i < (3*ROZMIAR/4))) prostokat[i] = 250;
		  else prostokat[i] = 0;
	  }
	  
	  PORTD ^= (1 << PORTD7);
	  _delay_ms(100);
	  PORTD ^= (1 << PORTD7);
	  
	  for (int i=0; i < ROZMIAR; i++){										// DEKLARACJA PI£Y
		  pila[i] = round(i*(250/ROZMIAR));
	  }
	
	PORTD |= (1 << PORTD7);
	
	sei();
	
	while (1)														// PÊTLA G£ÓWNA ---------------------------------
	{
		
		getMIDI();
		
		potencjometr_jeden = ADCpobierz(0);
		potencjometr_dwa = ADCpobierz(1);
		potencjometr_trzy = ADCpobierz(2);
		potencjometr_cztery = ADCpobierz(3);
		
		fine_tuning = 10-((ADCpobierz(4)/4)-5);						// Fine_tuning
		
		if (!(PIND & 0b01000000)) tryb_a = 1;					// wichajster
		else tryb_a = 0;
		
		if (!(PIND & 0b00100000)) tryb_b = 1;
		else tryb_b = 0;
		
		if ((MIDIsignal[0]) == 144){											// Zapis poprzednich nut
			
			counter++;
			nuta = MIDIsignal[1];
		}
		else {
			counter--;
		}
		
		if (counter){
			
			switch (nuta)
			// POOPISYWANE NUTY -----------------------------
			//		\/
			{
				
				case 12:
				OCR1A = 0x912E + (fine_tuning * 206);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 13:
				OCR1A = 0x8908 + (fine_tuning * 194);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 14:
				OCR1A = 0x8157 + (fine_tuning * 183);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 15:
				OCR1A = 0x7A15 + (fine_tuning * 173);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 16:
				OCR1A = 0x733B + (fine_tuning * 163);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 17:
				OCR1A = 0x6CC3 + (fine_tuning * 154);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 18:
				OCR1A = 0x66A8 + (fine_tuning * 146);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 19:
				OCR1A = 0x60E5 + (fine_tuning * 137);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 20:
				OCR1A = 0x5B75 + (fine_tuning * 130);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 21:
				OCR1A = 0x5653 + (fine_tuning * 122);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 22:
				OCR1A = 0x517B + (fine_tuning * 115);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 23:
				OCR1A = 0x4CE8 + (fine_tuning * 109);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 24:
				OCR1A = 0x4897 + (fine_tuning * 103);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 25:
				OCR1A = 0x4484 + (fine_tuning * 97);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 26:
				OCR1A = 0x40AC + (fine_tuning * 91);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 27:
				OCR1A = 0x3D0A + (fine_tuning * 86);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 28:
				OCR1A = 0x399D + (fine_tuning * 81);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 29:
				OCR1A = 0x3662 + (fine_tuning * 77);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 30:
				OCR1A = 0x3354 + (fine_tuning * 73);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 31:
				OCR1A = 0x3073 + (fine_tuning * 68);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 32:
				OCR1A = 0x2DBB + (fine_tuning * 65);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 33:
				OCR1A = 0x2B2A + (fine_tuning * 61);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 34:
				OCR1A = 0x28BD + (fine_tuning * 57);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 35:
				OCR1A = 0x2674 + (fine_tuning * 54);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 36:
				OCR1A = 0x244C + (fine_tuning * 51);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 37:
				OCR1A = 0x2242 + (fine_tuning * 48);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 38:
				OCR1A = 0x2056 + (fine_tuning * 45);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 39:
				OCR1A = 0x1E85 + (fine_tuning * 43);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 40:
				OCR1A = 0x1CCF + (fine_tuning * 40);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 41:
				OCR1A = 0x1B31 + (fine_tuning * 38);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 42:
				OCR1A = 0x19AA + (fine_tuning * 36);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 43:
				OCR1A = 0x1839 + (fine_tuning * 34);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 44:
				OCR1A = 0x16DD + (fine_tuning * 32);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 45:
				OCR1A = 0x1595 + (fine_tuning * 30);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 46:
				OCR1A = 0x145F + (fine_tuning * 28);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 47:
				OCR1A = 0x133A + (fine_tuning * 27);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 48:
				OCR1A = 0x1226 + (fine_tuning * 25);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 49:
				OCR1A = 0x1121 + (fine_tuning * 24);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 50:
				OCR1A = 0x102B + (fine_tuning * 22);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 51:
				OCR1A = 0x0F43 + (fine_tuning * 21);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 52:
				OCR1A = 0x0E67 + (fine_tuning * 20);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 53:
				OCR1A = 0x0D98 + (fine_tuning * 19);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 54:
				OCR1A = 0x0CD5 + (fine_tuning * 18);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 55:
				OCR1A = 0x0C1D + (fine_tuning * 17);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 56:
				OCR1A = 0x0B6F + (fine_tuning * 16);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 57:
				OCR1A = 0x0ACA + (fine_tuning * 15);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 58:
				OCR1A = 0x0A2F + (fine_tuning * 14);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 59:
				OCR1A = 0x099D + (fine_tuning * 13);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 60:
				OCR1A = 0x0913 + (fine_tuning * 12);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 61:
				OCR1A = 0x0891 + (fine_tuning * 12);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 62:
				OCR1A = 0x0815 + (fine_tuning * 11);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 63:
				OCR1A = 0x07A1 + (fine_tuning * 10);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 64:
				OCR1A = 0x0734 + (fine_tuning * 10);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 65:
				OCR1A = 0x06CC + (fine_tuning * 9);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 66:
				OCR1A = 0x066B + (fine_tuning * 9);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 67:
				OCR1A = 0x060E + (fine_tuning * 8);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 68:
				OCR1A = 0x05B7 + (fine_tuning * 8);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 69:
				OCR1A = 0x0565 + (fine_tuning * 7);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 70:
				OCR1A = 0x0518 + (fine_tuning * 7);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 71:
				OCR1A = 0x04CF + (fine_tuning * 6);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 72:
				OCR1A = 0x0489 + (fine_tuning * 6);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 73:
				OCR1A = 0x0448 + (fine_tuning * 6);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 74:
				OCR1A = 0x040B + (fine_tuning * 5);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 75:
				OCR1A = 0x03D1 + (fine_tuning * 5);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 76:
				OCR1A = 0x039A + (fine_tuning * 5);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 77:
				OCR1A = 0x0366 + (fine_tuning * 4);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 78:
				OCR1A = 0x0335 + (fine_tuning * 4);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 79:
				OCR1A = 0x0307 + (fine_tuning * 4);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 80:
				OCR1A = 0x02DC + (fine_tuning * 4);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 81:
				OCR1A = 0x02B3 + (fine_tuning * 3);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 82:
				OCR1A = 0x028C + (fine_tuning * 3);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 83:
				OCR1A = 0x0267 + (fine_tuning * 3);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 84:
				OCR1A = 0x0245 + (fine_tuning * 3);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 85:
				OCR1A = 0x0224 + (fine_tuning * 3);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 86:
				OCR1A = 0x0205 + (fine_tuning * 2);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 87:
				OCR1A = 0x01E8 + (fine_tuning * 2);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 88:
				OCR1A = 0x01CD + (fine_tuning * 2);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 89:
				OCR1A = 0x01B3 + (fine_tuning * 2);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 90:
				OCR1A = 0x019B + (fine_tuning * 2);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 91:
				OCR1A = 0x0184 + (fine_tuning * 2);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 92:
				OCR1A = 0x016E + (fine_tuning * 2);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 93:
				OCR1A = 0x0159 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 94:
				OCR1A = 0x0146 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 95:
				OCR1A = 0x0134 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 96:
				OCR1A = 0x0122 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 97:
				OCR1A = 0x0112 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 98:
				OCR1A = 0x0103 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 99:
				OCR1A = 0x00F4 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 100:
				OCR1A = 0x00E6 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 101:
				OCR1A = 0x00DA + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 102:
				OCR1A = 0x00CD + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 103:
				OCR1A = 0x00C2 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 104:
				OCR1A = 0x00B7 + (fine_tuning * 1);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 105:
				OCR1A = 0x00AD + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 106:
				OCR1A = 0x00A3 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 107:
				OCR1A = 0x009A + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 108:
				OCR1A = 0x0091 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 109:
				OCR1A = 0x0089 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 110:
				OCR1A = 0x0081 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 111:
				OCR1A = 0x007A + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 112:
				OCR1A = 0x0073 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 113:
				OCR1A = 0x006D + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 114:
				OCR1A = 0x0067 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 115:
				OCR1A = 0x0061 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 116:
				OCR1A = 0x005B + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 117:
				OCR1A = 0x0056 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 118:
				OCR1A = 0x0051 + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				case 119:
				OCR1A = 0x004D + (fine_tuning * 0);
				TCCR1B = (1 << WGM12) | (1 << CS10);
				break;
				
			}
			}	else {TCCR1B = 0x00;}										// CISZA
		}																// ----------------------------------------------


	}

	ISR(TIMER1_COMPA_vect){

		if (indeks_sampla == ROZMIAR) indeks_sampla = 0;

		if (!(tryb_a | tryb_b))													// WYBÓR TRYBU PRACY
			sample = (potencjometr_jeden * sinus[indeks_sampla]) + (potencjometr_dwa * sindwa[indeks_sampla]) + (potencjometr_trzy * sintrzy[indeks_sampla]) + (potencjometr_cztery * sincztery[indeks_sampla]);

		if (tryb_a)
			sample = (potencjometr_dwa * trojkat[indeks_sampla]) + (potencjometr_trzy * pila[indeks_sampla]) + (potencjometr_jeden * sinus[indeks_sampla]) + (potencjometr_cztery * prostokat[indeks_sampla]);

		if (tryb_b)
			sample = (potencjometr_dwa * trojkat[indeks_sampla]) ZNAK (potencjometr_trzy * pila[indeks_sampla]) ZNAK (potencjometr_jeden * sinus[indeks_sampla]) ZNAK (potencjometr_cztery * prostokat[indeks_sampla]);
		
		playsample(sample);
		indeks_sampla++;


	}



