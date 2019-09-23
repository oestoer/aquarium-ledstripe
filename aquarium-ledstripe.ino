//#include <Adafruit_NeoPixel.h>
#include <NeoPixelBrightnessBus.h>

#define PIN 11
#define PIXELCOUNT 12
#define DEBUG 0

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PIXELCOUNT, PIN);

#define colorSaturation 255
const RgbColor red(colorSaturation, 0, 0);
const RgbColor green(0, colorSaturation, 0);
const RgbColor blue(0, 0, colorSaturation);
const RgbColor yellow(colorSaturation, colorSaturation, 0);
const RgbColor white(200, 200, 200);
RgbColor darkYellow;
RgbColor warmWhite;

const uint8_t c_MinBrightness = 0;
const uint8_t c_MaxBrightness = 255;

uint8_t brightness;
int8_t direction; // current direction of dimming

uint8_t currentBlendStep = 0;
float maxBlendSteps = 255.0f;
bool stepResetMorning = false;
bool stepResetAfternoon = false;

uint32_t timer;
const uint16_t tick = 1000; // 100 = 0.1 sec
const uint16_t freq = 100000 / tick; // 100 000 / tick
const uint32_t sunriseLength = freq * 36; // 1 hour sunrise with 1000ms ticks
const uint32_t daylightLength = freq * 432; // 12 hours with 1000ms ticks
const uint32_t sunsetLength = freq * 25; // 1 hour sunrise with 1000ms ticks
const uint32_t dayMax = freq * 864; // 24 hours with 1000ms ticks


void setup() {
    delay(3000); // just in case
  
  if (DEBUG) {
    Serial.begin(115200);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initializing...");
    Serial.print("Sunrise end: ");
    Serial.println(sunriseLength);
    Serial.flush();
  }
  darkYellow = RgbColor::LinearBlend(red, yellow, 0.5f);
  warmWhite = RgbColor::LinearBlend(darkYellow, white, 0.3f);

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();

  delay(500);
  direction = 1; // -1 to dim first

  initColours();
  strip.SetBrightness(c_MinBrightness);
  strip.Show();

  timer = 0;
}

void loop() {
  timer++;
  brightness = strip.GetBrightness();

  if (DEBUG) {
    Serial.print("brightness: ");
    Serial.print(brightness);
    Serial.print(", timer: ");
    Serial.println(timer);
  }

  if (isMorning()) {
    sunrise();
  } else if (isAfternoon()) {
    sunshine();
  } else if (isEvening()) {
    sunset();
  } else if (isNight()) {
    dark();
  } else {
    // reset day
    timer = 0;
  }

  // show the results
  strip.Show();
  delay(tick);
}

void initColours() {
  for (uint8_t i = 0; i < PIXELCOUNT; i++) {
    if (i % 3 == 0) {
      strip.SetPixelColor(i, red);
    }
    if (i % 3 == 1) {
      strip.SetPixelColor(i, red);
    }
    if (i % 3 == 2) {
      strip.SetPixelColor(i, red);
    }
  }
}

bool isMorning() {
  return timer < sunriseLength;
}

bool isAfternoon() {
  return timer >= sunriseLength && timer < daylightLength;
}

bool isEvening() {
  return timer >= daylightLength && timer < daylightLength + sunsetLength;
}

bool isNight() {
  return timer >= daylightLength + sunsetLength && timer < dayMax;
}

void sunrise() {
  if (!stepResetMorning) {
    stepResetMorning = true;
    currentBlendStep = 0;
  }
  // swap diection of dim when limits are reached
  if (direction < 0 && brightness <= c_MinBrightness)
  {
    direction = 1;
  }

  if (brightness < c_MaxBrightness) {
    brightness += direction;
    strip.SetBrightness(brightness);
    if (brightness == 1) {
      // workaround for 0 birghtness that erases colors
      initColours();
    }
  }
  for (uint8_t i = 0; i < PIXELCOUNT; i++) {
    if (i % 3 == 0) {
      strip.SetPixelColor(i, blendColors(red, darkYellow));
    }
    if (i % 3 == 2) {
      strip.SetPixelColor(i, blendColors(red, darkYellow));
    }
  }
  if (currentBlendStep < maxBlendSteps) {
    currentBlendStep++;
  }
}

void sunshine() {
  if (!stepResetAfternoon) {
    stepResetAfternoon = true;
    currentBlendStep = 0;
    maxBlendSteps = 255.0f;
  }
  for (uint8_t i = 0; i < PIXELCOUNT; i++) {
    if (i % 3 == 0) {
      strip.SetPixelColor(i, blendColors(darkYellow, white));

    }
    if (i % 3 == 1) {
      strip.SetPixelColor(i, blendColors(red, darkYellow));

    }
    if (i % 3 == 2) {
      strip.SetPixelColor(i, blendColors(darkYellow, warmWhite));
    }
  }

  if (currentBlendStep < maxBlendSteps) {
    currentBlendStep++;
  }
}

void sunset() {
  if (direction > 0 && brightness >= c_MaxBrightness)
  {
    direction = -1;
  }

  if (brightness > c_MinBrightness) {
    brightness += direction;
    strip.SetBrightness(brightness);
  }
}

void dark() {
  strip.SetBrightness(c_MinBrightness);
}

RgbColor blendColors(RgbColor from, RgbColor to) {
  if (currentBlendStep >= maxBlendSteps) {
    return to;
  }
  if (DEBUG) {
    Serial.println(currentBlendStep / maxBlendSteps);
  }

  return RgbColor::LinearBlend(from, to, (currentBlendStep / maxBlendSteps));
}
