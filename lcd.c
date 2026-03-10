#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "lcd.h"
#include <stdlib.h>
#include "Arial12x12.h"

/*----------------------------------------------------------------------------
 *      ThLCD 1 'ThLCD_Name': Sample ThLCD
 *---------------------------------------------------------------------------*/
 
osMessageQueueId_t queueLCD;
 
osThreadId_t tid_ThLCD;                        // ThLCD id
 
void ThLCD (void *argument);                   // ThLCD function
 
void delay(uint32_t n_microsegundos);
void LCD_wr_data(unsigned char data);
void LCD_wr_cmd(unsigned char data);
void initPIN_Reset(void);
void initPIN_CS(void);
void initPIN_A0(void);
static void mySPI_callback(uint32_t event);

extern osThreadId_t tid_ThLCD;

static TIM_HandleTypeDef htim7;
void delay(uint32_t n_microsegundos);

extern ARM_DRIVER_SPI Driver_SPI1;
static ARM_DRIVER_SPI* SPIdrv = &Driver_SPI1;

static ARM_SPI_SignalEvent_t SPI_Event;
static ARM_SPI_STATUS SPI_Status;

static unsigned char buffer[512];

int Init_LCD (void) {
  
  queueLCD = osMessageQueueNew(QUEUE_LCD_SIZE, sizeof(MSG_LCD), NULL);
  
  tid_ThLCD = osThreadNew(ThLCD, NULL, NULL);
  if (tid_ThLCD == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void ThLCD (void *argument) {
  
  uint16_t offset;
  
  MSG_LCD msg;
  uint32_t status;
  
  uint8_t posicion;
  
  uint8_t select;
  
  osStatus_t statusQ; 
  
  clk_enable();
  LCD_reset();
  LCD_init();
  
  LCD_symbolToLocalBuffer(NULL, NULL, 1, 0);
  LCD_update();
	
  while (1) {
    
    status = osThreadFlagsGet();
    if(status == 2){
      
      resetPins();
      LCD_reset();
      LCD_init();
      
      LCD_symbolToLocalBuffer(NULL, NULL, 1, 0);
      LCD_update();
      
    }
		statusQ = osMessageQueueGet(queueLCD, &msg, NULL, 100);
    
    if(statusQ == osOK){
    
			LCD_symbolToLocalBuffer(NULL, NULL, msg.Reset, 0);
    
			posicion = LCD_center(msg.Buffer);
			
			if(!posicion){
				LCD_symbolToLocalBuffer(msg.Linea, NULL, 4, 0);
			}
			
			LCD_symbolToLocalBuffer(msg.Linea,msg.Buffer[0], msg.Linea + 2, posicion);
			
			posicion = posicion + Arial12x12[25*(msg.Buffer[0] - ' ')];
				
				for (int i = 1; msg.Buffer[i] != '\0'; i++) {
					posicion = posicion + Arial12x12[25*(msg.Buffer[i] - ' ')];
					if(posicion + 3 < 128){
						LCD_symbolToLocalBuffer(msg.Linea,msg.Buffer[i], 0, 0);
					}
				}
				if(msg.selec != 0){
    
					select = LCD_sel(msg.Buffer, msg.selec);
      
					offset = 25*(msg.Buffer[msg.selec] - ' ');
			
					for(int i = 0; i < 5 ;i++){
						LCD_write(select + i, 3, 0x04 + Arial12x12[offset + 4 + i*2]);
					}
				}
			LCD_update();
		}
			; // Insert ThLCD code here...
			osThreadYield();                            // suspend ThLCD
  }
}


void LCD_init(void){
  
  LCD_wr_cmd(0xAE);
  LCD_wr_cmd(0xA2);
  LCD_wr_cmd(0xA0);
  LCD_wr_cmd(0xC8);
  LCD_wr_cmd(0x22);
  LCD_wr_cmd(0x2F);
  LCD_wr_cmd(0x40);
  LCD_wr_cmd(0xAF);
  LCD_wr_cmd(0x81);
  LCD_wr_cmd(0x0A); //valor contraste, pone ?? en la practica
  LCD_wr_cmd(0xA4);
  LCD_wr_cmd(0xA6);
}

//--------------------LCD_symbolToLocalBuffer_L1---------------------
//Coge un simbolo y lo mete al buffer
void LCD_symbolToLocalBuffer(uint8_t line,uint8_t symbol, uint8_t reset, uint16_t posicionL){ //la posicion L da es el valor del bit mas a la izquierda
  uint8_t i, value1, value2 = 0;
  uint16_t offset = 0;

  if(symbol == NULL){
    
    symbol = ' ';
  }
  
  offset = 25*(symbol - ' ');
  static uint16_t posicionL1;
  static uint16_t posicionL2;
	
	if(reset == 4){
		if(!line){
			posicionL1 = 0;
		}else{
			posicionL2 = 0;
		}
	}else{
		if(posicionL != 0){
			if(!line){
			posicionL1= posicionL - 1;
			}else{
			posicionL2= posicionL - 1;
			}
		}
		
		if(reset == 2){
		
		for(int j=0;j<256;j++){
			buffer[j] = 0x00;
	 }
	 }else if(reset == 3){
		
		for(int j=256;j<512;j++){
			buffer[j] = 0x00;
	 }
	}
	if(reset == 1){
		posicionL1 = 0;
		posicionL2 = 0;
		for(int j=0;j<512;j++){
		buffer[j] = 0x00;
	 }
	}else{
		if(!line){
			for (i=0;i<12;i++){
				value1 = Arial12x12[offset+i*2+1];
				value2 = Arial12x12[offset+i*2+2];
				buffer[i + 0 + posicionL1] = value1;
				buffer[i + 128 +  posicionL1] = value2;
			}
			posicionL1 = posicionL1 + Arial12x12[offset];
			}else{
				for (i=0;i<12;i++){
					value1 = Arial12x12[offset+i*2+1];
					value2 = Arial12x12[offset+i*2+2];
					buffer[i + 256 + posicionL2]=value1;
					buffer[i + 384 +  posicionL2]=value2;
				}
				posicionL2 = posicionL2 + Arial12x12[offset];
			}
		}
	}
}
//------------------------Valor posicion para centrar

uint8_t LCD_center(char text[]){
  
  uint8_t posicion = 0;
  uint16_t offset = 0;
  
  for (int i = 0; (text[i] != '\0') && ((posicion + Arial12x12[offset]) < 128); i++) {
      offset = 25*(text[i] - ' ');
      posicion = posicion + Arial12x12[offset];
  }
  if(posicion >= 128){
		return 0;
	}
  posicion = (128 - posicion) / 2;
  
  return posicion;
}
//-----Busca la posicion de un valor escrito------------

uint8_t LCD_sel(char text[], uint8_t sel){
  
  uint8_t posicion;
  uint8_t select;
  uint16_t offset = 0;
  
  for (int i = 0; text[i] != '\0'; i++) {
      offset = 25*(text[i] - ' ');
      posicion = posicion + Arial12x12[offset];
      if(i == (sel - 1)){
        select = posicion;
      }
  }
  
  posicion = (128 - posicion) / 2;
  
  select = select + posicion;
  
  return select;
}
//-------------------------LCD BUFFER MANUAL----------------
void LCD_write(uint8_t columna, uint8_t pagina, uint8_t valor){
  
  buffer[columna + pagina * 128] = valor;
}



  //--------------------LCD ordenes------------------

void LCD_wr_data(unsigned char data){
  
  //GPIO_PIN_SET--> 1 --> Enciende  GPIO_PIN_RESET--> 0 --> Apaga
  uint32_t status;
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET); //CS->0
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13, GPIO_PIN_SET); //A0->1
  
  SPIdrv->Send(&data, 1); //el 1 es el numero de datos, en el enunciado dice 1
  
  status = osThreadFlagsWait(0x0001, osFlagsWaitAny, 0);
  if(status == 0){
    status = osThreadFlagsSet(tid_ThLCD, 0x0002);
  }
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_SET); //CS ->1
}
void LCD_wr_cmd(unsigned char cmd){

  uint32_t status;
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET); //CS->0
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13, GPIO_PIN_RESET); //A0->0
  
  SPIdrv->Send(&cmd, 1); //el 1 es el numero de datos, en el enunciado dice 1
  
  status = osThreadFlagsWait(0x0001, osFlagsWaitAny, 0);
  if(status == 0){
    status = osThreadFlagsSet(tid_ThLCD, 0x0002);
  }
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_SET); //CS ->1
}


