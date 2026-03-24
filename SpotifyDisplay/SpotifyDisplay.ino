#include <Arduino.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <SpotifyEsp32.h>
#include <SPI.h>

#define LCD_I2C_ADDR 0x27
#define BUTTON_PREV 2
#define BUTTON_PLAY_PAUSE 3
#define BUTTON_NEXT 4

char* SSID = "WIFI SSID";
const char* PASSWORD = "WIFI PASSWORD";
const char* CLIENT_ID = "CLIENT ID";
const char* CLIENT_SECRET = "CLIENT SECRET";

String lastArtist;
String lastTrackname;

Spotify sp(CLIENT_ID, CLIENT_SECRET);
LiquidCrystal_I2C lcd(LCD_I2C_ADDR, 16, 2);

enum Mode { MODE_SPOTIFY, MODE_POMODORO };
Mode currentMode = MODE_SPOTIFY;
bool pomodoroRunning = false;
unsigned long pomodoroStart = 0;
unsigned long pomodoroElapsed = 0;
const unsigned long POMODORO_WORK = 25 * 60 * 1000;
const unsigned long POMODORO_BREAK = 5 * 60 * 1000;
bool onBreak = false;

void displayPomodoro(unsigned long remaining) {
    lcd.clear();
    lcd.setCursor(0, 0);
    if (onBreak) {
        lcd.print("Break:");
    } else {
        lcd.print("Work:");
    }
    int minutes = remaining / 60000;
    int seconds = (remaining % 60000) / 1000;
    char buf[17];
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
    lcd.setCursor(0, 1);
    lcd.print(buf);
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PREV, INPUT_PULLUP);
    pinMode(BUTTON_PLAY_PAUSE, INPUT_PULLUP);
    pinMode(BUTTON_NEXT, INPUT_PULLUP);
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi");
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi...");
    while(WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.printf("\nConnected!\n");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString().c_str());
    sp.begin();
    while(!sp.is_auth()){
        sp.handle_client();
    }
    Serial.println("Authenticated");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Spotify OK");
}

void loop()
{
    static bool lastNext = HIGH;
    static bool lastPrev = HIGH;
    bool playPause = digitalRead(BUTTON_PLAY_PAUSE);
    bool next = digitalRead(BUTTON_NEXT);
    bool prev = digitalRead(BUTTON_PREV);


    static bool lastCombo = false;
    bool combo = (prev == LOW && next == LOW);
    if (!lastCombo && combo) {
        if (currentMode == MODE_SPOTIFY) {
            currentMode = MODE_POMODORO;
            pomodoroRunning = false;
            pomodoroElapsed = 0;
            onBreak = false;
            displayPomodoro(POMODORO_WORK);
        } else {
            currentMode = MODE_SPOTIFY;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Spotify OK");
        }
        delay(500);
    }
    lastCombo = combo;

    if (currentMode == MODE_POMODORO) {
        if (lastNext == HIGH && next == LOW) {
            if (!pomodoroRunning) {
                pomodoroRunning = true;
                pomodoroStart = millis();
            } else {
                pomodoroRunning = false;
                pomodoroElapsed += millis() - pomodoroStart;
            }
            delay(300);
        }
        lastNext = next;
        if (lastPrev == HIGH && prev == LOW) {
            pomodoroRunning = false;
            pomodoroElapsed = 0;
            onBreak = false;
            displayPomodoro(POMODORO_WORK);
            delay(300);
        }
        lastPrev = prev;
        unsigned long total = onBreak ? POMODORO_BREAK : POMODORO_WORK;
        unsigned long elapsed = pomodoroElapsed;
        if (pomodoroRunning) {
            elapsed += millis() - pomodoroStart;
        }
        if (elapsed >= total) {
            pomodoroRunning = false;
            pomodoroElapsed = 0;
            onBreak = !onBreak;
            displayPomodoro(onBreak ? POMODORO_BREAK : POMODORO_WORK);
        } else {
            displayPomodoro(total - elapsed);
        }
        delay(200);
        return;
    }
    lastNext = next;
    lastPrev = prev;

    if (digitalRead(BUTTON_PREV) == LOW) {
        sp.previous_track();
        delay(300);
    }
    if (digitalRead(BUTTON_PLAY_PAUSE) == LOW) {
        sp.toggle_play();
        delay(300);
    }
    if (digitalRead(BUTTON_NEXT) == LOW) {
        sp.next_track();
        delay(300);
    }
    String currentArtist = sp.current_artist_names();
    String currentTrackname = sp.current_track_name();
    String artistDisplay = currentArtist.substring(0, 16);
    String trackDisplay = currentTrackname.substring(0, 16);
    if (lastArtist != currentArtist && currentArtist != "Something went wrong" && !currentArtist.isEmpty()) {
        lastArtist = currentArtist;
        Serial.println("Artist: " + lastArtist);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(artistDisplay);
        lcd.setCursor(0, 1);
        lcd.print(trackDisplay);
    }
    if (lastTrackname != currentTrackname && currentTrackname != "Something went wrong" && currentTrackname != "null") {
        lastTrackname = currentTrackname;
        Serial.println("Track: " + lastTrackname);
        lcd.setCursor(0, 1);
        lcd.print(trackDisplay);
    }
    delay(2000);
}