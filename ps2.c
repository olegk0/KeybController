/**
 * Driver for a PS/2 mouse
 
 * this ps/2 interface is based on avr appnote 313
 * and http://www.computer-engineering.org/index.php?title=Main_Page
 *
 * very helpful was: http://www.mikrocontroller.net/topic/112361
 * and the asociated source code.
 *
 * data and clk state is high
 * 11 bit frame: 1 start bit, 8 bit data lsb first, 1 bit parity (odd), stop bit
 * 
 * start		lsb	data	msb		parity	  stop
 * 0			x x x x x x x x		x		  1
 * 
 * (c) Jens Carroll, Inventronik GmbH
 */


#define PS2_BUFF_SIZE 20
#define FALSE 0
#define TRUE (!FALSE)
#define SET_BIT(data, port, pin) ((data & 0x01) ? (port | _BV(pin)) : (port & ~_BV(pin)))
#define RESET_BIT(data, port, pin) ((data & 0x01) ? (port & ~_BV(pin)) : (port | _BV(pin)))

#define PS2_CLOCK		PD2
#define PS2_PORT		PORTD
#define PS2_PIN			PIND
#define PS2_DDR			DDRD
#define PS2_DATA		PD3

static volatile uint8_t ps2_data;						/* holds the received mouse data */
static volatile uint8_t edge, bitcount;			 /* edge: 0 = neg.	1 = pos. */
static volatile uint8_t *in_ptr, *out_ptr;
static volatile uint8_t ps2_buffcnt;
static uint8_t ps2_buffer[PS2_BUFF_SIZE];


/*
 * Clear mouse fifo
 */
void ps2_clear_buffer(void)
{
	in_ptr = out_ptr = ps2_buffer;
	ps2_buffcnt = 0;
}

/*
 * Initialize the PS/2 
 */
void ps2_init(void)
{
	/* MS clock and data as input */
	PS2_DDR  &= ~_BV(PS2_DATA);
	PS2_DDR &= ~_BV(PS2_CLOCK);
	/* MS clock and data to low */
	PS2_PORT  &= ~_BV(PS2_DATA);
	PS2_PORT &= ~_BV(PS2_CLOCK);
	
	GICR = _BV(INT0);		/* enable irpt 1 */
	MCUCR |= _BV(ISC01);	/* INT0 interrupt on falling edge */
	MCUCR &= ~_BV(ISC00);

	ps2_clear_buffer();
	edge = 0;				/* 0 = falling edge  1 = rising edge */
	bitcount = 11;
	TIMSK |= _BV(TOIE2); /* allow timer2 overflow */
	ps2_data = 0;
}

static void ps2buf_put(uint8_t c)
{
	// put character into buffer and incr ptr 
	*in_ptr++ = c;
	ps2_buffcnt++;

	// pointer wrapping
	if (in_ptr >= ps2_buffer + PS2_BUFF_SIZE)
		in_ptr = ps2_buffer;
}

uint8_t ps2buf_get(void)
{
	uint8_t byte = 0;

//	while (buffcnt == 0); // wait for data
	byte = *out_ptr++;	  // get byte

	if (out_ptr >= ps2_buffer + PS2_BUFF_SIZE) // pointer wrapping
		out_ptr = ps2_buffer;
	ps2_buffcnt--;	// decrement buffer count 

	return byte;
}


ISR(INT0_vect)
{
	if(TouchPadOn){
		if(t2_overflow())
			ps2_data = 0;
		t2_on_512us();

		if (!edge) {								/* routine entered at falling edge */
			if (bitcount < 11 && bitcount > 2) {	/* bit 3 to 10 is data. Parity bit, */
												/* start and stop bits are ignored. */
				ps2_data >>= 1;
				if ((PS2_PIN & _BV(PS2_DATA)))
					ps2_data |=  0x80;					/* store a '1' */
			}
			MCUCR |= _BV(ISC00);					/* set interrupt on rising edge */
			edge = 1;
		} else	{									/* routine entered at rising edge */
			MCUCR &= ~_BV(ISC00);					/* set interrupt on falling edge */
			edge = 0;
			bitcount--;
			if (bitcount == 0) {					/* all bits received */
				ps2buf_put(ps2_data);					/* Add data to buffer */
				ps2_data = 0;
				bitcount = 11;
			}
		}
	}
}