void LCD_update(void)
{
 int i;
 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB0); // Página 0

 for(i=0;i<128;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00); // 4 bits de la parte baja de la dirección a 0
 LCD_wr_cmd(0x10); // 4 bits de la parte alta de la dirección a 0
 LCD_wr_cmd(0xB1); // Página 1

 for(i=128;i<256;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB2); //Página 2
 for(i=256;i<384;i++){
 LCD_wr_data(buffer[i]);
 }

 LCD_wr_cmd(0x00);
 LCD_wr_cmd(0x10);
 LCD_wr_cmd(0xB3); // Pagina 3


 for(i=384;i<512;i++){
 LCD_wr_data(buffer[i]);
 }
}

void LCD_reset(void){

//--------------------inicialización del bus SPI--------------------------
  __HAL_RCC_SPI3_CLK_ENABLE();
  
  SPIdrv->Uninitialize();
  SPIdrv->Initialize(mySPI_callback);
  SPIdrv->PowerControl(ARM_POWER_FULL);
  SPIdrv->Control(ARM_SPI_MODE_MASTER |  ARM_SPI_CPOL1_CPHA1 |  //el  MSB_LSB es el predeterminado, podria no ponerse
                      ARM_SPI_MSB_LSB | ARM_SPI_DATA_BITS(8),20000000);
  
  SPIdrv->Control(ARM_SPI_CONTROL_SS, ARM_SPI_SS_ACTIVE);

//--------------------Inicialización pines LCD------------------------
  initPIN_Reset();
  initPIN_CS();
  initPIN_A0();
//-------------------generación señal reset para el LCD---------------
  
  HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_6);
  delay(20);
  HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_6);
  delay(1000);
}

