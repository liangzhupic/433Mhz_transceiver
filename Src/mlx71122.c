#include "mlx71122.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "tim.h"
#include "can.h"
#include "task.h"
#include "spi.h"
#include "stdlib.h"

unsigned int reverseBits(unsigned int num);

void init_mlx71122_fsk(void)
{
    uint8_t data[] = {0xe8, 0xd0};
    write_mlx71122_data(data); // r0
    data[0] = 0x8f;
    data[1] = 0x42;
    write_mlx71122_data(data); //r1
    data[0] = 0x0e;
    data[1] = 0x04;
    write_mlx71122_data(data); //r2
    data[0] = 0x12;
    data[1] = 0x16;
    write_mlx71122_data(data); //r3
    data[0] = 0xaa;
    data[1] = 0xca;
    write_mlx71122_data(data); //r5
    
    for(int i = 0; i < 8; i++)
    {
      read_mlx71122_data(data, i);
    }
}

void read_mlx71122_data(uint8_t* data, uint8_t adr)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET );
    uint8_t tx_data=0;
    tx_data |= (adr<<1)|(adr<<5);
    tx_data |= 0x11;

    Delayus(1); //200ns
    HAL_SPI_Transmit(&hspi2, &tx_data, 1, 10 );
    Delayus(1); //200ns
    
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET ); // end of transfer
    Delayus(1); //200ns
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET );
    
    HAL_SPI_Receive(&hspi2, data, 2, 10 );
    data[0] = reverseBits(data[0]);
    data[1] = reverseBits(data[1]);
}

void write_mlx71122_data(uint8_t* data)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET );
  
    Delayus(1); //100ns
    HAL_SPI_Transmit(&hspi2, data, 2, 10);
    Delayus(1); //200ns
  
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET ); // end of transfer
    Delayus(1); //200ns
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET );
}



unsigned int reverseBits(unsigned int num) 
{ 
    unsigned int count = 8 - 1; 
    unsigned int reverse_num = num; 
      
    num >>= 1;  
    while(num) 
    { 
       reverse_num <<= 1;        
       reverse_num |= num & 1; 
       num >>= 1; 
       count--; 
    } 
    reverse_num <<= count; 
    return reverse_num; 
} 