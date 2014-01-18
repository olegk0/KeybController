
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/wdt.h>

#include "usart.h"
#include "subs.h"
#include "vars.h"

#include "ps2.c"

void delay_ms0(unsigned char ms)
{
	while(ms>0)
	{
		delay(1000);
		ms--;
	}
}

void delay_ms(unsigned int ms)
{
	while(ms>0)
	{
		delay(1000);
		ms--;
	}
//	wdt_reset();
}

//----------------------------------------------

ISR (TIMER1_OVF_vect)
{
	TCNT1 = 0;
	TCCR1B=0;
}



void SwLEDs(void)
{
	if(LedFl & KEYBOARD_CAPS_LOCK_ON)
		PORTD &= ~16;
	else
		PORTD |= 16;
		
	if(LedFl & KEYBOARD_NUM_LOCK_ON)
		PORTD &= ~32;
	else
		PORTD |= 32;
}

uint8_t WaitUdata(void)
{
	uint8_t tmp;
		tmp = RXCMDTIMEOUT;
		while(tmp){
			if(USART_GetRxCount())
				break;
			tmp--;
			delay_ms0(1);
		}
		return(tmp);
}

uint8_t rxUdata(void)
{
	uint8_t rx, tmp, ret;

	rx = USART_GetChar();
	ret = ACKNOWLEDGE;
	switch(rx){
	case SET_KEYBOARD_INDICATORS:
		USART_PutChar(ACKNOWLEDGE);
		if(!WaitUdata()) return(1);
		tmp = USART_GetChar();
		LedFl = tmp;
		break;
	case SET_KEYBOARD_TYPEMATIC:
		USART_PutChar(ACKNOWLEDGE);
		if(!WaitUdata()) return(1);
		tmp = USART_GetChar();
		KeyDelay = (1+((tmp>>5) & 3))*250;
		if(KeyDelay<MINKEYDELAY || KeyDelay>MAXKEYDELAY){
			KeyDelay = DEFKEYDELAY;
			ret = FAILURE;
		}
		KeyPeriod = ((8+(tmp&7))*(1<<((tmp>>3)&3)))<<2;
		if(KeyPeriod<MINKEYPER || KeyPeriod>MAXKEYPER){
			KeyPeriod = DEFKEYPER;
			ret = FAILURE;
		}
		break;
	case KEYBOARD_RESET:
		USART_PutChar(ACKNOWLEDGE);
		wdt_enable(WDTO_120MS);
		while(1);
		break;
	case PS2_CMD:
//		USART_PutChar(ACKNOWLEDGE);
		if(!WaitUdata()) return(1);
		tmp = USART_GetChar();
		if(!ps2_send_byte(tmp))
			ret = FAILURE;
		else
			ret = 0;
		break;
/*
	case SELECT_SCAN_CODE_SET:
		USART_PutChar(ACKNOWLEDGE);
		
		USART_GetChar();
		break;
	case READ_KEYBOARD_ID:
*/
	default:
		ret = FAILURE;
	}
	if(ret) USART_PutChar(ret);
	return(ret);
}
void SendKeyData(unsigned char dt)
{
		USART_PutChar(KEY_CMD);
		USART_PutChar(dt);
}

void SendLongKey(unsigned char *code)
{
unsigned char i=0;

	while(*(code+i)){
		SendKeyData(*(code+i));
		i++;
	}

}

