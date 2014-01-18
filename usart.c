//***************************************************************************
//
//  Author(s)...: Pashgan    http://ChipEnable.Ru   
//
//  Target(s)...: ATMega8535
//
//  Compiler....: WINAVR
//
//  Description.: драйвер USART/UART с кольцевым буфером
//
//  Data........: 11.01.10 
//
//***************************************************************************
#include "usart.h"
#include "subs.h"


#define USART_BAUDRATE 2400
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) 

//макросы для разрешения и запрещения прерываний usart`a
#define EnableRxInt()   UCSRB |= (1<<RXCIE);
#define DisableRxInt()  UCSRB &= (~(1<<RXCIE));
#define EnableTxInt()   UCSRB |= (1<<TXCIE);
#define DisableTxInt()  UCSRB &= (~(1<<TXCIE));
//передающий буфер
static volatile uint8_t usartTxBuf[SIZE_TBUF];
static uint8_t txBufTail = 0;
static uint8_t txBufHead = 0;
static volatile uint8_t txCount = 0;

//приемный буфер
static volatile uint8_t usartRxBuf[SIZE_RBUF];
static uint8_t rxBufTail = 0;
static uint8_t rxBufHead = 0;
static volatile uint8_t rxCount = 0;


//инициализация usart`a
void USART_Init(void)
{
	UBRRL = (uint8_t)(BAUD_PRESCALE & 0xff);
	UBRRH = (uint8_t)(BAUD_PRESCALE >> 8); 
//разр. прерыв при приеме  разр приема, разр передачи. размер слова 8 разрядов
	UCSRB = (1<<RXCIE)|(1<<TXCIE)|(1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 1stop bit */
	UCSRC = (3<<UCSZ0)+128;

	USART_FlushTxBuf();
}

//______________________________________________________________________________
//возвращает колличество символов передающего буфера
uint8_t USART_GetTxCount(void)
{
  return txCount;  
}

//"очищает" передающий буфер
void USART_FlushTxBuf(void)
{
  txBufTail = 0;
  txBufHead = 0;
  txCount = 0;
}


//помещает символ в буфер, инициирует начало передачи
void USART_PutChar(uint8_t sym)
{
  //если модуль usart свободен и это первый символ
  //пишем его прямо в регистр UDR
	if((UCSRA & (1<<UDRE)) && (txCount == 0)){
		UDR = sym;
	}
	else {
		if (txCount < SIZE_TBUF){    //если в буфере еще есть место
		usartTxBuf[txBufTail] = sym; //помещаем в него символ
		txCount++;                   //инкрементируем счетчик символов
		txBufTail++;                 //и индекс хвоста буфера
		if (txBufTail == SIZE_TBUF) txBufTail = 0;
		}
	}

}

//функция посылающая строку по usart`у
void USART_SendStr(char * data)
{
  uint8_t sym;
  while(*data){
    sym = *data++;
    USART_PutChar(sym);
  }
}

//обработчик прерывания по завершению передачи 
ISR(USART_TX_vect)
{

	if (txCount > 0){              //если буфер не пустой
		UDR = usartTxBuf[txBufHead]; //записываем в UDR символ из буфера
		txCount--;                   //уменьшаем счетчик символов
		txBufHead++;                 //инкрементируем индекс головы буфера
		if (txBufHead == SIZE_TBUF) txBufHead = 0;
	}

} 

//______________________________________________________________________________
//возвращает колличество символов находящихся в приемном буфере
uint8_t USART_GetRxCount(void)
{
  return rxCount;  
}

//"очищает" приемный буфер
void USART_FlushRxBuf(void)
{
  DisableRxInt(); //запрещаем прерывание по заверщению приема
  rxBufTail = 0;
  rxBufHead = 0;
  rxCount = 0;
  EnableRxInt();
}

//чтение буфера
uint8_t USART_GetChar(void)
{
  uint8_t sym;
  if (rxCount > 0){                     //если приемный буфер не пустой  
    sym = usartRxBuf[rxBufHead];        //прочитать из него символ    
    rxCount--;                          //уменьшить счетчик символов
    rxBufHead++;                        //инкрементировать индекс головы буфера  
    if (rxBufHead == SIZE_RBUF) rxBufHead = 0;
    return sym;                         //вернуть прочитанный символ
  }
  return 0;
}

//прерывание по завершению приема
ISR(USART_RX_vect) 
{
  if (rxCount < SIZE_RBUF){                //если в буфере еще есть место                     
      usartRxBuf[rxBufTail] = UDR;        //считать символ из UDR в буфер
      rxBufTail++;                             //увеличить индекс хвоста приемного буфера 
      if (rxBufTail == SIZE_RBUF) rxBufTail = 0;  
      rxCount++;                                 //увеличить счетчик принятых символов
    }
	else
		usartRxBuf[0] = UDR;
} 
