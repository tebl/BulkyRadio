#pragma once
#include <Arduino.h>

#define PREFERENCES_RO true
#define PREFERENCES_RW false

extern const char *app_name;
extern const char *app_version;

#define MAX_STATIONS 6
#define MAX_RECONNECT 3
#define DELAY_RECONNECT 1
#define FIRST_STATION 0
#define MAX_FONTS 8

#ifdef CONFIG_IDF_TARGET_ESP32S3
  #define NEOPIXEL_PIN 48
  #define NEOPIXEL_NUM 1
  #define NEOPIXEL_LUM 5

  #define I2S_DOUT 10   // DIN on DAC
  #define I2S_BCLK 12   // Bit clock
  #define I2S_LRC 11    // WSEL on DAC
  #define I2S_MUTE 5

  #define ENC_SW 21
  #define ENC_DT 2
  #define ENC_CLK 1
#else
  #define ONBOARD_LED  2

  #define I2S_BCLK 27
  #define I2S_DOUT 25
  #define I2S_LRC 26
  #define I2S_MUTE 4

  #define I2C_SCK 22
  #define I2C_SDA 21

  #define ENC_SW 14
  #define ENC_DT 33
  #define ENC_CLK 32

  /*
  PIN_D2  ONBOARD_LED
  PIN_D4  I2S_MUTE
  PIN_D13 <unused>
  PIN_D14 ENC_SW
  PIN_D16 <unused>
  PIN_D17 <unused>
  PIN_D18 <unused>
  PIN_D19 <unused>
  PIN_D21 I2C_SDA
  PIN_D22 I2C_SCK
  PIN_D23 <unused>
  PIN_D25 I2S_DOUT
  PIN_D26 I2S_LRC
  PIN_D27 I2S_BCLK
  PIN_D32 ENC_CLK
  PIN_D33 ENC_DT
  */
#endif 

#define DEFAULT_VOLUME 10
#define MAX_VOLUME 21
#define DEFAULT_BALANCE 0
#define MIN_BALANCE -16
#define MAX_BALANCE 16

#define ENC_STEPS 2

#define ACTION_NONE 0
#define ACTION_NEXT 1
#define ACTION_PREV 2
#define ACTION_CLICK 3

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_COLS (OLED_WIDTH / 8)

#define INFO_LEN_STATION (2*OLED_COLS)+1
#define INFO_LEN_TITLE (3*OLED_COLS)+1
#define INFO_LEN_BITS OLED_COLS+1

/* Button debounce delay, meaning that a switch needs to be stable for at
 * least this many milliseconds before we recognize it as a valid press. 
 */
#define DEBOUNCE_DELAY 50
#define BUTTON_LONGPRESS 250

#define ICON_COUNT 17
#define ICON_PLAY 0
#define ICON_PAUSE 1
#define ICON_STOP 2
#define ICON_NOTE 3
#define ICON_ERROR 4
#define ICON_WIFI 5
#define ICON_HALF 6
#define ICON_FULL 7
#define ICON_VOLUME 8
#define ICON_LOWER 9
#define ICON_UPPER 10
#define ICON_UNCONNECTED 11
#define ICON_INFORMATION 12
#define ICON_OPTION 13
#define ICON_TRUNCATED 14
#define ICON_SYNC 15
#define ICON_MUTE 16

#define SCREEN_IDLE 10
extern uint8_t oled_icons[ICON_COUNT * 8];
#define SCREEN_SYNC_IDLE 2*SCREEN_IDLE
#define SCREEN_FONT_IDLE 3*SCREEN_IDLE
#define SCREEN_BALANCE_IDLE 3*SCREEN_IDLE

#define SCREEN_CONNECT 0
#define SCREEN_MAIN 1
#define SCREEN_VOLUME 2
#define SCREEN_MENU 3
#define SCREEN_VERSION 4
#define SCREEN_DETAILS 5
#define SCREEN_STATIONS 6
#define SCREEN_SYNC 7
#define SCREEN_FONT 8
#define SCREEN_BALANCE 9

#define OPTION_FIRST_ID 0
#define OPTION_STATIONS OPTION_FIRST_ID
#define OPTION_DETAILS 1
#define OPTION_VERSION 2
#define OPTION_SYNC 3
#define OPTION_FONT 4
#define OPTION_BALANCE 5
#define OPTION_LAST_ID OPTION_FIRST_ID+5

#define SYNC_IDLE 0
#define SYNC_STARTED 1
#define SYNC_ERROR 2
#define SYNC_PARSE_ERROR 3
#define SYNC_DONE 4

extern const char *option_title_stations;
extern const char *option_title_details;
extern const char *option_title_version;
extern const char *option_title_sync;
extern const char *option_title_font;
extern const char *option_title_balance;

#define STATUS_NONE 0x000000
#define STATUS_CONNECTING 0x0000FF
#define STATUS_ONLINE 0x00FF00
#define STATUS_BUSY 0xFF00FF
#define STATUS_ERROR 0xFF0000