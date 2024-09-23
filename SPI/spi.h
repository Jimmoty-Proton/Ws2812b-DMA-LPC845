/*
 * spi.h
 *
 *  Created on: 22 sep. 2024
 *      Author: Gustavo Alpern
 */

#ifndef SPI_SPI_H_
#define SPI_SPI_H_

//#include "inicializacion.h"
#include "tipos.h"
#include "LPC845.h"

//clase SPI incompleta, pensada solo para enviar datos usando el DMA
class SPIDMAFastTransmit{

protected:

	SPI_Type * m_SPIptr;
	uint32_t m_dataPort = 0;
	uint32_t m_dataPin = 0xFF;
	uint32_t m_clkPort = 0;
	uint32_t m_clkPin = 0xFF;
	bool spiSelection;

public:

	enum spiSelect_t{s_SPI0,s_SPI1};
	enum PINSelect_t{s_Data,s_CLK};
	enum clockout_t{NO_CLKOUT,CLOCKOUT};
	enum errores_t{OK,ERROR_PARAMETROS};
	SPIDMAFastTransmit(bool selectSPI, uint16_t clkdiv, uint32_t dataport, uint32_t datapin, uint32_t transferLength, bool enableCLKpin = NO_CLKOUT,  uint32_t clkport = 0xFF, uint32_t clkpin = 0xFF);
	uint8_t setPin(bool selector, uint32_t port, uint32_t pin);
	uint8_t setClk(uint16_t clk);
	uint8_t setTransferLength(uint32_t length);
	uint32_t * getTransmitDataPtr();
	~SPIDMAFastTransmit();
};


#endif /* SPI_SPI_H_ */
