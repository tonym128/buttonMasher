// Thank you very much to VolosR for the original code.
// https://www.youtube.com/watch?v=j8THAc1sMww
// https://github.com/VolosR/NewTTGOAnalogReadings

#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include <WiFi.h>
#include "time.h"
#include "Button2.h"
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 35

TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&lcd);
// TFT_eSprite sprite2 = TFT_eSprite(&lcd);

Button2 buttonA, buttonB;

#define SCALEX(x) (x * 4 / 5)
#define SCALEY(y) (y * 3 / 4)

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

struct button {
  int values[24] = {0};
  int pin = -1;
  int threshold = 70;
  int average = 0;
  bool pressed = 0;
  int colour = TFT_BLACK;
  bool digital = false;
};

button buttons[10];

char timeHour[3] = "00";
char timeMin[3] = "00";
char timeSec[3];

char m[12];
char y[5];
char d[3];
char dw[12];

int gw = SCALEX(204);
int gh = SCALEY(102);
int gx = SCALEX(110);
int gy = SCALEY(144);
int currentVal = 0;
#define gray 0x6B6D
#define blue 0x0967
#define orange 0xC260
#define purple 0x604D
#define green 0x1AE9

#define RIGHT 35

// void copySprite() {
//   for (int x = 0; x < 240; x++) {
//     for (int y = 0; y < 135; y++) {
//       sprite2.drawPixel(x,y,sprite.readPixelValue(x*4/3,y*5/4));
//     }
//   }
// }

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }

  strftime(timeHour, 3, "%H", &timeinfo);
  strftime(timeMin, 3, "%M", &timeinfo);
  strftime(timeSec, 3, "%S", &timeinfo);
  strftime(y, 5, "%Y", &timeinfo);
  strftime(m, 12, "%B", &timeinfo);

  strftime(dw, 10, "%A", &timeinfo);

  strftime(d, 3, "%d", &timeinfo);
}

bool buttonAPress = false;
bool buttonBPress = false;
int displayMode = 0;
int MAX_PIN_MODE = 9;
int MIN_PIN_MODE = 0;


int counter = 0;
int Min = gh / 2;
int Max = 0;
int average = 0;
String minT = "";
String maxT = "";
char pinString[] = "ALL";
char pinString2[] = "ON PIN ALL";

long lastMillis = 0;
int fps = 0;

int digitalReadPin(struct button *empty, struct button *current)
{
  int Push_button_state = current->pin == 0 ? buttonAPress : buttonBPress;

  int num = Push_button_state == HIGH ? 10 : 80;

  currentVal = num;

  for (int i = 0; i < 24; i++)
    empty->values[i] = current->values[i];

  for (int i = 23; i > 0; i--)
    current->values[i - 1] = empty->values[i];

  current->pressed = currentVal < current->threshold;
  current->values[23] = currentVal;
  if (currentVal > Max)
  {
    Max = currentVal;
    maxT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }
  if (currentVal < Min)
  {
    Min = currentVal;
    minT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }

  average = 0;
  for (int i = 0; i < 24; i++)
    average = average + current->values[i];
  average = average / 24;
  current->average = average;
  return average;
}


int touchReadPin(struct button *empty, struct button *current)
{
  int num = touchRead(current->pin);
  currentVal = num; // map(num,0,1024,0,gh);

  for (int i = 0; i < 24; i++)
    empty->values[i] = current->values[i];

  for (int i = 23; i > 0; i--)
    current->values[i - 1] = empty->values[i];

  current->pressed = currentVal < current->threshold;
  current->values[23] = currentVal;
  if (currentVal > Max)
  {
    Max = currentVal;
    maxT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }
  if (currentVal < Min)
  {
    Min = currentVal;
    minT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }

  average = 0;
  for (int i = 0; i < 24; i++)
    average = average + current->values[i];
  average = average / 24;
  current->average = average;
  return average;
}
void doubleClick(Button2 &btn)
{
  if (btn == buttonA)
  {
    displayMode -= 1;
  }
  else if (btn == buttonB)
  {
    displayMode += 1;
  }

  if (displayMode > MAX_PIN_MODE)
    displayMode = MIN_PIN_MODE;
  if (displayMode < MIN_PIN_MODE)
    displayMode = MAX_PIN_MODE;
}

