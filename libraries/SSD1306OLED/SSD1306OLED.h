#ifndef _SSD1306OLED_H_
#define _SSD1306OLED_H_
#include <WProgram.h>

#include <CircularBuffer.h>


#define swap(a, b) { uint8_t t = a; a = b; b = t; }

#define BLACK 0
#define WHITE 1

#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 64


#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2


#define SSD1306_USE_SPI_HARDWARE 
//#define SSD1306_USE_3WIRE

class SSD1306OLED {
 public:
  SSD1306OLED() 
  {}


  void ssd1306_reset(void);
  void ssd1306_init(uint8_t switchvcc);
  void ssd1306_command(uint8_t c);
  void ssd1306_data(uint8_t c);
  void ssd1306_set_brightness(uint8_t val);
  void clear_display(void);
  void clear();
  void invert(uint8_t i);
  void display();
  void sync();

  void setpixel(uint8_t x, uint8_t y, uint8_t color);
  void fillcircle(uint8_t x0, uint8_t y0, uint8_t r, 
		  uint8_t color);
  void drawcircle(uint8_t x0, uint8_t y0, uint8_t r, 
		  uint8_t color);
  void drawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
		uint8_t color);
  void fillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
		uint8_t color);
  void drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, 
		uint8_t color);
  void drawchar(uint8_t x, uint8_t line, uint8_t c);
  void drawstring(uint8_t x, uint8_t line, char *c);

  void drawbitmap(uint8_t x, uint8_t y, 
		  const uint8_t *bitmap, uint8_t w, uint8_t h,
		  uint8_t color);

 private:
  void spiwrite(uint8_t c);
#ifdef SSD1306_USE_3WIRE
  void spiwrite_3wire(bool isData, uint8_t c);
#endif

  //uint8_t buffer[128*64/8]; 

#ifdef SSD1306_USE_SPI_HARDWARE
  void spiwrite_HardwareCmd(uint8_t cmd);
  void spiwrite_HardwareData();

    #define SPIDATA_BUFFER_SIZE 8
    static CircularBuffer<uint8_t, uint8_t, SPIDATA_BUFFER_SIZE> tx_buf;
    static uint16_t tx_cbData;
    volatile static uint8_t  tx_init;
    volatile static uint8_t tx_isTransmitting;

public: 
    static void tx_isr_handler(void);
#endif // SSD1306_USE_SPI_HARDWARE
};
#endif