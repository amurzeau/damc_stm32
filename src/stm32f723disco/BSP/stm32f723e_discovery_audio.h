/**
  ******************************************************************************
  * @file    stm32f723e_discovery_audio.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32f723e_discovery_audio.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F723E_DISCOVERY_AUDIO_H
#define __STM32F723E_DISCOVERY_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* Include audio component Driver */
#include "../Components/wm8994/wm8994.h"
#include "stm32f723e_discovery.h"
#include <stdlib.h>

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32F723E_DISCOVERY
  * @{
  */
    
/** @defgroup STM32F723E_DISCOVERY_AUDIO STM32F723E_DISCOVERY_AUDIO
  * @{
  */

/** @defgroup STM32F723E_DISCOVERY_AUDIO_Exported_Types STM32F723E_DISCOVERY_AUDIO Exported Types
  * @{
  */
/**
  * @}
  */ 

/** @defgroup STM32F723E_DISCOVERY_AUDIO_Exported_Constants STM32F723E_DISCOVERY_AUDIO Exported Constants
  * @{
  */

/** @defgroup BSP_Audio_Out_Option BSP Audio Out Option
  * @{
  */
#define BSP_AUDIO_OUT_CIRCULARMODE      ((uint32_t)0x00000001) /* BUFFER CIRCULAR MODE */
#define BSP_AUDIO_OUT_NORMALMODE        ((uint32_t)0x00000002) /* BUFFER NORMAL MODE   */
#define BSP_AUDIO_OUT_STEREOMODE        ((uint32_t)0x00000004) /* STEREO MODE          */
#define BSP_AUDIO_OUT_MONOMODE          ((uint32_t)0x00000008) /* MONO MODE            */
/**
  * @}
  */
/** @defgroup BSP_Audio_Sample_Rate BSP Audio Sample Rate
  * @{
  */
#define BSP_AUDIO_FREQUENCY_96K         SAI_AUDIO_FREQUENCY_96K
#define BSP_AUDIO_FREQUENCY_48K         SAI_AUDIO_FREQUENCY_48K
#define BSP_AUDIO_FREQUENCY_44K         SAI_AUDIO_FREQUENCY_44K
#define BSP_AUDIO_FREQUENCY_32K         SAI_AUDIO_FREQUENCY_32K
#define BSP_AUDIO_FREQUENCY_22K         SAI_AUDIO_FREQUENCY_22K
#define BSP_AUDIO_FREQUENCY_16K         SAI_AUDIO_FREQUENCY_16K
#define BSP_AUDIO_FREQUENCY_11K         SAI_AUDIO_FREQUENCY_11K
#define BSP_AUDIO_FREQUENCY_8K          SAI_AUDIO_FREQUENCY_8K
/**
  * @}
  */
    
/*------------------------------------------------------------------------------
                          USER SAI defines parameters
 -----------------------------------------------------------------------------*/
/** CODEC_AudioFrame_SLOT_TDMMode In W8994 codec the Audio frame contains 4 slots : TDM Mode
  * TDM format :
  * +------------------|------------------|--------------------|-------------------+ 
  * | CODEC_SLOT0 Left | CODEC_SLOT1 Left | CODEC_SLOT0 Right  | CODEC_SLOT1 Right |
  * +------------------------------------------------------------------------------+
  */
/* To have 2 separate audio stream in Both headphone and speaker the 4 slot must be activated */
#define CODEC_AUDIOFRAME_SLOT_0123                   SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1 | SAI_SLOTACTIVE_2 | SAI_SLOTACTIVE_3

/* To have an audio stream in headphone only SAI Slot 0 and Slot 2 must be activated */ 
#define CODEC_AUDIOFRAME_SLOT_02                     SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_2 
/* To have an audio stream in speaker only SAI Slot 1 and Slot 3 must be activated */ 
#define CODEC_AUDIOFRAME_SLOT_13                     SAI_SLOTACTIVE_1 | SAI_SLOTACTIVE_3

                                                       
/* SAI peripheral configuration defines */
#define AUDIO_OUT_SAIx                           SAI2_Block_A
#define AUDIO_OUT_SAIx_CLK_ENABLE()              __HAL_RCC_SAI2_CLK_ENABLE()
#define AUDIO_OUT_SAIx_CLK_DISABLE()             __HAL_RCC_SAI2_CLK_DISABLE()
#define AUDIO_OUT_SAIx_AF                        GPIO_AF10_SAI2

#define AUDIO_OUT_SAIx_MCLK_ENABLE()             __HAL_RCC_GPIOI_CLK_ENABLE()
#define AUDIO_OUT_SAIx_MCLK_GPIO_PORT            GPIOI
#define AUDIO_OUT_SAIx_MCLK_PIN                  GPIO_PIN_4
#define AUDIO_OUT_SAIx_SD_FS_CLK_ENABLE()        __HAL_RCC_GPIOI_CLK_ENABLE()
#define AUDIO_OUT_SAIx_SD_FS_SCK_GPIO_PORT       GPIOI
#define AUDIO_OUT_SAIx_FS_PIN                    GPIO_PIN_7   
#define AUDIO_OUT_SAIx_SCK_PIN                   GPIO_PIN_5
#define AUDIO_OUT_SAIx_SD_PIN                    GPIO_PIN_6

/* SAI DMA Stream definitions */
#define AUDIO_OUT_SAIx_DMAx_CLK_ENABLE()         __HAL_RCC_DMA2_CLK_ENABLE()
#define AUDIO_OUT_SAIx_DMAx_STREAM               DMA2_Stream4
#define AUDIO_OUT_SAIx_DMAx_CHANNEL              DMA_CHANNEL_3
#define AUDIO_OUT_SAIx_DMAx_IRQ                  DMA2_Stream4_IRQn
#define AUDIO_OUT_SAIx_DMAx_PERIPH_DATA_SIZE     DMA_PDATAALIGN_WORD
#define AUDIO_OUT_SAIx_DMAx_MEM_DATA_SIZE        DMA_MDATAALIGN_WORD
#define DMA_MAX_SZE                              0xFFFF
   
#define AUDIO_OUT_SAIx_DMAx_IRQHandler           DMA2_Stream4_IRQHandler

/* Select the interrupt preemption priority for the DMA interrupt */
#define AUDIO_OUT_IRQ_PREPRIO                ((uint32_t)0x0E)   /* Select the preemption priority level(0 is the highest) */   

/*------------------------------------------------------------------------------
                        AUDIO IN CONFIGURATION
------------------------------------------------------------------------------*/
/* SAI IN peripheral configuration defines */
#define AUDIO_IN_SAIx                            SAI2_Block_B
#define AUDIO_IN_SAIx_CLK_ENABLE()               __HAL_RCC_SAI2_CLK_ENABLE()
#define AUDIO_IN_SAIx_CLK_DISABLE()              __HAL_RCC_SAI2_CLK_DISABLE()
#define AUDIO_IN_SAIx_SD_AF                      GPIO_AF10_SAI2

#define AUDIO_IN_SAIx_SD_ENABLE()                __HAL_RCC_GPIOG_CLK_ENABLE()
#define AUDIO_IN_SAIx_SD_GPIO_PORT               GPIOG
#define AUDIO_IN_SAIx_SD_PIN                     GPIO_PIN_10

#define AUDIO_IN_INT_GPIO_ENABLE()               __HAL_RCC_GPIOG_CLK_ENABLE()
#define AUDIO_IN_INT_GPIO_PORT                   GPIOG
#define AUDIO_IN_INT_GPIO_PIN                    GPIO_PIN_15
#define AUDIO_IN_INT_IRQ                         EXTI15_10_IRQn

/* SAI DMA Stream definitions */
#define AUDIO_IN_SAIx_DMAx_CLK_ENABLE()          __HAL_RCC_DMA2_CLK_ENABLE()
#define AUDIO_IN_SAIx_DMAx_STREAM                DMA2_Stream6
#define AUDIO_IN_SAIx_DMAx_CHANNEL               DMA_CHANNEL_3
#define AUDIO_IN_SAIx_DMAx_IRQ                   DMA2_Stream6_IRQn
#define AUDIO_IN_SAIx_DMAx_PERIPH_DATA_SIZE      DMA_PDATAALIGN_WORD
#define AUDIO_IN_SAIx_DMAx_MEM_DATA_SIZE         DMA_MDATAALIGN_WORD

#define AUDIO_IN_SAIx_DMAx_IRQHandler            DMA2_Stream6_IRQHandler
#define AUDIO_IN_INT_IRQHandler                  EXTI15_10_IRQHandler
  
/* Select the interrupt preemption priority and subpriority for the IT/DMA interrupt */
#define AUDIO_IN_IRQ_PREPRIO                ((uint32_t)0x0F)   /* Select the preemption priority level(0 is the highest) */


/*------------------------------------------------------------------------------
             CONFIGURATION: Audio Driver Configuration parameters
------------------------------------------------------------------------------*/

#define AUDIODATA_SIZE                      2   /* 16-bits audio data size */

/* Audio status definition */     
#define AUDIO_OK                            ((uint8_t)0)
#define AUDIO_ERROR                         ((uint8_t)1)
#define AUDIO_TIMEOUT                       ((uint8_t)2)

/* AudioFreq * DataSize (2 bytes) * NumChannels (Stereo: 2) */
#define DEFAULT_AUDIO_IN_FREQ               BSP_AUDIO_FREQUENCY_16K
#define DEFAULT_AUDIO_IN_BIT_RESOLUTION     ((uint8_t)16)
#define DEFAULT_AUDIO_IN_CHANNEL_NBR        ((uint8_t)2) /* Mono = 1, Stereo = 2 */
#define DEFAULT_AUDIO_IN_VOLUME             ((uint16_t)64)
   
/*------------------------------------------------------------------------------
                    OPTIONAL Configuration defines parameters
------------------------------------------------------------------------------*/

/* Delay for the Codec to be correctly reset */
#define CODEC_RESET_DELAY                   ((uint8_t)5)
   

/*------------------------------------------------------------------------------
                            OUTPUT DEVICES definition
------------------------------------------------------------------------------*/
/* Alias on existing output devices to adapt to headphones output */
#define OUTPUT_DEVICE_HEADPHONE1 OUTPUT_DEVICE_HEADPHONE
#define OUTPUT_DEVICE_HEADPHONE2 OUTPUT_DEVICE_SPEAKER 
   
/**
  * @}
  */
 
/** @defgroup STM32F723E_DISCOVERY_AUDIO_Exported_Variables STM32F723E_DISCOVERY_AUDIO Exported Variables
  * @{
  */
 /* Play  */
 extern SAI_HandleTypeDef haudio_out_sai;

 /* Record  */
 extern SAI_HandleTypeDef haudio_in_sai;
 /**
  * @}
  */
   
/** @defgroup STM32F723E_DISCOVERY_AUDIO_Exported_Macros STM32F723E_DISCOVERY_AUDIO Exported Macros
  * @{
  */
#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)
/**
  * @}
  */ 

/** @defgroup STM32F723E_DISCOVERY_AUDIO_OUT_Exported_Functions STM32F723E_DISCOVERY_AUDIO_OUT Exported Functions
  * @{
  */
uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
void    BSP_AUDIO_OUT_DeInit(void);
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size);
uint16_t BSP_AUDIO_OUT_GetRemainingCount(void);
uint8_t BSP_AUDIO_OUT_Pause(void);
uint8_t BSP_AUDIO_OUT_Resume(void);
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option);
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume);
void    BSP_AUDIO_OUT_SetFrequency(uint32_t AudioFreq);
void    BSP_AUDIO_OUT_SetAudioFrameSlot(uint32_t AudioFrameSlot);
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd);
uint8_t BSP_AUDIO_OUT_SetOutputMode(uint8_t Output);
void    BSP_AUDIO_OUT_ChangeBuffer(uint16_t *pData, uint16_t Size);

