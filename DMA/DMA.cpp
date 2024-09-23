/*
 * DMA.cpp
 *
 *  Created on: 30 ago. 2024
 *      Author: Gustavo Alpern
 *      V0.1
 */
#include "DMA.h"

void (*intACallbacks[25])(void);
void (*intBCallbacks[25])(void);


DMA::DMA(){

	SYSCON->SYSAHBCLKCTRL0 |= 1<<DMA_SYSAHBCLKCTRL_SHIFT;
	SYSCON->PRESETCTRL0 |=	1<<DMA_SYSAHBCLKCTRL_SHIFT;
	DMA0->CTRL |= 1 << 0;
	DMA0->SRAMBASE |= (uint32_t)&m_DMACH;
	for(int i=0; i < MAX_CHANNELS - 1; i++){
		m_canalesActivos[i] = false;
	}
}

DMA::~DMA(){
	DMA0->CTRL &=~(1<<0);
	SYSCON->SYSAHBCLKCTRL0 &= ~(1<<DMA_SYSAHBCLKCTRL_SHIFT);
	SYSCON->PRESETCTRL0 &=	~(1<<DMA_SYSAHBCLKCTRL_SHIFT);
	for(int i=0; i < MAX_CHANNELS - 1; i++){
		if(m_canalesActivos[i]){
			apagarCanal(i);
		}
	}
}

uint8_t DMA::crearDescriptorConfig(uint32_t * Configptr, bool recarga, uint8_t tamTransferencias, uint8_t incremOrigen, uint8_t incremDest, uint32_t cantTransferencias, bool enableIntA, bool enableIntB){

	if(Configptr == nullptr|| tamTransferencias > CUATRO_BYTES || tamTransferencias > CUATRO_BYTES || incremOrigen > X4_INC || incremDest > X4_INC || cantTransferencias > MAXCANTTRANSFERENCIAS){
		return ERROR_PARAMETROS;
	}

	*Configptr =1 | (recarga << 1) | (1 << 3) | (enableIntA << 4) | (enableIntB << 5) | (tamTransferencias << 8) | (incremOrigen << 12) | (incremDest << 14) | (cantTransferencias << 16);

	return OK;
}

uint8_t DMA::crearDescriptor(Descriptor * descriptorptr, bool recarga, uint8_t tamTransferencias, uint8_t incremOrigen, uint8_t incremDest, uint32_t cantTransferencias, bool enableIntA, bool enableIntB, void * origenAddr, void * destAddr, Descriptor * siguienteDesc){

	uint32_t Configuracion;
	uint32_t incrementoOrigen = 0;
	uint32_t incrementoDest = 0;

	if(descriptorptr == nullptr || origenAddr == nullptr || destAddr == nullptr || (siguienteDesc == nullptr && recarga)){
		return ERROR_PARAMETROS;
	}

	if(((int)descriptorptr & 31) != 0){
		return ERROR_NO_ALINEADO;
	}

	if((((int)origenAddr & 7) != 0 && incremOrigen == X1_INC) || (((int)origenAddr & 15) != 0 && incremOrigen == X2_INC) || (((int)origenAddr & 31) != 0 && incremOrigen == X4_INC)){
		return ERROR_NO_ALINEADO;
	}

	if((((int)destAddr & 7) != 0 && incremDest == X1_INC) || (((int)destAddr & 15) != 0 && incremDest == X2_INC) || (((int)destAddr & 31) != 0 && incremDest == X4_INC)){
		return ERROR_NO_ALINEADO;
	}

	if(crearDescriptorConfig(&Configuracion, recarga, tamTransferencias, incremOrigen, incremDest, cantTransferencias, enableIntA, enableIntB) != OK){
		return ERROR_PARAMETROS;
	}


	switch(incremOrigen){
	case X1_INC:
		incrementoOrigen = cantTransferencias;
		break;

	case X2_INC:
		incrementoOrigen = cantTransferencias * 2;
		break;

	case X4_INC:
		incrementoOrigen = cantTransferencias * 4;
		break;

	default:
		incrementoOrigen = 0;
		break;
	}

	switch(incremDest){
	case X1_INC:
		incrementoDest = cantTransferencias;
		break;

	case X2_INC:
		incrementoDest = cantTransferencias * 2;
		break;

	case X4_INC:
		incrementoDest = cantTransferencias * 4;
		break;

	default:
		incrementoDest = 0;
		break;
	}

	descriptorptr->TRNCFG = Configuracion;
	descriptorptr->SRCDATA = (void *)((uint32_t)origenAddr + incrementoOrigen);
	descriptorptr->DSTDATA = (void *)((uint32_t)destAddr + incrementoDest);
	descriptorptr->NXTDSC = (uint32_t) siguienteDesc;

	return OK;
}

