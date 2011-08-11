// some of this code was written by <cstone@pobox.com> originally; 
// it is in the public domain.

//#include <Wire.h>
#include <avr/pgmspace.h>
#include <WProgram.h>
#include <util/delay.h>
#include <stdlib.h>

#include "SSD1306OLED.h"
#include "glcdfont.c"

#include <spi.h>
#include <digitalWriteFast.h>
#include "pins_arduino.h"

#define sid MOSI
#define sclk SCK
#define dc 48
#define rst 49
#define cs SS



static uint8_t toggle = LOW;

static uint8_t is_reversed = 0;

// a handy reference to where the pages are on the screen
const uint8_t pagemap[] = { 0, 1, 2, 3, 4, 5, 6, 7};

const uint8_t command_init[] PROGMEM = 
{
  // Turn off the display
  SSD1306_DISPLAYOFF,  // 0xAE  

  SSD1306_SETLOWCOLUMN | 0x0,  // low col = 0
  SSD1306_SETHIGHCOLUMN | 0x0,  // hi col = 0
  
  // Set Display Start Line
  SSD1306_SETSTARTLINE | 0x0, // line #0

  // Set Contrast Control
  SSD1306_SETCONTRAST,  0xCF,  // 0x81  

  // Set Segment Remap
  SSD1306_SEGREMAP | 0x1, // 0xA1

  // Set Normal Display
  SSD1306_NORMALDISPLAY, // 0xA6

  // Set MUX Ratio
  SSD1306_SETMULTIPLEX, 0x3F, // 0xA8  // 0x3F 1/64 duty

  // Set Display Offset
  SSD1306_SETDISPLAYOFFSET, 0x00, // 0xD3 // no offset

  // Sec Osc Freq
  SSD1306_SETDISPLAYCLOCKDIV, 0x80, // 0xD5

  SSD1306_SETPRECHARGE, 0xf1, // 0xd9 
  
  // horizontal addressing mode
  SSD1306_MEMORYMODE, 0x00, // 0x20  

  // Set COM Pins hardware config ?? what code
  SSD1306_SETCOMPINS, 0x12, // 0xDA  // disable COM left/right remap

  SSD1306_SETVCOMDETECT, 0x40, // 0xDB
  
  //Set COM Output Scan Direction
  SSD1306_COMSCANDEC, // 0xC8

  // Enable charge pump regulator
  SSD1306_CHARGEPUMP, 0x14, //0x8D //ssd1306_command(0x10);  // disable
  
  // Display On
  SSD1306_DISPLAYON//  0xAF --turn on oled panel
};

const uint8_t command_initExternalVCC[] PROGMEM = 
{
  // Turn off the display
  SSD1306_DISPLAYOFF,  // 0xAE  

  // Set Normal Display
  SSD1306_NORMALDISPLAY, // 0xA6

  SSD1306_SETLOWCOLUMN | 0x0,  // low col = 0
  SSD1306_SETHIGHCOLUMN | 0x0,  // hi col = 0
  
  // Set Display Start Line
  SSD1306_SETSTARTLINE | 0x0, // line #0

  // Set Contrast Control
  SSD1306_SETCONTRAST, 0x7F,  // 0x81  // **external VCC **

  // Set Segment Remap
  SSD1306_SEGREMAP | 0x1, // 0xA1

  // Set MUX Ratio
  SSD1306_SETMULTIPLEX, 0x3F, // 0xA8  // 0x3F 1/64 duty

  // Set Display Offset
  SSD1306_SETDISPLAYOFFSET, 0x00, // 0xD3 // no offset

  // Sec Osc Freq
  SSD1306_SETDISPLAYCLOCKDIV, 0x80, // 0xD5

  SSD1306_SETPRECHARGE, 0x22, // 0xd9//  ** extern VCC **
  
  // horizontal addressing mode
  SSD1306_MEMORYMODE, 0x00, // 0x20  

  // Set COM Pins hardware config ?? what code
  SSD1306_SETCOMPINS, 0x12, // 0xDA  // disable COM left/right remap

  SSD1306_SETVCOMDETECT, 0x20, // 0xDB
  
  //Set COM Output Scan Direction
  SSD1306_COMSCANDEC, // 0xC8

  // Enable charge pump regulator
  SSD1306_CHARGEPUMP, 0x10, //0x8D   // disable // ** extern VCC **
  
  // Display On
  SSD1306_DISPLAYON//  0xAF --turn on oled panel
};


