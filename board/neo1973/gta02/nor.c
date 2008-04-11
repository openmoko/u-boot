#include <common.h>


flash_info_t flash_info[CFG_MAX_FLASH_BANKS];


ulong flash_init (void)
{
	flash_info[0].sector_count = 1;
	flash_info[0].start[0] = 0x18000000;
	memset(flash_info[0].protect, 0, CFG_MAX_FLASH_SECT);
	return flash_info[0].size = 0x200000;
}


void flash_print_info (flash_info_t * info)
{
}


int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	return ERR_PROG_ERROR;
}


int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	return ERR_PROG_ERROR;
}
