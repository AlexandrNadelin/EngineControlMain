#ifndef __DEVICE_DATA_H
#define __DEVICE_DATA_H
#include "main.h"

#define CRANKSHAFT_ALL_POS_NUM   (uint8_t)60/*Вместе с непросверленным отверстием*/
#define CRANKSHAFT_POS_NUM       (uint8_t)59
#define CAMSHAFT_POS_NUM         (uint8_t)6

typedef struct{	
	uint8_t misfireCylinder[6];//Пропуск зажигания цилиндра (отрицательное ускорение) // 0x01- ошибка, 0x00 - ошибки не было
	
	uint8_t fireErrorCylinder[6];//Ошибки искрообразования цилиндра (не в свое время) // 0x01- ошибка, 0x00 - ошибки не было, 0x81 - ошибка началась с этого цилиндра
	
	uint32_t misfireCounter;//счетчик ошибок пропусков зажигания (отрицательные ускорения)
	uint32_t fireErrorCounter;//счетчик ошибок искрообразования (по всем цилиндрам, а не только с которого началось)
	
	float cylinder_1_Time;//в миллисекундах//определение торможения (10-24 метка)
	float cylinder_2_Time;//в миллисекундах//определение торможения (30-44 метка)
	float cylinder_3_Time;//в миллисекундах//определение торможения (50-4 метка)
	float cylinder_4_Time;//в миллисекундах//определение торможения (50-4 метка)
	float cylinder_5_Time;//в миллисекундах//определение торможения (30-44 метка)
	float cylinder_6_Time;//в миллисекундах//определение торможения (10-24 метка)
	
	uint32_t crankshaftTicks[CRANKSHAFT_POS_NUM];//Каленвал //текущее время в тиках // Внимание !!! - два оборота коленвала - один оборот распредвала //вообще на коленвале 59 импульсов за оборот
	uint32_t camshaftTicks[CAMSHAFT_POS_NUM+1];  //Распредвал //текущее время в тиках //7 зубов, между 1 и 2 20 градусов, между 1 и 3 - 60 градусов, 1 и 4 - 120, 1 и 5 - 180, 1 и 6 -240, 1 и 7 - 300
	
	uint32_t crankshaftTickSpans[CRANKSHAFT_POS_NUM];//Каленвал //текущий промежуток времени в тиках // Внимание !!! - два оборота коленвала - один оборот распредвала //вообще на коленвале 59 импульсов за оборот
	uint32_t camshaftTickSpans[CAMSHAFT_POS_NUM+1];  //Распредвал //текущий промежуток времени в тиках //7 зубов, между 1 и 2 20 градусов, между 1 и 3 - 60 градусов, 1 и 4 - 120, 1 и 5 - 180, 1 и 6 -240, 1 и 7 - 300
	
	float crankshaftTimeSpans[CRANKSHAFT_POS_NUM];//Каленвал //текущий промежуток времени в миллисекундах // Внимание !!! - два оборота коленвала - один оборот распредвала //вообще на коленвале 59 импульсов за оборот
	float camshaftTimeSpans[CAMSHAFT_POS_NUM+1];  //Распредвал //текущий промежуток времени в миллисекундах //7 зубов, между 1 и 2 20 градусов, между 1 и 3 - 60 градусов, 1 и 4 - 120, 1 и 5 - 180, 1 и 6 -240, 1 и 7 - 300
			
	uint8_t isCylinderFiredMask;
	uint8_t currentCylinder;//номер цилиндра в формате 1-6 (не 0-5)
	uint8_t crankshaftFiringAllowed;
	uint8_t reserve;//uint8_t currentCylinderForAccelerationCalc;
	
	uint8_t isDataProcessAllowed;
	uint8_t crankshaftCurPos;
	uint8_t camshaftCurPos;
	uint8_t clearCounters;//Два Coil регистра 1- очистка счетчика ошибок пропусков зажигания 2 - очистка счетчика ошибок искрообразования(не в свое время)
	
}DeviceData;

#endif