unsigned char SendCode(unsigned char col,unsigned char rowmix, unsigned char cmd)
{
uint8_t i,tmp;

	if(!rowmix)
		return(1);

	for(i=0;i<8;i++)
		if(rowmix & (1<<i)){
			tmp = 0;
			if(FnKey ||(LedFl & KEYBOARD_NUM_LOCK_ON)){
				tmp = fncodes[col][i];

				if((tmp&0xf0) == EXTKEY){
					if(tmp&0x08)
						tmp = fncodes[tmp&0x7][i];
					else
						tmp = fncodes[col][tmp&0x7];
					if(tmp) SendKeyData(EXTKEY);
				}
			}

			if(tmp == 0){
				tmp = scodes[col][i];
				if((tmp&0xf0) == EXTKEY){
					if(tmp&0x08)
						tmp = scodes[tmp&0x7][i];
					else
						tmp = scodes[col][tmp&0x7];
					if(tmp) SendKeyData(EXTKEY);
				}
			}

			switch(tmp){
			case FNKEY:
				FnKey = cmd;
				break;
			case TPKEY:
				if(cmd == PRESSED)
					TouchPadOn = !TouchPadOn;
				break;
			case PAUSEKEY:
				if(cmd == PRESSED)
					SendLongKey((unsigned char *)PauseCode);
				break;
			case PrScrKEY:
				if(cmd == PRESSED)
					SendLongKey((unsigned char *)PrScrCode);
				break;
			default:
				if(tmp){
					if(cmd == RELEASE)
						SendKeyData(KEYBOARD_BREAK_CODE);
					SendKeyData(tmp);
				}
			}
		}
	
	return(0);
}

void CheckPS2Buf(void)
{

	while(buffcnt){
		USART_PutChar(PS2_CMD);
		USART_PutChar(ps2buf_get());
	}

}

void main (void)
{
uint32_t sout;
//uint8_t scbuf[20];
uint8_t lbuf[18]={ 0 }; //last state
uint8_t deb, cnt,st,st1,ind, pressed, released, notchanged;

	wdt_reset();
	wdt_disable();

	cli();
	PORTD = 0;
	DDRD = 0;
//init uart
	DDRD |= 2;
	USART_Init();

//	GICR = 0;     	//Запрет внешних прерываний
	ps2_init();
	TIMSK |= _BV(TOIE1); /* allow timer1 overflow */
	sei();			//int enable

//led portd 4-5
	PORTD |= (16+32);//off
	DDRD |= (16+32);

//8 bit PORTA input + pullup
	PORTA = 0xFF;
	DDRA = 0;
	SFIOR &= ~(1<<PUD);
//18 bit PORTC PORTB PORTD(6-7) output Z
	PORTC = 0;
	PORTB = 0;
	PORTD &= ~(64+128);

	wdt_reset();
	wdt_enable(WDTO_500MS); //TODO check
	LedFl = 0;
	USART_PutChar(KEYBOARD_COMPLETE_SUCCESS);//init ok
	while(1){

		sout=2;//to second col
		//18 bit PORTC PORTB PORTD(2-3) output Z
		//to first col
		DDRC=0;
		DDRB = 0;
		DDRD &= ~(64+128);
		DDRD |= 64;
		
		ind = 0;
		delay(10);//delay before first read TODO check
		for(cnt=0;cnt<18;cnt++){
			st = 0;
			deb = DEBOUNCE;
			while(deb){
				st |= ~PINA;//read curr state
				deb--;
			}
//set new cols data
			DDRD &= ~(64+128);
			DDRD |= (sout<<6) & (64+128);
			DDRB = (sout>>2) & 0xff;
			DDRC = (sout>>10) & 0xff;
			sout <<=1;
//process
			st1 = lbuf[cnt];//last state
			pressed = (~st1)&st;
			released = (~st)&st1;
			notchanged = st&st1;//for repeat
			if(pressed){
				SendCode(cnt,pressed,PRESSED);
				t1_on_Xms(KeyDelay)
			}
			SendCode(cnt,released,RELEASE);
			if(notchanged){
				if(t1_overflow()){
					SendCode(cnt,notchanged,REPEAT);//repeat
					t1_on_Xms(KeyPeriod)
				}
			}
				
			lbuf[cnt] = st;//last state
			if(TouchPadOn) CheckPS2Buf();
		}		

		if(USART_GetRxCount())
			rxUdata();

		wdt_reset();
	}	

}