// a 5x7 font table
extern uint8_t PROGMEM font[];

//#define OLED_BUFFER_SIZE 1024
// the memory buffer for the LCD
static uint8_t buffer[] = { 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x80, 0x80, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80,
0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0xFF,
0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00,
0x80, 0xFF, 0xFF, 0x80, 0x80, 0x00, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x8C, 0x8E, 0x84, 0x00, 0x00, 0x80, 0xF8,
0xF8, 0xF8, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xE0, 0xE0, 0xC0, 0x80,
0x00, 0xE0, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFF, 0xC7, 0x01, 0x01,
0x01, 0x01, 0x83, 0xFF, 0xFF, 0x00, 0x00, 0x7C, 0xFE, 0xC7, 0x01, 0x01, 0x01, 0x01, 0x83, 0xFF,
0xFF, 0xFF, 0x00, 0x38, 0xFE, 0xC7, 0x83, 0x01, 0x01, 0x01, 0x83, 0xC7, 0xFF, 0xFF, 0x00, 0x00,
0x01, 0xFF, 0xFF, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0x07, 0x01, 0x01, 0x01, 0x00, 0x00, 0x7F, 0xFF,
0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x7F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x01, 0xFF,
0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x0F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xC7, 0xC7, 0x8F,
0x8F, 0x9F, 0xBF, 0xFF, 0xFF, 0xC3, 0xC0, 0xF0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFC, 0xFC,
0xFC, 0xFC, 0xFC, 0xFC, 0xFC, 0xF8, 0xF8, 0xF0, 0xF0, 0xE0, 0xC0, 0x00, 0x01, 0x03, 0x03, 0x03,
0x03, 0x03, 0x01, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01,
0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x03, 0x03, 0x01, 0x01, 0x03, 0x03, 0x00, 0x00,
0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x00, 0x00, 0x03,
0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF9, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x1F, 0x0F,
0x87, 0xC7, 0xF7, 0xFF, 0xFF, 0x1F, 0x1F, 0x3D, 0xFC, 0xF8, 0xF8, 0xF8, 0xF8, 0x7C, 0x7D, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x0F, 0x07, 0x00, 0x30, 0x30, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xC0, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xC0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x1F,
0x0F, 0x07, 0x1F, 0x7F, 0xFF, 0xFF, 0xF8, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0xE0,
0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00,
0x00, 0xFC, 0xFE, 0xFC, 0x0C, 0x06, 0x06, 0x0E, 0xFC, 0xF8, 0x00, 0x00, 0xF0, 0xF8, 0x1C, 0x0E,
0x06, 0x06, 0x06, 0x0C, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0xFC,
0xFE, 0xFC, 0x00, 0x18, 0x3C, 0x7E, 0x66, 0xE6, 0xCE, 0x84, 0x00, 0x00, 0x06, 0xFF, 0xFF, 0x06,
0x06, 0xFC, 0xFE, 0xFC, 0x0C, 0x06, 0x06, 0x06, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0x00, 0xC0, 0xF8,
0xFC, 0x4E, 0x46, 0x46, 0x46, 0x4E, 0x7C, 0x78, 0x40, 0x18, 0x3C, 0x76, 0xE6, 0xCE, 0xCC, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x01, 0x07, 0x0F, 0x1F, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x1F, 0x0F, 0x03,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00,
0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x03, 0x07, 0x0E, 0x0C,
0x18, 0x18, 0x0C, 0x06, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x01, 0x0F, 0x0E, 0x0C, 0x18, 0x0C, 0x0F,
0x07, 0x01, 0x00, 0x04, 0x0E, 0x0C, 0x18, 0x0C, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00,
0x00, 0x0F, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x07,
0x07, 0x0C, 0x0C, 0x18, 0x1C, 0x0C, 0x06, 0x06, 0x00, 0x04, 0x0E, 0x0C, 0x18, 0x0C, 0x0F, 0x07,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


void SSD1306OLED::drawbitmap(uint8_t x, uint8_t y, 
			const uint8_t *bitmap, uint8_t w, uint8_t h,
			uint8_t color) {
  for (uint8_t j=0; j<h; j++) {
    for (uint8_t i=0; i<w; i++ ) {
      if (pgm_read_byte(bitmap + i + (j/8)*w) & _BV(j%8)) {
	setpixel(x+i, y+j, color);
      }
    }
  }
}

void SSD1306OLED::drawstring(uint8_t x, uint8_t line, char *c) {
  while (c[0] != 0) {
    drawchar(x, line, c[0]);
    c++;
    x += 6; // 6 pixels wide
    if (x + 6 >= SSD1306_LCDWIDTH) {
      x = 0;    // ran out of this line
      line++;
    }
    if (line >= (SSD1306_LCDHEIGHT/8))
      return;        // ran out of space :(
  }

}

void  SSD1306OLED::drawchar(uint8_t x, uint8_t line, uint8_t c) {
  for (uint8_t i =0; i<5; i++ ) {
    buffer[x + (line*128) ] = pgm_read_byte(font+(c*5)+i);
    x++;
  }
}


// bresenham's algorithm - thx wikpedia
void SSD1306OLED::drawline(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, 
		      uint8_t color) {
  uint8_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  uint8_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int8_t err = dx / 2;
  int8_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;}

  for (; x0<x1; x0++) {
    if (steep) {
      setpixel(y0, x0, color);
    } else {
      setpixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

// filled rectangle
void SSD1306OLED::fillrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
		      uint8_t color) {

  // stupidest version - just pixels - but fast with internal buffer!
  for (uint8_t i=x; i<x+w; i++) {
    for (uint8_t j=y; j<y+h; j++) {
      setpixel(i, j, color);
    }
  }
}

// draw a rectangle
void SSD1306OLED::drawrect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, 
		      uint8_t color) {
  // stupidest version - just pixels - but fast with internal buffer!
  for (uint8_t i=x; i<x+w; i++) {
    setpixel(i, y, color);
    setpixel(i, y+h-1, color);
  }
  for (uint8_t i=y; i<y+h; i++) {
    setpixel(x, i, color);
    setpixel(x+w-1, i, color);
  } 
}

// draw a circle outline
void SSD1306OLED::drawcircle(uint8_t x0, uint8_t y0, uint8_t r, 
			uint8_t color) {
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  setpixel(x0, y0+r, color);
  setpixel(x0, y0-r, color);
  setpixel(x0+r, y0, color);
  setpixel(x0-r, y0, color);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    setpixel(x0 + x, y0 + y, color);
    setpixel(x0 - x, y0 + y, color);
    setpixel(x0 + x, y0 - y, color);
    setpixel(x0 - x, y0 - y, color);
    
    setpixel(x0 + y, y0 + x, color);
    setpixel(x0 - y, y0 + x, color);
    setpixel(x0 + y, y0 - x, color);
    setpixel(x0 - y, y0 - x, color);
    
  }
}

