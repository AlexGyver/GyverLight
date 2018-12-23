/*
  Скетч к проекту "Эффектный светильник"
  Страница проекта (схемы, описания): https://alexgyver.ru/gyverlight/
  Исходники на GitHub: https://github.com/AlexGyver/gyverlight/
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2018
  https://AlexGyver.ru/
*/

/*
   Управление кнопкой/сенсором
  - Удержание - яркость
  - 1х тап - вкл/выкл
  - 2х тап - переключ режима
  - 3х тап - вкл/выкл белый свет
  - 4х тап - старт/стоп авто смены режимов
*/

// ************************** НАСТРОЙКИ ***********************
#define CURRENT_LIMIT 2000  // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
#define AUTOPLAY_TIME 120    // время между сменой режимов в секундах

#define NUM_LEDS 12         // количсетво светодиодов в одном отрезке ленты
#define NUM_STRIPS 8        // количество отрезков ленты (в параллели)
#define LED_PIN 6           // пин ленты
#define BTN_PIN 3           // пин кнопки/сенсора
#define MIN_BRIGHTNESS 20  // минимальная яркость при ручной настройке
#define BRIGHTNESS 250      // начальная яркость
#define FIRE_PALETTE 0      // разные типы огня (0 - 3). Попробуй их все! =)

// ************************** ДЛЯ РАЗРАБОТЧИКОВ ***********************
#define MODES_AMOUNT 6

#include "GyverButton.h"
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);

#include <FastLED.h>
CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;

#include "GyverTimer.h"
GTimer_ms effectTimer(60);
GTimer_ms autoplayTimer((long)AUTOPLAY_TIME * 1000);
GTimer_ms brightTimer(20);

int brightness = BRIGHTNESS;
int tempBrightness;
byte thisMode;

bool gReverseDirection = false;
boolean loadingFlag = true;
boolean autoplay = false;
boolean powerDirection = true;
boolean powerActive = false;
boolean powerState = true;
boolean whiteMode = false;
boolean brightDirection = false;
boolean wasStep = false;
boolean fade2br = false;


void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT / NUM_STRIPS);
  FastLED.setBrightness(brightness);
  FastLED.show();

  randomSeed(analogRead(0));
  touch.setTimeout(300);
  touch.setStepTimeout(50);

  if (FIRE_PALETTE == 0) gPal = HeatColors_p;
  else if (FIRE_PALETTE == 1) gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  else if (FIRE_PALETTE == 2) gPal = CRGBPalette16( CRGB::Black, CRGB::Blue, CRGB::Aqua,  CRGB::White);
  else if (FIRE_PALETTE == 3) gPal = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);
  FastLED.clear();
}

void loop() {
  touch.tick();
  if (touch.hasClicks()) {
    byte clicks = touch.getClicks();
    switch (clicks) {
      case 1:
        powerDirection = !powerDirection;
        powerActive = true;
        tempBrightness = brightness * !powerDirection;
        //if(whiteMode)whiteMode = false;
        break;
      case 2: if(powerDirection && !whiteMode)nextMode();
        break;
      case 3: if(powerDirection ){whiteMode = !whiteMode;
          if (whiteMode) {
            effectTimer.stop();
            fillAll(CRGB::White);
            FastLED.show();
          } else {
            effectTimer.start();
          }
        }
        break;
      case 4: if(powerDirection)autoplay = !autoplay;
        break;
      case 5:
        break;
    }
  }

  if (touch.isStep()) {
    wasStep = true;
    if (brightDirection) {
      brightness += 5;
    } else {
      brightness -= 5;
    }
    brightness = constrain(brightness, MIN_BRIGHTNESS, 255);
    FastLED.setBrightness(brightness);
    FastLED.show();

    if((brightness == MIN_BRIGHTNESS || brightness == 255) && !fade2br){
        for(int n = 0; n < 10; n++){
          fillAll(CRGB::White);
          FastLED.show();
          delay(30);
          fillAll(CRGB::Black);
          FastLED.show();
          delay(30);        
        }
      switch (thisMode) {
      case 0: lighter();
        break;
      case 1: lightBugs();
        break;
      case 2: colors();
        break;
      case 3: rainbow();
        break;
      case 4: sparkles();
        break;
      case 5: fire();
        break;
      }
      FastLED.show();
      fade2br=true;
    }
  }
  if (touch.isRelease()) {
    fade2br = false;
    if (wasStep) {
      wasStep = false;
      brightDirection = !brightDirection;
    }
  }

  if (effectTimer.isReady() && powerState) {
    switch (thisMode) {
      case 0: lighter();
        break;
      case 1: lightBugs();
        break;
      case 2: colors();
        break;
      case 3: rainbow();
        break;
      case 4: sparkles();
        break;
      case 5: fire();
        break;
    }
    FastLED.show();
  }

  if (autoplayTimer.isReady() && autoplay) {    // таймер смены режима
    nextMode();
  }

  brightnessTick();
}

void nextMode() {
  thisMode++;
  if (thisMode >= MODES_AMOUNT) thisMode = 0;
  loadingFlag = true;
  FastLED.clear();
}

void brightnessTick() {
  if (powerActive) {
    if (brightTimer.isReady()) {
      if (powerDirection) {
        powerState = true;
        tempBrightness += 10;
        if (tempBrightness > brightness) {
          tempBrightness = brightness;
          powerActive = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      } else {
        tempBrightness -= 10;
        if (tempBrightness < 0) {
          tempBrightness = 0;
          powerActive = false;
          powerState = false;
        }
        FastLED.setBrightness(tempBrightness);
        FastLED.show();
      }
    }
  }
}
