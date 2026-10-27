/*
  Mini Koro-sensei: a tiny motion-controlled digital pet

  Open in Arduino IDE and upload to a Seeed XIAO ESP32-C3.
  Libraries: Adafruit GFX, Adafruit SSD1306, Adafruit MPU6050, DHT sensor library.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <DHT.h>
#include <Preferences.h>
#include <math.h>

// =========================== SETTINGS ===========================

const int I2C_SDA_PIN = 6;
const int I2C_SCL_PIN = 7;
const int DHT_PIN = 3;
const int BUTTON_ONE_PIN = 4;
const int BUTTON_TWO_PIN = 5;

const bool USE_DHT11 = true;

const uint8_t OLED_ADDRESS = 0x3C;
const uint8_t MPU6050_ADDRESS = 0x68;

const int STARTING_MOOD = 70;
const int STARTING_STAMINA = 75;
const int STARTING_SWEETS = 65;

const bool RESET_SAVED_PET_ON_BOOT = false;

enum PetReaction {
  TEACH_REACTION,
  TICKLE_REACTION,
  SNACK_REACTION,
  PRAISE_REACTION,
};

struct MenuItem {
  const char *label;
  int moodChange;
  int staminaChange;
  int sweetsChange;
  PetReaction reaction;
};

const MenuItem MENU_ITEMS[] = {
  {"TEACH",  4, -6, -2, TEACH_REACTION},
  {"TICKLE", 10, -9, -1, TICKLE_REACTION},
  {"SNACK",  2,  4, 18, SNACK_REACTION},
  {"PRAISE", 8,  0,  0, PRAISE_REACTION},
};
const int MENU_ITEM_COUNT = sizeof(MENU_ITEMS) / sizeof(MENU_ITEMS[0]);

const float MENU_TILT_LIMIT = 6.0f;
const bool SWAP_MPU_AXES = false;
const float MENU_X_DIRECTION = 1.0f;
const float MENU_Y_DIRECTION = -1.0f;
const float MENU_CENTER_DEADZONE = 0.8f;
const float SHAKE_THRESHOLD = 7.0f;

const int SHAKE_MOOD_CHANGE = -6;
const int SHAKE_STAMINA_CHANGE = 3;
const int SHAKE_SWEETS_CHANGE = 0;

const int PET_SPRITE_WIDTH = 32;
const int PET_SPRITE_HEIGHT = 32;

const uint16_t PET_WALK_PIXEL_MS = 70;
const uint16_t PET_PRE_JUMP_MS = 230;
const uint16_t PET_JUMP_MS = 430;
const int PET_JUMP_HEIGHT = 16;
const uint32_t TEACH_DURATION_MS = 48000;
const uint16_t PRAISE_DURATION_MS = 1600;
const uint16_t TICKLE_LAP_MS = 800;
const uint8_t TICKLE_LAP_COUNT = 2;

const uint8_t PROGMEM PET_SPRITE[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x3f, 0xfc, 0x00,
  0x01, 0xff, 0xff, 0x80,
  0x07, 0xff, 0xff, 0xe0,
  0x0f, 0xff, 0xff, 0xf0,
  0x1f, 0xff, 0xff, 0xf8,
  0x1f, 0xff, 0xff, 0xf8,
  0x1f, 0xff, 0xff, 0xf8,
  0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0x00, 0x00, 0xfc,
  0x3f, 0x00, 0x00, 0xfc,
  0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0x00, 0x00, 0xfc,
  0x3f, 0x00, 0x00, 0xfc,
  0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0x00, 0x00, 0xfc,
  0x3f, 0x00, 0x00, 0xfc,
  0x3f, 0xff, 0xff, 0xfc,
  0x3f, 0xff, 0xff, 0xfc,
  0x1f, 0x80, 0x01, 0xf8,
  0x1f, 0x80, 0x01, 0xf8,
  0x0f, 0xff, 0xff, 0xf0,
  0x07, 0xff, 0xff, 0xe0,
  0x03, 0xff, 0xff, 0xc0,
  0x01, 0xff, 0xff, 0x80,
  0x00, 0xff, 0xff, 0x00,
  0x00, 0x7f, 0xfe, 0x00,
  0x01, 0x80, 0x01, 0x80,
  0x06, 0x00, 0x00, 0x60,
  0x18, 0x00, 0x00, 0x18,
};
// ===============================================================

const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int MENU_CENTER_X = 64;
const int MENU_CENTER_Y = 32;
const int MENU_ITEM_X[] = {64, 108, 64, 20};
const int MENU_ITEM_Y[] = {12, 32, 53, 32};
const int MENU_BALL_X_RANGE = 37;
const int MENU_BALL_Y_RANGE = 22;
const uint16_t BUTTON_DEBOUNCE_MS = 30;
const uint16_t MPU_READ_INTERVAL_MS = 30;
const uint16_t DHT_READ_INTERVAL_MS = 2200;
const uint16_t SHAKE_COOLDOWN_MS = 650;
const float STANDARD_GRAVITY = 9.80665f;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_MPU6050 mpu;
DHT dht(DHT_PIN, DHT11);
Preferences preferences;

struct PetState {
  int mood;
  int stamina;
  int sweets;
};

struct ButtonState {
  int pin;
  bool stableState;
  bool lastRawState;
  uint32_t lastChangedAt;
};

enum View {
  PET_VIEW,
  MENU_VIEW,
  STATS_VIEW,
};

PetState pet = {STARTING_MOOD, STARTING_STAMINA, STARTING_SWEETS};
ButtonState buttonOne = {BUTTON_ONE_PIN, HIGH, HIGH, 0};
ButtonState buttonTwo = {BUTTON_TWO_PIN, HIGH, HIGH, 0};
View currentView = PET_VIEW;

bool mpuFound = false;
bool dhtFound = false;
float accelerationX = 0.0f;
float accelerationY = 0.0f;
float accelerationZ = STANDARD_GRAVITY;
float temperatureC = NAN;
float humidity = NAN;
float menuBallX = MENU_CENTER_X;
float menuBallY = MENU_CENTER_Y;
float menuCenterAccelerationX = 0.0f;
float menuCenterAccelerationY = 0.0f;
int selectedMenuItem = -1;
uint32_t lastMpuReadAt = 0;
uint32_t lastDhtReadAt = 0;
uint32_t lastShakeAt = 0;
uint32_t shakeAnimationEndsAt = 0;
uint32_t petJumpStartedAt = 0;
uint32_t teachingUntil = 0;
uint32_t praiseAnimationEndsAt = 0;
uint32_t tickleRunStartedAt = 0;
int teachingPetX = 0;

int petWalkingX(uint32_t now);

int keepInRange(int value, int smallest, int largest) {
  return constrain(value, smallest, largest);
}

void changePet(int moodChange, int staminaChange, int sweetsChange) {
  pet.mood = keepInRange(pet.mood + moodChange, 0, 100);
  pet.stamina = keepInRange(pet.stamina + staminaChange, 0, 100);
  pet.sweets = keepInRange(pet.sweets + sweetsChange, 0, 100);
}

void loadPet() {
  preferences.begin("koro", false);
  if (RESET_SAVED_PET_ON_BOOT) {
    preferences.clear();
  }
  pet.mood = preferences.getInt("mood", STARTING_MOOD);
  pet.stamina = preferences.getInt("stam", STARTING_STAMINA);
  pet.sweets = preferences.getInt("swts", STARTING_SWEETS);
}

void savePet() {
  preferences.putInt("mood", pet.mood);
  preferences.putInt("stam", pet.stamina);
  preferences.putInt("swts", pet.sweets);
}

void setUpButton(ButtonState &button) {
  pinMode(button.pin, INPUT_PULLUP);
  button.stableState = digitalRead(button.pin);
  button.lastRawState = button.stableState;
  button.lastChangedAt = millis();
}

bool wasPressed(ButtonState &button) {
  const bool rawState = digitalRead(button.pin);
  const uint32_t now = millis();

  if (rawState != button.lastRawState) {
    button.lastRawState = rawState;
    button.lastChangedAt = now;
  }

  if (rawState != button.stableState &&
      now - button.lastChangedAt >= BUTTON_DEBOUNCE_MS) {
    button.stableState = rawState;
    return button.stableState == LOW;
  }

  return false;
}

void updateMpu() {
  if (!mpuFound || millis() - lastMpuReadAt < MPU_READ_INTERVAL_MS) {
    return;
  }
  lastMpuReadAt = millis();

  sensors_event_t acceleration;
  sensors_event_t gyro;
  sensors_event_t sensorTemperature;
  mpu.getEvent(&acceleration, &gyro, &sensorTemperature);
  accelerationX = acceleration.acceleration.x;
  accelerationY = acceleration.acceleration.y;
  accelerationZ = acceleration.acceleration.z;
}

void updateDht() {
  if (!dhtFound || millis() - lastDhtReadAt < DHT_READ_INTERVAL_MS) {
    return;
  }
  lastDhtReadAt = millis();

  const float newHumidity = dht.readHumidity();
  const float newTemperatureC = dht.readTemperature();
  if (!isnan(newHumidity)) {
    humidity = newHumidity;
  }
  if (!isnan(newTemperatureC)) {
    temperatureC = newTemperatureC;
  }
}

int petWalkingX(uint32_t now) {
  const int farthestX = SCREEN_WIDTH - PET_SPRITE_WIDTH;
  const uint32_t roundTrip = static_cast<uint32_t>(farthestX) * 2;
  const uint32_t step = (now / PET_WALK_PIXEL_MS) % roundTrip;
  return step <= farthestX ? step : roundTrip - step;
}

uint32_t tickleRunDuration() {
  return static_cast<uint32_t>(TICKLE_LAP_MS) * TICKLE_LAP_COUNT;
}

bool isTeaching() {
  return teachingUntil != 0 && millis() < teachingUntil;
}

bool isTickleRunning() {
  return tickleRunStartedAt != 0 &&
         millis() - tickleRunStartedAt < tickleRunDuration();
}

int tickleRunX(uint32_t now) {
  const int farthestX = SCREEN_WIDTH - PET_SPRITE_WIDTH;
  const uint32_t lapAge = (now - tickleRunStartedAt) % TICKLE_LAP_MS;
  const float progress = static_cast<float>(lapAge) / TICKLE_LAP_MS;
  return progress < 0.5f ? static_cast<int>(progress * 2.0f * farthestX)
                         : static_cast<int>((1.0f - progress) * 2.0f * farthestX);
}

void updatePetTimers() {
  if (teachingUntil != 0 && !isTeaching()) {
    teachingUntil = 0;
  }
  if (tickleRunStartedAt != 0 && !isTickleRunning()) {
    tickleRunStartedAt = 0;
  }
}

void updateMenuBall() {
  float xTilt = accelerationX - menuCenterAccelerationX;
  float yTilt = accelerationY - menuCenterAccelerationY;

  if (SWAP_MPU_AXES) {
    const float oldXTilt = xTilt;
    xTilt = yTilt;
    yTilt = oldXTilt;
  }
  xTilt = constrain(xTilt * MENU_X_DIRECTION, -MENU_TILT_LIMIT, MENU_TILT_LIMIT);
  yTilt = constrain(yTilt * MENU_Y_DIRECTION, -MENU_TILT_LIMIT, MENU_TILT_LIMIT);
  const float targetX = MENU_CENTER_X + (xTilt / MENU_TILT_LIMIT) * MENU_BALL_X_RANGE;
  const float targetY = MENU_CENTER_Y + (yTilt / MENU_TILT_LIMIT) * MENU_BALL_Y_RANGE;

  menuBallX += (targetX - menuBallX) * 0.20f;
  menuBallY += (targetY - menuBallY) * 0.20f;

  if (fabsf(xTilt) < MENU_CENTER_DEADZONE && fabsf(yTilt) < MENU_CENTER_DEADZONE) {
    selectedMenuItem = -1;
  } else if (fabsf(xTilt) > fabsf(yTilt)) {
    selectedMenuItem = xTilt > 0.0f ? 1 : 3;
  } else {
    selectedMenuItem = yTilt > 0.0f ? 2 : 0;
  }
}

void checkForShake() {
  if (!mpuFound || currentView != PET_VIEW) {
    return;
  }

  const float magnitude = sqrtf(accelerationX * accelerationX +
                                accelerationY * accelerationY +
                                accelerationZ * accelerationZ);
  const uint32_t now = millis();
  if (fabsf(magnitude - STANDARD_GRAVITY) >= SHAKE_THRESHOLD &&
      now - lastShakeAt >= SHAKE_COOLDOWN_MS) {
    lastShakeAt = now;
    teachingUntil = 0;
    shakeAnimationEndsAt = now + 350;
    changePet(SHAKE_MOOD_CHANGE, SHAKE_STAMINA_CHANGE, SHAKE_SWEETS_CHANGE);
    savePet();
  }
}

void openMenu() {
  currentView = MENU_VIEW;
  selectedMenuItem = -1;
  menuBallX = MENU_CENTER_X;
  menuBallY = MENU_CENTER_Y;
  menuCenterAccelerationX = accelerationX;
  menuCenterAccelerationY = accelerationY;
}

void chooseMenuItem() {
  if (selectedMenuItem == -1) {
    return;
  }

  const MenuItem &item = MENU_ITEMS[selectedMenuItem];
  changePet(item.moodChange, item.staminaChange, item.sweetsChange);
  savePet();
  const uint32_t now = millis();
  teachingUntil = 0;
  praiseAnimationEndsAt = 0;
  tickleRunStartedAt = 0;

  if (item.reaction == TEACH_REACTION) {
    petJumpStartedAt = 0;
    teachingPetX = petWalkingX(now);
    teachingUntil = now + TEACH_DURATION_MS;
  } else if (item.reaction == TICKLE_REACTION) {
    petJumpStartedAt = 0;
    tickleRunStartedAt = now;
    praiseAnimationEndsAt = now + tickleRunDuration();
  } else {
    petJumpStartedAt = now;
    if (item.reaction == PRAISE_REACTION) {
      praiseAnimationEndsAt = now + PRAISE_DURATION_MS;
    }
  }
  currentView = PET_VIEW;
}

void handleButtons() {
  if (wasPressed(buttonOne)) {
    if (currentView == MENU_VIEW) {
      chooseMenuItem();
    } else {
      openMenu();
    }
  }

  if (wasPressed(buttonTwo)) {
    currentView = currentView == STATS_VIEW ? PET_VIEW : STATS_VIEW;
  }
}

void drawNyu(int x, int y) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.print("Nyu?");
}

void drawNufufu(int x, int y) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(x, y);
  display.print("Nufu");
}

void drawPraiseText(uint32_t now, int petX, int petY) {
  if (now >= praiseAnimationEndsAt) {
    return;
  }

  const float progress =
      1.0f - static_cast<float>(praiseAnimationEndsAt - now) / PRAISE_DURATION_MS;
  const int rise = static_cast<int>(progress * 18.0f);
  drawNufufu(petX + 6, petY - 6 - rise);
  drawNyu(petX + 22, petY - 10 - rise / 2);
}

void drawSpeedLines(uint32_t now, int petX, int petY) {
  const int offset = static_cast<int>((now / 25) % 6);
  for (int i = 0; i < 3; i++) {
    const int y = petY + 8 + i * 6 + offset - 3;
    display.drawLine(petX - 10, y, petX - 2, y, SSD1306_WHITE);
  }
}

void drawTeachingText(uint32_t now, int petX, int petY) {
  const int wobble = static_cast<int>(sinf(now / 200.0f) * 2.0f);
  display.drawRect(petX + 6, petY - 16 + wobble, 20, 10, SSD1306_WHITE);
  display.drawLine(petX + 8, petY - 13 + wobble, petX + 22, petY - 13 + wobble,
                   SSD1306_WHITE);
  display.drawLine(petX + 8, petY - 10 + wobble, petX + 18, petY - 10 + wobble,
                   SSD1306_WHITE);
}

void drawPet() {
  display.clearDisplay();

  const uint32_t now = millis();
  int petX = isTeaching() ? teachingPetX : petWalkingX(now);
  if (isTickleRunning()) {
    petX = tickleRunX(now);
  }
  int petY = SCREEN_HEIGHT - PET_SPRITE_HEIGHT;

  if (now < shakeAnimationEndsAt) {
    petX += static_cast<int>(sinf(now / 18.0f) * 3.0f);
  }

  if (!isTeaching() && petJumpStartedAt != 0) {
    const uint32_t animationAge = now - petJumpStartedAt;

    if (animationAge < PET_PRE_JUMP_MS) {
      petX += static_cast<int>(sinf(now / 16.0f) * 3.0f);
    } else if (animationAge < PET_PRE_JUMP_MS + PET_JUMP_MS) {
      const float jumpProgress =
          static_cast<float>(animationAge - PET_PRE_JUMP_MS) / PET_JUMP_MS;
      petY -= static_cast<int>(sinf(jumpProgress * PI) * PET_JUMP_HEIGHT);
    } else {
      petJumpStartedAt = 0;
    }
  }

  petX = constrain(petX, 0, SCREEN_WIDTH - PET_SPRITE_WIDTH);
  display.drawBitmap(petX, petY, PET_SPRITE, PET_SPRITE_WIDTH, PET_SPRITE_HEIGHT,
                     SSD1306_WHITE);

  if (isTickleRunning() || isTeaching()) {
    drawSpeedLines(now, petX, petY);
  }
  if (isTeaching()) {
    drawTeachingText(now, petX, petY);
  }
  drawPraiseText(now, petX, petY);
}

void drawMenuItem(int item) {
  const int boxWidth = 37;
  const int boxHeight = 12;
  const int boxX = MENU_ITEM_X[item] - boxWidth / 2;
  const int boxY = MENU_ITEM_Y[item] - boxHeight / 2;
  const bool isSelected = item == selectedMenuItem;

  if (isSelected) {
    display.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
  } else {
    display.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_WHITE);
  }

  display.setTextSize(1);
  const int labelLength = strlen(MENU_ITEMS[item].label);
  display.setCursor(MENU_ITEM_X[item] - labelLength * 3, MENU_ITEM_Y[item] - 3);
  display.print(MENU_ITEMS[item].label);
  display.setTextColor(SSD1306_WHITE);
}

void drawMenu() {
  display.clearDisplay();
  display.drawLine(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, SSD1306_WHITE);
  display.drawLine(0, SCREEN_HEIGHT - 1, SCREEN_WIDTH - 1, 0, SSD1306_WHITE);
  for (int item = 0; item < MENU_ITEM_COUNT; item++) {
    drawMenuItem(item);
  }

  display.drawCircle(MENU_CENTER_X, MENU_CENTER_Y, 11, SSD1306_WHITE);
  display.fillCircle(static_cast<int>(menuBallX), static_cast<int>(menuBallY), 4,
                     SSD1306_WHITE);
}

void drawStatBar(int y, const char *label, int value) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(4, y);
  display.print(label);
  display.drawRect(48, y, 57, 8, SSD1306_WHITE);
  display.fillRect(49, y + 1, map(value, 0, 100, 0, 55), 6, SSD1306_WHITE);
  display.setCursor(109, y);
  display.print(value);
}

void drawStats() {
  display.clearDisplay();
  drawStatBar(5, "MOOD", pet.mood);
  drawStatBar(20, "STAM", pet.stamina);
  drawStatBar(35, "SWTS", pet.sweets);

  display.setCursor(4, 48);
  display.print("TARGET:1");

  display.setCursor(4, 55);
  if (!isnan(temperatureC)) {
    display.print("T");
    display.print(static_cast<int>(temperatureC));
    display.print("C ");
  } else {
    display.print("T-- ");
  }
  if (!isnan(humidity)) {
    display.print("H");
    display.print(static_cast<int>(humidity));
    display.print("%");
  } else {
    display.print("H--");
  }
}

void drawCurrentView() {
  if (currentView == MENU_VIEW) {
    drawMenu();
  } else if (currentView == STATS_VIEW) {
    drawStats();
  } else {
    drawPet();
  }
  display.display();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  setUpButton(buttonOne);
  setUpButton(buttonTwo);
  loadPet();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not found. Check wiring and address.");
    while (true) {
      delay(10);
    }
  }

  mpuFound = mpu.begin(MPU6050_ADDRESS, &Wire);
  if (mpuFound) {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  } else {
    Serial.println("MPU6050 not found. Menu tilt disabled.");
  }

  if (USE_DHT11) {
    dht.begin();
    dhtFound = true;
  }

  drawCurrentView();
}

void loop() {
  updateMpu();
  updateDht();
  updatePetTimers();
  handleButtons();

  if (currentView == MENU_VIEW) {
    updateMenuBall();
  } else {
    checkForShake();
  }

  drawCurrentView();
  delay(16);
}