void SSD1306OLED::fillcircle(uint8_t x0, uint8_t y0, uint8_t r, 
			uint8_t color) {
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  for (uint8_t i=y0-r; i<=y0+r; i++) {
    setpixel(x0, i, color);
  }

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
  
    for (uint8_t i=y0-y; i<=y0+y; i++) {
      setpixel(x0+x, i, color);
      setpixel(x0-x, i, color);
    } 
    for (uint8_t i=y0-x; i<=y0+x; i++) {
      setpixel(x0+y, i, color);
      setpixel(x0-y, i, color);
    }    
  }
}

// the most basic function, set a single pixel
void SSD1306OLED::setpixel(uint8_t x, uint8_t y, uint8_t color) {
  if ((x >= SSD1306_LCDWIDTH) || (y >= SSD1306_LCDHEIGHT))
    return;

  // x is which column
  if (color == WHITE) 
    buffer[x+ (y/8)*128] |= _BV((y%8));  
  else
    buffer[x+ (y/8)*128] &= ~_BV((y%8)); 
}


void SSD1306OLED::ssd1306_init(uint8_t vccstate) {

    pinModeFast(cs, OUTPUT);
    digitalWriteFast(cs, HIGH);

#ifdef SSD1306_USE_SPI_HARDWARE
      SPI.begin();
      SPI.setBitOrder(MSBFIRST);
      SPI.setDataMode(SPI_MODE0);
      SPI.setClockDivider(SPI_CLOCK_DIV16);
      SPI.attachInterrupt();
#else
      // set pin directions
      pinModeFast(sid, OUTPUT);
      pinModeFast(sclk, OUTPUT);
#endif

#ifndef SSD1306_USE_3WIRE
    pinModeFast(dc, OUTPUT);
#endif

    pinModeFast(rst, OUTPUT);
    pinModeFast(13, OUTPUT);

  digitalWriteFast(rst, HIGH);
  // VDD (3.3V) goes high at start, lets just chill for a ms
  delay(1);
  // bring0xset low
  digitalWriteFast(rst, LOW);
  // wait 10ms
  delay(10);

  // bring out of reset
  digitalWriteFast(rst, HIGH);
  // turn on VCC (9V?)

// steps from app note 3 Software Confirguration
// http://www.adafruit.com/datasheets/SSD1306.pdf
// http://www.adafruit.com/datasheets/UG-2864HSWEG01%20user%20guide.pdf
#ifdef SSD1306_USE_SPI_HARDWARE
  tx_isTransmitting = 1;
  tx_isr_handler();
#else
  for ( uint8_t cmd = 0; cmd < sizeof(command_init); cmd++ )
  {
      ssd1306_command(pgm_read_byte(command_init+cmd));
  }
#endif
}

