#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

#define LV_USE_DEV_VERSION 1
#define LV_COLOR_DEPTH 32
#define LV_COLOR_16_SWAP 0
#define LV_DPI 96
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0
#define LV_USE_LOG 0

/* Memory */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (128 * 1024)

/* HAL */
#define LV_TICK_CUSTOM 0

/* Display */
#define LV_USE_DRAW_SW 1
#define LV_DRAW_SW_ASM LV_DRAW_SW_ASM_NONE

/* Fonts */
#define LV_FONT_MONTSERRAT_8  1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_34 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_48 1

/* Widgets */
#define LV_USE_BTN         1
#define LV_USE_LABEL       1
#define LV_USE_DROPDOWN    1
#define LV_USE_SLIDER      1
#define LV_USE_SWITCH      1
#define LV_USE_ROLLER      1
#define LV_USE_LIST        1
#define LV_USE_TABVIEW     1
#define LV_USE_LED         1
#define LV_USE_MSGBOX     1
#define LV_USE_CHART       1
#define LV_USE_IMG         1
#define LV_USE_CANVAS      1
#define LV_USE_ANIMIMG     1
#define LV_USE_TEXTAREA    1
#define LV_USE_KEYBOARD    1
#define LV_USE_SPINBOX     1
#define LV_USE_CALENDAR    1
#define LV_USE_WIN         1
#define LV_USE_SPINNER     1
#define LV_USE_DROPDOWN    1
#define LV_USE_CHECKBOX    1

/* Themes */
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_BASIC 1
#define LV_USE_THEME_MONO 1

/* Input device */
#define LV_USE_INDEV 1

/* Disable ThorVG to avoid potential init hangs */
#define LV_USE_THORVG_INTERNAL 0
#define LV_USE_THORVG_EXTERNAL 0

/* File system */
#define LV_USE_FS_STDIO 0

/* Animations */
#define LV_USE_THEME_DEFAULT 1

#endif