uint8_t DMA::crearDescriptor(Descriptor * descriptorptr, uint32_t ConfiguracionDescriptor, void * origenAddr, void * destAddr, Descriptor * siguienteDesc){

	uint32_t incrementoOrigen = 0;
	uint32_t incrementoDest = 0;

	if(descriptorptr == nullptr || origenAddr == nullptr || destAddr == nullptr || (siguienteDesc == nullptr && (ConfiguracionDescriptor & (1 << 1)))){
		return ERROR_PARAMETROS;
	}

	if(((int)descriptorptr & 31) != 0){
		return ERROR_NO_ALINEADO;
	}

	if((((int)origenAddr & 7) != 0 && (ConfiguracionDescriptor & (3<<12)) >> 12 == X1_INC) || (((int)origenAddr & 15) != 0 && (ConfiguracionDescriptor & (3<<12)) >> 12 == X2_INC) || (((int)origenAddr & 31) != 0 && (ConfiguracionDescriptor & (3<<12)) >> 12 == X4_INC)){
		return ERROR_NO_ALINEADO;
	}

	if((((int)destAddr & 7) != 0 && (ConfiguracionDescriptor & (3<<14)) >> 14 == X1_INC) || (((int)destAddr & 15) != 0 && (ConfiguracionDescriptor & (3<<14)) >> 14 == X2_INC) || (((int)destAddr & 31) != 0 && (ConfiguracionDescriptor & (3<<14)) >> 14 == X4_INC)){
		return ERROR_NO_ALINEADO;
	}

	switch((ConfiguracionDescriptor & (3<<12)) >> 12){
	case X1_INC:
		incrementoOrigen = ((ConfiguracionDescriptor & (10<<16)) >> 16);
		break;

	case X2_INC:
		incrementoOrigen = ((ConfiguracionDescriptor & (10<<16)) >> 16) * 2;
		break;

	case X4_INC:
		incrementoOrigen = ((ConfiguracionDescriptor & (10<<16)) >> 16) * 4;
		break;

	default:
		incrementoOrigen = 0;
		break;
	}

	switch((ConfiguracionDescriptor & (3 << 14)) >> 14){
	case X1_INC:
		incrementoDest = ((ConfiguracionDescriptor & (10 << 16)) >> 16);
		break;

	case X2_INC:
		incrementoDest = ((ConfiguracionDescriptor & (10 << 16)) >> 16) * 2;
		break;

	case X4_INC:
		incrementoDest = ((ConfiguracionDescriptor & (10 << 16)) >> 16) * 4;
		break;

	default:
		incrementoDest = 0;
		break;
	}

	descriptorptr->TRNCFG = ConfiguracionDescriptor;
	descriptorptr->SRCDATA = (void *)((uint32_t)origenAddr + incrementoOrigen);
	descriptorptr->DSTDATA = (void *)((uint32_t)destAddr + incrementoDest);
	descriptorptr->NXTDSC = (uint32_t) siguienteDesc;

	return OK;
}

