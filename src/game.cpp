// Thank you very much to VolosR for the original code.
// https://www.youtube.com/watch?v=j8THAc1sMww
// https://github.com/VolosR/NewTTGOAnalogReadings

#include "Arduino.h"
#include "TFT_eSPI.h" /* Please use the TFT library provided in the library. */
#include <WiFi.h>
#include "time.h"
#include "Button2.h"

#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 35
int PIN_THRESHOLD = 70;

TFT_eSPI lcd = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&lcd);
// TFT_eSprite sprite2 = TFT_eSprite(&lcd);

Button2 buttonA, buttonB;

#define SCALEX(x) (x * 4 / 5)
#define SCALEY(y) (y * 3 / 4)

const char *ssid = "wifi";
const char *password = "password";

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

int values_old[24] = {0};

int valuesa[24] = {0};
int valuesb[24] = {0};
int valuesc[24] = {0};
int valuesd[24] = {0};
int valuese[24] = {0};
int valuesf[24] = {0};
int valuesg[24] = {0};
int valuesh[24] = {0};
int valuesi[24] = {0};

int colours[10];
bool digitalOrAnalog[10];

int *values[10];

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
int curent = 0;
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
    displayMode -= 1;
  }
  else if (btn == buttonB)
  {
    buttonBPress = 0;
    displayMode += 1;
  }

  if (displayMode > MAX_PIN_MODE)
    displayMode = MIN_PIN_MODE;
  if (displayMode < MIN_PIN_MODE)
    displayMode = MAX_PIN_MODE;
}

int pinArray[10];
bool configuredTime = false;

