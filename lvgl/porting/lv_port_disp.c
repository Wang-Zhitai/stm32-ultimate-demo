/**
 * @file lv_port_disp.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
#include <string.h>
#include "lvgl.h"
#include "ltdc.h"
#include "dma2d.h"

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
#define MY_DISP_HOR_RES 800
#endif

#ifndef MY_DISP_VER_RES
#warning Please define or replace the macro MY_DISP_VER_RES with the actual screen height, default value 240 is used for now.
#define MY_DISP_VER_RES 480
#endif

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888)) /*will be 2 for RGB565 */

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_display_t *disp;
volatile bool FirstFrameReady = false;
volatile bool SecondFrameReady = false;
uint32_t FrameBuffer = 0xC0000000;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);

    // /* Example 1
    //  * One buffer for partial rendering*/
    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_1_1[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL]; /*A buffer for 10 rows*/
    // lv_display_set_buffers(disp, buf_1_1, NULL, sizeof(buf_1_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // /* Example 2
    //  * Two buffers for partial rendering
    //  * In flush_cb DMA or similar hardware should be used to update the display in the background.*/
    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_2_1[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];

    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_2_2[MY_DISP_HOR_RES * 10 * BYTE_PER_PIXEL];
    // lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);

    // /* Example 3
    //  * Two buffers screen sized buffer for double buffering.
    //  * Both LV_DISPLAY_RENDER_MODE_DIRECT and LV_DISPLAY_RENDER_MODE_FULL works, see their comments*/
    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_3_1[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];

    // LV_ATTRIBUTE_MEM_ALIGN
    // static uint8_t buf_3_2[MY_DISP_HOR_RES * MY_DISP_VER_RES * BYTE_PER_PIXEL];
    // lv_display_set_buffers(disp, buf_3_1, buf_3_2, sizeof(buf_3_1), LV_DISPLAY_RENDER_MODE_DIRECT);
    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t buf_2_1[MY_DISP_HOR_RES * 80 * BYTE_PER_PIXEL] __attribute__((at(0x24000000)));

    LV_ATTRIBUTE_MEM_ALIGN
    static uint8_t buf_2_2[MY_DISP_HOR_RES * 80 * BYTE_PER_PIXEL] __attribute__((at(0x24000000 + MY_DISP_HOR_RES * 80 * BYTE_PER_PIXEL)));
    lv_display_set_buffers(disp, buf_2_1, buf_2_2, sizeof(buf_2_1), LV_DISPLAY_RENDER_MODE_PARTIAL);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /*You code here*/
    memset((uint8_t *)0xC0000000, 0x0000, MY_DISP_HOR_RES * MY_DISP_VER_RES * 3); // 开机清屏，全黑
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

void HAL_LTDC_LineEvenCallback(LTDC_HandleTypeDef *hltdc)
{
    // 重新载入参数，新显存地址生效，此时显示才会更新
    // 每次进入中断才会更新显示，这样能有效避免撕裂现象
    __HAL_LTDC_RELOAD_CONFIG(hltdc);
    HAL_LTDC_ProgramLineEvent(hltdc, 515); // 重新设置中断
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    if (disp_flush_enabled)
    {
        /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
        ltdc_color_fill(area->x1, area->y1, area->x2, area->y2, (uint32_t *)px_map, FrameBuffer);
    }

    if (lv_disp_flush_is_last(disp))
    {
        if (FrameBuffer == 0xC0000000)
        {
            FirstFrameReady = true;
            LTDC_Layer1->CFBAR = FrameBuffer;
            FrameBuffer = 0xC0119400;
        }
        else if (FrameBuffer == 0xC0119400)
        {
            SecondFrameReady = true;
            LTDC_Layer1->CFBAR = FrameBuffer;
            FrameBuffer = 0xC0000000;
        }
    }
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