//---------------------------CALLBACK-------------------------

static void mySPI_callback(uint32_t event)
{
  uint32_t status;
  
  switch (event){
    
    case ARM_SPI_EVENT_TRANSFER_COMPLETE:
      status = osThreadFlagsSet(tid_ThLCD, 0x0001);
    break;
    case ARM_SPI_EVENT_DATA_LOST:
      status = osThreadFlagsSet(tid_ThLCD, 0x0003);
    break;
    case ARM_SPI_EVENT_MODE_FAULT:
      status = osThreadFlagsSet(tid_ThLCD, 0x0003);
    break;
    default:
      
    break;
  }
}


//------------------------Delay---------------------------------
void delay(uint32_t n_microsegundos){ // NOS DA EL DELAY QUE LE INDICAMOS EN MICROSEGUNDOS

  __HAL_RCC_TIM7_CLK_ENABLE();

  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 83;
  htim7.Init.Period = n_microsegundos-1;

  HAL_TIM_Base_Init(&htim7);
  HAL_TIM_Base_Start(&htim7);


  __HAL_TIM_CLEAR_FLAG(&htim7, TIM_FLAG_UPDATE);
  while((__HAL_TIM_GET_FLAG(&htim7, TIM_FLAG_UPDATE) == RESET)){
  }
  
  __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);
  
  HAL_TIM_Base_Stop(&htim7);
  __HAL_TIM_SET_COUNTER(&htim7,0);
    
  
}

//--------------------pines del LCD-----------
void initPIN_Reset(void){ // PA6
    GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  //__HAL_RCC_GPIOA_CLK_ENABLE();
  
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, GPIO_PIN_SET);
}
void initPIN_CS(void){ //PD14
    GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  //__HAL_RCC_GPIOD_CLK_ENABLE();
    
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, GPIO_PIN_RESET);
}
void initPIN_A0(void){ //PF13
    GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  //__HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13, GPIO_PIN_SET);
}
void resetPins(void){
  
  HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, 0);
  HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14, 0);
  HAL_GPIO_WritePin(GPIOF,GPIO_PIN_13, 0);
}

//-------------------------------------------
void clk_enable(void){
  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
}
