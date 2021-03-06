/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick)
  *             - Collection : Application 
  *      
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *           
  *      
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_core.h"
#include "usbd_desc.h"
#include "usbd_req.h"


/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_HID_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 




/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req);

static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length);

static uint8_t  USBD_HID_DataIn (void  *pdev, uint8_t epnum);
/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Variables
  * @{
  */ 

USBD_Class_cb_TypeDef  USBD_HID_cb = 
{
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_HID_Setup,
  NULL, /*EP0_TxSent*/  
  NULL, /*EP0_RxReady*/
  USBD_HID_DataIn, /*DataIn*/
  NULL, /*DataOut*/
  NULL, /*SOF */
  NULL,
  NULL,      
  USBD_HID_GetCfgDesc,
#ifdef USB_OTG_HS_CORE  
  USBD_HID_GetCfgDesc, /* use same config as per FS */
#endif  
};

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */        
__ALIGN_BEGIN static uint32_t  USBD_HID_AltSet  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */      
__ALIGN_BEGIN static uint32_t  USBD_HID_Protocol  __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */  
__ALIGN_BEGIN static uint32_t  USBD_HID_IdleState __ALIGN_END = 0;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */ 
/* USB HID device Configuration Descriptor */
const u8 KeyboardReportDescriptor[]=
{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)	//63
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x05,                    //   REPORT_COUNT (5)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x05, 0x08,                    //   USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05,                    //   USAGE_MAXIMUM (Kana)
    0x91, 0x02,                    //   OUTPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x03,                    //   REPORT_SIZE (3)
    0x91, 0x03,                    //   OUTPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0,                           // END_COLLECTION
	//0xc0,
  };
