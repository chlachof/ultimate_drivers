/*
 * modify-2014.11.28 ultimate_drivers/ud_device_lcd/ud_device_lcd_hg160160/ud_device_lcd_hg160160.c
 * 大致构想是做一个简单的frame_buffer用以支持QT，直接使用系统内的FB驱动难度略高，主要是不能支持模块，
 * 不利于今后的维护，相当于修改了内核，故选择仿制内核FB驱动，让QT通过这个伪frame_buffer驱动液晶，
 * 该层不具备对设备的接口，只提供EXPORT_SYMBOL，接口在ud_device_lcd中
 * 适用范围：特殊(hg160160，ud_device_bus_emulated)
 */

#include <linux/module.h>
#include <linux/init.h>

#include <linux/fb.h>
#include "../../include/ud_device_lcd.h"
#include "../../include/ud_device_bus_emulated.h"
#include "../../include/common.h"

MODULE_LICENSE("Dual BSD/GPL");

extern int ud_bus_export_set_data (struct bus_struct *);
extern int ud_bus_export_get_data (struct bus_struct *);

#define LCD_REG_CMD (0)
#define LCD_REG_DAT (1)
#define LCD_REG_OTH (4)

//初始化液晶
int ud_lcd_init(void);
//设置色彩寄存器
int ud_lcd_setcolor(unsigned char u8_red, unsigned char u8_green, unsigned char u8_blue);
//显示空白
int ud_lcd_blank(int blank);
//显示一个实心矩形
void ud_lcd_fillrect(const struct fb_fillrect * ux_p_rect);
//区域拷贝
void ud_lcd_copyarea(const struct fb_copyarea * ux_p_region);
//显示图像
void ud_lcd_imageblit(const struct fb_image * ux_p_image);
//旋转坐标系
void ud_lcd_rotate(int angle);
void ud_lcd_rotate_check(int * i32_p_x, int * i32_p_y);
//刷新
void ud_lcd_refresh(void);

enum ud_lcd_rotate
{
    UD_LCD_ROTATE_0     = 0,
    UD_LCD_ROTATE_90    = 1,
    UD_LCD_ROTATE_180   = 2,
    UD_LCD_ROTATE_270   = 3,
};
#define X_MAX       (160)
#define Y_MAX       (160)
struct ud_lcd_struct
{
    enum ud_lcd_rotate  ux_rotate;
    unsigned char       u8_color;
    unsigned char       u8_p_lcd_dram[Y_MAX][X_MAX];
};
static struct ud_lcd_struct ud_lcd =
{
    .ux_rotate = UD_LCD_ROTATE_0,
    .u8_color = 0,
};

void ud_lcd_reset_backlight(int i32_rst, int i32_blon)
{
    unsigned char u8_tmp = 0;
    struct bus_struct x_bus;
    int i = 1000;


    x_bus.u32_bus_addr = LCD_REG_OTH;

    if(i32_rst)
    {
        u8_tmp |= 1<<3;
        x_bus.u32_bus_data = u8_tmp;
        ud_bus_export_set_data(&x_bus);
        u8_tmp = 0;
        x_bus.u32_bus_data = u8_tmp;
        while(i--);
    }

    if(i32_blon)
    {
        u8_tmp |= 1<<2;
        x_bus.u32_bus_data = u8_tmp;
    }
    ud_bus_export_set_data(&x_bus);
    i = 1000;
    while(i--);
}

int ud_lcd_init(void)
{
    struct bus_struct x_bus;

    ud_lcd_reset_backlight(1,1);

    x_bus.u32_bus_addr = LCD_REG_CMD;

    x_bus.u32_bus_data = 0xe2;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xae;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x26;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x2b;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xeb;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x81;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x73;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x89;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xc4;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x84;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xde;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xc8;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x18;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xa3;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xd6;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xd1;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0x84;
    ud_bus_export_set_data(&x_bus);
    x_bus.u32_bus_data = 0xad;
    ud_bus_export_set_data(&x_bus);

    return (0);
}


int ud_lcd_setcolor(unsigned char u8_red, unsigned char u8_green, unsigned char u8_blue)
{
    //255*3/2 总色彩过半就认为是白色的
    if((u8_red + u8_green + u8_blue) > 382)
    {
        ud_lcd.u8_color = 0;
    }
    else
    {
        ud_lcd.u8_color = 1;
    }

    return (0);
}

int ud_lcd_blank(int blank)
{
    return (0);
}

void ud_lcd_fillrect(const struct fb_fillrect * ux_p_rect)
{
    int i32_x, i32_y;
    ud_lcd_setcolor((ux_p_rect->color&0x00ff0000)>>16, (ux_p_rect->color&0x0000ff00)>>8, (ux_p_rect->color&0x000000ff));
    for(i32_y = ux_p_rect->dy; i32_y < ux_p_rect->dy + ux_p_rect->height; i32_y ++)
    {
        for(i32_x = ux_p_rect->dx; i32_x < ux_p_rect->dx + ux_p_rect->width; i32_y ++)
        {
            ud_lcd_rotate_check(&i32_x, &i32_y);
            if(i32_x >= 0 && i32_y >= 0 && i32_x < X_MAX && i32_y < Y_MAX)
            {
                printd("%d,%d:%d\n", i32_x, i32_y, ud_lcd.u8_color);
                ud_lcd.u8_p_lcd_dram[i32_y][i32_x] = ud_lcd.u8_color;
            }
        }
    }

}

