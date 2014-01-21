
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

void SendESCifNeed(uint8_t data)
{
	if( data == PS2_CMD || data == KEY_CMD || data == ESC_CMD || data == BSYNC ||
											data ==SESEND_CMD || data == SESRST_CMD)
		USART_PutChar(ESC_CMD);
}

void SetSession(uint8_t ses)
{
	if(ses != CurSession){
		CurSession = ses;
		USART_PutChar(ses);
	}
}

void CheckPS2Buf(void)
{
uint8_t tmp;

	if(ps2_buffcnt)
		SetSession(PS2_CMD);
	while(ps2_buffcnt){
		tmp = ps2buf_get();
		SendESCifNeed(tmp);
		USART_PutChar(tmp);
	}

}

void SendKeyRet(uint8_t dt)
{
		SetSession(KEY_CMD);
		USART_PutChar(dt);
}

void CmdToKeyb(uint8_t dt)
{
	uint8_t ret;

	ret = ACKNOWLEDGE;

	if(LastCmd){
		switch(LastCmd){
		case SET_KEYBOARD_INDICATORS:
			LedFl = dt;
			break;
		case SET_KEYBOARD_TYPEMATIC:
			KeyDelay = (1+((dt>>5) & 3))*250;
			if(KeyDelay<MINKEYDELAY)
				KeyDelay = MINKEYDELAY;
			if(KeyDelay>MAXKEYDELAY)
				KeyDelay = MAXKEYDELAY;
			KeyPeriod = ((8+(dt&7))*(1<<((dt>>3)&3)))<<2;
			if(KeyPeriod<MINKEYPER)
				KeyPeriod = MINKEYPER;
			if(KeyPeriod>MAXKEYPER)
				KeyPeriod = MAXKEYPER;
			break;
		case SELECT_SCAN_CODE_SET:
			if(!dt)
				ret = 2;
			else if(dt != 2 )
				ret = RESEND;
			break;
		default:
			ret = RESEND;
		}
		LastCmd = 0;
	}
	else{
		switch(dt){
		case KEYBOARD_RESET:
/*			wdt_enable(WDTO_120MS);
			while(1);
*/
			SendKeyRet(ACKNOWLEDGE);
			ret = KEYBOARD_COMPLETE_SUCCESS;
			break;
		case ATKBD_CMD_RESET_DEF:
		case ATKBD_CMD_RESET_DIS:
			SetDef();
			break;
		case READ_KEYBOARD_ID:
			SendKeyRet(0xAB);
			ret = 0x01;
			break;//
		case ATKBD_CMD_ENABLE:
			break;
		case SET_KEYBOARD_INDICATORS:
		case SET_KEYBOARD_TYPEMATIC:
		case SELECT_SCAN_CODE_SET:
			LastCmd = dt;
			break;
		default:
			ret = RESEND;
		}
	}

	SendKeyRet(ret);
}


void rxUdata(void)
{
	uint8_t rx;

	rx = USART_GetChar();
	
	if(ESCwas){
		ESCwas = 0;
		if(LastInSesion == PS2_CMD)
			ps2_send_byte(rx);
		else
			CmdToKeyb(rx);
		return;
	}
	
	switch(rx){
	case ESC_CMD:
		ESCwas = 1;
		break;
	case PS2_CMD:
	case KEY_CMD:
		SetSession(rx);
		LastInSesion = rx;
		break;
	case SESRST_CMD://session reset
		SetSession(KEY_CMD);
		LastInSesion = KEY_CMD;
		LastCmd = 0;
		ps2_init();//check
		break;
	case SESEND_CMD://for compatibility
	case BSYNC:
		break;
	default:
		if(LastInSesion == PS2_CMD)
			ps2_send_byte(rx);
		else
			CmdToKeyb(rx);
	}

}

void SendKeyData(uint8_t dt)
{
	SendESCifNeed(dt);
	USART_PutChar(dt);
}

void SendLongKey(uint8_t *code)
{
uint8_t i=0,tmp;
	do{
		tmp = pgm_read_byte(code+i);
		if(!tmp) break;
		SendKeyData(tmp);
		i++;
	}
	while(1);
}

uint8_t SendCode(uint8_t col,uint8_t rowmix, uint8_t cmd)
{
uint8_t i,tmp;

	if(!rowmix)
		return(1);
	SetSession(KEY_CMD);
	for(i=0;i<8;i++)
		if(rowmix & (1<<i)){
			tmp = 0;
			if(FnKey ||(LedFl & KEYBOARD_NUM_LOCK_ON)){
				tmp = pgm_read_byte(&(fncodes[col][i]));

				if((tmp&0xf0) == EXTKEY){
					if(tmp&0x08)
						tmp = pgm_read_byte(&(fncodes[tmp&0x7][i]));
					else
						tmp = pgm_read_byte(&(fncodes[col][tmp&0x7]));
					if(tmp) USART_PutChar(EXTKEY);
				}
			}

			if(tmp == 0){
				tmp = pgm_read_byte(&(scodes[col][i]));
				if((tmp&0xf0) == EXTKEY){
					if(tmp&0x08)
						tmp = pgm_read_byte(&(scodes[tmp&0x7][i]));
					else
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

	CurSession = KEY_CMD;
	LastInSesion = KEY_CMD;
	LastCmd = 0;
	ESCwas = 0;
	
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
		if(USART_GetRxCount())
			rxUdata();

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
		wdt_reset();
	}	

}


