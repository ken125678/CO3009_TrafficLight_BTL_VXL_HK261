#include "main.h"
#include "lcd.h"
#include "global.h"
#include <stdio.h>
#include <stdlib.h>

#define LCD_ADDRESS     (0x27 << 1)

#define LCD_RS          0x01
#define LCD_RW          0x02
#define LCD_EN          0x04
#define LCD_BACKLIGHT   0x08

static void lcd_write_byte(uint8_t data)
{
    HAL_I2C_Master_Transmit(&hi2c1,
                            LCD_ADDRESS,
                            &data,
                            1,
                            10);
}

static void lcd_write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble << 4) | LCD_BACKLIGHT;

    if (rs)
    {
        data |= LCD_RS;
    }

    lcd_write_byte(data);              // EN low
    lcd_write_byte(data | LCD_EN);     // EN high
    lcd_write_byte(data);              // EN low again
}

static void lcd_send_byte(uint8_t value, uint8_t rs)
{
    lcd_write_nibble(value >> 4, rs);
    lcd_write_nibble(value & 0x0F, rs);
}

void lcd_clear(void)
{
    lcd_send_byte(0x01, 0);
    HAL_Delay(2);
}

static void lcd_set_cursor(uint8_t row, uint8_t column)
{
    uint8_t address;

    if (row == 0)
        address = 0x00 + column;
    else
        address = 0x40 + column;

    lcd_send_byte(0x80 | address, 0);
}

static void lcd_write_line(uint8_t row, const char *text)
{
    lcd_set_cursor(row, 0);

    uint8_t i = 0;

    while (text[i] != '\0' && i < 16)
    {
        lcd_send_byte(text[i], 1);
        i++;
    }

    while (i < 16)
    {
        lcd_send_byte(' ', 1);
        i++;
    }
}

static void lcd_write_environment(void)
{
    char line[17];

    int temp = (int)(dht20_temp * 10);
    int humi = (int)(dht20_humi * 10);

    snprintf(line, sizeof(line),
             "T:%d.%dC H:%d.%d%%",
             temp / 10,
             abs(temp % 10),
             humi / 10,
             abs(humi % 10));

    lcd_write_line(1, line);
}

void lcd_update_task(void)
{
    char line[17];

    switch (status)
    {
        case AUTO_RED1_GREEN2:
        case AUTO_RED1_AMBER2:
        case AUTO_GREEN1_RED2:
        case AUTO_AMBER1_RED2:

            snprintf(line, sizeof(line),
                     "AUTO R1:%02d R2:%02d",
                     count_road1,
                     count_road2);

            lcd_write_line(0, line);
            lcd_write_environment();
            break;

        case MAN_RED1_GREEN2:
            lcd_write_line(0, "MAN R1:R R2:G");
            lcd_write_environment();
            break;

        case MAN_RED1_AMBER2:
            lcd_write_line(0, "MAN R1:R R2:A");
            lcd_write_environment();
            break;

        case MAN_GREEN1_RED2:
            lcd_write_line(0, "MAN R1:G R2:R");
            lcd_write_environment();
            break;

        case MAN_AMBER1_RED2:
            lcd_write_line(0, "MAN R1:A R2:R");
            lcd_write_environment();
            break;

        case TUNE_RED:
            snprintf(line, sizeof(line), "TUNE RED: %02ds", temp_red);
            lcd_write_line(0, line);
            snprintf(line, sizeof(line), "R:%02d G:%02d A:%02d",
                     temp_red, temp_green, temp_amber);
            lcd_write_line(1, line);
            break;

        case TUNE_GREEN:
            snprintf(line, sizeof(line), "TUNE GREEN: %02ds", temp_green);
            lcd_write_line(0, line);
            snprintf(line, sizeof(line), "R:%02d G:%02d A:%02d",
                     temp_red, temp_green, temp_amber);
            lcd_write_line(1, line);
            break;

        case TUNE_AMBER:
            snprintf(line, sizeof(line), "TUNE AMBER: %02ds", temp_amber);
            lcd_write_line(0, line);
            snprintf(line, sizeof(line), "R:%02d G:%02d A:%02d",
                     temp_red, temp_green, temp_amber);
            lcd_write_line(1, line);
            break;

        default:
            break;
    }
}

void lcd_init(void)
{
    HAL_Delay(50);

    lcd_write_nibble(0x03, 0);
    HAL_Delay(5);

    lcd_write_nibble(0x03, 0);
    HAL_Delay(1);

    lcd_write_nibble(0x03, 0);
    HAL_Delay(1);

    lcd_write_nibble(0x02, 0);
    HAL_Delay(1);

    lcd_send_byte(0x28, 0);  // 4-bit, 2-line, 5x8 font
    lcd_send_byte(0x08, 0);  // Display off
    lcd_clear();
    lcd_send_byte(0x06, 0);  // Cursor moves right
    lcd_send_byte(0x0C, 0);  // Display on, cursor off
}
