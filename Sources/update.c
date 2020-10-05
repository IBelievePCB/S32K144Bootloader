/*
 * update.c
 *
 *  Created on: 2018��4��13��
 *      Author: Administrator
 */
#include "update.h"
#include "lpuart0.h"
#include "xmodem.h"
#include "mflash.h"

uint32_t package_size = 0;

uint8_t lpuart0_default_rx_buffer[8];

/**
 * \brief   �������
 * \note    �ڸú�����,����Լ5��ÿ����ٶ�����λ��������������
 *          ����ͨ�������˳���������
 */
status_t updating(instance) {
    status_t ret = STATUS_ERROR;
    while(1) {
        if (xmodem_is_active(instance)) {
            ret = xmodem_download(instance);                 /* �����յ�����,ʹ�ô����������� */
            break;
        }
        OSIF_TimeDelay(190);
    }
    return ret;
}

/**
 * \brief   ��ʼ�������������������
 */
void update_init(uint8_t instance) {
    flash_pflash_init();
//    flexcan_init();
    LPUART_DRV_Init(instance, &lpuart0_State, &lpuart0_InitConfig0);
    LPUART_DRV_InstallRxCallback(instance, xmodem_uart_handler, NULL);
    LPUART_DRV_ReceiveData(instance, lpuart0_default_rx_buffer, 8);
    xmodem_init();
}

/**
 * \brief   ��������
 */
status_t update(uint8_t instance)
{
    status_t ret;
    update_init(instance);
    ret = updating(instance);
//    FLEXCAN_DRV_Deinit(INST_CANCOM0);
    LPUART_DRV_Deinit(instance);
    return ret;
}
void software_reset(void)
{
    __asm volatile ("dsb");
    S32_SCB->AIRCR = ((0x5FA << S32_SCB_AIRCR_VECTKEY_SHIFT)      |
            S32_SCB_AIRCR_SYSRESETREQ_MASK);
    __asm volatile ("dsb");
    while(1);                     /* wait until reset */
}
