#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_CS   10
#define BME_MOSI 11
#define BME_MISO 13
#define BME_SCK  12

// BME280 підключено через (програмний) SPI, окремо від I2C-шини OLED/RTC
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

const char *const WEEKDAY_NAMES[] = {"HeD", "nOH", "BT", "CP", "4T", "nT", "Cy6"};
static constexpr uint8_t RTC_ADDR = 0x68;
static constexpr int OLED_WIDTH = 128;
static constexpr int OLED_HEIGHT = 64;
static constexpr int OLED_RESET = -1;
static constexpr uint8_t OLED_ADDR = 0x3C;

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);

// BCD конвертація
uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// Зчитування часу та дати з DS1307
void read_datetime(uint8_t *h, uint8_t *m, uint8_t *s, uint8_t *dow, uint8_t *day, uint8_t *month, uint16_t *year) {
    uint8_t data[7];
   
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(0x00);
    Wire.endTransmission(false);
   
    Wire.requestFrom(RTC_ADDR, 7);
    data[0] = Wire.read();  // Seconds
    data[1] = Wire.read();  // Minutes
    data[2] = Wire.read();  // Hours
    data[3] = Wire.read();  // Day of week
    data[4] = Wire.read();  // Day
    data[5] = Wire.read();  // Month
    data[6] = Wire.read();  // Year
  
    *s = bcd_to_dec(data[0] & 0x7F);
    *m = bcd_to_dec(data[1]);
    *h = bcd_to_dec(data[2] & 0x3F);
    *dow = bcd_to_dec(data[3]);
    *day = bcd_to_dec(data[4]);
    *month = bcd_to_dec(data[5] & 0x1F);
    *year = 2000 + bcd_to_dec(data[6]);
}

// Ініціалізація OLED дисплея
void init_oled() {
    uint8_t init_cmds[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00,
        0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8,
        0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB,
        0x40, 0xA4, 0xA6, 0xAF
    };
   
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x00);  // Control Byte
    for (int i = 0; i < sizeof(init_cmds); i++) {
        Wire.write(init_cmds[i]);
    }
    Wire.endTransmission();
}

// Виведення часу на OLED (спрощено - потребує бібліотеки)
void print_time_oled(uint8_t hour, uint8_t minute, uint8_t second, uint8_t dow, uint8_t day, uint8_t month, uint16_t year)  {
    char date_line[32];
    char time_line[16];

    snprintf(date_line, sizeof(date_line), "%s %02u.%02u.%04u", WEEKDAY_NAMES[(dow >= 1 && dow <= 7) ? dow - 1 : 0], day, month, year);
    snprintf(time_line, sizeof(time_line), "%02u:%02u:%02u", hour, minute, second);


    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
	
	display.setTextSize(1);
	
    display.setCursor(0, 0);
    display.println("RTC time");
    display.println(date_line);
	
    display.setTextSize(2);
    display.setCursor(0, 24);
    display.println(time_line);
    display.display();

    // Для справжньої реалізації необхідні шрифтові дані та складна математика
    // Наведено структуру:
    // Wire.beginTransmission(OLED_ADDR);
    // Wire.write(0x40);  // Control Byte: дані

    // // Тут розташовується піксельна інформація для символів часу
    // Wire.endTransmission();
}

void setup() {
    Serial.begin(115200);
    Wire.begin(8, 9); 
    
    //init_oled();
    delay(100);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("SSD1306 init failed");
    }
 
    if (!bme.begin()) {
        Serial.println("BME280 not found!");
    }
 
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("RTC display starting...");
    display.display();

    Serial.println("System initialized!");
}

void loop() {
    uint8_t hour, min, sec, dow, day, month;
    uint16_t year;
   
    read_datetime(&hour, &min, &sec, &dow, &day, &month, &year);

    // Форматований вивід дати й часу
    Serial.print(WEEKDAY_NAMES[(dow >= 1 && dow <= 7) ? dow - 1 : 0]);
    Serial.print(" ");
    if (day < 10) Serial.print("0");
    Serial.print(day);
    Serial.print(".");
    if (month < 10) Serial.print("0");
    Serial.print(month);
    Serial.print(".");
    Serial.print(year);
    Serial.print(" ");
    if (hour < 10) Serial.print("0");
    Serial.print(hour);
    Serial.print(":");
    if (min < 10) Serial.print("0");
    Serial.print(min);
    Serial.print(":");
    if (sec < 10) Serial.print("0");
    Serial.println(sec);
   
    // Виведення на OLED (потребує бібліотеки)
    print_time_oled(hour, min, sec, dow, day, month, year);
  
    delay(1000);
}
