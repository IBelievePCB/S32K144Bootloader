/*
 * flexcan.h
 *
 *  Created on: 2018��4��8��
 *      Author: Administrator
 */

#ifndef MFLEXCAN_H_
#define MFLEXCAN_H_

#include "S32K144.h"
#include "clockMan1.h"
#include "canCom0.h"
#include "pin_mux.h"
#include "dmaController1.h"
#include "osif.h"
#include "xmodem.h"

#define RECEIVE_STD_MB  8               /**< \brief CAN��׼֡��������� */
#define RECEIVE_EXT_MB  9               /**< \brief CAN��չ֡��������� */

#define TRANSMIT_STD_MB 10              /**< \brief CAN��׼֡��������� */
#define TRANSMIT_EXT_MB 11              /**< \brief CAN��չ֡��������� */

//#define RXMB_GLOBALMASK 0X7FFFFFFF
#define RXMB_GLOBALMASK 0x00000000      /**< CAN��������ȫ��ID���� */
#define RXID_UPDATE     0x00000555      /**< CAN��������ID */

extern flexcan_msgbuff_t   g_can_receive_buff;  /**< can���յ������һ�����ݰ�*/

/**
 * \brief   ����������Ϣ
 */
extern flexcan_data_info_t rx_info;

/**
 * \brief   ����������Ϣ
 */
extern flexcan_data_info_t tx_info;

/**
 * \brief   ��ʼ��CAN
 */
void init_flexcan(void);

/**
 * \brief       ��ȡcan0ʱ��Ƶ��
 * \param[out]  flexcanSourceClock  ʱ��Ƶ��
 */
void flexcan_get_source_clock(uint32_t *flexcanSourceClock);



#endif /* MFLEXCAN_H_ */