void ud_lcd_copyarea(const struct fb_copyarea * ux_p_region)
{

}

void ud_lcd_imageblit(const struct fb_image * ux_p_image)
{

}

void ud_lcd_rotate_check(int * i32_p_x, int * i32_p_y)
{
    int i32_x, i32_y;
    i32_x = *i32_p_x;
    i32_y = *i32_p_y;

    if(ud_lcd.ux_rotate == UD_LCD_ROTATE_90)
    {
        i32_x = X_MAX - 1 - i32_x;
        *i32_p_x = i32_y;
        *i32_p_y = i32_x;
    }
    else if(ud_lcd.ux_rotate == UD_LCD_ROTATE_180)
    {
        i32_x = X_MAX - 1 - i32_x;
        i32_y = Y_MAX - 1 - i32_y;
        *i32_p_y = i32_y;
        *i32_p_x = i32_x;
    }
    else if(ud_lcd.ux_rotate == UD_LCD_ROTATE_270)
    {
        i32_y = Y_MAX - 1 - i32_y;
        *i32_p_x = i32_y;
        *i32_p_y = i32_x;
    }
}

void ud_lcd_rotate(int i32_angle)
{
    int i32_tmp = 0;
    i32_tmp = ((i32_angle /90)%4);
    if(i32_tmp < 0)
    {
        i32_tmp = 4 + i32_tmp;
    }

    if(i32_tmp == 0)
    {
        ud_lcd.ux_rotate = UD_LCD_ROTATE_0;
    }
    else if(i32_tmp == 1)
    {
        ud_lcd.ux_rotate = UD_LCD_ROTATE_90;
    }
    else if(i32_tmp == 2)
    {
        ud_lcd.ux_rotate = UD_LCD_ROTATE_180;
    }
    else
    {
        ud_lcd.ux_rotate = UD_LCD_ROTATE_270;
    }
}

void ud_lcd_refresh(void)
{
    unsigned int u32_x;
    unsigned int u32_y;
    struct bus_struct x_bus;
    unsigned short u16_data = 0;

    for(u32_y = 0; u32_y < Y_MAX; u32_y++)
    {
        x_bus.u32_bus_addr = LCD_REG_CMD;
        x_bus.u32_bus_data = (0x60 |  (u32_y & 0x0f));
        ud_bus_export_set_data(&x_bus);
        x_bus.u32_bus_data = (0x70 | ((u32_y & 0xf0)>>4));
        ud_bus_export_set_data(&x_bus);
        x_bus.u32_bus_data = (0x05);
        ud_bus_export_set_data(&x_bus);
        x_bus.u32_bus_data = (0x12);
        ud_bus_export_set_data(&x_bus);
        for(u32_x = 0; u32_x < X_MAX; u32_x++)
        {
            if(u32_x%3 == 0)
            {
                u16_data = 0;
                if(ud_lcd.u8_p_lcd_dram[u32_y][u32_x] == 1)
                {
                    u16_data |= 0xf800;
                }
            }
            if(u32_x%3 == 1)
            {
                if(ud_lcd.u8_p_lcd_dram[u32_y][u32_x] == 1)
                {
                    u16_data |= 0x07e0;
                }
            }
            if(u32_x%3 == 2)
            {
                if(ud_lcd.u8_p_lcd_dram[u32_y][u32_x] == 1)
                {
                    u16_data |= 0x001f;
                }
                x_bus.u32_bus_addr = LCD_REG_DAT;
                x_bus.u32_bus_data = ((u16_data & 0xff00) >> 8);
                ud_bus_export_set_data(&x_bus);
                x_bus.u32_bus_data = (u16_data & 0x00ff);
                ud_bus_export_set_data(&x_bus);
            }
        }
        x_bus.u32_bus_data = ((u16_data & 0xff00) >> 8);
        ud_bus_export_set_data(&x_bus);
        x_bus.u32_bus_data = (u16_data & 0x00ff);
        ud_bus_export_set_data(&x_bus);
    }
}

static int __init ud_lcd160160_module_init (void)
{
    struct fb_fillrect rect =
    {
        .dx = 50,
        .dy = 50,
        .width = 60,
        .height = 60,
        .color = 0,
    };

    ud_lcd_init();
    ud_lcd_fillrect(&rect);
    //ud_lcd_refresh();

    printd("insmod successfully\n");
    return (0);
}

static void __exit ud_lcd160160_module_exit (void)
{
    printd("rmmod successfully\n");
}

EXPORT_SYMBOL(ud_lcd_init);
EXPORT_SYMBOL(ud_lcd_setcolor);
EXPORT_SYMBOL(ud_lcd_blank);
EXPORT_SYMBOL(ud_lcd_fillrect);
EXPORT_SYMBOL(ud_lcd_copyarea);
EXPORT_SYMBOL(ud_lcd_imageblit);
EXPORT_SYMBOL(ud_lcd_rotate);
EXPORT_SYMBOL(ud_lcd_refresh);

module_init(ud_lcd160160_module_init);
module_exit(ud_lcd160160_module_exit);