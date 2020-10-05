/* ###################################################################
**     Filename    : main.c
**     Processor   : S32K144
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.c
** @version 01.00
** @brief
**         Main module.
**         This module contains user's application code.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
/* MODULE main */


/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "pin_mux.h"
#include "clockMan1.h"
#include "canCom0.h"
#include "osif1.h"
#include "dmaController1.h"
#include "lpuart0.h"
#include "flash0.h"
#include "crc.h"
#include "update.h"
#include "xmodem.h"

  volatile int exit_code = 0;
/* User includes (#include below this line is not maintained by Processor Expert) */



typedef void(*App_t)(void);
#define IMAGE_ADDR  APP_IMAGE_START     /**< \brief Ӧ�ó�����ڵ�ַ */

void jump_to_app(uint32_t image_address) {
    App_t app_jump;
    uint32_t mspval = *(uint32_t *)image_address;
    app_jump = (App_t)(*(uint32_t *)(APP_IMAGE_START + 4)); /* ����λ�ж�����. */

    S32_SCB->VTOR = image_address;
    __asm__ __volatile__("msr msp,%0"::"r"(mspval):"memory");
    __asm__ __volatile__("msr psp,%0"::"r"(mspval):"memory");

    app_jump();
}

/*!
  \brief The main function for the project.
  \details The startup initialization sequence is the following:
 * - startup asm routine
 * - main()
*/
int main(void)
{
  /* Write your local variable definition here */
    int i;
    uint8_t update_string[] = "update uart 1";
    uint8_t instance;
    uint8_t write = 0;

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  #ifdef PEX_RTOS_INIT
    PEX_RTOS_INIT();                   /* Initialization of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of Processor Expert internal initialization.                    ***/

  /* Write your code here */
  /* For example: for(;;) { } */
    flash_EEPROM_init();
    /* ����Ƿ���Ҫ�������� */
//    extern const uint32_t *__StackTop;
//    if (*((uint32_t *)IMAGE_ADDR) != *__StackTop) {
//        write = 1;
//    }
    if (write == 1)
    {
        flash_write_EEPROM(1024,update_string, 13);
    }


    /* ����Ƿ���Ҫ�������� */
    for (i=0; i<6; i++) {
        if (((uint8_t *)flash0_InitConfig0.EERAMBase)[1024+i] != update_string[i]) {
            break;
        }
    }
    if (i == 6) {                /* ��EEPROM�е�ƫ��1024�ֽ��д洢����update */
        /* ��ʼ��ʱ�� */
        CLOCK_SYS_Init(g_clockManConfigsArr, CLOCK_MANAGER_CONFIG_CNT,
                g_clockManCallbacksArr, CLOCK_MANAGER_CALLBACK_CNT);
        CLOCK_SYS_UpdateConfiguration(0, CLOCK_MANAGER_POLICY_FORCIBLE);
        /* ��ʼ������ */
        PINS_DRV_Init(NUM_OF_CONFIGURED_PINS, g_pin_mux_InitConfigArr);
        instance = ((uint8_t *)flash0_InitConfig0.EERAMBase)[1024+12] - '0';
        if (instance < LPUART_INSTANCE_COUNT)
        {
            if (update(instance) == STATUS_SUCCESS) /* �������� */
            {
                uint8_t mask_eeprom[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
                flash_write_EEPROM(1024,mask_eeprom, 6);    /* ����EEPROM�еõ����� */
            }
        }
        software_reset();
    }

    /* ��ת��APP */
    jump_to_app(IMAGE_ADDR);
    for(;;) {

    }


  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;) {
    if(exit_code != 0) {
      break;
    }
  }
  return exit_code;
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.1 [05.21]
**     for the Freescale S32K series of microcontrollers.
**
** ###################################################################
*/
