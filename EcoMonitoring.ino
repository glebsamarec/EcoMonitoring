#include <SPI.h> // Підключення бібліотеки SPI
#include <LoRa.h> // Підключення бібліотеки LoRA
#include <Adafruit_BME280.h> // Підключення бібліотеки для датчика BME280
#include <Adafruit_Sensor.h> // Підключення універсальної бібліотеки для датчиків
#include <AGS02MA.h> // Підключення бібліотеки для роботи з датчиком AGS02MA
#include <TroykaMQ.h> // Підключення бібліотеки для роботи з датчиками MQ
#include "MHZ19_uart.h" // Підключення бібліотеки для роботи з датчиком MHZ19
#include <PMS.h> // Підключення бібліотеки для роботи з датчиком PMS5003

#define ss 15 //  призначаеємо піну 15 назву
#define rst 16 // призначаємо піну 16 назву
#define dio0 2 // призначаємо піну 2 назву
#define MQ7PIN A0 // призначаємо пін для датчика MQ7

Adafruit_BME280 bme; // Створюэмо об`єкт для роботи з датчиком BME280
MQ7 mq7(MQ7PIN); // Створюэмо об`єкт для роботи з датчиком MQ7
MHZ19_uart mhz19; // Створюэмо об`єкт для роботи з датчиком MHZ19
PMS pms(Serial); // Створюэмо об`єкт для роботи з датчиком PMS5003
PMS::DATA data; // Створення команди для роботи з датчиком PMS5003
AGS02MA AGS(26); // Створюэмо об`єкт для роботи з датчиком AGS02MA

void setup() {
   Serial.begin(9600); // Налаштування швидкості передачі данних
   pms.passiveMode(); // Налаштування датчика PMS5003
   int status; // Створення змінної для статусу датчика mhz19
   Wire.begin(); // Ініціалізація бібліотеки Wire та підключення шини I2C
// Перевірка датчика BME280
  while (!Serial);
  if (!bme.begin(0x76)) // Перевірка данних з датчика BME280 за адрессою 0x76
{
  Serial.println("Error! BME"); // Помилка датчика BME280
  while (1);
}
//
//Налаштування та перевірка модуля Lora
  LoRa.setPins(ss, rst, dio0); // Присвоєння модулю Lora пінів
  if (!LoRa.begin(433E6)) { // Призначення частоти 433 для Lora та перевірка на запуск
    Serial.println("Starting LoRa failed!"); // Помилка модуля Lora
    while (1);
  }
//   
// Налаштування та перевірка датчика mhz19
  mhz19.begin(2, 3); // Присвоєння модулю mhz19 пінів
  mhz19.setAutoCalibration(false); // Відключення автокалібрування модуля
  status = mhz19.getStatus(); // Первинна перевірка статусу модуля
  delay(2000);
  status = mhz19.getStatus(); // Вторинна перевірка статусу модуля
  if (status != 0){ 
    Serial.println("Starting mhz19 failed!"); // Помилка модуля MHZ19
    while (1); 
  }
//
// Калібрування датчика mq7
if (!mq7.isCalibrated()){ // Якщо калібровки не було
mq7.calibrate(); // Калібровка модуля mq7
}
//
// Перевірка та налаштування датчика AGS02MA
AGS.begin(); // Запуск датчика
uint8_t ma = AGS.getMode(); // Статус режиму роботи модуля
if (ma != 0){ // Якщо модуль налаштовано не правильно
AGS.setPPBMode(); // Налаштування модуля AGS02MA на PPM режим
}
//
}

void loop() {
//Робота з датчиком MQ7
float carbonmonoxide=mq7.readCarbonMonoxide(); // Запис даних з датчика MQ7
//
//Робота з датчиком PMS5003
pms.wakeUp(); // Пробудження датчика PMS5003
delay(30000); // Затримка для пробудження датчика
pms.requestRead(); // Запрос читання даних с датчика
    bool pm1;
    bool pm2;
    bool pm10 ;
if (pms.read(data)) // Перевірка даних з датчика
  {
    pm1 = data.PM_AE_UG_1_0; // Запис даних ПМ1
    pm2 = data.PM_AE_UG_2_5; // Запис даних ПМ2.5
    pm10 = data.PM_AE_UG_10_0; // Запис данних ПМ10
  }
  else
  {
    Serial1.println("No data."); // Інформування що датчик не дав даних
  }
 pms.sleep(); // Встановлення режиму сну в датчику PMS5003
//
//Робота з датчиком MHZ19
int mhz19ppm = mhz19.getPPM(); // Запис данних з датчика mhz19
//
//Робота з датчиком BME280
float temp = bme.readTemperature(); // Запис температури з датчика BME280
int pressure = bme.readPressure() / 100.0F; // Запис тиску з датчика BME280
int humidity = bme.readHumidity(); // Запис вологості з датчика BME280
// Робота з датчиком AGS02MA
//
while (AGS.isHeated() == false) // Перевірка датчика AGS02MA на нагрів
  {
    delay(5000); // Затримка для нагріву датчика
  }
  float agsppm = AGS.readPPM(); // Запис даних з датчика AGS02MA
//
// Робота з модулем Lora
 LoRa.beginPacket(); // Початок пакету 
 LoRa.print(carbonmonoxide); // Пакет з данними з датчиків
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(pm1);// Пакет з данними з датчика
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(pm2);// Пакет з данними з датчика
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(pm10);// Пакет з данними з датчика
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(mhz19ppm);// Пакет з данними з датчика
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(temp);// Пакет з данними з датчика
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(pressure);// Пакет з данними з датчика
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(humidity);// Пакет з данними з датчика
 LoRa.print(";");// Пакет з знаком розділення данних
 LoRa.print(agsppm);// Пакет з данними з датчика
 LoRa.endPacket(); // Кінець пакету
//

delay(180000); // Затримка 30 хвилин 
}
