/*
 * DMA.h
 *
 *  Created on: 30 ago. 2024
 *      Author: Gustavo Alpern
 *      Clase DMA v0.1
 */

#ifndef DMA_DMA_H_
#define DMA_DMA_H_
#include "tipos.h"
#include "LPC845.h"
#if defined (__cplusplus)
	extern "C" {
	void DMA_IRQHandler(void);
	}
#endif
#define MAX_CHANNELS 25
#define MAX_PRIORITY 7
#define DMA_SYSAHBCLKCTRL_SHIFT 29
#define MAXCANTTRANSFERENCIAS 1023

/*TODO: -Agregar configuracion BURST para FIFO
 * 		-Agregar configuracion de triggers por DMA TRIGMUX (cap 14)
 * 		-Documentar todas las funciones
 */
typedef struct Desc{
	__IO uint32_t TRNCFG;//Transfer configuration como XFERCFG
	__IO void* SRCDATA;// Source end address.
	__IO void* DSTDATA;//Destination end address
	__IO uint32_t NXTDSC;//Link to next descriptor
}Descriptor;

typedef struct {
	struct {
	__IO uint32_t TRNCFG;//Transfer configuration como XFERCFG
	__IO void* SRCDATA;// Source end address.
	__IO void* DSTDATA;//Destination end address
	__IO uint32_t NXTDSC;//Link to next descriptor
	}CHANNEL[25];

}DMACHANNEL_Type;

class DMA{
private:
	DMACHANNEL_Type __attribute__((aligned(512))) m_DMACH;	//Memoria dedicada al DMA para los Descriptores internos, alineada 9bytes ptr - > DMA0CH[8:0] =0
	bool m_canalesActivos[MAX_CHANNELS];
	void (*m_callbackINTA) (void);
	void (*m_callbackINTB) (void);
public:
	//Listado de canales los cuales puede acceder el DMA
	enum canales_t{
		USART0_RX_DMACH = 0,
		USART0_TX_DMACH,
		USART1_RX_DMACH,
		USART1_TX_DMACH,
		USART2_RX_DMACH,
		USART2_TX_DMACH,
		USART3_RX_DMACH,
		USART3_TX_DMACH,
		USART4_RX_DMACH,
		USART4_TX_DMACH,
		SPI0_RX_DMACH,
		SPI0_TX_DMACH,
		SPI1_RX_DMACH,
		SPI1_TX_DMACH,
		I2C0_SLV_DMACH,
		I2C0_MST_DMACH,
		I2C1_SLV_DMACH,
		I2C1_MST_DMACH,
		I2C2_SLV_DMACH,
		I2C2_MST_DMACH,
		I2C3_SLV_DMACH,
		I2C3_MST_DMACH,
		DAC0_DMACH,
		DAC1_DMACH,
		CAPT_DMACH,
	};
	enum recarga_t{NO_RELOAD=0,RELOAD};//Recargar descriptor al completar todas las transferencias
	enum sizeTransferencia_t{UN_BYTE=0,DOS_BYTES,CUATRO_BYTES};//Tamaño de transferencias
	enum incrementoTransferencia_t{NO_INC=0,X1_INC,X2_INC,X4_INC};//Incremento del puntero de origen/destino al finalizar una transferencia
	enum error_t {OK,ERROR_CANAL_USADO,ERROR_PARAMETROS,ERROR_NO_ALINEADO};
	enum reqPerifericos_t{NO_PERIFREQUESTS=0,PERIFREQUEST};
	enum interrSel_t{NO_INT,ENINT};
	//Constructor/es
	DMA();
	//Metodos
	//IMPORTANTE: 1er argumento a modificar, el retorno es solo la devolucion de errores
	uint8_t crearDescriptorConfig(uint32_t * Configptr, bool recarga, uint8_t tamTransferencias, uint8_t incremOrigen, uint8_t incremDest, uint32_t cantTransferencias, bool enableIntA, bool enableIntB);//MaxCantTransferencias = 1024
	uint8_t crearDescriptor(Descriptor * descriptorptr, bool recarga, uint8_t tamTransferencias, uint8_t incremOrigen, uint8_t incremDest, uint32_t cantTransferencias, bool enableIntA, bool enableIntB, void * origenAddr, void * destAddr, Descriptor * siguienteDesc=nullptr);
	uint8_t crearDescriptor(Descriptor * descriptorptr, uint32_t ConfiguracionDescriptor, void * origenAddr, void * destAddr, Descriptor * siguienteDesc=nullptr);

	uint8_t prenderCanal(Descriptor* descriptorptr, uint8_t canal, bool enReqPerif, uint8_t prioridad); //0 es la mayor prioridad, 7 es la menor prioridad
	uint8_t apagarCanal(uint8_t canal);
	uint8_t enableINT();
	uint8_t disableINT();
	uint8_t enableCHINT(uint8_t canal);
	uint8_t disableCHINT(uint8_t canal);
	uint8_t instalarCallBackA(uint8_t canal, void (*callbackA) (void)= nullptr);
	uint8_t instalarCallBackB(uint8_t canal, void (*callbackB) (void)= nullptr);
	uint8_t forzarEnvio(uint8_t canal);

	//Destructor
	~DMA();
};



#endif /* DMA_DMA_H_ */
