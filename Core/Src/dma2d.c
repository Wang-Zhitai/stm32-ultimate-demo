/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    dma2d.c
 * @brief   This file provides code for the configuration
 *          of the DMA2D instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "dma2d.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

DMA2D_HandleTypeDef hdma2d;

/* DMA2D init function */
void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspInit 0 */

  /* USER CODE END DMA2D_MspInit 0 */
    /* DMA2D clock enable */
    __HAL_RCC_DMA2D_CLK_ENABLE();

    /* DMA2D interrupt Init */
    HAL_NVIC_SetPriority(DMA2D_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);
  /* USER CODE BEGIN DMA2D_MspInit 1 */

  /* USER CODE END DMA2D_MspInit 1 */
  }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef* dma2dHandle)
{

  if(dma2dHandle->Instance==DMA2D)
  {
  /* USER CODE BEGIN DMA2D_MspDeInit 0 */

  /* USER CODE END DMA2D_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_DMA2D_CLK_DISABLE();

    /* DMA2D interrupt Deinit */
    HAL_NVIC_DisableIRQ(DMA2D_IRQn);
  /* USER CODE BEGIN DMA2D_MspDeInit 1 */

  /* USER CODE END DMA2D_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
void ltdc_color_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t *px_map, uint32_t framebuffer)
{
  uint16_t offline;
  uint32_t addr;

  // 行偏移：按字节计算，每个像素3字节
  offline = (800 - (x2 - x1 + 1)) * 3;

  // 地址计算：每个像素3字节
  addr = ((uint32_t)framebuffer + 3 * (800 * y1 + x1));

  SCB_CleanInvalidateDCache();
  __HAL_RCC_DMA2D_CLK_ENABLE(); /* 使能DM2D时钟 */

  DMA2D->CR &= ~(DMA2D_CR_START);            /* 先停止DMA2D */
  DMA2D->CR = DMA2D_M2M;                     /* 存储器到存储器模式 */
  DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_RGB888; /* 改为RGB888格式 */
  DMA2D->FGOR = 0;                           /* 前景层行偏移为0 */
  DMA2D->OOR = offline;                      /* 设置行偏移 */

  DMA2D->FGMAR = (uint32_t)px_map;                    /* 源地址 */
  DMA2D->OMAR = addr;                                 /* 输出存储器地址 */
  DMA2D->NLR = (y2 - y1 + 1) | ((x2 - x1 + 1) << 16); /* 设定行数寄存器 */
  DMA2D->CR |= DMA2D_CR_START;                        /* 启动DMA2D */

  __HAL_DMA2D_ENABLE_IT(&hdma2d, DMA2D_IT_TC);
  __HAL_DMA2D_ENABLE(&hdma2d);
}
/* USER CODE END 1 */
