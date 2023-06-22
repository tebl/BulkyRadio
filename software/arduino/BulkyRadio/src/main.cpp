#include <Arduino.h>
#include <WiFi.h>
#include <Audio.h>
#include <Wire.h>
#include <U8x8lib.h>
#include <AiEsp32RotaryEncoder.h>
#include <ezButton.h>
#include <Preferences.h>
#include "constants.h"
#include "settings.h"

Preferences preferences;
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ENC_CLK, ENC_DT, -1, -1, ENC_STEPS);
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
ezButton button(ENC_SW);

Audio audio;
bool is_muted = true;

bool screen_updated = true;
unsigned long last_update = millis();
uint8_t cur_screen = SCREEN_CONNECT;
uint8_t cur_volume = DEFAULT_VOLUME;
uint8_t cur_option = 0;
uint8_t cur_station = 0;
uint8_t sel_station = 0;

bool info_updated = false;
char info_station[INFO_LEN_STATION];
char info_title[INFO_LEN_TITLE];
char info_bits[INFO_LEN_BITS];
bool connected = false;

uint8_t get_action() {
  if (button.isReleased()) {
    return ACTION_CLICK;
  }

  long value = rotaryEncoder.encoderChanged(); 
  if (value < 0) return ACTION_NEXT;
  if (value > 0) return ACTION_PREV;
  return ACTION_NONE;
}

void set_screen(uint8_t new_screen) {
  cur_screen = new_screen;
  last_update = millis();
  screen_updated = true;
}

bool timeout(uint8_t new_screen, uint8_t seconds) {
  if (last_update == 0) return false;
  if ((millis() - last_update) > (seconds * 1000)) {
    set_screen(new_screen);
    return true;
  }
  return false;
}

void set_muted(bool muted) {
  is_muted = muted;
  digitalWrite(I2S_MUTE, (is_muted ? HIGH : LOW));
}

void mute_dac() {
  set_muted(true);
}

void unmute_dac() {
  set_muted(false);
}


void handle_screen_connect() {
  // audio.connecttohost("https://lyd.nrk.no/nrk_radio_p3_mp3_h");
  audio.connecttohost("http://s1.a1radio.nl:8054/;");
  // https://lyd.nrk.no/nrk_radio_p3_aac_l
  // 
  // http://lyd.nrk.no/nrk_radio_p3_mp3_h

  if (audio.isRunning()) {
    unmute_dac();
    set_screen(SCREEN_MAIN);
  } else {
    delay(500);
  }
}

void write_volume() {
  preferences.begin(app_name, PREFERENCES_RW);
  preferences.putShort("volume", cur_volume);
  preferences.end();
  audio.setVolume(cur_volume);
}

void volume_up() {
  set_screen(SCREEN_VOLUME);
  if (cur_volume < MAX_VOLUME) {
    cur_volume = cur_volume + 1;
  }
  write_volume();
}

void volume_down() {
  set_screen(SCREEN_VOLUME);
  if (cur_volume > 0) {
    cur_volume = cur_volume - 1;
  }
  write_volume();
}

void handle_screen_main() {
  switch (get_action()) {
    case ACTION_CLICK:
      cur_option = OPTION_FIRST_ID;
      set_screen(SCREEN_MENU);
      break;

    case ACTION_NEXT:
      volume_up();
      break;
    case ACTION_PREV:
      volume_down();
      break;
  }
}

void handle_screen_volume() {
  if (timeout(SCREEN_MAIN, SCREEN_MAX_IDLE)) return;

  switch (get_action()) {
    case ACTION_CLICK:
      set_screen(SCREEN_MAIN);
      break;
    case ACTION_NEXT:
      volume_up();
      break;
    case ACTION_PREV:
      volume_down();
      break;
  }
}

void handle_screen_info() {
  if (timeout(SCREEN_MENU, SCREEN_MAX_IDLE)) return;

  switch (get_action()) {
    case ACTION_CLICK:
      cur_option = OPTION_BACK;
      set_screen(SCREEN_MENU);
      break;
  }
}