void SSD1306OLED::ssd1306_reset(void) {
/*

When RES# input is LOW, the chip is initialized with the following status: 
x1. Display is OFF 
2. 128 x 64 Display Mode 
3. Normal segment and display data column address and row address mapping (SEG0 mapped to 
address 00h and COM0 mapped to address 00h) 
4. Shift register data clear in serial interface 
5. Display start line is set at display RAM address 0 
6. Column address counter is set at 0 
7. Normal scan direction of the COM outputs 
8. Contrast control register is set at 7Fh 
9. Normal display mode (Equivalent to A4h command*/
  ssd1306_command(SSD1306_DISPLAYOFF);  // 0xAE


}

void SSD1306OLED::invert(uint8_t i) {
  if (i) {
    ssd1306_command(SSD1306_INVERTDISPLAY);
  } else {
    ssd1306_command(SSD1306_NORMALDISPLAY);
  }
}


inline void SSD1306OLED::spiwrite(uint8_t val) {
	uint8_t i;
#ifndef _SPIWRITE_UNROLLED
	for (i = 0; i < 8; i++)  {
        digitalWriteFast(sid, !!(val & (1 << (7 - i)))); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	
	}
#else
    uint8_t b7 = !!(val & B10000000);
    uint8_t b6 = !!(val & B01000000);
    uint8_t b5 = !!(val & B00100000);
    uint8_t b4 = !!(val & B00010000);
    uint8_t b3 = !!(val & B00001000);
    uint8_t b2 = !!(val & B00000100);
    uint8_t b1 = !!(val & B00000010);
    uint8_t b0 = !!(val & B00000001);

        digitalWriteFast(sid, b7); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b6); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b5); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b4); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b3); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b2); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b1); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b0); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	

        digitalWriteFast(sid, b7); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	
#endif
}

#ifdef SSD1306_USE_3WIRE

inline void SSD1306OLED::spiwrite_3wire(bool isData, uint8_t val) {
        digitalWriteFast(sid, isData); 
        digitalWriteFast(sclk, HIGH);
        __asm__ __volatile__ ("nop \n\tnop \n\t");
		digitalWriteFast(sclk, LOW);	
        spiwrite(val);
}
#endif


inline void SSD1306OLED::ssd1306_command(uint8_t c) { 
#ifdef SSD1306_USE_SPI_HARDWARE
        spiwrite_HardwareCmd(c);
#else
  #ifdef SSD1306_USE_3WIRE
        digitalWriteFast(cs, LOW);
        spiwrite_3wire(false, c);
        digitalWriteFast(cs, HIGH);
  #else
        digitalWriteFast(dc, LOW);
        digitalWriteFast(cs, LOW);
        spiwrite(c);
        digitalWriteFast(cs, HIGH);
  #endif
#endif
}

void SSD1306OLED::ssd1306_data(uint8_t c) {
#ifdef SSD1306_USE_3WIRE
      digitalWriteFast(cs, LOW);
      spiwrite_3wire(true, c);
      digitalWriteFast(cs, HIGH);
#else
      digitalWriteFast(dc, HIGH);
      digitalWriteFast(cs, LOW);
      spiwrite(c);
      digitalWriteFast(cs, HIGH);
#endif
}

void SSD1306OLED::ssd1306_set_brightness(uint8_t val) {
  
}


