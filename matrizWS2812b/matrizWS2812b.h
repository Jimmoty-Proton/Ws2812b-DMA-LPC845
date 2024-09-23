/*
 * matrizWS2812b.h
 *
 *  Created on: 22 sep. 2024
 *      Author: Gustavo Alpern
 */

#ifndef MATRIZWS2812B_MATRIZWS2812B_H_
#define MATRIZWS2812B_MATRIZWS2812B_H_
//#include "inicializacion.h"
#include "../SPI/spi.h"
#include "../DMA/DMA.h"
#include "tipos.h"
#include "LPC845.h"
#define CLKDIVIDER 12
#define MAXTRANSFER 7

class matrizLed : public SPIDMAFastTransmit{
private:

	uint8_t m_xSize;
	uint8_t m_ySize;
	uint8_t * m_LEDbuffer0;
	uint8_t * m_LEDbuffer1;
	uint8_t * m_LEDbufferPtr0;
	uint8_t * m_LEDbufferPtr1;
	DMA *m_dmaPtr;
	bool spiSelection;
	bool change;
	Descriptor __attribute__((aligned(32))) LEDdesc;
	void DMAcallback(void);

public:
	enum spiSelect_t{s_SPI0,s_SPI1};
	enum errores_t{OK,ERROR_PARAMETROS};
	matrizLed(DMA * dmaPtr,bool selectSPI,uint8_t xTam, uint8_t yTam,uint32_t port,uint32_t pin);
	uint8_t setMatrizSize(uint8_t xTam,uint8_t yTam);
	uint8_t setMatrizPin(uint32_t port,uint32_t pin);
	uint8_t setLed(uint8_t x, uint8_t y, uint8_t R, uint8_t G, uint8_t B);
	uint8_t setLed(uint8_t x, uint8_t y, uint32_t RGB);
	uint8_t clearLed();
	uint8_t displayLed();

	~matrizLed();
};


#endif /* MATRIZWS2812B_MATRIZWS2812B_H_ */