void pressed(Button2 &btn)
{
  if (btn == buttonA)
  {
    buttonAPress = 1;
  }
  else if (btn == buttonB)
  {
    buttonBPress = 1;
  }
}

void released(Button2 &btn)
{
  if (btn == buttonA)
  {
    buttonAPress = 0;
  }
  else if (btn == buttonB)
  {
    buttonBPress = 0;
  }
}

bool configuredTime = false;

void buildThresholds() {
  int average;
  for (int i = 0; i < 24; i++) {
    for (int j = 1; j < 10; j++) {
        if (buttons[j].digital) 
          digitalReadPin(&buttons[0],&buttons[j]);
        else 
          touchReadPin(&buttons[0],&buttons[j]);
    }
  }

  for (int j = 1; j < 10; j++)
    buttons[j].threshold = buttons[j].average*3/4;
}

void setup(void)
{
  buttons[0].pin = -1;
  buttons[1].pin = 2;
  buttons[2].pin = 15;
  buttons[3].pin = 13;
  buttons[4].pin = 12;
  buttons[5].pin = 32;
  buttons[6].pin = 33;
  buttons[7].pin = 27;
  buttons[8].pin = 0;
  buttons[9].pin = 35;

  buttons[1].colour = TFT_RED;
  buttons[2].colour = TFT_BLUE;
  buttons[3].colour = TFT_YELLOW;
  buttons[4].colour = TFT_GREEN;
  buttons[5].colour = TFT_ORANGE;
  buttons[6].colour = TFT_BROWN;
  buttons[7].colour = TFT_VIOLET;
  buttons[8].colour = TFT_SKYBLUE;
  buttons[9].colour = TFT_CYAN;

  buttons[0].digital = 0;
  buttons[1].digital = 0;
  buttons[2].digital = 0;
  buttons[3].digital = 0;
  buttons[4].digital = 0;
  buttons[5].digital = 0;
  buttons[6].digital = 0;
  buttons[7].digital = 0;
  buttons[8].digital = 1;
  buttons[9].digital = 1;

  Serial.begin(115200);
  lcd.init();
  lcd.fillScreen(TFT_BLACK);
  lcd.setRotation(1);

  WiFiManager wm;
  bool res;
  sprite.createSprite(240, 135);
  sprite.setTextDatum(3);
  sprite.setSwapBytes(true);
  sprite.setTextColor(TFT_RED);
  sprite.drawCentreString("Initialising WiFi",0,0,3);
  sprite.pushSprite(0, 0);

  res = wm.autoConnect(); // auto generated AP name from chipid
  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
  }

  buildThresholds();

  buttonA.begin(BUTTON_A_PIN);
  buttonA.setPressedHandler(pressed);
  buttonA.setReleasedHandler(released);
  buttonA.setDoubleClickHandler(doubleClick);

  buttonB.begin(BUTTON_B_PIN);
  buttonB.setPressedHandler(pressed);
  buttonB.setReleasedHandler(released);
  buttonB.setDoubleClickHandler(doubleClick);
}

void drawPoint(int i, int num1, int num2, int color)
{
  sprite.drawLine(gx + (i * SCALEX(17)), gy - num1, gx + ((i + 1) * SCALEX(17)), gy - num2, color);
  sprite.drawLine(gx + (i * SCALEX(17)), gy - num1 - 1, gx + ((i + 1) * SCALEX(17)), gy - num2 - 1, color);
}

