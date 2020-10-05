#ifndef __XMODEM_H__
#define __XMODEM_H__

#include <stdbool.h>
#include "S32K144.h"
#include "status.h"
#include "mflexcan.h"

#define APP_IMAGE_START     0xA000

/*
 * \brief   XMODEMģʽ���س�ʼ��
 * \details ��Ӧ�ó����׵�ַ, ���ڽ��ն��г�ʼ��
 */
void xmodem_init(void);

/*
 * \brief   XMODEM��������,������¼��
 * \retval  STATUS_SUCCESS  ���ճɹ�
 * \        STATUS_ERROR    ���մ���
 */
status_t xmodem_download(uint8_t instance);

/*
 * \brief   ����λ����������
 * \details XMODEM����ģʽ��Ҫ��Ƭ������λ������������,
 *          ��λ�����յ������ʼ��������,��������20ms
 *          ֮��ż����λ���Ƿ���Ӧ
 * \retval  ture    ��λ������Ӧ
 *          false   ��λ��û����Ӧ
 */
bool xmodem_is_active(uint8_t instance);

/*
 * \brief   �򴮿ڽ��ջ�����ĩβ���һ�ֽ�����
 * \param   byte    ��ӵ�����
 */
void xmodem_queue_byte(uint8_t byte);

/*
 * \brief   �򴮿ڷ���һ�ֽ�����
 * \param   x   ���͵�����
 */
void xmodem_putchar(uint8_t instance, uint8_t byte);

/*
 * \brief   �ж����ݻ��������Ϊ������
 * \retval  true    ���ݻ�������������
 *          false   ���ݻ�����Ϊ��
 */
bool xmodem_getchar_present(void);

/*
 * \brief   ���ڽ����жϻص�����
 * \param   instance    �������
 * \param   lpuartState ����״̬
 */
void xmodem_uart_handler(uint32_t instance, void * lpuartState);

/*
 * \brief   CAN�����жϻص�����
 * \param   instance    CAN���
 * \param   eventType   �ж��¼�����
 * \param   state       CAN״̬
 */
void xmodem_can_handler(uint8_t instance, flexcan_event_type_t eventType,
        struct FlexCANState * state);
#endif
