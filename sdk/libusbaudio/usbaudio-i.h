/* 
 * File:	 usbaudio.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/usb.h>
#include <sys/cdc.h>

#define USBD_AUDIO_FREQ 22050

#define EP0_ADDR 0
#define EP0_MAX_PKT_SIZE 64

#define EP_OUT_ADDR 1
#define EP_IN_ADDR  2
#define EP_INT_ADDR 3

#ifndef CDC_EP_OUT_MAX_PKT_SIZE
#define CDC_EP_OUT_MAX_PKT_SIZE 64
#endif

#ifndef CDC_EP_IN_MAX_PKT_SIZE 
#define CDC_EP_IN_MAX_PKT_SIZE 64
#endif

#ifndef CDC_EP_INT_MAX_PKT_SIZE 
#define CDC_EP_INT_MAX_PKT_SIZE 64
#endif

struct usbaudio_descriptor_config {
	struct usb_descriptor_configuration cfg;
	struct usb_descriptor_interface comm_if;
	struct cdc_header_descriptor hdr;
	struct cdc_call_management_descriptor cm;
	struct cdc_abstract_control_management_descriptor acm;
	struct cdc_union_1slave_descriptor un;
	struct usb_descriptor_endpoint ep_int;
	struct usb_descriptor_interface if_data;
	struct usb_descriptor_endpoint ep_out;
	struct usb_descriptor_endpoint ep_in;
} __attribute__((__packed__));

struct usb_str_entry {
	const uint8_t * str;
	uint8_t len;
};

extern const struct usb_descriptor_device usbaudio_desc_dev;
extern const struct usbaudio_descriptor_config usbaudio_desc_cfg;
extern const uint8_t * const usbaudio_str[];

/* Audio Commands enumeration */
typedef enum
{
	AUDIO_CMD_START = 1,
	AUDIO_CMD_PLAY,
	AUDIO_CMD_STOP,
}AUDIO_CMD_TypeDef;


typedef enum
{
	AUDIO_OFFSET_NONE = 0,
	AUDIO_OFFSET_HALF,
	AUDIO_OFFSET_FULL,
	AUDIO_OFFSET_UNKNOWN,
}
AUDIO_OffsetTypeDef;



typedef struct
{
	int8_t  (*Init)         (uint32_t  AudioFreq, uint32_t Volume, uint32_t options);
	int8_t  (*DeInit)       (uint32_t options);
	int8_t  (*AudioCmd)     (uint8_t* pbuf, uint32_t size, uint8_t cmd);
	int8_t  (*VolumeCtl)    (uint8_t vol);
	int8_t  (*MuteCtl)      (uint8_t cmd);
	int8_t  (*PeriodicTC)   (uint8_t cmd);
	int8_t  (*GetState)     (void);
}USBD_AUDIO_ItfTypeDef;

#define AUDIO_OUT_PACKET                              (uint32_t)(((USBD_AUDIO_FREQ * 2 * 2) /1000))
#define AUDIO_DEFAULT_VOLUME                          70

/* Number of sub-packets in the audio transfer buffer. You can modify this value but always make sure
   that it is an even number and higher than 3 */
#define AUDIO_OUT_PACKET_NUM                          80
/* Total size of the audio transfer buffer */
#define AUDIO_TOTAL_BUF_SIZE                          ((uint32_t)(AUDIO_OUT_PACKET * AUDIO_OUT_PACKET_NUM))

#define AUDIO_SAMPLE_FREQ(frq)      (uint8_t)(frq), (uint8_t)((frq >> 8)), (uint8_t)((frq >> 16))

#define AUDIO_PACKET_SZE(frq)          (uint8_t)(((frq * 2 * 2)/1000) & 0xFF), \
	                                       (uint8_t)((((frq * 2 * 2)/1000) >> 8) & 0xFF)

/* Following USB Device Speed */
typedef enum
{
	USBD_SPEED_HIGH  = 0,
	USBD_SPEED_FULL  = 1,
	USBD_SPEED_LOW   = 2,
}USBD_SpeedTypeDef;

