#define TRACK_STEP 40

/* ЛАМПА */

int posDrop;
int upperBrightness; 
void lampOn() {
  if (loadingFlag) {
    loadingFlag = false;
    posDrop = NUM_LEDS - 1;
    upperBrightness = 0;
  }
  FastLED.clear();
  posDrop -= 3;
  if (posDrop > 0) {
    leds[posDrop + 2] = CRGB::White;
    leds[posDrop + 2] %= 30;
    leds[posDrop + 1] = CRGB::White;
    leds[posDrop + 1] %= 50;
    leds[posDrop] = CRGB::White;
    leds[posDrop] %= 100;
  } else {
    if (lightBrightness > upperBrightness) {
      FastLED.setBrightness(upperBrightness);
    } else {
      FastLED.setBrightness(lightBrightness);
      lampOk = true;
    }
    upperBrightness += 10;
    fillAll(CRGB::White);
  }
}
void fillAll(CRGB newcolor) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = newcolor;
  }
}

/* ОГОНЁК */

int8_t position;
boolean direction;
void lighter() {
  if (loadingFlag) {
    loadingFlag = false;
    direction = true;
    position = 0;
  }
  FastLED.clear();
  if (direction) {
    if (++position > NUM_LEDS - 2) direction = false;
  } else {
    if (--position < 1) direction = true;
  }
  leds[position] = CRGB::White;
}

/* СВЕТЛЯЧКИ */

#define MAX_SPEED 30
#define BUGS_AMOUNT 3
int8_t speed[BUGS_AMOUNT];
int8_t pos[BUGS_AMOUNT];
CRGB bugColors[BUGS_AMOUNT];
void lightBugs() {
  if (loadingFlag) {
    loadingFlag = false;
    for (byte i = 0; i < BUGS_AMOUNT; i++) {
      pos[i] = random(0, NUM_LEDS);
      speed[i] += random(-5, 6);
      // bugColors[i] = CHSV(random(0, 9) * 28, 255, 255);
    }
    bugColors[0] = CRGB::Red;
    bugColors[1] = CRGB::Green;
    bugColors[2] = CRGB::Blue;
  }
  FastLED.clear();
  for (byte i = 0; i < BUGS_AMOUNT; i++) {
    speed[i] += random(-5, 6);
    if (speed[i] == 0) {
      speed[i] += (-5, 6);
    }

    if (abs(speed[i]) > MAX_SPEED) {
      speed[i] = 0;
    }
    pos[i] += speed[i] / 10;
    if (pos[i] < 0) {
      pos[i] = 0;
      speed[i] = -speed[i];
    }
    if (pos[i] > NUM_LEDS - 1) {
      pos[i] = NUM_LEDS - 1;
      speed[i] = -speed[i];
    }
    leds[pos[i]] = bugColors[i];
  }
}

/* ЦВЕТА */

byte hue;
void colors() {
  hue += 2;
  CRGB thisColor = CHSV(hue, 255, 255);
  fillAll(CHSV(hue, 255, 255));
}

/* РАДУГА */

void rainbow() {
  hue += 2;
  for (byte i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((byte)(hue + i * float(255 / NUM_LEDS)), 255, 255);
  }
}

/* КОНФЕТТИ */

void sparkles() {
  byte thisNum = random(0, NUM_LEDS);
  if (getPixColor(thisNum) == 0) leds[thisNum] = CHSV(random(0, 255), 255, 255);
  fade();
}
uint32_t getPixColor(int thisPixel) {
  return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8 ) | (long)leds[thisPixel].b);
}
void fade() {
  for (byte i = 0; i < NUM_LEDS; i++) {
    uint32_t thisColor = getPixColor(i);
    if (thisColor == 0) continue;
    byte rgb[3];
    rgb[0] = (thisColor >> 16) & 0xff;
    rgb[1] = (thisColor >> 8) & 0xff;
    rgb[2] = thisColor & 0xff;
    byte maximum = max(max(rgb[0], rgb[1]), rgb[2]);
    float coef = 0;
    if (maximum >= TRACK_STEP) coef = (float)(maximum - TRACK_STEP) / maximum;
    for (byte i = 0; i < 3; i++) {
      rgb[i] = (rgb[i] > 0) ? ((float)rgb[i] * coef) : 0;
    }
    leds[i] = CRGB(rgb[0], rgb[1], rgb[2]);
  }
}

/* ОГОНЬ */

#define COOLING  55
#define SPARKING 120
bool gReverseDirection = false;
CRGBPalette16 p;
void fire(byte pn) {
  if (loadingFlag) {
    loadingFlag = false;
    switch (pn) {
      case 1:
        p = CRGBPalette16(CRGB::Black, CRGB::Green, CRGB::Yellow, CRGB::White);        
        break;
      case 2:
        p = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
        break;
      default:
        p = CRGBPalette16(CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
        break;
    }
  }
  random16_add_entropy(random());
  static byte heat[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8(heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }
  for (int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  if (random8() < SPARKING) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  for (int j = 0; j < NUM_LEDS; j++) {
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette(p, colorindex);
    int pixelnumber;
    pixelnumber = gReverseDirection ? ((NUM_LEDS - 1) - j) : j;
    leds[pixelnumber] = color;
  }
}
