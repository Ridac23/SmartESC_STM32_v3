/*
 * M365_Dashboard.c
 *
 *  Created on: Nov 27, 2021
 *      Author: stancecoke
 */


#include "main.h"
#include "config.h"
#include "stm32f1xx_hal.h"
#include "print.h"
#include "M365_Dashboard.h"
enum { STATE_LOST, STATE_START_DETECTED, STATE_LENGTH_DETECTED };
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;
static uint8_t ui8_rx_buffer[64];
static uint8_t ui8_dashboardmessage[64];
static uint8_t ui8_tx_buffer[64];
static uint8_t ui8_oldpointerposition=0;
static uint8_t ui8_recentpointerposition=0;
static uint8_t ui8_messagestartpos=255;
static uint8_t ui8_messagelength=0;
static uint8_t ui8_state= STATE_LOST;



void M365Dashboard_init() {
//        CLEAR_BIT(huart3.Instance->CR3, USART_CR3_EIE);
	if (HAL_UART_Receive_DMA(&huart3, (uint8_t*) ui8_rx_buffer, sizeof(ui8_rx_buffer)) != HAL_OK) {
		Error_Handler();
	}
}

void search_DashboardMessage(MotorState_t *MS, MotorParams_t *MP){

	ui8_recentpointerposition = sizeof(ui8_rx_buffer) - DMA1_Channel3->CNDTR; //Pointer of UART1RX DMA Channel

		while(ui8_oldpointerposition!=ui8_recentpointerposition){

			switch (ui8_state) {
			case STATE_LOST: { //if no message start is detected yet, search for start pattern 0x55 0xAA
				if(ui8_rx_buffer[ui8_oldpointerposition]==0xAA&&ui8_rx_buffer[ui8_oldpointerposition-1]==0x55){
					ui8_messagestartpos=ui8_oldpointerposition-1;
					ui8_state=STATE_START_DETECTED;
				}
			}
				break;

			case STATE_START_DETECTED: { //read the lenght of the message
				if(ui8_oldpointerposition==ui8_messagestartpos+2){
					ui8_messagelength=ui8_rx_buffer[ui8_oldpointerposition]+4;
					ui8_state=STATE_LENGTH_DETECTED;
				}
			}
				break;
			case STATE_LENGTH_DETECTED: { //read whole message and call processing
				if(ui8_oldpointerposition==ui8_messagestartpos+ui8_messagelength){
					memcpy(ui8_rx_buffer+ui8_messagestartpos,ui8_dashboardmessage,ui8_messagelength);
					process_DashboardMessage( MS,  MP, (uint8_t*)&ui8_dashboardmessage,ui8_messagelength);
					ui8_state=STATE_LOST;
				}
			}
				break;
			} //end switch
			ui8_oldpointerposition=(ui8_oldpointerposition+1)% sizeof(ui8_rx_buffer);

		}// end of while

}

void process_DashboardMessage(MotorState_t *MS, MotorParams_t *MP, uint8_t *message, uint8_t length ){
	HAL_UART_Transmit_DMA(&huart3, (uint8_t*) &message, length);

}