#include "sgl.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

void sgl_InitPainter(sgl_painter_t* painter, void* buffer, uint32_t buffer_size, uint32_t hor_res, uint32_t ver_res, sgl_mode_t mode)
{
    painter->buffer = buffer;
    painter->buffer_size = buffer_size;
    painter->hor_res = hor_res;
    painter->ver_res = ver_res;
    painter->visible.left = 0;
    painter->visible.right = hor_res - 1;
    painter->visible.top = 0;
    painter->visible.bottom = ver_res - 1;
    painter->mode = mode;
    
    painter->buffer_width = hor_res;
    painter->buffer_height = ver_res;
    painter->offset_x = 0;
    painter->offset_y = 0;
    
    painter->font_info = font_get_font_info(font_Song_Typeface_8x16);
    painter->background = 0;
    painter->rotate = sgl_rotate_0;
    painter->frame_start_cb = NULL;
    painter->frame_end_cb = NULL;
    painter->draw_piexl = sgl_draw_piexl_mono;
}

void sgl_paint_start(sgl_painter_t* painter)
{
    painter->frame_start_cb();
//    if(painter->page_start_raw == 0 && painter->frame_start_cb)
//        painter->frame_start_cb();
    memset(painter->buffer, painter->background, painter->buffer_size);
}

void sgl_paint_end(sgl_painter_t* painter)
{
    painter->frame_end_cb();
    painter->page_start_raw += painter->page_width;
    if(painter->page_start_raw >= painter->ver_res)
    {
        painter->page_start_raw = 0;
//        if(painter->frame_end_cb != NULL)
//            painter->frame_end_cb();
    }
    painter->page_end_raw = painter->page_start_raw + painter->page_width;
}

void sgl_display(sgl_painter_t* painter)
{
    sgl_paint_start(painter);
    if(painter->draw)
        painter->draw(painter);
    sgl_paint_end(painter);
    if(painter->flush)
        painter->flush(painter->buffer, painter->buffer_size);
}

void sgl_set_draw(sgl_painter_t* painter, void (* draw)(struct sgl_painter_t*))
{
    painter->draw = draw;
}

void sgl_set_flush(sgl_painter_t* painter, void (* flush)(void*, uint32_t))
{
    painter->flush = flush;
}

void sgl_set_draw_piexl(sgl_painter_t* painter, void (* draw_piexl)(sgl_painter_t*, int, int, uint32_t))
{
    painter->draw_piexl = draw_piexl;
}

void sgl_set_frame_start_cb(sgl_painter_t* painter, void (* frame_start_cb)(void))
{
    painter->frame_start_cb = frame_start_cb;
}

void sgl_set_frame_end_cb(sgl_painter_t* painter, void (* frame_end_cb)(void))
{
    painter->frame_end_cb = frame_end_cb;
}

void sgl_set_font(sgl_painter_t* painter, font_t font)
{
    painter->font_info = font_get_font_info(font);
}

void sgl_set_background(sgl_painter_t* painter, uint32_t background)
{
    painter->background = background;
}

void sgl_set_screen_rotation(sgl_painter_t* painter, sgl_rotate_t rotate)
{
    painter->rotate = rotate;
}

static void sgl_set_region(sgl_painter_t* painter, int x0, int y0, int x1, int y1)
{
    painter->offset_x = x0;
    painter->offset_y = y0;
    painter->buffer_width = x1 - x0 + 1;
    painter->buffer_height = y1 -y0 + 1;
}

void sgl_set_visible_rect(sgl_painter_t* painter, int left, int top, int right, int bottom)
{
    if(left < 0)
        left = 0;
    if(top < 0)
        top = 0;
    if(right > painter->hor_res - 1)
        right = painter->hor_res - 1;
    if(bottom > painter->ver_res - 1)
        bottom = painter->ver_res - 1;
    painter->visible.left = left;
    painter->visible.top = top;
    painter->visible.right = right;
    painter->visible.bottom = bottom;
}

void sgl_reset_visible_rect(sgl_painter_t* painter)
{
    painter->visible.left = 0;
    painter->visible.top = 0;
    painter->visible.right = painter->hor_res - 1;
    painter->visible.bottom = painter->ver_res - 1;
}

void sgl_draw_piexl_mono(sgl_painter_t* painter, int x, int y, uint32_t color)
{
    switch(color)
    {
        case color_mono_light:      painter->buffer[(y / 8) * painter->buffer_width + x] |= (1<<(y % 8));     break;
        case color_mono_dark:       painter->buffer[(y / 8) * painter->buffer_width + x] &= ~(1<<(y % 8));    break;
        case color_mono_inverse:    painter->buffer[(y / 8) * painter->buffer_width + x] ^= (1<<(y % 8));     break;
    }
}

