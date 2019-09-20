#ifndef __mlx71122__
#define __mlx71122__
#include "FreeRTOS.h"
#include "semphr.h"


void read_mlx71122_data(uint8_t* data, uint8_t adr);
void write_mlx71122_data(uint8_t* data);
void init_mlx71122_fsk(void);

#endif