/*
 * matrizWS2812b.cpp
 *
 *  Created on: 22 sep. 2024
 *      Author: Gustavo Alpern
 */

#include "matrizWS2812b.h"
#include "../SPI/spi.h"
#include "../DMA/DMA.h"
matrizLed::matrizLed(DMA * dmaPtr, bool selectSPI, uint8_t xTam, uint8_t yTam, uint32_t port, uint32_t pin):
SPIDMAFastTransmit(selectSPI, CLKDIVIDER, port, pin, MAXTRANSFER), m_xSize(xTam), m_ySize(yTam), m_dmaPtr(dmaPtr), spiSelection(selectSPI)
{
	m_LEDbuffer0 = new __attribute__((aligned(32))) uint8_t[xTam * yTam * 24 + 1];
	m_LEDbuffer1 = new __attribute__((aligned(32))) uint8_t[xTam * yTam * 24 + 1];

	m_LEDbuffer0[0] = 0;
	m_LEDbuffer1[0] = 0;
	m_LEDbufferPtr0 = m_LEDbuffer0;
	m_LEDbufferPtr1 = m_LEDbuffer1;

	uint32_t * dataptr = getTransmitDataPtr();
	uint8_t channelsel;

	if(selectSPI == s_SPI0){
		channelsel = DMA::SPI0_TX_DMACH;
	}
	else{
		channelsel = DMA::SPI1_TX_DMACH;
	}

	dmaPtr->crearDescriptor(&LEDdesc, DMA::RELOAD, DMA::UN_BYTE, DMA::X1_INC, DMA::NO_INC, xTam * yTam * 9 + 1, DMA::NO_INT, DMA::NO_INT, m_LEDbufferPtr1, (void *)dataptr,&LEDdesc);
	dmaPtr->prenderCanal(&LEDdesc, channelsel, DMA::PERIFREQUEST, 0);

}

uint8_t matrizLed::setMatrizSize(uint8_t xTam, uint8_t yTam){

	delete[] m_LEDbuffer0;
	delete[] m_LEDbuffer1;

	m_LEDbuffer0 = new __attribute__((aligned(32))) uint8_t[xTam * yTam * 24 + 1];
	m_LEDbuffer1 = new __attribute__((aligned(32))) uint8_t[xTam * yTam * 24 + 1];

	m_LEDbuffer0[0] = 0;
	m_LEDbuffer1[0] = 0;

	m_LEDbufferPtr0 = m_LEDbuffer0;
	m_LEDbufferPtr1 = m_LEDbuffer1;

	m_xSize = xTam;
	m_ySize = yTam;

	return OK;
}
uint8_t matrizLed::setMatrizPin(uint32_t port, uint32_t pin){

	if(setPin(s_Data,port,pin)!=OK){
		return ERROR_PARAMETROS;
	}

	return OK;
}
uint8_t matrizLed::setLed(uint8_t x, uint8_t y, uint8_t R, uint8_t G, uint8_t B){

	int countb=0;
	const uint8_t estados[3]={1, 1, 0};
	const uint8_t estados2[3]={1, 0, 0};

	if(x > m_xSize - 1 || y > m_ySize - 1){
		return ERROR_PARAMETROS;
	}

	int num = x + y*m_xSize;

	for(int i = num * 3 * 24; i < (num + 1) * 3 * 24; i++){

		if(countb > 47){
			if((B >> (7 - ((countb / 3) & 7))) & 1){
				m_LEDbufferPtr0[i / 8 + 1] = (m_LEDbufferPtr0[i / 8 + 1] & (~(1 << (7 - (i & 7))))) | (estados[i % 3] << (7 - (i & 7)));
			}
			else{
				m_LEDbufferPtr0[i / 8 + 1] = (m_LEDbufferPtr0[i / 8 + 1] & (~(1 << (7 - (i & 7))))) | (estados2[i % 3] << (7 - (i & 7)));
			}
		}
		if(countb < 48 && countb > 23){
			if((R >> (7 - ((countb / 3) & 7))) & 1){
				m_LEDbufferPtr0[i / 8 + 1] = (m_LEDbufferPtr0[i / 8 + 1] & (~(1 << (7 - (i & 7))))) | (estados[i % 3] << (7 - (i & 7)));
			}
			else{
				m_LEDbufferPtr0[i / 8 + 1] = (m_LEDbufferPtr0[i / 8 + 1] & (~(1 << (7 - (i & 7))))) | (estados2[i % 3] << (7 - (i & 7)));
			}
		}
		if(countb < 24){
			if((G >> (7 - ((countb / 3) & 7))) & 1){
				m_LEDbufferPtr0[i / 8 + 1] = (m_LEDbufferPtr0[i / 8 + 1] & (~(1 << (7 - (i & 7))))) | (estados[i % 3] << (7 - (i & 7)));
			}
			else{
				m_LEDbufferPtr0[i / 8 + 1] = (m_LEDbufferPtr0[i / 8 + 1] & (~(1 << (7 - (i & 7))))) | (estados2[i % 3] << (7 - (i & 7)));
			}
		}
		if(i > 0)
		countb++;
		if(countb > 71) countb = 0;
	}

	return OK;
}
uint8_t matrizLed::setLed(uint8_t x, uint8_t y, uint32_t RGB){

	if(RGB > 0xFFFFFF || x > m_xSize - 1 || y > m_ySize - 1){
		return ERROR_PARAMETROS;
	}
	uint8_t R = (RGB & 0x00FF0000) >> 16;
	uint8_t G = (RGB & 0x0000FF00) >> 8;
	uint8_t B = (RGB & 0x000000FF) >> 0;
	setLed(x, y, R, G, B);

	return OK;
}
uint8_t matrizLed::clearLed(){

	const uint8_t estados2[3]={1, 0, 0};

	for(int i = 0 ; i < m_xSize * m_ySize * 3 * 24 ; i++){
		m_LEDbufferPtr0[i / 8 + 1] = (m_LEDbufferPtr0[i / 8 + 1] & (~(1 << (7 - (i & 7))))) | (estados2[i % 3] << (7 - (i & 7)));
	}

	return OK;
}
uint8_t matrizLed::displayLed(){

	if(change == true){
		change = !change;
		m_LEDbufferPtr0 = &(m_LEDbuffer0[0]);
		m_LEDbufferPtr1 = &(m_LEDbuffer1[0]);
	}
	else{
		change = !change;
		m_LEDbufferPtr0 = &(m_LEDbuffer1[0]);
		m_LEDbufferPtr1 = &(m_LEDbuffer0[0]);
	}

	if(spiSelection == s_SPI0){
		m_dmaPtr->forzarEnvio(DMA::SPI0_TX_DMACH);
	}
	else{
		m_dmaPtr->forzarEnvio(DMA::SPI1_TX_DMACH);
	}

	return OK;
}
matrizLed::~matrizLed(){

	if(spiSelection == s_SPI0){
		m_dmaPtr->apagarCanal(DMA::SPI0_TX_DMACH);
	}
	else{
		m_dmaPtr->apagarCanal(DMA::SPI1_TX_DMACH);
	}

	delete[] m_LEDbuffer0;
	delete[] m_LEDbuffer1;
}