void handle_screen_menu() {
  if (timeout(SCREEN_MAIN, SCREEN_MAX_IDLE)) return;

  switch (get_action()) {
    case ACTION_CLICK:
      switch(cur_option) {
        case OPTION_STATIONS: set_screen(SCREEN_STATIONS); break;
        case OPTION_DETAILS: set_screen(SCREEN_DETAILS); break;
        case OPTION_VERSION: set_screen(SCREEN_VERSION); break;
        case OPTION_BACK: set_screen(SCREEN_MAIN); break;
      }
      break;
    
    case ACTION_NEXT:
      last_update = millis();
      if (cur_option != OPTION_LAST_ID) {
        cur_option++;
        screen_updated = true;
      }
      break;

    case ACTION_PREV:
      last_update = millis();
      if (cur_option != OPTION_FIRST_ID) {
        cur_option--;
        screen_updated = true;
      }
      break;
  }
}

void handle_screen_stations() {
  if (timeout(SCREEN_MENU, SCREEN_MAX_IDLE)) return;
}

void handle_actions() {
  if (cur_screen != SCREEN_CONNECT && !audio.isRunning()) {
    mute_dac();
    set_screen(SCREEN_CONNECT);
  }

  switch(cur_screen) {
    case SCREEN_CONNECT: handle_screen_connect(); break;
    case SCREEN_MAIN: handle_screen_main(); break;
    case SCREEN_VOLUME: handle_screen_volume(); break;
    case SCREEN_MENU: handle_screen_menu(); break;
    case SCREEN_VERSION: handle_screen_info(); break;
    case SCREEN_DETAILS: handle_screen_info(); break;
    case SCREEN_STATIONS: handle_screen_stations(); break;
  }
}

void oled_icon(uint8_t x, uint8_t y, uint8_t num) {
  if (num < ICON_COUNT) {
    u8x8.drawTile(x, y, 1, oled_icons + (8 * num));
  }
}

uint8_t center_string(bool using_icon, const char *title) {
  uint8_t cols = 16;
  if (using_icon) cols - 2;
  if (strlen(title) > cols) return 0;
  return ((cols - strlen(title)) / 2) - 1;
}

void oled_title(uint8_t icon, const char *title) {
    uint8_t x = center_string(icon != -1, title);

    if (icon != -1) {
      oled_icon(x, 0, icon);
      u8x8.drawString(x + 2, 0, title);
    } else {
      u8x8.drawString(x, 0, title);
    }
}

void update_screen_connect() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_icon(0, 0, ICON_WIFI);
    u8x8.drawString(2, 0, WIFI_SSID);
    u8x8.drawString(0, 2, "  Connecting...");
    screen_updated = false;
  }
}

void oled_span(uint8_t x, uint8_t y, const char *string, uint8_t num_lines = 2) {
  bool truncated = false;

  uint8_t cur_x = x;
  uint8_t cur_y = y;
  for (int pos = 0; pos < strlen(string); pos++) {
    u8x8.drawGlyph(cur_x, cur_y, string[pos]);
    cur_x++;
    if (cur_x == OLED_COLS) {
      cur_x = 0;
      cur_y++;
      if (cur_y == (y + num_lines)) {
        truncated = true;
        break;
      }
    }
  }

  if (truncated) oled_icon(15, y + (num_lines - 1), ICON_TRUNCATED);
}

void update_screen_main() {
  if (screen_updated || info_updated) {
    u8x8.clearDisplay();

    oled_title(ICON_WIFI, WIFI_SSID);
    oled_span(0, 2, info_station, 2);
    oled_span(0, 5, info_title, 3);

    info_updated = false;
    screen_updated = false;
  }
}

void oled_percent(uint8_t percent, uint8_t y, bool show_value = false) {
  uint8_t x = 2;
  if (show_value) x = 0;

  oled_icon(x, y, ICON_LOWER);
  oled_icon(x + 11, y, ICON_UPPER);
  if (percent > 0) {
    for (int i = 0; i < 10; i++) {
      if (percent >= 5 + (i * 10)) oled_icon(x + i + 1, y, ICON_HALF);
      if (percent >= 10 + (i * 10)) oled_icon(x + i + 1, y, ICON_FULL);
    }
  }

  if (show_value) {
    u8x8.setCursor(x + 13, y);
    u8x8.print(percent);
  }
}

