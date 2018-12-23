#define TRACK_STEP 40

// ****************************** ОГОНЁК ******************************
int8_t position;
boolean direction;

void lighter() {
  FastLED.clear();
  if (direction) {
    position++;
    if (position > NUM_LEDS - 2) {
      direction = false;
    }
  } else {
    position--;
    if (position < 1) {
      direction = true;
    }
  }

  leds[position] = CRGB::White;
}

// ****************************** СВЕТЛЯЧКИ ******************************
#define MAX_SPEED 15
#define BUGS_AMOUNT 5
int8_t speed[BUGS_AMOUNT];
int8_t pos[BUGS_AMOUNT];
CRGB bugColors[BUGS_AMOUNT];

void lightBugs() {
  if (loadingFlag) {
    loadingFlag = false;
    for (byte i = 0; i < BUGS_AMOUNT; i++) {
      bugColors[i] = CHSV(random(0, 9) * 28, 255, 255);
      pos[i] = random(0, NUM_LEDS);
      speed[i] += random(-5, 6);
    }
  }
  
 
  
  for (byte i = 0; i < BUGS_AMOUNT; i++) {
    if(random(5) == 1)speed[i] += random(-10, 10);
    if (speed[i] == 0) speed[i] += random(-1,1)*11;
    if(random(200)==3) bugColors[i] = CHSV(random(0, 9) * 28, 255, 255);
    if (abs(speed[i]) > MAX_SPEED) speed[i] = random(-5, 6);
    pos[i] += speed[i] / 10;
    if (pos[i] < 0) {
      pos[i] = 0;
      speed[i] = -speed[i]/1.5;
    }
    if (pos[i] > NUM_LEDS - 1) {
      pos[i] = NUM_LEDS - 1;
      speed[i] = -speed[i]/1.5;
    }

    leds[pos[i]].r = bugColors[i].r;
    leds[pos[i]].g = bugColors[i].g;
    leds[pos[i]].b = bugColors[i].b;
  }

  for(byte i = 0; i < NUM_LEDS; i++){
     if(i != pos[i]) leds[i] = CRGB(constrain(leds[i].r-40,0,255),constrain(leds[i].g-40,0,255),constrain(leds[i].b-40,0,255) );
  }
}

// ****************************** ЦВЕТА ******************************
byte hue;
void colors() {
  hue += 1;
  CRGB thisColor = CHSV(hue, 255, 255);
  fillAll(CHSV(hue, 255, 255));
}

// ****************************** РАДУГА ******************************
void rainbow() {
  hue += 2;
  for (byte i = 0; i < NUM_LEDS; i++)
    leds[i] = CHSV((byte)(hue + i * float(255 / NUM_LEDS)), 255, 255);
}

// ****************************** КОНФЕТТИ ******************************
void sparkles() {
  byte thisNum = random(0, NUM_LEDS);
  if (getPixColor(thisNum) == 0)
    leds[thisNum] = CHSV(random(0, 255), 255, 255);
  for(int i = 0; i < NUM_LEDS; i++){
     CRGB c = (leds[i]);
     leds[i] = CRGB(constrain(c.r-random(3,10),0,255),constrain(c.g-random(3,10),0,255),constrain(c.b-random(3,10),0,255) );
  }
  //fade();
}

// ****************************** ОГОНЬ ******************************
#define COOLING  55
// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 110

void fire() {
  random16_add_entropy( random());
  Fire2012WithPalette(); // run simulation frame, using palette colors
}

void Fire2012WithPalette()
{
  // Массив значений температуры для каждого диода
  static byte heat[NUM_LEDS];

  // Шаг 1.  Охладим немного каждый светодиод
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Шаг 2.  Тепло с каждого диода поднимается вверх и немного уменьшается
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Шаг 3.  Случайным образом закидываем в огонь горячие точки
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Шаг 4.  Трансформируем тепло в цвет для диодов
  for ( int j = 0; j < NUM_LEDS; j++) {
    // масшатируем тепло 0-255 в 0-240
    // для более качественного результата
    byte colorindex = scale8( heat[j], 240);
    CRGB color = ColorFromPalette( gPal, colorindex);
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

// ****************** СЛУЖЕБНЫЕ ФУНКЦИИ *******************
void fade() {
  for (byte i = 0; i < NUM_LEDS; i++) {
    // измеряяем цвет текущего пикселя
    uint32_t thisColor = getPixColor(i);

    // если 0, то пропускаем действия и переходим к следующему
    if (thisColor == 0) continue;

    // разбиваем цвет на составляющие RGB
    byte rgb[3];
    rgb[0] = (thisColor >> 16) & 0xff;
    rgb[1] = (thisColor >> 8) & 0xff;
    rgb[2] = thisColor & 0xff;

    // ищем максимум
    byte maximum = max(max(rgb[0], rgb[1]), rgb[2]);
    float coef = 0;

    // если есть возможность уменьшить
    if (maximum >= TRACK_STEP)
      // уменьшаем и находим коэффициент уменьшения
      coef = (float)(maximum - TRACK_STEP) / maximum;

    // далее все цвета умножаем на этот коэффициент
    for (byte i = 0; i < 3; i++) {
      if (rgb[i] > 0) rgb[i] = (float)rgb[i] * coef;
      else rgb[i] = 0;
    }
    leds[i] = CRGB(rgb[0], rgb[1], rgb[2]);
  }
}

// залить все
void fillAll(CRGB newcolor) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = newcolor;
  }
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(int thisPixel) {
  return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8 ) | (long)leds[thisPixel].b);
}
