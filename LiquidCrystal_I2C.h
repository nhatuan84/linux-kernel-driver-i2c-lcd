#ifndef FDB_LIQUID_CRYSTAL_I2C_H
#define FDB_LIQUID_CRYSTAL_I2C_H

typedef unsigned char uint8_t;

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04 // Enable bit
#define Rw 0x02 // Read/Write bit
#define Rs 0x01 // Register select bit

void LiquidCrystal_I2C(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize);

void begin(void);
void clear(void);
void home(void);
void noDisplay(void);
void display(void);
void noBlink(void);
void blink(void);
void noCursor(void);
void cursor(void);
void scrollDisplayLeft(void);
void scrollDisplayRight(void);
void printLeft(void);
void printRight(void);
void leftToRight(void);
void rightToLeft(void);
void shiftIncrement(void);
void shiftDecrement(void);
void noBacklight(void);
void backlight(void);
uint8_t getBacklight(void);
void autoscroll(void);
void noAutoscroll(void);
void createChar(uint8_t, uint8_t[]);
void setCursor(uint8_t, uint8_t);
int write(uint8_t);
void command(uint8_t);

void blink_on(void);
void blink_off(void);
void cursor_on(void);
void cursor_off(void);
void setBacklight(uint8_t new_val);
void print(const char *s, int len);

typedef void (*dlm)(int ms);
typedef void (*dlu)(int us);
typedef int (*wr)(uint8_t);

void assign_basic_funcs(dlm _dlm, dlu _dlu, wr _wr);

#endif // FDB_LIQUID_CRYSTAL_I2C_H
