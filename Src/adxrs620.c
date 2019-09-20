#include "adxrs620.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "tim.h"
#include "can.h"
#include "task.h"
#include "spi.h"
#include "uc_memory.h"
#include "math.h"

void GyroTaskCreate(void);
void GyroTask(void *p);
void read_gyro_data( uint8_t* data);
void write_gyro_data(uint8_t* data);
void calibrate_imu(int n);

SemaphoreHandle_t Gyro_sem;

BaseType_t GYRO_t= pdFALSE;
TaskHandle_t task_gyro_h;

uint8_t raw_gyro_data[2]={2};
volatile float raw_gyro,gyro = 0;
float IntegralYaw= 0;
volatile float offset_gyro = 0;

void GyroTaskCreate(void)
{
  Gyro_sem = xSemaphoreCreateBinary();

  GYRO_t = xTaskCreate(GyroTask,
              "Gyro_acquisition",
              2048,
              (void*)1 ,
               10,
              &task_gyro_h
              );
  if(GYRO_t==!pdPASS)
  {
      vTaskDelete(task_gyro_h);
  }
 

}

void GyroTask(void *p)
{
  uint8_t tx_data[2]={0xef,0xfe};
  offset_gyro = FEE_ReadDataFloat(0x00);
  if(isnan(offset_gyro))
    offset_gyro = 0;
  write_gyro_data(tx_data);  //reload default parameters
  int cnt = 0;
  float tmp=0;
  for(;;)
  {
    if(pdTRUE==xSemaphoreTake(Gyro_sem,portMAX_DELAY))
    {
     read_gyro_data(raw_gyro_data);
     raw_gyro =((float)(((uint16_t)raw_gyro_data[0]<<8)|raw_gyro_data[1])- 32768) * (2.5/32768) /0.007;
     raw_gyro -= offset_gyro;
     tmp += raw_gyro;
     
     if(cnt ==10)
     {
        cnt = 0;
        gyro = tmp/10;
        tmp = 0;
        //CAN1_Send_Msg(0x80, (int16_t)(gyro*100), 0, 0, 0, 1);
       CAN1_Send_float(0x80, gyro, 0, 1);
        IntegralYaw += gyro * 0.001;
     }
     if(raw_gyro_data[0]==0 && raw_gyro_data[1]==0)
     {
       HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET );
     }
     
     cnt++;
    }
  }
}

void read_gyro_data(uint8_t* data)
{
    uint8_t tx_data[2]={0xdf,0xdf};
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET );    //start the conversion
    Delayus(10); //200ns
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET ); 
  
    //while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET){}
    
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET );      // pull down cs bit
    Delayus(1); //200ns
    HAL_SPI_TransmitReceive(&hspi1, tx_data, data, 2, 10 );
    Delayus(1); //200ns
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET ); // end of transfer
}

void write_gyro_data(uint8_t* data)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET );      // pull down cs bit
    Delayus(1); //100ns
    HAL_SPI_Transmit(&hspi1,  (uint8_t*)(&data), 2, 2);
    Delayus(1); //200ns
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET ); // end of transfer
}
  
void calibrate_gyro(int n)
{
    int64_t tmp=0;
    for(int i = 0; i< n; i++)
    {
        Delayus(1000);
        if( i%1000 == 0 ){
            HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
        }
        read_gyro_data(raw_gyro_data);
        tmp += (((uint16_t)raw_gyro_data[0]<<8)|raw_gyro_data[1])- 32768;
    }
    offset_gyro = ((float)tmp/n) * (2.5/32768) /0.007;

    FEE_WriteDataFloat(0x00, offset_gyro);
}