#include "stm32f4xx_conf.h"
#include "pl_uart_debug.h"

#include "ff.h"


#define BYTE_TO_READ 8


FRESULT open(  FIL*,   const char*) ;
void loop(void);


 
int main(void)
{

    FRESULT fr;
    FATFS fs;
    FIL fil;

FIL fil1;

 uint16_t ByteRead = BYTE_TO_READ;;

    char Buffer[BYTE_TO_READ + 1];    

    init_debug_usart();
    /* Open or create a log file and ready to append */
    usart_put_string("Start\r\n");
    fr = f_mount(&fs, "", 1);
    usart_put_string("Mount result: \r\n");
    usart_put_int(fr);
    fr = open(&fil, "test");
    //  f_lseek(fp, f_size(fp));
    usart_put_string("\r\nFile open result: \r\n");
    usart_put_int(fr);
    usart_put_string("\r\nOpen result: "); usart_put_int(fr); usart_put_string("\r\n");
    
    while (ByteRead >= BYTE_TO_READ) {
      fr = f_read(&fil, (void *) Buffer, BYTE_TO_READ, (UINT*) &ByteRead);
      Buffer[BYTE_TO_READ ] = 0;
      usart_put_string(Buffer);
      
    };

    f_lseek(&fil, 0);
    /*    
    ByteRead = BYTE_TO_READ;

    while (ByteRead >= BYTE_TO_READ) {
      fr = f_read(&fil, (void *) Buffer, BYTE_TO_READ, (UINT*) &ByteRead);
      Buffer[BYTE_TO_READ ] = 0;
      usart_put_string(Buffer);
      
    };
    */
    usart_put_string("\r\nFile reading finished: \r\n");
    
    

    /* Close the file */
    f_close(&fil);

    loop();
    
    return 0;
}




FRESULT open (
    FIL* fp,            /* [OUT] File object to create */
    const char* path    /* [IN]  File name to be opened */
)
{
    FRESULT fr;

    /* Opens an existing file. If not exist, creates a new file. */
    fr = f_open(fp, path, FA_READ );
     if (fr == FR_OK) {

        fr = f_lseek(fp, 0);
        if (fr != FR_OK)
            f_close(fp);
    } 
   
    return fr;
}


void loop(void) {
  
  while(1) {

    /* delay(1000); */
    /* GPIOD->BSRRH = GPIO_Pin_13 ;		 */
    /* delay(1000);				 */
    /* GPIOD->BSRRL = GPIO_Pin_13 ; */
  };
}

