#include "mmu.h"
#include "dbg_print.h"

#include <stdint.h>  // uint_t
#include <stdbool.h> // bool
#include <string.h>  // memset
#include <stdio.h>   // sprintf

typedef enum {
    PS_Clean = 0,
    PS_Dirty = !PS_Clean
} PageState_t;

static int32_t Sector_Find(PageState_t wantedPageState);
static int32_t Page_Find  (PageState_t wantedPageState);

static PageState_t Page_Get_State(uint32_t pageAddress);

static int32_t LoadFromMemory_Header(uint8_t *buffer);
static int32_t LoadFromMemory_Data(uint8_t *buffer);

static void Memory_Save(uint8_t *buffer, size_t buffer_len, uint32_t pageAddress);
static void Memory_Load(uint8_t *buffer, size_t buffer_len, uint32_t pageAddress);
static void Memory_Sector_Erase(uint32_t pageAddress);

static uint32_t Sector_Get_FreeCount(void);
static uint32_t Page_Get_FreeCount(uint8_t sectorNumber);

#define SECTOR_ADDRESS(sector)      ((sector) * SECTOR_LENGTH * PAGE_LENGTH)
#define PAGE_ADDRESS(sector, page)  (SECTOR_ADDRESS(sector) + (page) * PAGE_LENGTH)

static int32_t currentSector = 0;
static int32_t currentPage = 0;

// Figure out whether it a header or a data buffer.
// In the first case it looks for a new sector,
// In the second it looks for a next page in a current sector.l
int32_t uMMU_Save(BufferType_t type, uint8_t *buffer)
{
    bool sectorsLeft = false;
    bool pagesLeft = false;

    switch (type) {
        case BT_Header: {
            currentSector = Sector_Find(PS_Clean);
            if (currentSector >= 0) {
                sectorsLeft = true;
                currentPage = 0;
            }
        } break;
        case BT_Data: {
            currentPage = Page_Find(PS_Clean);
            if (currentPage >= 0) {
                pagesLeft = true;
            }
        } break;
        default: {
            return -1;
        } break;
    }

    dbg_printf("MMU: Curr. Sector: %u\n", currentSector);
    dbg_printf("MMU: Curr. Page: %u\n",   currentPage);

    if (sectorsLeft || pagesLeft) {
        uint32_t pageAddress = PAGE_ADDRESS(currentSector, currentPage);
        Memory_Save(buffer, PAGE_LENGTH, pageAddress);
        return 0;
    }

    return -1;
}

int32_t uMMU_Load(BufferType_t type, uint8_t *buffer)
{
    switch (type) {
        case BT_Header: return LoadFromMemory_Header(buffer);
        case BT_Data:   return LoadFromMemory_Data(buffer);
        default:        return -1;
    }
}

uMMU_State_t uMMU_State(uint8_t sector_number)
{
    uMMU_State_t uMMU_State = {
        .freeSectorCount = Sector_Get_FreeCount(),
        .freePageCount   = Page_Get_FreeCount(sector_number)
    };

    return uMMU_State;
}

// Check the first page of every sector. If there is a non-empty page,
// save sector as current, fetch the page and mark it to be cleaned.
static int32_t LoadFromMemory_Header(uint8_t *buffer)
{
    SEND_STR("MMU: Load header\n");
    currentSector = Sector_Find(PS_Dirty);
    currentPage = 0;  // currentPage has to be reset with the currentSector

    dbg_printf("MMU: Curr. Sector: %u\n", currentSector);
    dbg_printf("MMU: Curr. Page: %u\n",   currentPage);

    if (currentSector >= 0) {
        uint32_t pageAddress = SECTOR_ADDRESS(currentSector);
        Memory_Load(buffer, PAGE_LENGTH, pageAddress);
        currentPage++;
        return 0;
    }

    return -1;
}

static int32_t LoadFromMemory_Data(uint8_t *buffer)
{
    SEND_STR("MMU: Load data\n");
    uint32_t pageAddress = PAGE_ADDRESS(currentSector, currentPage);

    dbg_printf("MMU: Curr. Sector: %u\n", currentSector);
    dbg_printf("MMU: Curr. Page: %u\n",   currentPage);

    if (Page_Get_State(pageAddress) == PS_Dirty) {
        Memory_Load(buffer, PAGE_LENGTH, pageAddress);
        currentPage++;
        return 0;
    }

    // Clean page is found in a drity sector --
    // assuming all preceding pages have been uploaded already --
    // erase the sector.
    SEND_STR("MMU: Erasing sector\n");
    Memory_Sector_Erase(pageAddress);

    return -1;
}

static uint32_t Sector_Get_FreeCount(void)
{
    uint32_t freeSectorCount = 0;

    for (uint32_t sectorNumber = 0; sectorNumber < SECTOR_NUMBER; ++sectorNumber) {
        uint32_t pageAddress = SECTOR_ADDRESS(sectorNumber);
        if (Page_Get_State(pageAddress) == PS_Clean) {

            dbg_printf("MMU: Sector %u is free\n", sectorNumber);

            freeSectorCount++;
        }
    }

    return freeSectorCount;
}

static uint32_t Page_Get_FreeCount(uint8_t sectorNumber)
{
    uint32_t freePageCount = 0;
    const uint32_t startPageNumber = sectorNumber * SECTOR_LENGTH;
    const uint32_t endPageNumber = startPageNumber + SECTOR_LENGTH;

    for (uint32_t pageNumber = startPageNumber; pageNumber < endPageNumber; ++pageNumber) {
        uint32_t pageAddress = PAGE_ADDRESS(sectorNumber, pageNumber);
        if (Page_Get_State(pageAddress) == PS_Clean) {
            freePageCount++;
        }
    }

    return freePageCount;
}

static int32_t Sector_Find(PageState_t wantedPageState)
{
    for (uint32_t sectorNumber = 0; sectorNumber < SECTOR_NUMBER; ++sectorNumber) {
        uint32_t pageAddress = SECTOR_ADDRESS(sectorNumber);

        dbg_printf("MMU: Trying Addr.: %u\n", pageAddress);

        if (Page_Get_State(pageAddress) == wantedPageState) {
            return sectorNumber;
        }
    }

    return -1;
}

static int32_t Page_Find(PageState_t wantedPageState)
{
    for (uint32_t pageNumber = 0; pageNumber < SECTOR_LENGTH; ++pageNumber) {
        uint32_t pageAddress = PAGE_ADDRESS(currentSector, pageNumber);

        dbg_printf("MMU: Trying Addr.: %u\n", pageAddress);

        if (Page_Get_State(pageAddress) == wantedPageState) {
            return pageNumber;
        }
    }

    return -1;
}

static PageState_t Page_Get_State(uint32_t pageAddress)
{
    PageState_t pageState;
    uint8_t byte;

    Memory_Load(&byte, 1, pageAddress);
    pageState = byte == 0xFF ? PS_Clean : PS_Dirty;

    return pageState;
}

static void Memory_Save(uint8_t *buffer, size_t buffer_len, uint32_t pageAddress)
{
    //
}

static void Memory_Load(uint8_t *buffer, size_t buffer_len, uint32_t pageAddress)
{
    //
}

static void Memory_Sector_Erase(uint32_t pageAddress)
{
    //
}