const static uint8_t USBD_HID_CfgDesc[89]  =
{
 /***************����������***********************/
    0x09,		//bLength�ֶ�
    USB_CONFIGURATION_DESCRIPTOR_TYPE,		//bDescriptorType�ֶ�
    //wTotalLength�ֶ�
    89%128,
    /* wTotalLength: Bytes returned */
    89/128,

    0x03,	//bNumInterfaces�ֶ�
    0x01,	//bConfiguration�ֶ�
    0x00,	//iConfigurationz�ֶ�
    0x80,	//bmAttributes�ֶ�
    0x32,	//bMaxPower�ֶ�

    /*******************��һ���ӿ�������*********************/
    0x09,	//bLength�ֶ�
    0x04,	//bDescriptorType�ֶ�
    0x00,	//bInterfaceNumber�ֶ�
    0x00,	//bAlternateSetting�ֶ�
    0x02,	//bNumEndpoints�ֶ�
    0x03,	//bInterfaceClass�ֶ�
    0x01,	//bInterfaceSubClass�ֶ�
    0x01,	//bInterfaceProtocol�ֶ�
    0x00,	//iConfiguration�ֶ�

    /******************HID������************************/
    0x09,	//bLength�ֶ�
    0x21,	//bDescriptorType�ֶ�
    0x10,	//bcdHID�ֶ�
    0x01,
    0x21,	//bCountyCode�ֶ�
    0x01,	//bNumDescriptors�ֶ�
    0x22,	//bDescriptorType�ֶ�

    //bDescriptorLength�ֶΡ�
    //�¼��������ĳ��ȡ��¼�������Ϊ���̱�����������
    sizeof(KeyboardReportDescriptor)&0xFF,
    (sizeof(KeyboardReportDescriptor)>>8)&0xFF,

    /**********************����˵�������***********************/
    0x07,	//bLength�ֶ�
    0x05,	//bDescriptorType�ֶ�
    0x81,	//bEndpointAddress�ֶ�
    0x03,	//bmAttributes�ֶ�
    0x10,	//wMaxPacketSize�ֶ�
    0x00,
    0x0A,	//bInterval�ֶ�

    /**********************����˵�������***********************/
    0x07,	//bLength�ֶ�
    0x05,	//bDescriptorType�ֶ�
    0x01,	//bEndpointAddress�ֶ�
    0x03,	//bmAttributes�ֶ�
    0x10,	//wMaxPacketSize�ֶ�
    0x00,
    0x0A,	//bInterval�ֶ�

    /*******************�ڶ����ӿ�������*********************/
    0x09,	//bLength�ֶ�
    0x04,	//bDescriptorType�ֶ�
    0x01,	//bInterfaceNumber�ֶ�
    0x00,	//bAlternateSetting�ֶ�
    0x01,	//bNumEndpoints�ֶ�
    0x03,	//bInterfaceClass�ֶ�
    0x01,	//bInterfaceSubClass�ֶ�
    0x02,	//bInterfaceProtocol�ֶ�
    0x00,	//iConfiguration�ֶ�

    /******************HID������************************/
    0x09,	//bLength�ֶ�
    0x21,	//bDescriptorType�ֶ�
    0x10,	//bcdHID�ֶ�
    0x01,
    0x21,	//bCountyCode�ֶ�
    0x01,	//bNumDescriptors�ֶ�
    0x22,	//bDescriptorType�ֶ�
    sizeof(MouseReportDescriptor)&0xFF,		//bDescriptorLength�ֶ�
    (sizeof(MouseReportDescriptor)>>8)&0xFF,

    /**********************����˵�������***********************/
    0x07,	//bLength�ֶ�
    0x05,	//bDescriptorType�ֶ�
    0x82,	//bEndpointAddress�ֶ�
    0x03,	//bmAttributes�ֶΡ�D1~D0Ϊ�˵㴫������ѡ��
    0x40,	//wMaxPacketSize�ֶ�
    0x00,
    0x0A, 	//bInterval�ֶ�

    /*******************�������ӿ�������*********************/
    0x09,   /* bLength: Interface Descriptor size */
    0x04,   /* bDescriptorType: */
    /*      Interface descriptor type */
    0x02,   /* bInterfaceNumber: Number of Interface */
    0x00,   /* bAlternateSetting: Alternate setting */
    0x02,   /* bNumEndpoints*/
    0x08,   /* bInterfaceClass: MASS STORAGE Class */
    0x06,   /* bInterfaceSubClass : SCSI transparent*/
    0x50,   /* nInterfaceProtocol */
    4,          /* iInterface: */
    /* 18 */
    0x07,   /*Endpoint descriptor length = 7*/
    0x05,   /*Endpoint descriptor type */
    0x83,   /*Endpoint address (IN, address 1) */
    0x02,   /*Bulk endpoint type */
    0x40,   /*Maximum packet size (64 bytes) */
    0x00,
    0x00,   /*Polling interval in milliseconds */
    /* 25 */
    0x07,   /*Endpoint descriptor length = 7 */
    0x05,   /*Endpoint descriptor type */
    0x04,   /*Endpoint address (OUT, address 2) */
    0x02,   /*Bulk endpoint type */
    0x40,   /*Maximum packet size (64 bytes) */
    0x00,
    0x00,     /*Polling interval in milliseconds*/
    /*32*/
    
    
    
    
    
    
    
    
    
//    
//    0x09,   /* bLength: Interface Descriptor size */
//    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: Interface */
//    /* Interface descriptor type */
//    0x03,   /* bInterfaceNumber: Number of Interface */
//    0x00,   /* bAlternateSetting: Alternate setting */
//    0x01,   /* bNumEndpoints: One endpoints used */
//    0x02,   /* bInterfaceClass: Communication Interface Class */
//    0x02,   /* bInterfaceSubClass: Abstract Control Model */
//    0x01,   /* bInterfaceProtocol: Common AT commands */
//    0x00,   /* iInterface: */
//    
//    /*Header Functional Descriptor*/
//    0x05,   /* bLength: Endpoint Descriptor size */
//    0x24,   /* bDescriptorType: CS_INTERFACE */
//    0x00,   /* bDescriptorSubtype: Header Func Desc */
//    0x10,   /* bcdCDC: spec release number */
//    0x01,
//    
//    /*Call Managment Functional Descriptor*/
//    0x05,   /* bFunctionLength */
//    0x24,   /* bDescriptorType: CS_INTERFACE */
//    0x01,   /* bDescriptorSubtype: Call Management Func Desc */
//    0x00,   /* bmCapabilities: D0+D1 */
//    0x04,   /* bDataInterface: 1 */
//    
//    /*ACM Functional Descriptor*/
//    0x04,   /* bFunctionLength */
//    0x24,   /* bDescriptorType: CS_INTERFACE */
//    0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
//    0x02,   /* bmCapabilities */
//    
//    
//    /*Union Functional Descriptor*/
//    0x05,   /* bFunctionLength */
//    0x24,   /* bDescriptorType: CS_INTERFACE */
//    0x06,   /* bDescriptorSubtype: Union func desc */
//    0x03,   /* bMasterInterface: Communication class interface */
//    0x04,   /* bSlaveInterface0: Data Class Interface */
//    
//    
//    /*Endpoint 2 Descriptor*/
//    0x07,   /* bLength: Endpoint Descriptor size */
//    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
//    0x86,   /* bEndpointAddress: (IN2) */
//    0x03,   /* bmAttributes: Interrupt */
//    VIRTUAL_COM_PORT_INT_SIZE,      /* wMaxPacketSize: */
//    0x00,
//    0xFF,   /* bInterval: */
//    
//    
//    
//    
//    /*Data class interface descriptor*/
//    0x09,   /* bLength: Endpoint Descriptor size */
//    USB_INTERFACE_DESCRIPTOR_TYPE,  /* bDescriptorType: */
//    0x04,   /* bInterfaceNumber: Number of Interface */
//    0x00,   /* bAlternateSetting: Alternate setting */
//    0x02,   /* bNumEndpoints: Two endpoints used */
//    0x0A,   /* bInterfaceClass: CDC */
//    0x00,   /* bInterfaceSubClass: */
//    0x00,   /* bInterfaceProtocol: */
//    0x01,   /* iInterface: */
//    
//    /*Endpoint 3 Descriptor*/
//    0x07,   /* bLength: Endpoint Descriptor size */
//    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
//    0x07,   /* bEndpointAddress: (OUT3) */
//    0x02,   /* bmAttributes: Bulk */
//    VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
//    0x00,
//    0x00,   /* bInterval: ignore for Bulk transfer */
//    
//    /*Endpoint 1 Descriptor*/
//    0x07,   /* bLength: Endpoint Descriptor size */
//    USB_ENDPOINT_DESCRIPTOR_TYPE,   /* bDescriptorType: Endpoint */
//    0x85,   /* bEndpointAddress: (IN1) */
//    0x02,   /* bmAttributes: Bulk */
//    VIRTUAL_COM_PORT_DATA_SIZE,             /* wMaxPacketSize: */
//    0x00,
//    0x00    /* bInterval */
} ;

