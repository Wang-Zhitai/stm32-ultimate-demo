/**
 * @file bsp.c
 * @author {WangZhitai} ({1930998910@qq.com})
 * @brief
 * @version 0.0.1
 * @date 2025-10-27
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "bsp.h"
#include "bsp_gt911.h"

uint8_t BSP_Init(void)
{
    BSP_GT911_Init();
    return 0;
}
