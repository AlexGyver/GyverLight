/*
  Управление
  - 1х тап - вкл/выкл
  - 2х тап - переключ режима
  - удержание - яркость
  - удержание выключенного - отключение всего
*/

#define CURRENT_LIMIT 2000    // лимит по току в миллиамперах, автоматически управляет яркостью, 0 - выключить лимит

#define CHIPSET WS2812        // тип светодиода
#define COLOR_ORDER GRB       // порядок цветов пикселя
#define NUM_LEDS 35           // количество светодиодов в одном отрезке ленты
#define NUM_STRIPS 4          // количество отрезков ленты (параллельное соединение)
#define RGB_PIN 6             // пин ленты

#define BTN_PIN 2             // пин кнопки
#define MIN_BRIGHTNESS 10     // минимальная яркость
#define MAX_BRIGHTNESS 200    // максимальная яркость
#define BRIGHTNESS 100        // начальная яркость

#define MODES_AMOUNT 8        // количество режимов

#include <FastLED.h>
CRGB leds[NUM_LEDS];

#include "GyverButton.h"
GButton btn(BTN_PIN, HIGH_PULL, NORM_OPEN);

#include "GyverTimer.h"
GTimer_ms loopEffect(60);
GTimer_ms loopBrightnes(400);
GTimer_ms loopPower(60);

bool powerActive = false;             // состояние питания
bool powerDisAll = false;             // полное отключение
byte nowMode = 0;                     // текущий режим

bool powerBlinkInit = true;           // инициализация выключенной кнопки
bool powerBlinkDirection = true;      // направление моргания выключенной кнопки
int powerBlinkBrightness = 0;         // уровень яркости выключенной кнопки

bool lampOk = false;                  // период анимации эффекта лампы
bool lightDirection = true;           // направление смены яркости
int lightBrightness = BRIGHTNESS;     // уровень яркости

bool loadingFlag = true;              // инициализация эффекта

void setup() {
  Serial.begin(9600);

  randomSeed(analogRead(0));   // инициализация генератора райндома

  FastLED.addLeds<CHIPSET, RGB_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT / NUM_STRIPS);
  FastLED.setBrightness(0);
  FastLED.show();

  loopEffect.stop();
  loopBrightnes.stop();

  btn.setDebounce(80);
  btn.setTimeout(300);
  btn.setClickTimeout(400);
  btn.setStepTimeout(300);
}

void loop() {

  btn.tick(); // кнопка

  if (btn.isSingle()) singleClick();
  else if (btn.isDouble()) doubleClick();
  else if (btn.isHold()) holdedPress();


  if (!powerActive) {

    if (powerBlinkInit) {
      powerBlinkInit = false;
      FastLED.clear();
      leds[NUM_LEDS - 1] = CRGB::White;
    }
    if (loopPower.isReady()) updatePowerBlink();

  } else {

    if (nowMode > 0) {
      if (loopEffect.isReady()) updateEffect();
    } else {
      if (!lampOk && loopEffect.isReady()) updateEffect();
    }

  }
}

void updateEffect() {
  switch (nowMode) {
    case 0:
      lampOn();
      break;
    case 1:
      lighter();
      break;
    case 2:
      lightBugs();
      break;
    case 3:
      sparkles();
      break;      
    case 4:
      colors();
      break;
    case 5:
      rainbow();
      break;
    case 6:
      fire(0);
      break;
    case 7:
      fire(1);
      break;
    case 8:
      fire(2);
      break;
  }
  FastLED.show();
}

void updatePowerBlink() {
  if (powerBlinkDirection) {
    powerBlinkDirection = !(++powerBlinkBrightness >= 30);
  } else {
    powerBlinkDirection = (--powerBlinkBrightness <= 0);
  }
  powerBlinkBrightness = constrain(powerBlinkBrightness, 0, 30);
  FastLED.setBrightness(powerBlinkBrightness);
  FastLED.show();
}

void updateBrightness(byte step) {
  if (lightDirection) {
    lightBrightness += step;
    lightDirection = !(lightBrightness >= MAX_BRIGHTNESS);
  } else {
    lightBrightness -= step;
    lightDirection = (lightBrightness <= MIN_BRIGHTNESS);
  }
  lightBrightness = constrain(lightBrightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  FastLED.setBrightness(lightBrightness);
  FastLED.show();
}

void setMode() {
  loadingFlag = true;
  lampOk = false;
  if (++nowMode > MODES_AMOUNT) nowMode = 0;
  FastLED.clear();
}

void singleClick() {
  if (!powerActive) {

    nowMode = 0;
    powerBlinkInit = true;
    powerBlinkBrightness = 0;
    powerBlinkDirection = true;
    powerDisAll = false;
    powerActive = true;

    loopEffect.start();
    loopBrightnes.start();
    loopPower.stop();

    FastLED.setBrightness(lightBrightness);
    FastLED.show();

  } else {

    powerActive = false;
    loadingFlag = true;
    lampOk = false;

    loopEffect.stop();
    loopBrightnes.stop();
    loopPower.start();

    FastLED.setBrightness(0);
    FastLED.show();

  }
}

void doubleClick() {
  if (powerActive) setMode();
}

void holdedPress() {
  if (powerActive) {
    if (loopBrightnes.isReady()) updateBrightness(10);
  } else {
    if (!powerDisAll && powerBlinkBrightness < 1) {
      powerDisAll = true;

      loopPower.stop();

      FastLED.setBrightness(0);
      FastLED.show();
    }
  }
}