void SSD1306OLED::display(void) {

  ssd1306_command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
  ssd1306_command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
  ssd1306_command(SSD1306_SETSTARTLINE | 0x0); // line #0

#ifdef SSD1306_USE_SPI_HARDWARE
      spiwrite_HardwareData();
#else
      for (uint16_t i=0; i<sizeof(buffer); i++) {
        ssd1306_data(buffer[i]);
      }
#endif
}

void SSD1306OLED::sync(void) {
#ifdef SSD1306_USE_SPI_HARDWARE
    while((volatile uint8_t)tx_isTransmitting){}
#endif
}
// clear everything
void SSD1306OLED::clear(void) {
  sync();
  memset(buffer, 0, sizeof(buffer));
}

void SSD1306OLED::clear_display(void) {
 
}

#ifdef SSD1306_USE_SPI_HARDWARE

inline void SSD1306OLED::spiwrite_HardwareCmd( uint8_t cmd )
{
    while(!tx_buf.push(cmd)){} // push the command onto the stack
    if ( cmd == 0xFF )         // a cmd code of 0xFF is escaped with a 0xFF, so just send another
    {
        while(!tx_buf.push(0xFF)){}
    }
    // we are assuming that the ISR can't send the above message before we exit
    // otherwise we need to disable the interrupt before to prevent a race on tx_isTransmitting
    if ( !tx_isTransmitting )  
    {
        tx_isTransmitting = 1;
        tx_isr_handler();
    }

}

inline void SSD1306OLED::spiwrite_HardwareData()
{
    while(!tx_buf.push(0xFF)){} // data is indicated by an escaped 0xFF, 0x00 sequence
    while(!tx_buf.push(0x00)){}
    // we are assuming that the ISR can't send the above message before we exit
    // otherwise we need to disable the interrupt before to prevent a race on tx_isTransmitting
    if ( !tx_isTransmitting )
    {
        tx_isTransmitting = 1;
        tx_isr_handler();
    }
}

void SSD1306OLED::tx_isr_handler(void) 
{
    digitalWriteFast(SS, HIGH);

    if ( tx_cbData > 0 )
    {
        tx_cbData--;
        uint16_t offset = sizeof(buffer)-tx_cbData-1;
        digitalWriteFast(SS, LOW);
        SPI.transferNonBlocking(buffer[offset]);
    }
    else if ( tx_init > 0 )
    {
        digitalWriteFast(dc, LOW);
        digitalWriteFast(SS, LOW);
        SPI.transferNonBlocking(pgm_read_byte(command_init+sizeof(command_init)-tx_init));
        tx_init--;
    }
    else if ( !tx_buf.empty() )
    {
        static uint8_t lastSPI = 0x00;
        uint8_t spi = tx_buf.front();
        tx_buf.pop();
        if ( spi != 0xFF          // processing a command packet
             && lastSPI != 0xFF   // and make sure we aren't in the middle of an escape seq.
           )
        {   
            digitalWriteFast(dc, LOW);
            digitalWriteFast(SS, LOW);
            SPI.transferNonBlocking(spi);
        }
        else  // check for escape sequence
        {
            if ( !tx_buf.empty() || lastSPI == 0xFF )
            {
                if ( lastSPI != 0xFF )
                {
                    spi = tx_buf.front();
                    tx_buf.pop();
                }
                if ( spi == 0xFF ) // check for escaped 0xFF command 
                {
                    digitalWriteFast(dc, LOW);
                }
                else // else escaped data
                {
                    digitalWriteFast(dc, HIGH);
                    tx_cbData = sizeof(buffer)-1;
                    spi = buffer[0];
                }
                digitalWriteFast(SS, LOW);
                SPI.transferNonBlocking(spi);
                spi = 0x00;
            }
            else // interrupt during escape sequence - wait until 
            {
            }
        }

        lastSPI = spi;
    }
    else
    {
        digitalWriteFast(dc, LOW);
        tx_isTransmitting = 0;
    }
}

CircularBuffer<uint8_t, uint8_t, SPIDATA_BUFFER_SIZE> SSD1306OLED::tx_buf;
uint16_t SSD1306OLED::tx_cbData = 0;
volatile uint8_t SSD1306OLED::tx_init = sizeof(command_init);
volatile uint8_t SSD1306OLED::tx_isTransmitting = 0;

ISR(SPI_STC_vect)
{
    SSD1306OLED::tx_isr_handler();
}
#endif // SSD1306_USE_SPI_HARDWARE