void loop()
{
  if (configuredTime == false && WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected.");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    configuredTime = true;
  }

  fps = 1000 / (millis() - lastMillis);
  lastMillis = millis();

  buttonA.loop();
  buttonB.loop();

  average = 0;
  if (counter == 0)
    printLocalTime();

  counter++;
  if (counter == 50)
    counter = 0;

  for (int i = 1; i < 10; i++)
  {
    average += buttons[i].digital ? digitalReadPin(&buttons[0],&buttons[i]) : touchReadPin(&buttons[0],&buttons[i]);
  }

  average /= 9;

  sprite.fillSprite(TFT_BLACK);

  sprite.setTextColor(TFT_WHITE, blue);
  sprite.fillRoundRect(SCALEX(6), SCALEY(5), SCALEX(38), SCALEY(32), 3, blue);
  sprite.fillRoundRect(SCALEX(52), SCALEY(5), SCALEX(38), SCALEY(32), 3, blue);
  sprite.fillRoundRect(SCALEX(6), SCALEY(42), SCALEX(80), SCALEY(12), 3, blue);
  sprite.fillRoundRect(SCALEX(6), SCALEY(82), SCALEX(78), SCALEY(76), 3, purple);
  sprite.fillRoundRect(SCALEX(6), SCALEY(58), SCALEX(80), SCALEY(18), 3, green);
  sprite.drawString(String(timeHour), SCALEX(10), SCALEY(26), 4);
  sprite.drawString(String(timeMin), SCALEX(56), SCALEY(26), 4);
  sprite.drawString(String(m) + " " + String(d), SCALEX(10), SCALEY(48));

  sprite.drawString(String(timeSec), gx - SCALEX(14), SCALEY(14), 2);
  sprite.setTextColor(TFT_WHITE, purple);
  sprite.drawString("CURR: " + String(average), SCALEX(10), SCALEY(92), 2);

  sprite.drawString("MIN: " + String(Min), SCALEX(10), SCALEY(112), 2);

  sprite.drawString("MAX: " + String(Max), SCALEX(10), SCALEY(138), 2);

  sprite.setTextColor(TFT_SILVER, purple);
  sprite.drawString(String(maxT), SCALEX(10), SCALEY(152));
  sprite.drawString(String(minT), SCALEX(10), SCALEY(122));
  sprite.setTextColor(TFT_WHITE, green);
  sprite.drawString("SPEED:" + String(fps) + " fps", SCALEX(10), SCALEY(68));
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);

  if (displayMode == 0)
  {
    sprintf(pinString, "All");
  }
  else
    sprintf(pinString, "%d", buttons[displayMode].pin);

  sprintf(pinString2, "PIN %s", pinString);
  sprite.drawString(pinString2, gx + 10, 25);

  sprite.setFreeFont();

  for (int i = 1; i < 12; i++)
  {
    sprite.drawLine(gx + (i * SCALEX(17)), gy, gx + (i * SCALEX(17)), gy - gh, gray);
  }

  for (int i = 1; i < 6; i++)
  {
    sprite.drawLine(gx, gy - (i * SCALEX(17)), gx + gw, gy - (i * SCALEX(17)), gray);
  }

  sprite.drawLine(gx, gy, gx + gw, gy, TFT_WHITE);
  sprite.drawLine(gx, gy, gx, gy - gh, TFT_WHITE);

  for (int j = 1; j < 10; j++)
  {
    int x = gx + j * SCALEX(10);
    int y = SCALEY(16);
    sprite.fillRect(x, y, SCALEX(10), SCALEX(10), buttons[j].pressed ? buttons[j].colour : TFT_BLACK);

    for (int i = 0; i < 23; i++)
    {
      if (displayMode == 0 | displayMode == j)
        drawPoint(i, map(buttons[j].values[i], 0, Max, 0, gh), map(buttons[j].values[i + 1], 0, Max, 0, gh), buttons[j].colour);
    }
  }

  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  // sprite.drawString("BAT:" + String(analogRead(4)), gx + 160, 16);
  // sprite.drawString("MOD:" + String(Mode), gx + 160, 26);

  sprite.pushSprite(0, 0);
  // sprite.pushSprite(0, 0);
  char *s = new char[256];
  sprintf(s,"");
  for (int i = 1; i < 10; i++) {
    sprintf(s + strlen(s),"p%d:d%d:v%d:a%d ",buttons[i].pin, buttons[i].digital, buttons[i].values[23] ,buttons[i].pressed);
  }
  Serial.println(s);
  delete (s);

  // delay(5);
}
