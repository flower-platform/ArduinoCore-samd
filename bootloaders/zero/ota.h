/*
 * ota.h
 *
 * Created: 2016-01-11 11:09:25 AM
 *  Author: Claudiu Matei
 */ 

#ifndef _OTA_H_
#define _OTA_H_
 
#define OTA_SIGNATURE 0x4650424c

void check_copy_new_application();

void flashWrite(const volatile void* address, const volatile void* data, uint32_t size);
	
#endif // _OTA_H_