void setup(void)
{
  pinArray[0] = -1;
  pinArray[1] = 2;
  pinArray[2] = 15;
  pinArray[3] = 13;
  pinArray[4] = 12;
  pinArray[5] = 32;
  pinArray[6] = 33;
  pinArray[7] = 27;
  pinArray[8] = 0;
  pinArray[9] = 35;

  values[0] = values_old;
  values[1] = valuesa;
  values[2] = valuesb;
  values[3] = valuesc;
  values[4] = valuesd;
  values[5] = valuese;
  values[6] = valuesf;
  values[7] = valuesg;
  values[8] = valuesh;
  values[9] = valuesi;

  colours[1] = TFT_RED;
  colours[2] = TFT_BLUE;
  colours[3] = TFT_YELLOW;
  colours[4] = TFT_GREEN;
  colours[5] = TFT_ORANGE;
  colours[6] = TFT_BROWN;
  colours[7] = TFT_VIOLET;
  colours[8] = TFT_SKYBLUE;
  colours[9] = TFT_CYAN;

  digitalOrAnalog[0] = 1;
  digitalOrAnalog[1] = 1;
  digitalOrAnalog[2] = 1;
  digitalOrAnalog[3] = 1;
  digitalOrAnalog[4] = 1;
  digitalOrAnalog[5] = 1;
  digitalOrAnalog[6] = 1;
  digitalOrAnalog[7] = 1;
  digitalOrAnalog[8] = 0;
  digitalOrAnalog[9] = 0;

  Serial.begin(115200);
  lcd.init();
  lcd.fillScreen(TFT_BLACK);
  lcd.setRotation(1);

  WiFi.begin(ssid, password);
  if (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  // sprite.createSprite(320, 170);
  // sprite.setTextDatum(3);
  // sprite.setSwapBytes(true);

  sprite.createSprite(240, 135);
  sprite.setTextDatum(3);
  sprite.setSwapBytes(true);

  buttonA.begin(BUTTON_A_PIN);
  buttonA.setPressedHandler(pressed);
  buttonA.setReleasedHandler(released);

  buttonB.begin(BUTTON_B_PIN);
  buttonB.setPressedHandler(pressed);
  buttonB.setReleasedHandler(released);
}

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

int digitalReadPin(int pin, int *array1, int *array2)
{
  int Push_button_state = pin == 0 ? buttonAPress : buttonBPress;

  int num = Push_button_state == HIGH ? 10 : 140;

  curent = num;

  for (int i = 0; i < 24; i++)
    array2[i] = array1[i];

  for (int i = 23; i > 0; i--)
    array1[i - 1] = array2[i];

  array1[23] = curent;
  if (array1[23] > Max)
  {
    Max = array1[23];
    maxT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }
  if (array1[23] < Min)
  {
    Min = array1[23];
    minT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }

  average = 0;
  for (int i = 0; i < 24; i++)
    average = average + array1[i];
  average = average / 24;
  return average;
}

int touchReadPin(int pin, int *array1, int *array2)
{
  int num = touchRead(pin);
  curent = num; // map(num,0,1024,0,gh);

  for (int i = 0; i < 24; i++)
    array2[i] = array1[i];

  for (int i = 23; i > 0; i--)
    array1[i - 1] = array2[i];

  array1[23] = curent;
  if (array1[23] > Max)
  {
    Max = array1[23];
    maxT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }
  if (array1[23] < Min)
  {
    Min = array1[23];
    minT = String(timeHour) + ":" + String(timeMin) + ":" + String(timeSec);
  }

  average = 0;
  for (int i = 0; i < 24; i++)
    average = average + array1[i];
  average = average / 24;
  return average;
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
    average += digitalOrAnalog[i] ? touchReadPin(pinArray[i], values[i], values[0]) : digitalReadPin(pinArray[i], values[i], values[0]);
  }

  average /= 9;

  sprite.fillSprite(TFT_BLACK);
  // sprite.setTextDatum(4);
  sprite.setTextColor(TFT_WHITE, blue);
  sprite.fillRoundRect(SCALEX(6), SCALEY(5), SCALEX(38), SCALEY(32), 3, blue);
  sprite.fillRoundRect(SCALEX(52), SCALEY(5), SCALEX(38), SCALEY(32), 3, blue);
  sprite.fillRoundRect(SCALEX(6), SCALEY(42), SCALEX(80), SCALEY(12), 3, blue);
  sprite.fillRoundRect(SCALEX(6), SCALEY(82), SCALEX(78), SCALEY(76), 3, purple);
  sprite.fillRoundRect(SCALEX(6), SCALEY(58), SCALEX(80), SCALEY(18), 3, green);
  sprite.drawString(String(timeHour), SCALEX(10), SCALEY(24), 4);
  sprite.drawString(String(timeMin), SCALEX(56), SCALEY(24), 4);
  sprite.drawString(String(m) + " " + String(d), SCALEX(10), SCALEY(48));
  // sprite.drawString(String(dw)+", "+String(y),10,58);

  // sprite.drawString(String(analogRead(18)),gx+160,26);

  sprite.drawString(String(timeSec), gx - SCALEX(14), SCALEY(14), 2);
  sprite.setTextColor(TFT_WHITE, purple);
  sprite.drawString("CURR: " + String(average), SCALEX(10), SCALEY(92), 2);

  sprite.drawString("MIN: " + String(Min), SCALEX(10), SCALEY(108), 2);

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
    sprintf(pinString, "%d", pinArray[displayMode]);

  sprintf(pinString2, "PIN %s", pinString);
  sprite.drawString(pinString2, gx + 10, 30);

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
    sprite.fillRect(x, y, SCALEX(10), SCALEX(10), values[j][23] < PIN_THRESHOLD ? colours[j] : TFT_BLACK);

    for (int i = 0; i < 23; i++)
    {
      if (displayMode == 0 | displayMode == j)
        drawPoint(i, map(values[j][i], 0, Max, 0, gh), map(values[j][i + 1], 0, Max, 0, gh), colours[j]);
    }
  }

  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  // sprite.drawString("BAT:" + String(analogRead(4)), gx + 160, 16);
  // sprite.drawString("MOD:" + String(Mode), gx + 160, 26);

  // copySprite();
  sprite.pushSprite(0, 0);
  // sprite.pushSprite(0, 0);
  char *s = new char[256];
  sprintf(s, "p%d:d%d:v%d, p%d:d%d:v%d, p%d:d%d:v%d, p%d:d%d:v%d, p%d:d%d:v%d, p%d:d%d:v%d, p%d:d%d:v%d, p%d:d%d:v%d, p%d:d%d:v%d", 
  pinArray[1], digitalOrAnalog[1], values[1][23] , 
  pinArray[2], digitalOrAnalog[2], values[2][23] , 
  pinArray[3], digitalOrAnalog[3], values[3][23] , 
  pinArray[4], digitalOrAnalog[4], values[4][23] , 
  pinArray[5], digitalOrAnalog[5], values[5][23] , 
  pinArray[6], digitalOrAnalog[6], values[6][23] , 
  pinArray[7], digitalOrAnalog[7], values[7][23] , 
  pinArray[8], digitalOrAnalog[8], values[8][23] , 
  pinArray[9], digitalOrAnalog[9], values[9][23]);
  Serial.println(s);
  delete (s);

  // delay(5);
}
