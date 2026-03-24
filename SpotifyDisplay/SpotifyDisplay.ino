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