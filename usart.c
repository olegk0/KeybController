//***************************************************************************
//
//  Author(s)...: Pashgan    http://ChipEnable.Ru   
//
//  Target(s)...: ATMega8535
//
//  Compiler....: WINAVR
//
//  Description.: ������� USART/UART � ��������� �������
//
//  Data........: 11.01.10 
//
//***************************************************************************
#include "usart.h"
#include "subs.h"


#define USART_BAUDRATE 2400
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1) 

//������� ��� ���������� � ���������� ���������� usart`a
#define EnableRxInt()   UCSRB |= (1<<RXCIE);
#define DisableRxInt()  UCSRB &= (~(1<<RXCIE));
#define EnableTxInt()   UCSRB |= (1<<TXCIE);
#define DisableTxInt()  UCSRB &= (~(1<<TXCIE));
//���������� �����
static volatile uint8_t usartTxBuf[SIZE_TBUF];
static uint8_t txBufTail = 0;
static uint8_t txBufHead = 0;
static volatile uint8_t txCount = 0;

//�������� �����
static volatile uint8_t usartRxBuf[SIZE_RBUF];
static uint8_t rxBufTail = 0;
static uint8_t rxBufHead = 0;
static volatile uint8_t rxCount = 0;


//������������� usart`a
void USART_Init(void)
{
	UBRRL = (uint8_t)(BAUD_PRESCALE & 0xff);
	UBRRH = (uint8_t)(BAUD_PRESCALE >> 8); 
//����. ������ ��� ������  ���� ������, ���� ��������. ������ ����� 8 ��������
	UCSRB = (1<<RXCIE)|(1<<TXCIE)|(1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 1stop bit */
	UCSRC = (3<<UCSZ0)+128;

	USART_FlushTxBuf();
}

//______________________________________________________________________________
//���������� ����������� �������� ����������� ������
uint8_t USART_GetTxCount(void)
{
  return txCount;  
}

//"�������" ���������� �����
void USART_FlushTxBuf(void)
{
  txBufTail = 0;
  txBufHead = 0;
  txCount = 0;
}


//�������� ������ � �����, ���������� ������ ��������
void USART_PutChar(uint8_t sym)
{
  //���� ������ usart �������� � ��� ������ ������
  //����� ��� ����� � ������� UDR
	if((UCSRA & (1<<UDRE)) && (txCount == 0)){
		UDR = sym;
	}
	else {
		if (txCount < SIZE_TBUF){    //���� � ������ ��� ���� �����
		usartTxBuf[txBufTail] = sym; //�������� � ���� ������
		txCount++;                   //�������������� ������� ��������
		txBufTail++;                 //� ������ ������ ������
		if (txBufTail == SIZE_TBUF) txBufTail = 0;
		}
	}

}

//������� ���������� ������ �� usart`�
void USART_SendStr(char * data)
{
  uint8_t sym;
  while(*data){
    sym = *data++;
    USART_PutChar(sym);
  }
}

//���������� ���������� �� ���������� �������� 
ISR(USART_TX_vect)
{

	if (txCount > 0){              //���� ����� �� ������
		UDR = usartTxBuf[txBufHead]; //���������� � UDR ������ �� ������
		txCount--;                   //��������� ������� ��������
		txBufHead++;                 //�������������� ������ ������ ������
		if (txBufHead == SIZE_TBUF) txBufHead = 0;
	}

} 

//______________________________________________________________________________
//���������� ����������� �������� ����������� � �������� ������
uint8_t USART_GetRxCount(void)
{
  return rxCount;  
}

//"�������" �������� �����
void USART_FlushRxBuf(void)
{
  DisableRxInt(); //��������� ���������� �� ���������� ������
  rxBufTail = 0;
  rxBufHead = 0;
  rxCount = 0;
  EnableRxInt();
}

//������ ������
uint8_t USART_GetChar(void)
{
  uint8_t sym;
  if (rxCount > 0){                     //���� �������� ����� �� ������  
    sym = usartRxBuf[rxBufHead];        //��������� �� ���� ������    
    rxCount--;                          //��������� ������� ��������
    rxBufHead++;                        //���������������� ������ ������ ������  
    if (rxBufHead == SIZE_RBUF) rxBufHead = 0;
    return sym;                         //������� ����������� ������
  }
  return 0;
}

//���������� �� ���������� ������
ISR(USART_RX_vect) 
{
  if (rxCount < SIZE_RBUF){                //���� � ������ ��� ���� �����                     
      usartRxBuf[rxBufTail] = UDR;        //������� ������ �� UDR � �����
      rxBufTail++;                             //��������� ������ ������ ��������� ������ 
      if (rxBufTail == SIZE_RBUF) rxBufTail = 0;  
      rxCount++;                                 //��������� ������� �������� ��������
    }
	else
		usartRxBuf[0] = UDR;
} 
