/*
 * ota.c
 *
 * Created: 2016-01-11 11:09:25 AM
 *  Author: Flower
 */ 

#include "sam.h"

#define OTA_SIGNATURE 0x4650424c

void flashWrite(const volatile void* address, const volatile void* data, uint32_t size) {
	const uint32_t pageSizes[] = { 8, 16, 32, 64, 128, 256, 512, 1024 };
	const uint32_t PAGE_SIZE = pageSizes[NVMCTRL->PARAM.bit.PSZ];
	const uint32_t ROW_SIZE = PAGE_SIZE * 4;
	const uint32_t NUM_PAGES = NVMCTRL->PARAM.bit.NVMP;

	size = (size + 3) / 4; // size <-- number of 32-bit integers
	volatile uint32_t* src_addr = (volatile uint32_t*) data;
	volatile uint32_t* dst_addr = (volatile uint32_t*) address;

	// Disable automatic page write
	NVMCTRL->CTRLB.bit.MANW = 1;
	
	// Do writes in pages
	while (size) {
		// if we are at the beginning of a row, erase it first
		if ((uint32_t) dst_addr % ROW_SIZE == 0) {
			NVMCTRL->ADDR.reg = ((uint32_t) dst_addr) / 2;
			NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
			while (!NVMCTRL->INTFLAG.bit.READY) { }
		}
		
		// Execute "PBC" Page Buffer Clear
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
		while (NVMCTRL->INTFLAG.bit.READY == 0) { }

		// Fill page buffer
		uint32_t i;
		for (i=0; i<(PAGE_SIZE/4) && size; i++) {
			*dst_addr = *src_addr;
			src_addr++;
			dst_addr++;
			size--;
		}

		// Execute "WP" Write Page
		NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
		while (NVMCTRL->INTFLAG.bit.READY == 0) { }
	}
	
}

void check_copy_new_application() {
	uint32_t otaSignature = *(uint32_t*) 0x3FF00;
	if (otaSignature != OTA_SIGNATURE) {
		return;
	}
	int32_t address = *(uint32_t*) 0x3FF04;
	int32_t size = *(uint32_t*) 0x3FF08;
	if (address <= 0) {
		return;
	}
	volatile uint32_t *src_addr = (volatile uint32_t*) address;
	volatile uint32_t *dst_addr = (volatile uint32_t*) 0x002000;
	flashWrite(dst_addr, src_addr, size);
	uint32_t clear = 0xFFFFFFFF;
	flashWrite((void*) 0x3FF00, &clear, 4);
}