ISR(TIMER2_OVF_vect)
{
    TCCR2 = 0; /* stop timer2 and reset TCCR2 to indicate the overflow */
    TCNT2 = 0;
}


/*
 * Send one byte to the ps2 device (here mouse)
 *
 * returns TRUE if no timeout occurred and the device responds with ACK
 * otherwise FALSE
 */
static uint8_t ps2_send_byte(uint8_t data)
{
	uint8_t j, result = FALSE, parity = 0;

//	if (!t2_overflow())
//		return FALSE;		/* send in progress */

	GICR &= ~_BV(INT0);	/* disable INT0 */

	/* MS clock and data to high */
	PS2_DDR &= ~_BV(PS2_CLOCK);
	PS2_DDR &= ~_BV(PS2_DATA);
//	PS2_DATA_PORT  |= _BV(PS2_DATA);
//	PS2_CLOCK_PORT |= _BV(PS2_CLOCK);
	
	/* MS clock and data as outputs */
//	PS2_DATA_DDR  |= _BV(PS2_DATA);
//	PS2_CLOCK_DDR |= _BV(PS2_CLOCK);

	/* MS clock now to low */
	PS2_DDR |= _BV(PS2_CLOCK);
//	PS2_CLOCK_PORT &= ~_BV(PS2_CLOCK);

	/* minimum delay between clock low and data low */
	delay(120);

	/* next MS data to low */
	PS2_DDR  |= _BV(PS2_DATA);
//	PS2_DATA_PORT &= ~_BV(PS2_DATA);

	/* send start bit (just with this delay) */
	delay(20);

	/* release MS clock as input - hi*/
	PS2_DDR &= ~_BV(PS2_CLOCK);
	delay(50);

	j = 0;
	t2_on_65ms();

	do {
		/* wait until data gets low (ack from device) */
		while ((PS2_PIN & _BV(PS2_CLOCK)) && !t2_overflow());

		/* timer2 overflow? */
		if (t2_overflow()) break;

		if (j<8) {
			PS2_DDR = RESET_BIT(data, PS2_DDR, PS2_DATA);
//			PS2_DATA_PORT = SET_BIT(data, PS2_DATA_PORT, PS2_DATA);
			if (data & 0x01) {
				parity ^= 0x01;
			}

			data >>= 1;
		} else if (j==8) {
			/* insert parity */
			PS2_DDR = RESET_BIT(~parity, PS2_DDR, PS2_DATA);
//			PS2_DATA_PORT = SET_BIT(~parity, PS2_DATA_PORT, PS2_DATA);
		} else if (j>8) {
			/* MS clock and data as inputs again */
			PS2_DDR &= ~_BV(PS2_DATA);
			PS2_DDR &= ~_BV(PS2_CLOCK);

			if (j==10) {	
				/* receive ACK eventually
				   wait until data gets low (ack from device) */
				while ((PS2_PIN & _BV(PS2_DATA)) && !t2_overflow());
				if (!t2_overflow())
					result = TRUE;

				while ((PS2_PIN & _BV(PS2_DATA)) && (PS2_PIN & _BV(PS2_CLOCK)) && !t2_overflow());
				if (t2_overflow())
					result = FALSE;
				break;
			}
		}
		
		/* wait until clock gets high or timeout */
		while ((!(PS2_PIN & _BV(PS2_CLOCK))) && !t2_overflow());
		if (t2_overflow())
			break;
		j++;
	} while (j<11);

	/* MS clock and data as input */
	PS2_DDR &= ~_BV(PS2_DATA);
	PS2_DDR &= ~_BV(PS2_CLOCK);

	/* clear interrupt flag bit (write a 1) to prevent ISR entry upon irpt enable */
	GIFR = _BV(INTF0);

	/* enable mouse irpt */
	GICR |= _BV(INT0);

	/* stop timer */
	t2_off();

	return result;
}

/*
 * Send one byte to the ps2 device (here mouse) and wait for
 * an ACK (0xFA)
 */
 /*
uint8_t ps2_send_cmd(uint8_t data)
{
	uint8_t result = FALSE;

	if (ps2_send_byte(data)) {
		delay(50);

		// did we receive ACK before timeout?
		if (buffcnt!=0 && ps2buf_get()==0xFA)
			result = TRUE;
	}
	return result;
}
*/