void update_screen_volume() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(ICON_VOLUME, "Volume");
    uint8_t percent = map(cur_volume, 0, MAX_VOLUME, 0, 100);
    oled_percent(percent, 4, true);
    screen_updated = false;
  }
}

void update_screen_version() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(ICON_INFORMATION, "Version");
    u8x8.drawString(6, 4, app_version);
    screen_updated = false;
  }
}

void update_screen_details() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(ICON_INFORMATION, "Details");

    u8x8.drawString(0, 3, "Codec:");
    u8x8.drawString(0, 4, audio.getCodecname());

    u8x8.drawString(0, 5, "Bitrate:");
    u8x8.drawString(0, 6, info_bits);
    screen_updated = false;
  }
}

void update_screen_stations() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(ICON_NOTE, "Stations");

    screen_updated = false;
  }
}

void update_menu_option(uint8_t id, uint8_t y, const char *title) {
  if (cur_option == id) {
    oled_icon(0, y, ICON_OPTION);
  } else {
    u8x8.drawGlyph(0, y, ' ');
  }
  u8x8.drawString(1, y, title);
}

void update_screen_menu() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(ICON_INFORMATION, "Menu");

    update_menu_option(OPTION_STATIONS, 2, "Stations");
    update_menu_option(OPTION_DETAILS, 3, "Details");
    update_menu_option(OPTION_VERSION, 4, "Version");
    update_menu_option(OPTION_BACK, 5, "Back");
    screen_updated = false;
  }
}

void update_screen() {
  switch(cur_screen) {
    case SCREEN_CONNECT: update_screen_connect(); break;
    case SCREEN_MAIN: update_screen_main(); break;
    case SCREEN_VOLUME: update_screen_volume(); break;
    case SCREEN_MENU: update_screen_menu(); break;
    case SCREEN_VERSION: update_screen_version(); break;
    case SCREEN_DETAILS: update_screen_details(); break;
    case SCREEN_STATIONS: update_screen_stations(); break;
  }
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void setup() {
  Serial.begin(115200);
  preferences.begin(app_name, PREFERENCES_RO);
  cur_volume = preferences.getShort("volume", 10);
  preferences.end();

  rotaryEncoder.disableAcceleration();
  rotaryEncoder.begin();
  rotaryEncoder.setup(readEncoderISR);
  button.setDebounceTime(DEBOUNCE_DELAY);

  u8x8.begin();
  // u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  // u8x8.setFont(u8x8_font_5x8_f);
  // u8x8.setFont(u8x8_font_pxplusibmcga_f);
  // u8x8.setFont(u8x8_font_pxplusibmcgathin_f);
  u8x8.setFont(u8x8_font_pxplustandynewtv_f);

  pinMode(ONBOARD_LED, OUTPUT);
  mute_dac();
  pinMode(I2S_MUTE, OUTPUT);

  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(ONBOARD_LED, LOW);
    delay(500);
    digitalWrite(ONBOARD_LED, HIGH);
  }
  digitalWrite(ONBOARD_LED, HIGH);
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(cur_volume);
}

void loop() {
  button.loop();
  handle_actions();
  update_screen();

  audio.loop();
}

void copy_string(char *destination, const char *source, const uint8_t max_length) {
  bool eof = false;
  for (int i = 0; i < (max_length - 1); i++) {
    if (!eof) {
      if (source[i] == 0) {
        eof = true;
      }

      if (destination[i] != source[i]) {
        destination[i] = source[i];
        info_updated = true;
      }
    } else destination[i] = 0;
  }
  destination[max_length - 1] = 0;
}

// void audio_info(const char *info){
//     Serial.print("info        "); Serial.println(info);
// }

void audio_showstation(const char *info) {
  copy_string(info_station, info, INFO_LEN_STATION);
}

void audio_showstreamtitle(const char *info) {
  copy_string(info_title, info, INFO_LEN_TITLE);
}

void audio_bitrate(const char *info) {
  copy_string(info_bits, info, INFO_LEN_BITS);
}