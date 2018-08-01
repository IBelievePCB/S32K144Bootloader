/*
 * flexcan.c
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */
#include "mflexcan.h"

flexcan_msgbuff_t g_can_receive_buff;

/**
 * \brief   ����������Ϣ
 */
flexcan_data_info_t g_rx_info = {
        .data_length = 8,
        .enable_brs = 1,
        .fd_enable = 0,
        .fd_padding = 0,
        .is_remote = 0,
        .msg_id_type = FLEXCAN_MSG_ID_STD
};

/**
 * \brief   ����������Ϣ
 */
flexcan_data_info_t g_tx_info = {
        .data_length = 8,
        .enable_brs = 1,
        .fd_enable = 0,
        .fd_padding = 0,
        .is_remote = 0,
        .msg_id_type = FLEXCAN_MSG_ID_STD
};

/**
 * \brief   ��ʼ��CAN
 */
void flexcan_init(void) {
    /* �Ƚ�can��λ */
    FLEXCAN_DRV_Deinit(INST_CANCOM0);
    /* ����CAN���� */
    FLEXCAN_DRV_Init(0, &canCom0_State, &canCom0_InitConfig);
    /* ����ȫ�������� */
    FLEXCAN_DRV_SetRxMbGlobalMask(0, FLEXCAN_MSG_ID_EXT, RXMB_GLOBALMASK);
    /* ���ý������� */
    FLEXCAN_DRV_ConfigRxMb(0, RECEIVE_STD_MB, &g_rx_info, RXID_UPDATE);
    /* ���ý��ջص����� */
    FLEXCAN_DRV_InstallEventCallback(0, xmodem_can_handler, NULL);
    /* ��ʼ���� */
    FLEXCAN_DRV_Receive(0, RECEIVE_STD_MB, &g_can_receive_buff);
}


/**
 * \brief       ��ȡcan0ʱ��Ƶ��
 * \param[out]  flexcanSourceClock  ʱ��Ƶ��
 */
void flexcan_get_source_clock(uint32_t *flexcanSourceClock) {
    if (canCom0_InitConfig.pe_clock == FLEXCAN_CLK_SOURCE_SYS){
        CLOCK_SYS_GetFreq(CORE_CLOCK, flexcanSourceClock);
    } else {
        uint32_t i = (SCG->SOSCDIV & SCG_SOSCDIV_SOSCDIV2_MASK) >> SCG_SOSCDIV_SOSCDIV2_SHIFT;
        CLOCK_SYS_GetFreq(SOSC_CLOCK, flexcanSourceClock);
        for (i=i-1; i>0; i--) {
            *flexcanSourceClock >>= 1;
        }
    }
}

