#ifndef OLED_H
#define OLED_H

typedef enum {
    FONT6x8 = 1,
    FONT8x16,
}Font;

uint32_t OledInit(void);
void OledSetPosition(uint8_t x, uint8_t y);
void OledFillScreen(uint8_t data);
void OledShowChar(uint8_t x, uint8_t y, uint8_t ch, Font font);
void OledShowString(uint8_t x, uint8_t y,  const char *str, Font font);

#endif  // OLED_H