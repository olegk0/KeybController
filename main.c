
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

void delay_ms0(uint8_t ms)
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

//----------------------------------------------

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

void SetDef(void)
{
	LedFl = 0;
	TouchPadOn = 1;
	FnKey=0;
	KeyDelay = DEFKEYDELAY;//ms
	KeyPeriod = DEFKEYPER;//ms
	SwLEDs();
}

uint8_t WaitRdata(void)
{
	uint8_t tmp;
		tmp = RXCMDTIMEOUT;
		while(tmp){
			delay_ms0(1);
			if(USART_GetRxCount())
				return USART_GetChar();
			tmp--;
		}
		return(0);
}

void SendESCifNeed(uint8_t data)
{
	if( data == PS2_CMD || data == KEY_CMD || data == ESC_CMD )
		USART_PutChar(ESC_CMD);
}

void CheckPS2Buf(void)
{
uint8_t tmp;

	if(ps2_buffcnt)
		USART_PutChar(PS2_CMD);
	while(ps2_buffcnt){
		tmp = ps2buf_get();
		SendESCifNeed(tmp);
		USART_PutChar(tmp);
	}

}

void SendKeyRet(uint8_t dt)
{
		USART_PutChar(KEY_CMD);
		USART_PutChar(dt);
}


uint8_t rxUdata(void)
{
	uint8_t rx, tmp, ret;

	rx = USART_GetChar();
	ret = ACKNOWLEDGE;
	switch(rx){
	case SET_KEYBOARD_INDICATORS:
		SendKeyRet(ACKNOWLEDGE);
		tmp = WaitRdata();
		if(!tmp) return(1);
		LedFl = tmp;
		break;
	case SET_KEYBOARD_TYPEMATIC:
		SendKeyRet(ACKNOWLEDGE);
		tmp = WaitRdata();
		if(!tmp) return(1);
		KeyDelay = (1+((tmp>>5) & 3))*250;
		if(KeyDelay<MINKEYDELAY)
			KeyDelay = MINKEYDELAY;
		if(KeyDelay>MAXKEYDELAY)
			KeyDelay = MAXKEYDELAY;
		KeyPeriod = ((8+(tmp&7))*(1<<((tmp>>3)&3)))<<2;
		if(KeyPeriod<MINKEYPER)
			KeyPeriod = MINKEYPER;
		if(KeyPeriod>MAXKEYPER)
			KeyPeriod = MAXKEYPER;
		break;
	case KEYBOARD_RESET:
		SendKeyRet(ACKNOWLEDGE);
		wdt_enable(WDTO_120MS);
		while(1);
		break;
	case PS2_CMD:
		tmp = WaitRdata();
		if(!tmp) return(1);
		if(!ps2_send_byte(tmp))
			ret = RESEND;
		else
			ret = 0;
		break;
	case ATKBD_CMD_RESET_DEF:
	case ATKBD_CMD_RESET_DIS:
		SetDef();
		SendKeyRet(ACKNOWLEDGE);
		break;
	case READ_KEYBOARD_ID:
		SendKeyRet(ACKNOWLEDGE);
		SendKeyRet(0xAB);
		SendKeyRet(0x01);
		ret = 0;
		break;
	case SELECT_SCAN_CODE_SET:
		SendKeyRet(ACKNOWLEDGE);
		tmp = WaitRdata();
		if(!tmp){
			SendKeyRet(2);
			ret = 0;
		}
		else if(tmp != 2 )
			ret = RESEND;
		break;
	case ATKBD_CMD_ENABLE:
		break;
	default:
		if(rx > 0xe0)
			ret = RESEND;
		else
			ret = 0;
	}
	if(ret) SendKeyRet(ret);
	return(ret);
}

void SendKeyData(uint8_t dt)
{
	SendESCifNeed(dt);
	USART_PutChar(dt);
}

void SendLongKey(uint8_t *code)
{
uint8_t i=0;

	while(*(code+i)){
		SendKeyData(*(code+i));
		i++;
	}

}

uint8_t SendCode(uint8_t col,uint8_t rowmix, uint8_t cmd)
{
uint8_t i,tmp;

	if(!rowmix)
		return(1);
	USART_PutChar(KEY_CMD);
	for(i=0;i<8;i++)
		if(rowmix & (1<<i)){
			tmp = 0;
			if(FnKey ||(LedFl & KEYBOARD_NUM_LOCK_ON)){
				//tmp = fncodes[col][i];
				tmp = pgm_read_byte(&(fncodes[col][i]));

				if((tmp&0xf0) == EXTKEY){
					if(tmp&0x08)
						//tmp = fncodes[tmp&0x7][i];
						tmp = pgm_read_byte(&(fncodes[tmp&0x7][i]));
					else
						//tmp = fncodes[col][tmp&0x7];
						tmp = pgm_read_byte(&(fncodes[col][tmp&0x7]));
					if(tmp) USART_PutChar(EXTKEY);
				}
			}

			if(tmp == 0){
				//tmp = scodes[col][i];
				tmp = pgm_read_byte(&(scodes[col][i]));
				if((tmp&0xf0) == EXTKEY){
					if(tmp&0x08)
						//tmp = scodes[tmp&0x7][i];
						tmp = pgm_read_byte(&(scodes[tmp&0x7][i]));
					else
						//tmp = scodes[col][tmp&0x7];
						tmp = pgm_read_byte(&(scodes[col][tmp&0x7]));
					if(tmp) USART_PutChar(EXTKEY);
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
					SendLongKey((uint8_t *)PauseCode);
				break;
			case PrScrKEY:
				if(cmd == PRESSED)
					SendLongKey((uint8_t *)PrScrCode);
				break;
			default:
				if(tmp){
					if(cmd == RELEASE)
						USART_PutChar(KEYBOARD_BREAK_CODE);
					SendKeyData(tmp);
				}
			}
		}
	
	return(0);
}


void main (void)
{
uint32_t sout;
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

//led portd 4-5
	PORTD |= (16+32);//off
	DDRD |= (16+32);

	SetDef();

//8 bit PORTA input + pullup
	PORTA = 0xFF;
	DDRA = 0;
	SFIOR &= ~(1<<PUD);
//18 bit PORTC PORTB PORTD(6-7) output Z
	PORTC = 0;
	PORTB = 0;
	PORTD &= ~(64+128);

	sei();			//int enable
	wdt_reset();
	wdt_enable(WDTO_500MS); //TODO check
	SendKeyRet(KEYBOARD_COMPLETE_SUCCESS);//init ok

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
			CheckPS2Buf();
		}		

		if(USART_GetRxCount())
			rxUdata();

		wdt_reset();
	}	

}


