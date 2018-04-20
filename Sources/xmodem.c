#include "xmodem.h"
#include "crc.h"
#include "mflash.h"
#include "interrupt_manager.h"
#include "string.h"

#define ENABLE_XMODEM       1


#define SOH     0x01 
#define EOT     0x04 
#define ACK     0x06 
#define NAK     0x15 
#define CAN     0x18 
#define ESC     0x1b 

#define __PKTLEN_128  128
#define __XMODEM_BUFLEN	256
//#pragma location = ".textrw" 

#ifndef ALIGN
/* Compiler Related Definitions */
#ifdef __CC_ARM                         /* ARM Compiler */
    #define ALIGN(n)                    __attribute__((aligned(n)))
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
    #define PRAGMA(x)                   _Pragma(#x)
    #define ALIGN(n)                    PRAGMA(data_alignment=n)
#elif defined (__GNUC__)                /* GNU GCC Compiler */
    #define ALIGN(n)                    __attribute__((aligned(n)))
#endif /* Compiler Related Definitions */
#endif


#if ENABLE_XMODEM

uint32_t __g_flash_app_addr;
uint8_t  __g_is_update_by_uart = 1;

static uint8_t __g_callback_buffer[__XMODEM_BUFLEN];
volatile static uint32_t __g_readOffset, __g_writeOffset;

/*
 * \brief   �ӻ�������ȡ����ֽ�
 * \param   buffer[out]     ��ȡ��������
 * \param   byteCount[in]   ��ȡ���ֽ���
 * \note    �����������û���㹻����������,��������������,
 *          ֱ����ȡ���㹻����
 */
static void read_bytes(uint8_t *p_buffer, uint32_t byteCount)
{
    uint32_t currentBytesRead = 0;

    while(currentBytesRead != byteCount)
    {
        if (__g_readOffset != __g_writeOffset)
        {
            p_buffer[currentBytesRead++] = __g_callback_buffer[__g_readOffset++];
            __g_readOffset &= __XMODEM_BUFLEN - 1;
        }
    }
}

/*
 * \brief   ��ȡһ��XMODEM��,����У��
 * \param   buffer[out]     XMODEM���ݰ��е����ݸ���
 * \param   idx             XMODEM���ݰ����
 * \retval  0   ��ȡ��У��ɹ�
 *          -1  У��ʧ��
 */
static int read_packet(uint8_t *p_buffer, uint8_t idx)
{
    uint8_t  seq[2],crc1,crc2;
    uint16_t  crc16, verify16;
	
	read_bytes(seq,2);

	read_bytes(p_buffer,__PKTLEN_128);
	CRC_DRV_Init(INST_CRC, &crc_InitConfig0);
	CRC_DRV_WriteData(INST_CRC, p_buffer, __PKTLEN_128);
	verify16 = CRC_DRV_GetCrcResult(INST_CRC);
	
	read_bytes(&crc1,1);
	read_bytes(&crc2,1);
    crc16  = ((uint16_t)crc1 << 8)|crc2;

    if ((crc16 != verify16) || (seq[0] != idx) || (seq[1] != (uint8_t) ((~(uint32_t)idx)&0xff)))
        return(-1);

    return(0);
}

/*
 * \brief   ��Program Flash��д������
 * \param   data[in]        д�������
 * \param   data_size[in]   д�����ݵĴ�С
 * \retval  STATUS_SUCCESS  д��ɹ�
 *          STATUS_ERROR    д��ʧ��
 */
static status_t xmodem_write_image(uint8_t *p_data, uint16_t data_size)
{

    uint32_t fail_addr;
    ALIGN(8) static uint8_t abuf[FEATURE_FLS_PF_BLOCK_SECTOR_SIZE];
    INT_SYS_DisableIRQGlobal();
    
    if((__g_flash_app_addr & (FEATURE_FLS_PF_BLOCK_SECTOR_SIZE-1)) == 0)
    {
        if (flash_pflash_erase_sectors(__g_flash_app_addr/FEATURE_FLS_PF_BLOCK_SECTOR_SIZE, 1) != STATUS_SUCCESS) {
            INT_SYS_EnableIRQGlobal();
            return STATUS_ERROR;
        }
    }
    
    memcpy(abuf, p_data, data_size);
    if (flash_write_PFLASH(__g_flash_app_addr, data_size, abuf, &fail_addr) != STATUS_SUCCESS) {
        INT_SYS_EnableIRQGlobal();
        return STATUS_ERROR;
    }

    INT_SYS_EnableIRQGlobal();
    __g_flash_app_addr += data_size;

	return STATUS_SUCCESS;
}

/*
 * \brief   XMODEMģʽ���س�ʼ��
 * \details ��Ӧ�ó����׵�ַ, ���ڽ��ն��г�ʼ��
 */
void xmodem_init(void)
{
	__g_flash_app_addr = APP_IMAGE_START;
	__g_readOffset = 0;
	__g_writeOffset = 0;
}

