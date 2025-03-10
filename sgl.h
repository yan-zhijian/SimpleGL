#ifndef __SIMPLEGL_H
#define __SIMPLEGL_H

#include "stm32f10x.h"
#include "font.h"

#define sgl_FORMAT_STRING_BUFFERSIZE 100

#define color_mono_light    0
#define color_mono_dark     1
#define color_mono_inverse  2

typedef enum
{
    sgl_mode_full_buffer,
    sgl_mode_page,
    sgl_mode_Partial,
} sgl_mode_t;

typedef enum
{
    sgl_fillstyle_filled,
    sgl_fillstyle_outlined,
} sgl_fillstyle_t;

typedef enum
{
    sgl_dir_up = 0,
    sgl_dir_down,
    sgl_dir_left,
    sgl_dir_right,
} sgl_dir_t;

#define sgl_dir_default sgl_dir_up

typedef enum
{
    sgl_align_up_left = 0,
    sgl_align_up_right,
    sgl_align_down_left,
    sgl_align_down_right,
    sgl_align_center,
    sgl_align_up_center,
    sgl_align_down_center,
    sgl_align_left_center,
    sgl_align_right_center,
} sgl_align_t;

#define sgl_align_default sgl_align_up_left

/**
  * alignment:
  * 
  *     up_left           up_center           up_right
  *             +-------------+-------------+
  *             |                           |
  *             |                           |
  * left_center +        center(x,y)        + right_center
  *             |                           |
  *             |                           |
  *             +-------------+-------------+
  *   down_left          down_center          down_right
  *
  */

typedef enum
{
    sgl_rotate_0 = 0,
    sgl_rotate_90,
    sgl_rotate_180,
    sgl_rotate_270,
} sgl_rotate_t;

#define sgl_rotate_default sgl_rotate_0

/**
  * rotate shape:坐标轴固定，图像旋转
  *
  * -------------------------
  * |rotate |   x'  |   y'  |
  * |-------|-------|-------|
  * |   0   |   x   |   y   |
  * |   90  |  w-y  |   x   |
  * |  -90  |   y   |  h-x  |
  * |  180  |  w-x  |  h-y  |
  * -------------------------
  *
  * rotate axis:图像固定，坐标轴旋转
  *
  * -------------------------
  * |rotate |   x'  |   y'  |
  * |-------|-------|-------|
  * |   0   |   x   |   y   |
  * |   90  |   y   |  w-x  |
  * |  -90  |  h-y  |   x   |
  * |  180  |  w-x  |  h-y  |
  * -------------------------
  *
  * w(width):hor_res-1  h(height):ver_res-1
  *
  */

typedef enum
{
    sgl_circle_section_upper_right = 0x01,
    sgl_circle_section_upper_left  = 0x02,
    sgl_circle_section_lower_right = 0x04,
    sgl_circle_section_lower_left  = 0x08,
    sgl_circle_section_all         = 0xFF,
} sgl_circle_section_t;

/**
  * buffer structrue:
  *
  * |------------width------------|
  * ----------buffer_width----------------start_y-------
  * |                             |     |              |
  * |           page_1            | buffer_height      |
  * |                             |     |              |
  * |-----------screen------------|--------end_y-----height
  * |                             |                    |
  * |           page_2            |                    |
  * |                             |                    |
  * ----------------------------------------------------
  *
  */

typedef struct
{
    int left;
    int right;
    int top;
    int bottom;
} sgl_rect_t;


typedef struct sgl_painter_t
{
    uint8_t* buffer;                                    // 缓冲区指针
    int buffer_size;                                    // 缓冲区大小
    int hor_res;                                        // 屏幕水平分辨率(X坐标范围的大小)
    int ver_res;                                        // 屏幕垂直分辨率(Y坐标范围的大小)
    
    int page_start_raw;                                 // 页起始行
    int page_end_raw;                                   // 页结束行
    int page_width;                                     // 页宽
    
    int offset_x;                                       // X坐标偏移
    int offset_y;                                       // Y坐标偏移
    
    int buffer_width;                                   // 缓冲区宽度
    int buffer_height;                                  // 缓冲区高度
    sgl_rect_t visible;                                 // 可见矩形区域
    font_info_t font_info;                              // 字体信息
    sgl_rotate_t rotate;                                // 屏幕旋转参数
    uint32_t background;                                // 背景颜色(默认为0)
    sgl_mode_t mode;                                    // 绘制模式
    void (* draw_piexl)(struct sgl_painter_t* painter,  // 绘制像素函数指针
        int x, int y, uint32_t color);
    void (* draw)(struct sgl_painter_t* painter);       // 绘制回调函数
    void (* flush)(void* buffer, uint32_t buffer_size); // 发送数据回调函数
    void (* frame_start_cb)(void);                      // 帧起始回调函数
    void (* frame_end_cb)(void);                        // 帧结束回调函数
} sgl_painter_t;

