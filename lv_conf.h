#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_USE_DEV_VERSION 1
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 1
#define LV_DPI 128
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR 0
#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 0
#define LV_USE_ASSERT_MEM 0
#define LV_USE_ASSERT_STR 0
#define LV_USE_ASSERT_OBJ 0
#define LV_USE_ASSERT_STYLE 0

/* Memory settings */
#define LV_MEM_CUSTOM 0
#if LV_MEM_CUSTOM == 0
#define LV_MEM_SIZE (64 * 1024)
#endif

/* HAL settings */
#define LV_TICK_CUSTOM 0
#define LV_SPRINTF_CUSTOM 0

/* Display driver */
#define LV_USE_DRAW_SW 1
#define LV_USE_DRAW_VG_LITE 0
#define LV_USE_DRAW_PX 0

/* Widgets */
#define LV_USE_BTN 1
#define LV_USE_LABEL 1
#define LV_USE_IMG 1
#define LV_USE_SLIDER 1
#define LV_USE_SWITCH 1
#define LV_USE_DROPDOWN 1
#define LV_USE_ROLLER 1
#define LV_USE_TEXTAREA 1
#define LV_USE_CHART 1
#define LV_USE_TABLE 1
#define LV_USE_MENU 1
#define LV_USE_MSGBOX 1
#define LV_USE_TABVIEW 1
#define LV_USE_WIN 1
#define LV_USE_SPINBOX 1
#define LV_USE_KEYBOARD 1
#define LV_USE_LED 1

/* Touch driver */
#define LV_USE_INDEV 1

/* File system */
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0

/* PNG decoder (for LVGL images) */
#define LV_USE_PNG 0
#define LV_USE_BMP 0
#define LV_USE_SJPG 0
#define LV_USE_GIF 0
#define LV_USE_QRCODE 0
#define LV_USE_BARCODE 0

/* Animations */
#define LV_USE_THEME_DEFAULT 1
#define LV_USE_THEME_BASIC 1
#define LV_USE_THEME_MONO 1

#endif /* LV_CONF_H */
