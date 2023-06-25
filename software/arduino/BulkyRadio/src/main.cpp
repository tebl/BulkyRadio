#include <AiEsp32RotaryEncoder.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Audio.h>
#include <ezButton.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <U8x8lib.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "constants.h"
#include "settings.h"

Preferences preferences;
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ENC_CLK, ENC_DT, -1, -1, ENC_STEPS);
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE);
ezButton button(ENC_SW);

Audio audio;
uint8_t reconnects = 0;
bool is_muted = true;
bool is_halted = !AUTOPLAY;

bool screen_updated = true;
unsigned long last_update = millis();
uint8_t cur_screen = AUTOPLAY ? SCREEN_CONNECT : SCREEN_MAIN;
uint8_t cur_volume = DEFAULT_VOLUME;
uint8_t cur_option = 0;
uint8_t cur_station = 0;
uint8_t sel_station = 0;

bool info_updated = false;
char info_station[INFO_LEN_STATION];
char info_title[INFO_LEN_TITLE];
char info_bits[INFO_LEN_BITS];
bool connected = false;
uint8_t sync_state = 0;

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

void set_sync_state(uint8_t new_state) {
  sync_state = new_state;
  screen_updated = true;
  last_update = millis();
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

void set_halted(bool value) {
  if (value) {
    audio.stopSong();
    mute_dac();
  } else {
    reconnects = 0;
  }
  is_halted = value;
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

void perform_connect() {
  preferences.begin(app_name, PREFERENCES_RO);

  char key[] = "s0";
  key[1] = '0' + sel_station;

  String url = preferences.getString(key);
  if (url != NULL) {
    audio.connecttohost(url.c_str());
  }

  preferences.end();
}

void handle_actions_connect() {
  audio.stopSong();
  info_station[0] = 0;
  info_bits[0] = 0;
  info_title[0] = 0;

  perform_connect();
  if (audio.isRunning()) {
    set_halted(false);
    unmute_dac();
    set_screen(SCREEN_MAIN);
  } else {
    reconnects++;
    if (reconnects == MAX_RECONNECT) {
      set_halted(true);
      set_screen(SCREEN_MAIN);
    } else {
      delay(DELAY_ATTEMPT * 1000);
    }
  }
}

void handle_actions_main() {
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

void handle_actions_volume() {
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

void handle_actions_info() {
  if (timeout(SCREEN_MAIN, SCREEN_MAX_IDLE)) return;

  switch (get_action()) {
    case ACTION_CLICK:
      set_screen(SCREEN_MAIN);
      break;
  }
}

void handle_actions_menu() {
  if (timeout(SCREEN_MAIN, SCREEN_MAX_IDLE)) return;

  switch (get_action()) {
    case ACTION_CLICK:
      switch(cur_option) {
        case OPTION_STATIONS:
          cur_station = sel_station;
          set_screen(SCREEN_STATIONS);
          break;
        case OPTION_DETAILS: set_screen(SCREEN_DETAILS); break;
        case OPTION_VERSION: set_screen(SCREEN_VERSION); break;
        case OPTION_SYNC:
          set_sync_state(SYNC_IDLE);
          set_screen(SCREEN_SYNC);
          break;
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

void write_station() {
  sel_station = cur_station;

  preferences.begin(app_name, PREFERENCES_RW);
  preferences.putShort("station", sel_station);
  preferences.end();
}

void handle_actions_stations() {
  if (timeout(SCREEN_MAIN, SCREEN_MAX_IDLE)) return;

  switch (get_action()) {
    case ACTION_CLICK:
      write_station();
      set_screen(SCREEN_CONNECT);
      break;
    
    case ACTION_NEXT:
      last_update = millis();
      if (cur_station != (MAX_STATIONS - 1)) {
        cur_station++;
        screen_updated = true;
      }
      break;

    case ACTION_PREV:
      last_update = millis();
      if (cur_station != FIRST_STATION) {
        cur_station--;
        screen_updated = true;
      }
      break;
  }
}

void preferences_put_string(const char *key, const char *value) {
  preferences.putString(key, value);
  Serial.print(key);
  Serial.print(":");
  Serial.println(value);
}

void preferences_delete(const char *key) {
  if (preferences.isKey(key)) {
    Serial.print("Removing ");
    Serial.println(key);
    preferences.remove(key);
  }
}

void preferences_generate_name(const char *key_title, const char *station_url) {
  if (strncmp(station_url, "http://", 7) == 0) station_url += 7;
  if (strncmp(station_url, "https://", 8) == 0) station_url += 8;
  if (strncmp(station_url, "www.", 4) == 0) station_url += 4;

  char name[OLED_COLS];
  strncat(name, station_url, 14);
  preferences_put_string(key_title, name);
}

void perform_sync() {
  WiFiClientSecure client;
  HTTPClient https;
  
  client.setCACert(sync_ca);
  if (https.begin(client, sync_url)) {
    int httpCode = https.GET();

    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        const size_t capacity = JSON_OBJECT_SIZE(6) + 300;
        DynamicJsonDocument doc(capacity);
        DeserializationError error = deserializeJson(doc, https.getString());

        if (error) {
          Serial.print("Parse error: ");
          Serial.println(error.f_str());
          set_sync_state(SYNC_PARSE_ERROR);
        } else {
          preferences.begin(app_name, PREFERENCES_RW);

          char key[] = "s0"; 
          char key_title[] = "s0t";
          for (int i = 0; i < MAX_STATIONS; i++) {
            key[1] = '0' + i;
            key_title[1] = '0' + i;

            if (doc.containsKey(key) and strlen(doc[key]) > 0) {
              preferences_put_string(key, doc[key]);

              if (doc.containsKey(key_title) and strlen(doc[key_title]) > 0) {
                preferences_put_string(key_title, doc[key_title]);
              } else {
                preferences_generate_name(key_title, doc[key]);
              }
            } else {
              preferences_delete(key);
              preferences_delete(key_title);
            }
          }
          preferences.end();

          set_sync_state(SYNC_DONE);
        }
      }
    } else {
      set_sync_state(SYNC_ERROR);
    }

    https.end();
  }  
}

void handle_actions_sync() {
  switch (sync_state) {
    case SYNC_IDLE:
      set_sync_state(SYNC_STARTED);
      return;
    
    case SYNC_STARTED:
      set_halted(true);
      perform_sync();
      return;

    case SYNC_PARSE_ERROR:
    case SYNC_DONE:
    case SYNC_ERROR:
    default:
      break;
  }

  if (timeout(SCREEN_CONNECT, SCREEN_SYNC_IDLE)) return;
  switch (get_action()) {
    case ACTION_CLICK:
      set_screen(SCREEN_CONNECT);
      break;
  }
}

void handle_actions() {
  if (!is_halted && !audio.isRunning()) {
    mute_dac();
    set_screen(SCREEN_CONNECT);
  }

  switch(cur_screen) {
    case SCREEN_CONNECT: handle_actions_connect(); break;
    case SCREEN_MAIN: handle_actions_main(); break;
    case SCREEN_VOLUME: handle_actions_volume(); break;
    case SCREEN_MENU: handle_actions_menu(); break;
    case SCREEN_VERSION: handle_actions_info(); break;
    case SCREEN_DETAILS: handle_actions_info(); break;
    case SCREEN_STATIONS: handle_actions_stations(); break;
    case SCREEN_SYNC: handle_actions_sync(); break;
  }
}

void oled_icon(uint8_t x, uint8_t y, uint8_t num) {
  if (num < ICON_COUNT) {
    u8x8.drawTile(x, y, 1, oled_icons + (8 * num));
  }
}

uint8_t wlan_icon() {
  switch (WiFi.status()) {
  case WL_CONNECTED:
    return ICON_WIFI;

  default:
    return ICON_UNCONNECTED;
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

void update_screen_connect() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(wlan_icon(), WIFI_SSID);
    u8x8.drawString(0, 2, "  Connecting...");
    screen_updated = false;
  }
}

void update_screen_main() {
  if (screen_updated || info_updated) {
    u8x8.clearDisplay();

    if (is_halted) {
      oled_title(wlan_icon(), WIFI_SSID);
      oled_icon(1, 4, ICON_STOP);
      u8x8.drawString(3, 4, "Not playing");
  } else {
      oled_title(ICON_WIFI, WIFI_SSID);
      oled_span(0, 2, info_station, 2);
      oled_span(0, 5, info_title, 3);
    }

    info_updated = false;
    screen_updated = false;
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

void update_station_option(uint8_t id, uint8_t y, const char *title) {
  if (cur_station == id) {
    oled_icon(0, y, ICON_OPTION);
  } else {
    if (id == sel_station) {
      oled_icon(0, y, ICON_NOTE);
    } else {
      u8x8.drawGlyph(0, y, ' ');
    }
  }
  u8x8.drawString(1, y, title);
}

void update_screen_stations() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(ICON_NOTE, "Stations");
    preferences.begin(app_name, PREFERENCES_RO);

    char key[] = "s0"; 
    char key_title[] = "s0t";
    for (int i = 0; i < MAX_STATIONS; i++) {
      key[1] = '0' + i;
      key_title[1] = '0' + i;

      update_station_option(i, 2 + i, preferences.getString(key_title).c_str());
    }

    preferences.end();
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
    update_menu_option(OPTION_SYNC, 5, "Sync");
    screen_updated = false;
  }
}

void update_screen_sync() {
  if (screen_updated) {
    u8x8.clearDisplay();
    oled_title(ICON_SYNC, "Sync");

    switch(sync_state) {
      case SYNC_DONE:
        u8x8.drawString(5, 4, "Done!");
        break;

      case SYNC_ERROR:
        u8x8.drawString(3, 4, "Failed!");
        break;

      case SYNC_PARSE_ERROR:
        u8x8.drawString(3, 2, "Parse error!");
        break;

      default:
        u8x8.drawString(2, 4, "Working ...");
        break;
    }

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
    case SCREEN_SYNC: update_screen_sync(); break;
  }
}

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}

void setup() {
  Serial.begin(115200);
  preferences.begin(app_name, PREFERENCES_RO);
  cur_volume = preferences.getShort("volume", 10);
  sel_station = preferences.getShort("station", 0);
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

void audio_showstation(const char *info) {
  copy_string(info_station, info, INFO_LEN_STATION);
}

void audio_showstreamtitle(const char *info) {
  copy_string(info_title, info, INFO_LEN_TITLE);
}

void audio_bitrate(const char *info) {
  copy_string(info_bits, info, INFO_LEN_BITS);
}