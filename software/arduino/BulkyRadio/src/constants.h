#pragma once
const char *app_name = "BulkyRadio";
const char *app_version = "0.1";

#define PREFERENCES_RO true
#define PREFERENCES_RW false

#define MAX_STATIONS 6
#define MAX_RECONNECT 3
#define DELAY_ATTEMPT 1
#define FIRST_STATION 0

#define ONBOARD_LED  2
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26
#define I2S_MUTE 4
#define DEFAULT_VOLUME 10
#define MAX_VOLUME 21

#define ENC_SW 14
#define ENC_DT 33
#define ENC_CLK 32
#define ENC_STEPS 4

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

#define ICON_COUNT 16
uint8_t oled_icons[ICON_COUNT * 8] = {
  0x00, 0x7E, 0x7E, 0x3C, 0x3C, 0x18, 0x18, 0x00, // Play
  0x00, 0x7E, 0x7E, 0x00, 0x00, 0x7E, 0x7E, 0x00, // Pause
  0x00, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x00, // Stop
  0x00, 0x20, 0x50, 0x3E, 0x02, 0x04, 0x00, 0x00, // Note
  0x7E, 0xFF, 0xFF, 0xA1, 0xA1, 0xFF, 0xFF, 0x7E, // Error
  0x00, 0x70, 0x08, 0x64, 0x12, 0xCA, 0x2A, 0xA0, // Wifi
  0x00, 0x7E, 0xFF, 0xFF, 0x7E, 0x00, 0x00, 0x00, // Half-bar
  0x00, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x00, // Full-bar
  0x18, 0x24, 0x24, 0x24, 0x42, 0x81, 0xFF, 0x00, // Volume
  0x00, 0x00, 0x00, 0x00, 0x7E, 0xFF, 0x81, 0x00, // Lower bounds
  0x00, 0x81, 0xFF, 0x7E, 0x00, 0x00, 0x00, 0x00, // Upper bounds
  0x00, 0x09, 0x04, 0x04, 0x09, 0xC0, 0x20, 0xA0, // Unconnected
  0x0C, 0x12, 0x22, 0x44, 0x44, 0x22, 0x12, 0x0C, // Information
  0x00, 0x00, 0xFE, 0x82, 0x44, 0x28, 0x10, 0x00, // Option
  0x60, 0x60, 0x00, 0x60, 0x60, 0x00, 0x60, 0x60, // Truncated
  0x7E, 0x81, 0x81, 0x89, 0x99, 0x3E, 0x18, 0x08, // Sync
};
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

#define SCREEN_MAX_IDLE 5
#define SCREEN_SYNC_IDLE 2*SCREEN_MAX_IDLE
#define SCREEN_CONNECT 0
#define SCREEN_MAIN 1
#define SCREEN_VOLUME 2
#define SCREEN_MENU 3
#define SCREEN_VERSION 4
#define SCREEN_DETAILS 5
#define SCREEN_STATIONS 6
#define SCREEN_SYNC 7

#define OPTION_FIRST_ID 0
#define OPTION_STATIONS OPTION_FIRST_ID
#define OPTION_DETAILS OPTION_FIRST_ID+1
#define OPTION_VERSION OPTION_FIRST_ID+2
#define OPTION_SYNC OPTION_FIRST_ID+3
#define OPTION_LAST_ID OPTION_SYNC

#define SYNC_IDLE 0
#define SYNC_STARTED 1
#define SYNC_ERROR 2
#define SYNC_PARSE_ERROR 3
#define SYNC_DONE 4