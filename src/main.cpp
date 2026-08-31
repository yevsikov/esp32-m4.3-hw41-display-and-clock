#include <Arduino.h>
#include <Wire.h>
#define RTC_ADDR 0x68
#define OLED_ADDR 0x3C

// BCD конвертація
uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// Зчитування часу з DS1307
void read_time(uint8_t *h, uint8_t *m, uint8_t *s) {
    uint8_t data[3];
   
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(0x00);
    Wire.endTransmission(false);
   
    Wire.requestFrom(RTC_ADDR, 3);
    data[0] = Wire.read();  // Seconds
    data[1] = Wire.read();  // Minutes
    data[2] = Wire.read();  // Hours
  
    *s = bcd_to_dec(data[0] & 0x7F);
    *m = bcd_to_dec(data[1]);
    *h = bcd_to_dec(data[2] & 0x3F);
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
void print_time_oled(uint8_t h, uint8_t m, uint8_t s) {

    // Для справжньої реалізації необхідні шрифтові дані та складна математика
    // Наведено структуру:
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x40);  // Control Byte: дані

    // Тут розташовується піксельна інформація для символів часу
    Wire.endTransmission();
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);  // ESP32: SDA=21, SCL=22
    
    init_oled();
    delay(100);
 
    Serial.println("System initialized!");
}

void loop() {
    uint8_t hour, min, sec;
   
    read_time(&hour, &min, &sec);
   
    // Форматований вивід
    Serial.print(hour);
    Serial.print(":");
    if (min < 10) Serial.print("0");
    Serial.print(min);
    Serial.print(":");
    if (sec < 10) Serial.print("0");
    Serial.println(sec);
   
    // Виведення на OLED (потребує бібліотеки)
    // print_time_oled(hour, min, sec);
  
    delay(1000);
}

// Очікуваний вивід:
// 12:30:47
// 12:30:48
// 12:30:49