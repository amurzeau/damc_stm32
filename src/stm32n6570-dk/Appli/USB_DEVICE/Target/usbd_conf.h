/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_conf.h
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_conf.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"

 /* USER CODE BEGIN INCLUDE */

 /* USER CODE END INCLUDE */

 /** @addtogroup USBD_OTG_DRIVER
  * @{
  */

 /** @defgroup USBD_CONF USBD_CONF
  * @brief Configuration file for Usb otg low level driver.
  * @{
  */

 /** @defgroup USBD_CONF_Exported_Variables USBD_CONF_Exported_Variables
  * @brief Public variables.
  * @{
  */

 /**
  * @}
  */

 /** @defgroup USBD_CONF_Exported_Defines USBD_CONF_Exported_Defines
  * @brief Defines for configuration of the Usb device.
  * @{
  */

 /*---------- -----------*/
 /*---------- -----------*/
#define USBD_MAX_NUM_CONFIGURATION 1U
 /*---------- -----------*/
#define USBD_MAX_STR_DESC_SIZ 512U
 /*---------- -----------*/
#define USBD_DEBUG_LEVEL 0U
 /*---------- -----------*/
#define USBD_LPM_ENABLED 1U
 /*---------- -----------*/
#define USBD_SELF_POWERED 1U
 /*---------- -----------*/
#define USBD_AUDIO_FREQ               48000U
#define USBD_AUDIO_CHANNELS           2
#define USBD_AUDIO_BYTES_PER_SAMPLE   2
#define AUDIO_OUT_NUMBER              2
#define AUDIO_IN_NUMBER               1
#define USBD_MAX_NUM_INTERFACES       (AUDIO_OUT_NUMBER + AUDIO_IN_NUMBER + 2)
#define USBD_SUPPORT_USER_STRING_DESC 1

 /****************************************/
 /* #define for FS and HS identification */
#define DEVICE_FS 0
#define DEVICE_HS 1

 /**
  * @}
  */

 /** @defgroup USBD_CONF_Exported_Macros USBD_CONF_Exported_Macros
  * @brief Aliases.
  * @{
  */

 /* Memory management macros */

 /** Alias for memory allocation. */
#define USBD_malloc malloc

 /** Alias for memory release. */
#define USBD_free free

 /** Alias for memory set. */
#define USBD_memset memset

 /** Alias for memory copy. */
#define USBD_memcpy memcpy

 /** Alias for delay. */
#define USBD_Delay HAL_Delay

 /* DEBUG macros */

#if (USBD_DEBUG_LEVEL > 0)
#define USBD_UsrLog(...) \
   printf(__VA_ARGS__);   \
   printf("\n");
#else
#define USBD_UsrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 1)

#define USBD_ErrLog(...) \
   printf("ERROR: ");     \
   printf(__VA_ARGS__);   \
   printf("\n");
#else
#define USBD_ErrLog(...)
#endif

#if (USBD_DEBUG_LEVEL > 2)
#define USBD_DbgLog(...) \
   printf("DEBUG : ");    \
   printf(__VA_ARGS__);   \
   printf("\n");
#else
#define USBD_DbgLog(...)
#endif

 /**
  * @}
  */

 /** @defgroup USBD_CONF_Exported_Types USBD_CONF_Exported_Types
  * @brief Types.
  * @{
  */

 /**
  * @}
  */

 /** @defgroup USBD_CONF_Exported_FunctionsPrototype USBD_CONF_Exported_FunctionsPrototype
  * @brief Declaration of public functions for Usb device.
  * @{
  */

 /* Exported functions -------------------------------------------------------*/

 /**
  * @}
  */

 /**
  * @}
  */

 /**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CONF__H__ */
