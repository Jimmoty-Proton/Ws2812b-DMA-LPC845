/*
 * spi.cpp
 *
 *  Created on: 22 sep. 2024
 *      Author: Gustavo Alpern
 */

#include "spi.h"


SPIDMAFastTransmit::SPIDMAFastTransmit(bool selectSPI, uint16_t clkdiv, uint32_t dataport, uint32_t datapin, uint32_t transferLength, bool enableCLKpin,  uint32_t clkport, uint32_t clkpin){

	if(selectSPI == s_SPI0){
		m_SPIptr = (SPI_Type *)SPI0_BASE;
	}
	else{
		m_SPIptr = (SPI_Type *)SPI1_BASE;
	}

	spiSelection = selectSPI;

	setPin(s_Data, dataport, datapin);
	if(enableCLKpin == CLOCKOUT){
		setPin(s_CLK, clkport, clkpin);
	}

	if(selectSPI == s_SPI0){
		SYSCON->SYSAHBCLKCTRL0 |= 1 << 11;
		SYSCON->PRESETCTRL0 |=	1 << 11;
		SYSCON->FCLKSEL[9] = 0;
	}
	else{
		SYSCON->SYSAHBCLKCTRL0 |= 1 << 12;
		SYSCON->PRESETCTRL0 |=	1 << 12;
		SYSCON->FCLKSEL[10] = 0;
	}

	m_SPIptr->CFG |= 1 | (1 << 2) | (0 << 8);
	m_SPIptr->DIV = clkdiv;

	if(transferLength > 15){
		m_SPIptr->TXDATCTL = 1 << 22 | 15 << 24;
	}
	else{
		m_SPIptr->TXDATCTL = 1 << 22 | transferLength << 24;
	}

}

uint8_t SPIDMAFastTransmit::setPin(bool selector, uint32_t port,uint32_t pin){

	if(port > 1 || pin > 31){
		return ERROR_PARAMETROS;
	}

	SYSCON->SYSAHBCLKCTRL0 |= 1<<7;
	if(spiSelection == s_SPI0){
		if(selector == s_Data){
			SWM0->PINASSIGN.PINASSIGN4 = (SWM0->PINASSIGN.PINASSIGN4 & 0xFFFFFF00) | (pin + 32 * port);
		}
		else{
			SWM0->PINASSIGN.PINASSIGN3 = (SWM0->PINASSIGN.PINASSIGN3 & 0x00FFFFFF) | ((pin + 32 * port) << 24);
		}
	}
	else{
		if(selector == s_Data){
			SWM0->PINASSIGN.PINASSIGN5 = (SWM0->PINASSIGN.PINASSIGN5 & 0x00FFFFFF) | (pin + 32 * port);
		}
		else{
			SWM0->PINASSIGN.PINASSIGN5 = (SWM0->PINASSIGN.PINASSIGN5 & 0xFF00FFFF) | ((pin + 32 * port) << 24);
		}
	}
	SYSCON->SYSAHBCLKCTRL0 &= ~(1<<7);

	return OK;
}

uint32_t * SPIDMAFastTransmit::getTransmitDataPtr(){

	return (uint32_t *)&m_SPIptr->TXDAT;
}

uint8_t SPIDMAFastTransmit::setClk(uint16_t clk){

	m_SPIptr->DIV = clk;

	return OK;
}
uint8_t SPIDMAFastTransmit::setTransferLength(uint32_t length){

	if(length > 15){
		return ERROR_PARAMETROS;
	}

	m_SPIptr->TXDATCTL = 1 << 22 | length << 24;

	return OK;
}

SPIDMAFastTransmit::~SPIDMAFastTransmit(){

	if(spiSelection == s_SPI0){
		SYSCON->SYSAHBCLKCTRL0 &= ~(1 << 11);
		SYSCON->PRESETCTRL0 &= ~(1 << 11);
		SYSCON->FCLKSEL[9] = 7;
	}
	else{
		SYSCON->SYSAHBCLKCTRL0 &= ~(1 << 12);
		SYSCON->PRESETCTRL0 &= ~(1 << 12);
		SYSCON->FCLKSEL[10] = 7;
	}

	m_SPIptr->CFG &= ~(1 | (1 << 2));

	setPin(s_Data,0,0xFF);
	setPin(s_CLK,0,0xFF);
}