/* Following USB Device status */
typedef enum {
	USBD_OK   = 0,
	USBD_BUSY,
	USBD_FAIL,
}USBD_StatusTypeDef;

typedef  struct  usb_setup_req
{
	uint8_t   bmRequest;
	uint8_t   bRequest;
	uint16_t  wValue;
	uint16_t  wIndex;
	uint16_t  wLength;
}USBD_SetupReqTypedef;

struct _USBD_HandleTypeDef;

typedef struct _Device_cb
{
	uint8_t  (*Init)             (struct _USBD_HandleTypeDef *pdev , uint8_t cfgidx);
	uint8_t  (*DeInit)           (struct _USBD_HandleTypeDef *pdev , uint8_t cfgidx);
	/* Control Endpoints*/
	uint8_t  (*Setup)            (struct _USBD_HandleTypeDef *pdev , USBD_SetupReqTypedef  *req);
	uint8_t  (*EP0_TxSent)       (struct _USBD_HandleTypeDef *pdev );
	uint8_t  (*EP0_RxReady)      (struct _USBD_HandleTypeDef *pdev );
	/* Class Specific Endpoints*/
	uint8_t  (*DataIn)           (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);
	uint8_t  (*DataOut)          (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);
	uint8_t  (*SOF)              (struct _USBD_HandleTypeDef *pdev);
	uint8_t  (*IsoINIncomplete)  (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);
	uint8_t  (*IsoOUTIncomplete) (struct _USBD_HandleTypeDef *pdev , uint8_t epnum);

	uint8_t  *(*GetHSConfigDescriptor)(uint16_t *length);
	uint8_t  *(*GetFSConfigDescriptor)(uint16_t *length);
	uint8_t  *(*GetOtherSpeedConfigDescriptor)(uint16_t *length);
	uint8_t  *(*GetDeviceQualifierDescriptor)(uint16_t *length);
#if (USBD_SUPPORT_USER_STRING == 1)
	uint8_t  *(*GetUsrStrDescriptor)(struct _USBD_HandleTypeDef *pdev ,uint8_t index,  uint16_t *length);
#endif

} USBD_ClassTypeDef;

/* USB Device descriptors structure */
typedef struct
{
	uint8_t  *(*GetDeviceDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
	uint8_t  *(*GetLangIDStrDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
	uint8_t  *(*GetManufacturerStrDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
	uint8_t  *(*GetProductStrDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
	uint8_t  *(*GetSerialStrDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
	uint8_t  *(*GetConfigurationStrDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
	uint8_t  *(*GetInterfaceStrDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
#if (USBD_LPM_ENABLED == 1)
	uint8_t  *(*GetBOSDescriptor)( USBD_SpeedTypeDef speed , uint16_t *length);
#endif
} USBD_DescriptorsTypeDef;

/* USB Device handle structure */
typedef struct
{
	uint32_t                status;
	uint32_t                total_length;
	uint32_t                rem_length;
	uint32_t                maxpacket;
} USBD_EndpointTypeDef;

/* USB Device handle structure */
typedef struct _USBD_HandleTypeDef
{
	uint8_t                 id;
	uint32_t                dev_config;
	uint32_t                dev_default_config;
	uint32_t                dev_config_status;
	USBD_SpeedTypeDef       dev_speed;
	USBD_EndpointTypeDef    ep_in[15];
	USBD_EndpointTypeDef    ep_out[15];
	uint32_t                ep0_state;
	uint32_t                ep0_data_len;
	uint8_t                 dev_state;
	uint8_t                 dev_old_state;
	uint8_t                 dev_address;
	uint8_t                 dev_connection_status;
	uint8_t                 dev_test_mode;
	uint32_t                dev_remote_wakeup;

	USBD_SetupReqTypedef    request;
	USBD_DescriptorsTypeDef *pDesc;
	USBD_ClassTypeDef       *pClass;
	void                    *pClassData;
	void                    *pUserData;
	void                    *pData;
} USBD_HandleTypeDef;