void sgl_draw_piexl_rgb565(sgl_painter_t* painter, int x, int y, uint32_t color)
{
    painter->buffer[(x + painter->buffer_width * y) * 2] = color >> 8;
    painter->buffer[(x + painter->buffer_width * y) * 2 + 1] = color;
}

void sgl_align(int* x, int* y, int width, int height, sgl_align_t align)
{
    switch(align)
    {
        case sgl_align_up_left:        break;
        case sgl_align_up_right:       *x -= width - 1;break;
        case sgl_align_down_left:      *y -= height - 1;break;
        case sgl_align_down_right:     *x -= width - 1;*y -= height - 1;break;
        case sgl_align_center:         *x -= width / 2;*y -= height / 2;break;
        case sgl_align_up_center:      *x -= width / 2;break;
        case sgl_align_down_center:    *x -= width / 2;*y -= height - 1;break;
        case sgl_align_left_center:    *y -= height / 2;break;
        case sgl_align_right_center:   *x -= width - 1;*y -= height / 2;break;
        default:break;
    }
}

void sgl_rotated2original(int* x, int* y, uint32_t hor_res, uint32_t ver_res, sgl_rotate_t rotate)
{
    int temp = *x;
    if(rotate == sgl_rotate_90)
    {
        *x = (hor_res - 1) - *y;
        *y = temp;
    }
    else if(rotate == sgl_rotate_270)
    {
        *x = *y;
        *y = (ver_res - 1) - temp;
    }
    else if(rotate == sgl_rotate_180)
    {
        *x = (hor_res - 1) - *x;
        *y = (ver_res - 1) - *y;
    }
}

void sgl_original2rotated(int* x, int* y, uint32_t hor_res, uint32_t ver_res, sgl_rotate_t rotate)
{
    int temp = *x;
    if(rotate == sgl_rotate_90)
    {
        *x = *y;
        *y = (hor_res - 1) - temp;
    }
    else if(rotate == sgl_rotate_270)
    {
        *x = (ver_res - 1) - *y;
        *y = temp;
    }
    else if(rotate == sgl_rotate_180)
    {
        *x = (hor_res - 1) - *x;
        *y = (ver_res - 1) - *y;
    }
}

inline static void sgl_remap(sgl_painter_t* painter, int* x, int* y)
{
    if(painter->rotate)
        sgl_rotated2original(x, y, painter->hor_res, painter->ver_res, painter->rotate);
    
    if(painter->mode == sgl_mode_page)
        *y -= painter->page_start_raw;
    
    if(painter->mode == sgl_mode_Partial)
    {
        *x -= painter->offset_x;
        *y -= painter->offset_y;
    }
}

inline static void sgl_swap(int *var0, int *var1)
{
    *var0 = *var0 ^ *var1;
    *var1 = *var0 ^ *var1;
    *var0 = *var0 ^ *var1;
}