// 初始化函数
void sgl_InitPainter(sgl_painter_t* painter, void* buffer, uint32_t buffer_size, uint32_t hor_res, uint32_t ver_res, sgl_mode_t mode);
void sgl_display(sgl_painter_t* painter);
void sgl_set_draw(sgl_painter_t* painter, void (* draw)(struct sgl_painter_t*));
void sgl_set_flush(sgl_painter_t* painter, void (* flush)(void*, uint32_t));
// 功能函数
void sgl_set_draw_piexl(sgl_painter_t* painter, void (* draw_piexl)(sgl_painter_t*, int, int, uint32_t));
void sgl_set_frame_start_cb(sgl_painter_t* painter, void (* frame_start_cb)(void));
void sgl_set_frame_end_cb(sgl_painter_t* painter, void (* frame_end_cb)(void));
void sgl_set_font(sgl_painter_t* painter, font_t font);
void sgl_set_background(sgl_painter_t* painter, uint32_t background);
void sgl_set_screen_rotation(sgl_painter_t* painter, sgl_rotate_t rotate);
void sgl_paint_start(sgl_painter_t* painter);
void sgl_paint_end(sgl_painter_t* painter);
void sgl_set_visible_rect(sgl_painter_t* painter, int left, int top, int right, int bottom);
void sgl_reset_visible_rect(sgl_painter_t* painter);
void sgl_draw_piexl_mono(sgl_painter_t* painter, int x, int y, uint32_t color);
void sgl_draw_piexl_rgb565(sgl_painter_t* painter, int x, int y, uint32_t color);
void sgl_align(int* x, int* y, int width, int height, sgl_align_t align);
void sgl_rotated2original(int* x, int* y, uint32_t hor_res, uint32_t ver_res, sgl_rotate_t rotate);
void sgl_original2rotated(int* x, int* y, uint32_t hor_res, uint32_t ver_res, sgl_rotate_t rotate);
// 图型函数
void sgl_draw_point(sgl_painter_t* painter, int x, int y, uint32_t color);
void sgl_draw_hline(sgl_painter_t* painter, int x0, int x1, int y, uint32_t color);
void sgl_draw_vline(sgl_painter_t* painter, int y0, int y1, int x, uint32_t color);
void sgl_draw_hline_with_length(sgl_painter_t* painter, int x, int y, int length, uint32_t color);
void sgl_draw_vline_with_length(sgl_painter_t* painter, int x, int y, int length, uint32_t color);
void sgl_draw_line(sgl_painter_t* painter, int x0, int y0, int x1, int y1, uint32_t color);
void sgl_draw_polyline(sgl_painter_t* painter, int* x, int* y, int size, uint32_t color);
void sgl_draw_polygon(sgl_painter_t* painter, int* x, int* y, int size, sgl_fillstyle_t fillstyle, uint32_t color);
void sgl_draw_triangle(sgl_painter_t* painter, int x0, int y0, int x1, int y1, int x2, int y2, sgl_fillstyle_t fillstyle, uint32_t color);
void sgl_draw_rectangle(sgl_painter_t* painter, int x0, int y0, int x1, int y1, sgl_fillstyle_t fillstyle, uint32_t color);
void sgl_draw_rectangle_with_length(sgl_painter_t* painter, int x, int y, int width, int height, sgl_fillstyle_t fillstyle, uint32_t color);
void sgl_draw_outlined_circle_section(sgl_painter_t* painter, int xc, int yc, int r, sgl_circle_section_t section, uint32_t color);
void sgl_draw_filled_circle_section(sgl_painter_t* painter, int xc, int yc, int r, sgl_circle_section_t section, uint32_t color);
void sgl_draw_circle(sgl_painter_t* painter, int xc, int yc, int r, sgl_fillstyle_t fillstyle, uint32_t color);
void sgl_draw_outlined_ellipse_section(sgl_painter_t* painter, int xc, int yc, int rx, int ry, sgl_circle_section_t section, uint32_t color);
void sgl_draw_filled_ellipse_section(sgl_painter_t* painter, int xc, int yc, int rx, int ry, sgl_circle_section_t section, uint32_t color);
void sgl_draw_ellipse(sgl_painter_t* painter, int xc, int yc, int rx, int ry, sgl_fillstyle_t fillstyle, uint32_t color);
void sgl_draw_round_rectangle(sgl_painter_t* painter, int x0, int y0, int x1, int y1, int r, sgl_fillstyle_t fillstyle, uint32_t color);
void sgl_draw_round_rectangle_with_length(sgl_painter_t* painter, int x, int y, int width, int height, int r, sgl_fillstyle_t fillstyle, uint32_t color);
// 字符函数
void sgl_show_mono_image(sgl_painter_t* painter, int x, int y, int width, int height, const uint8_t* image, sgl_dir_t dir, uint32_t color);
void sgl_show_string(sgl_painter_t* painter, int x, int y, const char* str, int length, sgl_align_t align, sgl_dir_t dir, uint32_t color);
int sgl_show_format_string(sgl_painter_t* painter, int x, int y, sgl_align_t align, sgl_dir_t dir, uint32_t color, const char* format, ...);

#endif