/* User Callbacks: user has to implement these functions in his code if they are needed. */
/* This function is called when the requested data has been completely transferred.*/
void    BSP_AUDIO_OUT_TransferComplete_CallBack(void);

/* This function is called when half of the requested buffer has been transferred. */
void    BSP_AUDIO_OUT_HalfTransfer_CallBack(void);

/* This function is called when an Interrupt due to transfer error on or peripheral
   error occurs. */
void    BSP_AUDIO_OUT_Error_CallBack(void);

/* These function can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void  BSP_AUDIO_OUT_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t AudioFreq, void *Params);
void  BSP_AUDIO_OUT_MspInit(SAI_HandleTypeDef *hsai, void *Params);
void  BSP_AUDIO_OUT_MspDeInit(SAI_HandleTypeDef *hsai, void *Params);

/**
  * @}
  */ 

/** @defgroup STM32F723E_DISCOVERY_AUDIO_IN_Exported_Functions STM32F723E_DISCOVERY_AUDIO_IN Exported Functions
  * @{
  */
uint8_t BSP_AUDIO_IN_Init(uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
uint8_t BSP_AUDIO_IN_InitEx(uint16_t InputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);
uint8_t BSP_AUDIO_IN_OUT_Init(uint16_t InputDevice, uint16_t OutputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr, uint8_t OutVolume, uint8_t InVolume);
void    BSP_AUDIO_IN_DeInit(void);
uint8_t BSP_AUDIO_IN_Record(uint16_t *pData, uint32_t Size);
uint16_t BSP_AUDIO_IN_GetRemainingCount(void);
uint8_t BSP_AUDIO_IN_Stop(uint32_t Option);
uint8_t BSP_AUDIO_IN_Pause(void);
uint8_t BSP_AUDIO_IN_Resume(void);
uint8_t BSP_AUDIO_IN_SetVolume(uint8_t Volume);
void    BSP_AUDIO_IN_DeInit(void);

/* User Callbacks: user has to implement these functions in his code if they are needed. */
/* This function should be implemented by the user application.
   It is called into this driver when the current buffer is filled to prepare the next
   buffer pointer and its size. */
void    BSP_AUDIO_IN_TransferComplete_CallBack(void);
void    BSP_AUDIO_IN_HalfTransfer_CallBack(void);

/* This function is called when an Interrupt due to transfer error on or peripheral
   error occurs. */
void BSP_AUDIO_IN_Error_CallBack(void);
     
/* These function can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void BSP_AUDIO_IN_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t AudioFreq, void *Params);
void BSP_AUDIO_IN_MspInit(SAI_HandleTypeDef *hsai, void *Params);
void BSP_AUDIO_IN_MspDeInit(SAI_HandleTypeDef *hsai, void *Params);

/**
  * @}
  */ 

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

#endif /* __STM32F723E_DISCOVERY_AUDIO_H */