inline static int sgl_pnpoly(int nvert, int *vertx, int *verty, int testx, int testy)///////////////////////////////////////////////
{
    int i, j, c = 0;
    for (i = 0, j = nvert-1; i < nvert; j = i++) {
        if ( ((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
    c = !c;
  }
  return c;
}

inline static int sgl_clip(sgl_painter_t* painter, int *left, int *top, int *right, int *bottom)
{
    sgl_rect_t visible = painter->visible;
    if(*left > visible.right || *right < visible.left || *top > visible.bottom || *bottom < visible.top)
        return -1;
    if(*left < visible.left)
    *left = visible.left;
    if(*right > visible.right)
    *right = visible.right;
    if(*top < visible.top)
    *top = visible.top;
    if(*bottom > visible.bottom)
    *bottom = visible.bottom;
    return 0;
}

inline static int sgl_check_point(sgl_painter_t* painter, int x, int y)
{
    sgl_rect_t visible = painter->visible;
    if(x > visible.right || x < visible.left || y > visible.bottom || y < visible.top)
        return -1;
    else
        return 0;
}

void sgl_draw_point(sgl_painter_t* painter, int x, int y, uint32_t color)
{
    if(sgl_check_point(painter, x, y))
        return;
    sgl_remap(painter, &x, &y);
    painter->draw_piexl(painter, x, y, color);
}

inline static void sgl_draw_original_hline(sgl_painter_t* painter, int x0, int x1, int y, uint32_t color)
{
    int i;
    for(i = x0; i <= x1; ++i)
    {
        painter->draw_piexl(painter, i, y, color);
    }
}

inline static void sgl_draw_original_vline(sgl_painter_t* painter, int y0, int y1, int x, uint32_t color)
{
    int i;
    for(i = y0; i <= y1; ++i)
    {
        painter->draw_piexl(painter, x, i, color);
    }
}

inline static void sgl_draw_original_line(sgl_painter_t* painter, int x0, int y0, int x1, int y1, uint32_t color)
{
    int i;
    int errorx = 0, errory = 0, delta_x, delta_y, distance, inc_x, inc_y, x = x0, y = y0; 
    
    delta_x = x1 - x0;
    delta_y = y1 - y0;
    delta_x = (delta_x ^ (delta_x >> 31)) - (delta_x >> 31); // 求delta_x绝对值
    delta_y = (delta_y ^ (delta_y >> 31)) - (delta_y >> 31); // 求delta_y绝对值
    
    if(delta_x == 0)
        inc_x = 0;
    else
        inc_x = x1 > x0 ? 1 : -1;
    if(delta_y == 0)
        inc_y = 0;
    else
        inc_y = y1 > y0 ? 1 : -1;
    distance = delta_x > delta_y ? delta_x : delta_y;
    
    for(i = 0; i < distance; ++i) // 不绘制终点(x1, y1)
    {
        errorx += delta_x;
        errory += delta_y;
        if(errorx > distance)
        {
            errorx -= distance;
            x += inc_x;
        }
        if(errory > distance)
        {
            errory -= distance;
            y += inc_y;
        }
        painter->draw_piexl(painter, x, y, color);
    }
}

void sgl_draw_hline(sgl_painter_t* painter, int x0, int x1, int y, uint32_t color)
{
    int y0 = y;
    int y1 = y;
    if(x0 > x1)
        sgl_swap(&x0, &x1);
    if(sgl_clip(painter, &x0, &y0, &x1, &y1))
        return;
    sgl_remap(painter, &x0, &y0);
    sgl_remap(painter, &x1, &y1);
    switch(painter->rotate)
    {
        case sgl_rotate_0:sgl_draw_original_hline(painter, x0, x1, y0, color);break;
        case sgl_rotate_90:sgl_draw_original_vline(painter, y0, y1, x0, color);break;
        case sgl_rotate_180:sgl_draw_original_hline(painter, x1, x0, y0, color);break;
        case sgl_rotate_270:sgl_draw_original_vline(painter, y1, y0, x0, color);break;
        default:break;
    }
}

void sgl_draw_vline(sgl_painter_t* painter, int y0, int y1, int x, uint32_t color)
{
    int x0 = x;
    int x1 = x;
    if(y0 > y1)
        sgl_swap(&y0, &y1);
    if(sgl_clip(painter, &x0, &y0, &x1, &y1))
        return;
    sgl_remap(painter, &x0, &y0);
    sgl_remap(painter, &x1, &y1);
    switch(painter->rotate)
    {
        case sgl_rotate_0:sgl_draw_original_vline(painter, y0, y1, x0, color);break;
        case sgl_rotate_90:sgl_draw_original_hline(painter, x1, x0, y0, color);break;
        case sgl_rotate_180:sgl_draw_original_vline(painter, y1, y0, x0, color);break;
        case sgl_rotate_270:sgl_draw_original_hline(painter, x0, x1, y0, color);break;
        default:break;
    }
}

void sgl_draw_hline_with_length(sgl_painter_t* painter, int x, int y, int length, uint32_t color)
{
    if(length > 0)
        sgl_draw_hline(painter, x, x + length - 1, y, color);
    else if(length < 0)
        sgl_draw_hline(painter, x + length + 1, x, y, color);
}

void sgl_draw_vline_with_length(sgl_painter_t* painter, int x, int y, int length, uint32_t color)
{
    if(length > 0)
        sgl_draw_vline(painter, y, y + length - 1, x, color);
    else if(length < 0)
        sgl_draw_vline(painter, y + length + 1, y, x, color);
}

void sgl_draw_line(sgl_painter_t* painter, int x0, int y0, int x1, int y1, uint32_t color)
{
    if(x0 > x1)
        sgl_swap(&x0, &x1);
    if(y0 > y1)
        sgl_swap(&y0, &y1);
    if(sgl_clip(painter, &x0, &y0, &x1, &y1))
        return;
    sgl_remap(painter, &x0, &y0);
    sgl_remap(painter, &x1, &y1);
    sgl_draw_original_line(painter, x0, y0, x1, y1, color);
}

void sgl_draw_polyline(sgl_painter_t* painter, int* x, int* y, int size, uint32_t color)
{
    int i;
    for(i = 0; i < size - 1; ++i) // 不绘制终点(x[size - 1], y[size - 1])
        sgl_draw_line(painter, x[i], y[i], x[i + 1], y[i + 1], color);
}

void sgl_draw_polygon(sgl_painter_t* painter, int* x, int* y, int size, sgl_fillstyle_t fillstyle, uint32_t color)///////////////////////////////////////////////
{
    if(size < 3)
        return;
    if(fillstyle == sgl_fillstyle_outlined)
    {
        sgl_draw_polyline(painter, x, y, size, color);
        sgl_draw_line(painter, x[size - 1], y[size - 1], x[0], y[0], color);
    }
    else
    {
        
    }
}

void sgl_draw_triangle(sgl_painter_t* painter, int x0, int y0, int x1, int y1, int x2, int y2, sgl_fillstyle_t fillstyle, uint32_t color)///////////////////////////////////////////////
{
    if(fillstyle == sgl_fillstyle_outlined)
    {
        sgl_draw_line(painter, x0, y0, x1, y1, color);
        sgl_draw_line(painter, x1, y1, x2, y2, color);
        sgl_draw_line(painter, x2, y2, x0, y0, color);
    }
    else
    {
        
    }
}

void sgl_draw_rectangle(sgl_painter_t* painter, int x0, int y0, int x1, int y1, sgl_fillstyle_t fillstyle, uint32_t color)
{
    int i, j;
    if(x0 > x1)
        sgl_swap(&x0, &x1);
    if(y0 > y1)
        sgl_swap(&y0, &y1);
    if(sgl_clip(painter, &x0, &y0, &x1, &y1))
        return;
    sgl_remap(painter, &x0, &y0);
    sgl_remap(painter, &x1, &y1);
    if(x0 > x1)
        sgl_swap(&x0, &x1);
    if(y0 > y1)
        sgl_swap(&y0, &y1);
    if(fillstyle == sgl_fillstyle_outlined)
    {
        for(i = x0; i <= x1; ++i)
        {
            painter->draw_piexl(painter, i, y0, color);
            painter->draw_piexl(painter, i, y1, color);
        }
        for(i = y0 + 1; i < y1; ++i)
        {
            painter->draw_piexl(painter, x0, i, color);
            painter->draw_piexl(painter, x1, i, color);
        }
    }
    else
    {
        for(i = y0; i <= y1; ++i)
        {
            for(j = x0; j <= x1;  ++j)
                painter->draw_piexl(painter, j, i, color);
        }
    }
}

void sgl_draw_rectangle_with_length(sgl_painter_t* painter, int x, int y, int width, int height, sgl_fillstyle_t fillstyle, uint32_t color)
{
    if(width > 0 && height > 0)
        sgl_draw_rectangle(painter, x, y, x + width - 1, y + height - 1, fillstyle, color);
    else if(width > 0 && height < 0)
        sgl_draw_rectangle(painter, x, y, x + width - 1, y + height + 1, fillstyle, color);
    else if(width < 0 && height > 0)
        sgl_draw_rectangle(painter, x, y, x + width + 1, y + height - 1, fillstyle, color);
    else if(width < 0 && height < 0)
        sgl_draw_rectangle(painter, x, y, x + width + 1, y + height + 1, fillstyle, color);
}

void sgl_draw_outlined_circle_section(sgl_painter_t* painter, int xc, int yc, int r, sgl_circle_section_t section, uint32_t color)
{
    /*
    +-----------> X
    |   6  7
    | 5      8
    | 4      1
    |   3  2
    v
    Y
    */
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    while(1)
    {
        if (d < 0)
            d = d + 4 * x + 6;
        else
        {
            d = d + 4 * (x - y) + 10;
            --y;
        }
        ++x;
        if(x < y)
        {
            if(section & sgl_circle_section_lower_right)
            {
                sgl_draw_point(painter, xc + y, yc + x, color); // 1
                sgl_draw_point(painter, xc + x, yc + y, color); // 2
            }
            if(section & sgl_circle_section_lower_left)
            {
                sgl_draw_point(painter, xc - x, yc + y, color); // 3
                sgl_draw_point(painter, xc - y, yc + x, color); // 4
            }
            if(section & sgl_circle_section_upper_left)
            {
                sgl_draw_point(painter, xc - y, yc - x, color); // 5
                sgl_draw_point(painter, xc - x, yc - y, color); // 6
            }
            if(section & sgl_circle_section_upper_right)
            {
                sgl_draw_point(painter, xc + x, yc - y, color); // 7
                sgl_draw_point(painter, xc + y, yc - x, color); // 8
            }
        }
        else
            break;
    }
    if(x == y)
    {
        if(section & sgl_circle_section_lower_right)
            sgl_draw_point(painter, xc + y, yc + x, color); // 1, 2
        if(section & sgl_circle_section_lower_left)
            sgl_draw_point(painter, xc - x, yc + y, color); // 3, 4
        if(section & sgl_circle_section_upper_left)
            sgl_draw_point(painter, xc - y, yc - x, color); // 5, 6
        if(section & sgl_circle_section_upper_right)
            sgl_draw_point(painter, xc + x, yc - y, color); // 7, 8
    }
}

void sgl_draw_filled_circle_section(sgl_painter_t* painter, int xc, int yc, int r, sgl_circle_section_t section, uint32_t color)
{
    /*
    +-----------> X
    |   6  7
    | 5      8
    | 4      1
    |   3  2
    v
    Y
    */
    int i;
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;
    while(1)
    {
        if (d < 0)
            d = d + 4 * x + 6;
        else
        {
            d = d + 4 * (x - y) + 10;
            --y;
        }
        ++x;
        if(x < y)
        {
            if(section & sgl_circle_section_lower_right)
            {
                sgl_draw_hline(painter, xc + x + 1, xc + y, yc + x, color); // 1
                sgl_draw_vline(painter, yc + x + 1, yc + y, xc + x, color); // 2
            }
            if(section & sgl_circle_section_lower_left)
            {
                sgl_draw_vline(painter, yc + x + 1, yc + y, xc - x, color); // 3
                sgl_draw_hline(painter, xc - y, xc - x - 1, yc + x, color); // 4
            }
            if(section & sgl_circle_section_upper_left)
            {
                sgl_draw_hline(painter, xc - y, xc - x - 1, yc - x, color); // 5
                sgl_draw_vline(painter, yc - y, yc - x - 1, xc - x, color); // 6
            }
            if(section & sgl_circle_section_upper_right)
            {
                sgl_draw_vline(painter, yc - y, yc - x - 1, xc + x, color); // 7
                sgl_draw_hline(painter, xc + x + 1, xc + y, yc - x, color); // 8
            }
        }
        else
            break;
    }
    for(i = 1; i < x; ++i)
    {
        if(section & sgl_circle_section_lower_right)
            sgl_draw_point(painter, xc + i, yc + i, color); // 1, 2
        if(section & sgl_circle_section_lower_left)
            sgl_draw_point(painter, xc - i, yc + i, color); // 3, 4
        if(section & sgl_circle_section_upper_left)
            sgl_draw_point(painter, xc - i, yc - i, color); // 5, 6
        if(section & sgl_circle_section_upper_right)
            sgl_draw_point(painter, xc + i, yc - i, color); // 7, 8
    }
    if(x == y)
    {
        if(section & sgl_circle_section_lower_right)
            sgl_draw_point(painter, xc + y, yc + x, color); // 1, 2
        if(section & sgl_circle_section_lower_left)
            sgl_draw_point(painter, xc - x, yc + y, color); // 3, 4
        if(section & sgl_circle_section_upper_left)
            sgl_draw_point(painter, xc - y, yc - x, color); // 5, 6
        if(section & sgl_circle_section_upper_right)
            sgl_draw_point(painter, xc + x, yc - y, color); // 7, 8
    }
}

void sgl_draw_circle(sgl_painter_t* painter, int xc, int yc, int r, sgl_fillstyle_t fillstyle, uint32_t color)
{
    if(r == 0)
        return;
    else if(r < 0)
        r = -r;
    if(r == 1)
    {
        sgl_draw_point(painter, xc, yc, color);
        return;
    }
    --r;
    if(fillstyle == sgl_fillstyle_outlined)
    {
        sgl_draw_point(painter, xc + r, yc, color); // 8, 1
        sgl_draw_point(painter, xc, yc + r, color); // 3, 2
        sgl_draw_point(painter, xc - r, yc, color); // 5, 4
        sgl_draw_point(painter, xc, yc - r, color); // 6, 7
        sgl_draw_outlined_circle_section(painter, xc, yc, r, sgl_circle_section_all, color);
    }
    else
    {
        sgl_draw_point(painter, xc, yc, color); // xc, yc
        sgl_draw_hline(painter, xc + 1, xc + r, yc, color); // 8, 1
        sgl_draw_vline(painter, yc + 1, yc + r, xc, color); // 3, 2
        sgl_draw_hline(painter, xc - r, xc - 1, yc, color); // 5, 4
        sgl_draw_vline(painter, yc - r, yc - 1, xc, color); // 6, 7
        sgl_draw_filled_circle_section(painter, xc, yc, r, sgl_circle_section_all, color);
    }
}

void sgl_draw_outlined_ellipse_section(sgl_painter_t* painter, int xc, int yc, int rx, int ry, sgl_circle_section_t section, uint32_t color)
{
    int x = 0, y = ry, d1, d2;
    int rx_2 = rx * rx;
    int ry_2 = ry * ry;
    d1 = 4 * ry_2 + rx_2 * (1 - 4 * ry);
    while(2 * ry_2 * (x + 1) < rx_2 * (2 * y - 1))
    {
        if(d1 <= 0)
        {
            d1 += 4 * ry_2 * (2 * x + 3);
            ++x;
        }
        else
        {
            d1 += 4 * (ry_2 * (2 * x + 3) + rx_2 * (-2 * y + 2));
            ++x;
            --y;
        }
        if(section & sgl_circle_section_lower_right)
            sgl_draw_point(painter, xc + x, yc + y, color);
        if(section & sgl_circle_section_upper_right)
            sgl_draw_point(painter, xc + x, yc - y, color);
        if(section & sgl_circle_section_lower_left)
            sgl_draw_point(painter, xc - x, yc + y, color);
        if(section & sgl_circle_section_upper_left)
            sgl_draw_point(painter, xc - x, yc - y, color);
    }
    d2 = ry_2 * (2 * x + 1) * (2 * x + 1) + 4 * (rx_2 * (y - 1) * (y - 1) - rx_2 * ry_2);
    while(y > 0)
    {
        if(d2 <= 0)
        {
            d2 += 4 * (ry_2 * (2 * x + 2) + rx_2 * (-2 * y + 3));
            ++x;
            --y;
        }
        else
        {
            d2 += 4 * (rx_2 * (-2 * y + 3));
            --y;
        }
        if(section & sgl_circle_section_lower_right)
            sgl_draw_point(painter, xc + x, yc + y, color);
        if(section & sgl_circle_section_upper_right)
            sgl_draw_point(painter, xc + x, yc - y, color);
        if(section & sgl_circle_section_lower_left)
            sgl_draw_point(painter, xc - x, yc + y, color);
        if(section & sgl_circle_section_upper_left)
            sgl_draw_point(painter, xc - x, yc - y, color);
    }
}

void sgl_draw_filled_ellipse_section(sgl_painter_t* painter, int xc, int yc, int rx, int ry, sgl_circle_section_t section, uint32_t color)
{
    int x = 0, y = ry, d1, d2;
    int rx_2 = rx * rx;
    int ry_2 = ry * ry;
    d1 = 4 * ry_2 + rx_2 * (1 - 4 * ry);
    while(2 * ry_2 * (x + 1) < rx_2 * (2 * y - 1))
    {
        if(d1 <= 0)
        {
            d1 += 4 * ry_2 * (2 * x + 3);
            ++x;
            if(section & sgl_circle_section_lower_right)
                sgl_draw_point(painter, xc + x, yc + y, color);
            if(section & sgl_circle_section_upper_right)
                sgl_draw_point(painter, xc + x, yc - y, color);
            if(section & sgl_circle_section_lower_left)
                sgl_draw_point(painter, xc - x, yc + y, color);
            if(section & sgl_circle_section_upper_left)
                sgl_draw_point(painter, xc - x, yc - y, color);
        }
        else
        {
            d1 += 4 * (ry_2 * (2 * x + 3) + rx_2 * (-2 * y + 2));
            ++x;
            --y;
            sgl_draw_hline(painter, xc + 1, xc + x, yc + y, color);
            sgl_draw_hline(painter, xc + 1, xc + x, yc - y, color);
            sgl_draw_hline(painter, xc - x, xc - 1, yc + y, color);
            sgl_draw_hline(painter, xc - x, xc - 1, yc - y, color);
        }
    }
    d2 = ry_2 * (2 * x + 1) * (2 * x + 1) + 4 * (rx_2 * (y - 1) * (y - 1) - rx_2 * ry_2);
    while(y > 0)
    {
        if(d2 <= 0)
        {
            d2 += 4 * (ry_2 * (2 * x + 2) + rx_2 * (-2 * y + 3));
            ++x;
            --y;
        }
        else
        {
            d2 += 4 * (rx_2 * (-2 * y + 3));
            --y;
        }
        sgl_draw_hline(painter, xc + 1, xc + x, yc + y, color);
        sgl_draw_hline(painter, xc + 1, xc + x, yc - y, color);
        sgl_draw_hline(painter, xc - x, xc - 1, yc + y, color);
        sgl_draw_hline(painter, xc - x, xc - 1, yc - y, color);
    }
}

void sgl_draw_ellipse(sgl_painter_t* painter, int xc, int yc, int rx, int ry, sgl_fillstyle_t fillstyle, uint32_t color)
{
    if(rx == 0 || ry == 0)
        return;
    if(rx < 0)
        rx = -rx;
    if(ry < 0)
        ry = -ry;
    if(rx == 1 && ry == 1)
    {
        sgl_draw_point(painter, xc, yc, color);
        return;
    }
    if(rx == 1)
    {
        sgl_draw_vline(painter, yc -ry + 1, yc + ry - 1, xc, color);
        return;
    }
    else if(ry == 1)
    {
        sgl_draw_hline(painter, xc -rx + 1, xc + rx - 1, yc, color);
        return;
    }
    --rx;
    --ry;
    if(fillstyle == sgl_fillstyle_outlined)
    {
        sgl_draw_point(painter, xc + rx, yc, color); // 1, 2
        sgl_draw_point(painter, xc, yc + ry, color); // 3, 4
        sgl_draw_point(painter, xc - rx, yc, color); // 5, 6
        sgl_draw_point(painter, xc, yc - ry, color); // 7, 8
        sgl_draw_outlined_ellipse_section(painter, xc, yc, rx, ry, sgl_circle_section_all, color);
    }
    else
    {
        sgl_draw_point(painter, xc, yc, color); // (xc, yc)
        sgl_draw_hline(painter, xc + 1, xc + rx, yc, color); // 8, 1
        sgl_draw_vline(painter, yc + 1, yc + ry, xc, color); // 2, 3
        sgl_draw_hline(painter, xc - rx, xc - 1, yc, color); // 4, 5
        sgl_draw_vline(painter, yc - ry, yc - 1, xc, color); // 6, 7
        sgl_draw_filled_ellipse_section(painter, xc, yc, rx, ry, sgl_circle_section_all, color);
    }
}

void sgl_draw_round_rectangle(sgl_painter_t* painter, int x0, int y0, int x1, int y1, int r, sgl_fillstyle_t fillstyle, uint32_t color)
{
    sgl_rect_t rect;
    int dx, dy;
    if(x0 > x1)
        sgl_swap(&x0, &x1);
    if(y0 > y1)
        sgl_swap(&y0, &y1);
    dx = x1 - x0;
    dy = y1 - y0;
    if(dx < 2)
    {
        if(dx == 0)
            sgl_draw_vline(painter, y0, y1, x0, color);
        else
        {
            sgl_draw_vline(painter, y0, y1, x0, color);
            sgl_draw_vline(painter, y0, y1, x1, color);
        }
        return;
    }
    if(dy < 2)
    {
        if(dy == 0)
            sgl_draw_hline(painter, x0, x1, y0, color);
        else
        {
            sgl_draw_hline(painter, x0, x1, y0, color);
            sgl_draw_hline(painter, x0, x1, y1, color);
        }
        return;
    }
    if(dy > dx)
        sgl_swap(&dx, &dy);
    if(r < 0)
        r = -r;
    if(2 * r > dy)
        r = dy / 2;
    rect.left = x0 + r;
    rect.top = y0 + r;
    rect.right = x1 - r;
    rect.bottom = y1 - r;
    if(fillstyle == sgl_fillstyle_outlined)
    {
        sgl_draw_hline(painter, rect.left, rect.right, y0, color);
        sgl_draw_hline(painter, rect.left, rect.right, y1, color);
        sgl_draw_vline(painter, rect.top, rect.bottom, x0, color);
        sgl_draw_vline(painter, rect.top, rect.bottom, x1, color);
        sgl_draw_outlined_circle_section(painter, rect.left, rect.top, r, sgl_circle_section_upper_left, color);
        sgl_draw_outlined_circle_section(painter, rect.right, rect.top, r, sgl_circle_section_upper_right, color);
        sgl_draw_outlined_circle_section(painter, rect.left, rect.bottom, r, sgl_circle_section_lower_left, color);
        sgl_draw_outlined_circle_section(painter, rect.right, rect.bottom, r, sgl_circle_section_lower_right, color);
    }
    else
    {
        sgl_draw_rectangle(painter, rect.left, rect.top, rect.right, rect.bottom, sgl_fillstyle_filled, color);
        sgl_draw_rectangle(painter, rect.left, y0, rect.right, rect.top - 1, sgl_fillstyle_filled, color);
        sgl_draw_rectangle(painter, rect.left, rect.bottom + 1, rect.right, y1, sgl_fillstyle_filled, color);
        sgl_draw_rectangle(painter, x0, rect.top, rect.left - 1, rect.bottom, sgl_fillstyle_filled, color);
        sgl_draw_rectangle(painter, rect.right + 1, rect.top, x1, rect.bottom, sgl_fillstyle_filled, color);
        sgl_draw_filled_circle_section(painter, rect.left, rect.top, r, sgl_circle_section_upper_left, color);
        sgl_draw_filled_circle_section(painter, rect.right, rect.top, r, sgl_circle_section_upper_right, color);
        sgl_draw_filled_circle_section(painter, rect.left, rect.bottom, r, sgl_circle_section_lower_left, color);
        sgl_draw_filled_circle_section(painter, rect.right, rect.bottom, r, sgl_circle_section_lower_right, color);
    }
}

void sgl_draw_round_rectangle_with_length(sgl_painter_t* painter, int x, int y, int width, int height, int r, sgl_fillstyle_t fillstyle, uint32_t color)
{
    if(width > 0 && height > 0)
        sgl_draw_round_rectangle(painter, x, y, x + width - 1, y + height - 1, r, fillstyle, color);
    else if(width > 0 && height < 0)
        sgl_draw_round_rectangle(painter, x, y, x + width - 1, y + height + 1, r, fillstyle, color);
    else if(width < 0 && height > 0)
        sgl_draw_round_rectangle(painter, x, y, x + width + 1, y + height - 1, r, fillstyle, color);
    else if(width < 0 && height < 0)
        sgl_draw_round_rectangle(painter, x, y, x + width + 1, y + height + 1, r, fillstyle, color);
}

void sgl_show_mono_image(sgl_painter_t* painter, int x, int y, int width, int height, const uint8_t* image, sgl_dir_t dir, uint32_t color)
{
    int i, j;
    switch(dir)
    {
        case sgl_dir_up:
            for(i = 0; i < width; ++i)
                for(j = 0; j < height; ++j)
                    if((image[i + (j / 8 * width)] & (1<<(j % 8))))
                        sgl_draw_point(painter, x + i, y + j, color);
            break;
        case sgl_dir_right:
            for(i = 0; i < width; ++i)
                for(j = 0; j < height; ++j)
                    if((image[i + (j / 8 * width)] & (1<<(j % 8))))
                        sgl_draw_point(painter, x + height - j - 1, y + i, color);
            break;
        case sgl_dir_left:
            for(i = 0; i < width; ++i)
                for(j = 0; j < height; ++j)
                    if((image[i + (j / 8 * width)] & (1<<(j % 8))))
                        sgl_draw_point(painter, x + j, y + width -i - 1, color);
            break;
        case sgl_dir_down:
            for(i = 0; i < width; ++i)
                for(j = 0; j < height; ++j)
                    if((image[i + (j / 8 * width)] & (1<<(j % 8))))
                        sgl_draw_point(painter, x + width -i - 1, y + height - j - 1, color);
            break;
        default:break;
    }
}

void sgl_show_string(sgl_painter_t* painter, int x, int y, const char* str, int length, sgl_align_t align, sgl_dir_t dir, uint32_t color)
{
    int i;
    int fontwidth = painter->font_info.width;
    int fontheight = painter->font_info.height;
    switch(dir)
    {
        case sgl_dir_up:
            sgl_align(&x, &y, fontwidth * length, fontheight, align);
            for(i = 0; i < length; ++i)
                sgl_show_mono_image(painter, x + i * fontwidth, y, fontwidth, fontheight, &painter->font_info.font[(str[i] - ' ') * painter->font_info.fontsize], dir, color);
            break;
        case sgl_dir_right:
            sgl_align(&x, &y, fontheight, fontwidth * length, align);
            for(i = 0; i < length; ++i)
                sgl_show_mono_image(painter, x, y + i * fontwidth, fontwidth, fontheight, &painter->font_info.font[(str[i] - ' ') * painter->font_info.fontsize], dir, color);
            break;
        case sgl_dir_left:
            sgl_align(&x, &y, fontheight, fontwidth * length, align);
            y += (length - 1) * fontwidth;
            for(i = 0; i < length; ++i)
                sgl_show_mono_image(painter, x, y - i * fontwidth, fontwidth, fontheight, &painter->font_info.font[(str[i] - ' ') * painter->font_info.fontsize], dir, color);
            break;
        case sgl_dir_down:
            sgl_align(&x, &y, fontwidth * length, fontheight, align);
            x += (length - 1) * fontwidth;
            for(i = 0; i < length; ++i)
                sgl_show_mono_image(painter, x - i * fontwidth, y, fontwidth, fontheight, &painter->font_info.font[(str[i] - ' ') * painter->font_info.fontsize], dir, color);
            break;
        default:break;
    }
}

int sgl_show_format_string(sgl_painter_t* painter, int x, int y, sgl_align_t align, sgl_dir_t dir, uint32_t color, const char* format, ...)
{
    char buffer[sgl_FORMAT_STRING_BUFFERSIZE];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sgl_FORMAT_STRING_BUFFERSIZE, format, args);
    va_end(args);
    sgl_show_string(painter, x, y, buffer, length, align, dir, color);
    return length;
}
