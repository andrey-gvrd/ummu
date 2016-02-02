#ifndef __UMMU_H__
#define __UMMU_H__

#include <stdint.h>	// uint_t

#define PAGE_LENGTH   1
#define PAGE_NUMBER   1
#define SECTOR_LENGTH 1
#define SECTOR_NUMBER 1

#if PAGE_LENGTH == 0 || PAGE_NUMBER == 0|| SECTOR_LENGTH == 0 || SECTOR_NUMBER == 0
#error "Define IC parameters first."
#endif

typedef enum {
  BT_Header = 1,
  BT_Data
} BufferType_t;

int32_t uMMU_Save(BufferType_t type, uint8_t *buffer);
int32_t uMMU_Load(BufferType_t type, uint8_t *buffer);

typedef struct {
	uint32_t freeSectorCount;
	uint32_t freePageCount;
} uMMU_State_t;

uMMU_State_t uMMU_State(uint8_t sectorNumber);

#endif // __UMMU_H__