uint8_t DMA::prenderCanal(Descriptor* descriptorptr, uint8_t canal, bool enReqPerif, uint8_t prioridad){

	if(canal > MAX_CHANNELS - 1 || prioridad > MAX_PRIORITY || descriptorptr == nullptr){
		return ERROR_PARAMETROS;
	}

	if(((int)descriptorptr & 31) != 0){
		return ERROR_NO_ALINEADO;
	}

	if(m_canalesActivos[canal]){
		return ERROR_CANAL_USADO;
	}

	DMA0->CHANNEL[canal].CFG |= enReqPerif | (prioridad<<16);
	DMA0->CHANNEL[canal].XFERCFG = descriptorptr->TRNCFG;
	m_DMACH.CHANNEL[canal].TRNCFG = descriptorptr->TRNCFG;
	m_DMACH.CHANNEL[canal].SRCDATA = descriptorptr->SRCDATA;
	m_DMACH.CHANNEL[canal].DSTDATA = descriptorptr->DSTDATA;
	m_DMACH.CHANNEL[canal].NXTDSC= descriptorptr->NXTDSC;
	m_canalesActivos[canal] = true;
	DMA0->COMMON->SETVALID |= 1 << canal;
	DMA0->COMMON->ENABLESET |= 1 << canal;

	return OK;
}

uint8_t DMA::apagarCanal(uint8_t canal){

	if(canal > MAX_CHANNELS - 1){
		return ERROR_PARAMETROS;
	}

	DMA0->COMMON->ENABLECLR |=1 << canal;

	return OK;
}

uint8_t DMA::forzarEnvio(uint8_t canal){

	if(canal > MAX_CHANNELS - 1){
		return ERROR_PARAMETROS;
	}

	DMA0->CHANNEL[canal].XFERCFG |= 1 << 2;

	return OK;
}

uint8_t DMA::enableINT(){

	NVIC->ISER[0] |= (1 << 20);
	return OK;
}
uint8_t DMA::disableINT(){

	NVIC->ICER[0] |= (1 << 20);

	for(int i = 0; i<MAX_CHANNELS - 1; i++){
		disableCHINT(i);
	}

	return OK;
}
uint8_t DMA::enableCHINT(uint8_t canal){

	if(canal>MAX_CHANNELS-1){
		return ERROR_PARAMETROS;
	}

	DMA0->COMMON->INTENSET |= 1 << canal;

	return OK;
}
uint8_t DMA::disableCHINT(uint8_t canal){

	if(canal>MAX_CHANNELS-1){
		return ERROR_PARAMETROS;
	}

	DMA0->COMMON->INTENCLR |= 1 << canal;

	return OK;
}
uint8_t DMA::instalarCallBackA(uint8_t canal, void (*callbackA) (void)){
	if(canal>MAX_CHANNELS-1 || callbackA == nullptr){
		return ERROR_PARAMETROS;
	}
	intACallbacks[canal] = callbackA;
	return OK;
}
uint8_t DMA::instalarCallBackB(uint8_t canal, void (*callbackB) (void)){
	if(canal>MAX_CHANNELS-1 || callbackB == nullptr){
		return ERROR_PARAMETROS;
	}
	intBCallbacks[canal] = callbackB;
	return OK;
}

void DMA_IRQHandler(void){

	uint32_t funcsel=0;

	if(DMA0->COMMON->INTA > 0){
		while(!((DMA0->COMMON->INTA >> funcsel) & 1)){
			funcsel++;
		}
		if(intACallbacks[funcsel] != nullptr){
			intACallbacks[funcsel]();
		}
		DMA0->COMMON->INTA |= 1 << funcsel;
	}

	funcsel=0;

	if(DMA0->COMMON->INTB > 0){
		while(!((DMA0->COMMON->INTB >> funcsel) & 1)){
			funcsel++;
		}
		if(intBCallbacks[funcsel] != nullptr){
			intBCallbacks[funcsel]();
		}
		DMA0->COMMON->INTB |= 1 << funcsel;
	}
}
