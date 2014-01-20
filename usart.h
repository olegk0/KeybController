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
#ifndef USART_H
#define USART_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define SIZE_TBUF 200       //� ������ ��������� ������� - <255
#define SIZE_RBUF 30

void USART_Init(void); //������������� usart`a
uint8_t USART_GetTxCount(void); //����� ����� �������� ����������� ������
void USART_FlushTxBuf(void); //�������� ���������� �����
void USART_PutChar(uint8_t sym); //�������� ������ � �����
void USART_SendStr(char * data); //������� ������ �� usart`�
uint8_t USART_GetRxCount(void); //����� ����� �������� � �������� ������
void USART_FlushRxBuf(void); //�������� �������� �����
uint8_t USART_GetChar(void); //��������� �������� ����� usart`a 

#endif //USART_H