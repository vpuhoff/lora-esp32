#include <SPI.h>
#include <LoRa.h>

// Схема подключения модуля LoRa SX1278 к ESP32 Basic:
// SX1278	ESP32 (GPIO)
// 3.3V	3.3V
// GND	GND
// NSS	GPIO15
// DIO0	GPIO4
// SCK	GPIO18
// MISO	GPIO19
// MOSI	GPIO23
// RST	GPIO14

// Схема подключения модуля LoRa SX1278 к	ESP32-S3
// 3.3V	3.3V
// GND	GND
// NSS	GPIO10
// DIO0	GPIO9
// SCK	GPIO12
// MISO	GPIO13
// MOSI	GPIO11
// RST	GPIO14

#if defined(CONFIG_IDF_TARGET_ESP32S3) // Если используется ESP32-S3
  #define SS   10
  #define RST  14
  #define DIO0 9
  #define LED_BUILTIN 48
#elif defined(CONFIG_IDF_TARGET_ESP32) // Если используется стандартный ESP32
  #define SS   15
  #define RST  14
  #define DIO0 4
  #define LED_BUILTIN 2
#else
  #error "Unsupported ESP32 variant"
#endif


void blinkLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(delayTime);
    digitalWrite(LED_BUILTIN, LOW);
    delay(delayTime);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  LoRa.setPins(SS, RST, DIO0);

  Serial.println("Initializing LoRa...");
  int attempts = 0;
  while (!LoRa.begin(433E6) && attempts < 5) { // 5 попыток инициализации
    Serial.println("LoRa init failed, retrying...");
    blinkLED(2, 200);  // Двойное мигание при ошибке инициализации
    delay(2000);
    attempts++;
  }

  if (attempts == 5) {
    Serial.println("LoRa init failed permanently. Blinking indefinitely...");
    while (true) { // Бесконечное мигание при фатальной ошибке
      blinkLED(1, 200);
    }
  }

  Serial.println("LoRa started successfully!");
  blinkLED(3, 100);  // 3 быстрых мигания при успешной инициализации

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);
}

void sendPacket(const char* message) {
  Serial.println("Sending packet...");
  LoRa.beginPacket();
  LoRa.print(message);
  LoRa.endPacket();
  blinkLED(3, 50);  // 3 быстрых мигания при отправке сообщения
}

void loop() {
  // Периодически (раз в 5 секунд) отправляем hello пакеты
  sendPacket("Hello receiver!")
  
  // Обрабатываем входящие сообщения в отдельном потоке 
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }
    incoming.trim(); // Убираем возможные лишние символы

    Serial.print("Received: ");
    Serial.println(incoming);

    if (incoming == "ACK") {
      ackReceived = true;
      Serial.println("ACK received!");
      blinkLED(2, 100);
      break;
    }
    if (incoming == "Hello receiver!") {
      ackReceived = true;
      Serial.println("Hello received!");
      blinkLED(2, 100);
      break;
    }
  }


  delay(50);
  // Это сделать отдельной таской для мониторинга
  UBaseType_t freeStack = uxTaskGetStackHighWaterMark(NULL); // NULL для текущей задачи
  Serial.printf("Free stack: %d bytes\n", freeStack);
}