#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
  #if defined ( __ICCARM__ ) /*!< IAR Compiler */
    #pragma data_alignment=4   
  #endif
#endif /* USB_OTG_HS_INTERNAL_DMA_ENABLED */ 

__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
{
  0x05,   0x01,
  0x09,   0x02,
  0xA1,   0x01,
  0x09,   0x01,
  
  0xA1,   0x00,
  0x05,   0x09,
  0x19,   0x01,
  0x29,   0x03,
  
  0x15,   0x00,
  0x25,   0x01,
  0x95,   0x03,
  0x75,   0x01,
  
  0x81,   0x02,
  0x95,   0x01,
  0x75,   0x05,
  0x81,   0x01,
  
  0x05,   0x01,
  0x09,   0x30,
  0x09,   0x31,
  0x09,   0x38,
  
  0x15,   0x81,
  0x25,   0x7F,
  0x75,   0x08,
  0x95,   0x03,
  
  0x81,   0x06,
  0xC0,   0x09,
  0x3c,   0x05,
  0xff,   0x09,
  
  0x01,   0x15,
  0x00,   0x25,
  0x01,   0x75,
  0x01,   0x95,
  
  0x02,   0xb1,
  0x22,   0x75,
  0x06,   0x95,
  0x01,   0xb1,
  
  0x01,   0xc0
}; 

/**
  * @}
  */ 

/** @defgroup USBD_HID_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_Init (void  *pdev, 
                               uint8_t cfgidx)
{
  
  /* Open EP IN */
  DCD_EP_Open(pdev,
              HID_IN_EP,
              HID_IN_PACKET,
              USB_OTG_EP_INT);
  
  /* Open EP OUT */
  DCD_EP_Open(pdev,
              HID_OUT_EP,
              HID_OUT_PACKET,
              USB_OTG_EP_INT);
	
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_HID_DeInit (void  *pdev, 
                                 uint8_t cfgidx)
{
  /* Close HID EPs */
  DCD_EP_Close (pdev , HID_IN_EP);
  DCD_EP_Close (pdev , HID_OUT_EP);
  
  
  return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_Setup (void  *pdev, 
                                USB_SETUP_REQ *req)
{
  uint16_t len = 0;
  uint8_t  *pbuf = NULL;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :  
    switch (req->bRequest)
    {
      
      
    case HID_REQ_SET_PROTOCOL:
      USBD_HID_Protocol = (uint8_t)(req->wValue);
      break;
      
    case HID_REQ_GET_PROTOCOL:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_Protocol,
                        1);    
      break;
      
    case HID_REQ_SET_IDLE:
      USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
      break;
      
    case HID_REQ_GET_IDLE:
      USBD_CtlSendData (pdev, 
                        (uint8_t *)&USBD_HID_IdleState,
                        1);        
      break;      
      
    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL; 
    }
    break;
    
  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR: 
      if( req->wValue >> 8 == HID_REPORT_DESC)
      {
        len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
        pbuf = HID_MOUSE_ReportDesc;
      }
      else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
      {
        
//#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
//        pbuf = USBD_HID_Desc;   
//#else
        pbuf = USBD_HID_CfgDesc + 0x12;
//#endif 
        len = MIN(USB_HID_DESC_SIZ , req->wLength);
      }
      
      USBD_CtlSendData (pdev, 
                        pbuf,
                        len);
      
      break;
      
    case USB_REQ_GET_INTERFACE :
      USBD_CtlSendData (pdev,
                        (uint8_t *)&USBD_HID_AltSet,
                        1);
      break;
      
    case USB_REQ_SET_INTERFACE :
      USBD_HID_AltSet = (uint8_t)(req->wValue);
      break;
    }
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport     (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len)
{
  if (pdev->dev.device_status == USB_OTG_CONFIGURED )
  {
    DCD_EP_Tx (pdev, HID_IN_EP, report, len);
  }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_HID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
  *length = sizeof (USBD_HID_CfgDesc);
  return USBD_HID_CfgDesc;
}

/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_HID_DataIn (void  *pdev, 
                              uint8_t epnum)
{
  
  /* Ensure that the FIFO is empty before a new transfer, this condition could 
  be caused by  a new transfer before the end of the previous transfer */
  DCD_EP_Flush(pdev, HID_IN_EP);
  return USBD_OK;
}

/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