/*
 * \brief   ����λ����������
 * \details XMODEM����ģʽ��Ҫ��Ƭ������λ������������,
 *          ��λ�����յ������ʼ��������,��������20ms
 *          ֮��ż����λ���Ƿ���Ӧ
 * \retval  ture    ��λ������Ӧ
 *          false   ��λ��û����Ӧ
 */
bool xmodem_is_active(void)
{
    uint8_t ack = 'C';
    g_tx_info.data_length = 1;
    FLEXCAN_DRV_Send(INST_CANCOM0, TRANSMIT_STD_MB, &g_tx_info, 0x00, &ack);
	xmodem_putchar(ack);
	OSIF_TimeDelay(10);
	if(xmodem_getchar_present())
		return true;
	else
		return false;
}

/*
 * \brief   CAN�����жϻص�����
 * \param   instance    CAN���
 * \param   eventType   �ж��¼�����
 * \param   state       CAN״̬
 */
void xmodem_can_handler(uint8_t instance, flexcan_event_type_t eventType,
        struct FlexCANState * state)
{
    if (eventType == FLEXCAN_EVENT_RX_COMPLETE){
        if (g_can_receive_buff.msgId == RXID_UPDATE) {
            uint32_t i = 0;
            for (i=0; i<g_can_receive_buff.dataLen; i++) {
                xmodem_queue_byte(g_can_receive_buff.data[i]);
            }
        }
        FLEXCAN_DRV_Receive(0, RECEIVE_STD_MB, &g_can_receive_buff);
        LPUART_HAL_SetIntMode(LPUART0, LPUART_INT_RX_DATA_REG_FULL, false);
        __g_is_update_by_uart = 0;
    }
}

/*
 * \brief   ���ڽ����жϻص�����
 * \param   instance    �������
 * \param   lpuartState ����״̬
 */
void xmodem_uart_handler(uint32_t instance, void * lpuartState)
{
    uint8_t byte;
    if(LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_DATA_REG_FULL))
    {
        LPUART_HAL_Getchar(LPUART0, &byte);
        xmodem_queue_byte(byte);
        FLEXCAN_HAL_SetMsgBuffIntCmd(CAN0, RECEIVE_STD_MB, false);
        __g_is_update_by_uart = 1;
    }
    
    if(LPUART_HAL_GetStatusFlag(LPUART0, LPUART_RX_OVERRUN))
    {
        LPUART_HAL_Getchar(LPUART0, &byte);
    }
    
}

/*
 * \brief   �򴮿ڽ��ջ�����ĩβ���һ�ֽ�����
 * \param   byte    ��ӵ�����
 */
void xmodem_queue_byte(uint8_t byte)
{
    __g_callback_buffer[__g_writeOffset++] = byte;
    __g_writeOffset &= __XMODEM_BUFLEN - 1;
}

/*
 * \brief   �򴮿ڷ���һ�ֽ�����
 * \param   x   ���͵�����
 */
void xmodem_putchar(uint8_t byte)
{
    if (__g_is_update_by_uart) {
        while ((LPUART0->STAT & LPUART_STAT_TDRE_MASK) == 0);
        LPUART_HAL_Putchar(LPUART0, byte);
    } else {
        g_tx_info.data_length = 1;
        FLEXCAN_DRV_Send(INST_CANCOM0, TRANSMIT_STD_MB, &g_tx_info, 0x00, &byte);
    }
}

/*
 * \brief   �ж����ݻ��������Ϊ������
 * \retval  true    ���ݻ�������������
 *          false   ���ݻ�����Ϊ��
 */
bool xmodem_getchar_present(void)
{
	return __g_readOffset != __g_writeOffset;
}

/*
 * \brief   XMODEM��������,������¼��
 * \retval  STATUS_SUCCESS  ���ճɹ�
 * \        STATUS_ERROR    ���մ���
 */
status_t xmodem_download(void)
{
    uint8_t ch;
    int done;
	uint8_t buffer[__PKTLEN_128];
    uint8_t idx = 0x01;
    
    uint32_t size = 0;

    done = 0;
    while(done == 0) 
    {
		read_bytes(&ch,1);

        switch(ch) 
        {   
            case SOH:
                done = read_packet(buffer, idx);

                if (done == 0)
                {
                    idx++;
                    size += __PKTLEN_128;
					if (xmodem_write_image((uint8_t*)buffer, __PKTLEN_128) != STATUS_SUCCESS) {
					    return STATUS_ERROR;
					}
					xmodem_putchar(ACK);
                }
				else
				{
					done = 0;
					xmodem_putchar(NAK);
				}
                break;
            case EOT:
                xmodem_putchar(ACK);

                done = size;
                break;

            case CAN:
                return STATUS_ERROR;
            case ESC:
				done = -1;
				break;	
				
			default:
				xmodem_putchar(NAK);
				break;
        }
    }

    return STATUS_SUCCESS;
}

#endif /* XMODEM */
