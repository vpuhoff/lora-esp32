#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_NeoPixel.h>

// LoRa configuration
#define LORA_FREQUENCY    433E6
#define LORA_SPREADING    7
#define LORA_BANDWIDTH    125E3
#define LORA_CODING_RATE  8
#define LORA_MAX_ATTEMPTS 5

#if defined(CONFIG_IDF_TARGET_ESP32S3)
  #define SS          10
  #define RST         14
  #define DIO0        9
  #define LED_PIN     48
  #define NUM_LEDS    1
#elif defined(CONFIG_IDF_TARGET_ESP32)
  #define SS          15
  #define RST         14
  #define DIO0        4
  #define LED_BUILTIN 2
#else
  #error "Unsupported ESP32 variant"
#endif

SemaphoreHandle_t loraMutex;

#if defined(CONFIG_IDF_TARGET_ESP32S3)
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

void blinkLED(int times, int delayTime, uint8_t red = 255, uint8_t green = 255, uint8_t blue = 255) {
  #if defined(CONFIG_IDF_TARGET_ESP32S3)
    strip.setBrightness(50); // Ограничиваем яркость
    strip.setPixelColor(0, strip.Color(red, green, blue));
    strip.show();
    vTaskDelay(pdMS_TO_TICKS(delayTime));
    strip.clear();
    strip.show();
  #else
    for (int i = 0; i < times; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      vTaskDelay(pdMS_TO_TICKS(delayTime));
      digitalWrite(LED_BUILTIN, LOW);
      vTaskDelay(pdMS_TO_TICKS(delayTime));
    }
  #endif
}

void taskSendHello(void *parameter) {
  while (true) {
    if (xSemaphoreTake(loraMutex, pdMS_TO_TICKS(1000))) {
      uint32_t startTime = millis(); // Запоминаем время начала отправки
      LoRa.beginPacket();
      LoRa.print("Hello");
      LoRa.endPacket();
      uint32_t duration = millis() - startTime; // Вычисляем длительность
      Serial.printf("Hello packet sent, transmission time: %u ms\n", duration);
      blinkLED(3, 50, 255, 0, 0); // Красный
      xSemaphoreGive(loraMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(random(2000, 3001))); // Случайная задержка
  }
}

void taskReceive(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(loraMutex, pdMS_TO_TICKS(2000))) {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        String incoming = "";
        while (LoRa.available()) {
          incoming += (char)LoRa.read();
        }
        incoming.trim();

        Serial.print("Received: ");
        Serial.println(incoming);

        if (incoming == "Hello") {
          Serial.println("Hello received! Sending ACK...");
          LoRa.beginPacket();
          LoRa.print("ACK");
          LoRa.endPacket();
          blinkLED(2, 300, 0, 255, 0); // Зелёный
        } else if (incoming == "ACK") {
          Serial.println("ACK received!");
          blinkLED(2, 100, 0, 0, 255); // Синий
        }
      }
      xSemaphoreGive(loraMutex);
    } else {
      Serial.println("Failed to acquire mutex for receive!");
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void taskMonitorStack(void *parameter) {
  for (;;) {
    UBaseType_t freeStack = uxTaskGetStackHighWaterMark(NULL);
    Serial.printf("Free stack: %d bytes\n", freeStack);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Serial.begin(115200);
  #if defined(CONFIG_IDF_TARGET_ESP32S3)
    strip.begin();
    strip.setBrightness(50);
    strip.show();
  #else
    pinMode(LED_BUILTIN, OUTPUT);
  #endif

  LoRa.setPins(SS, RST, DIO0);
  Serial.println("Initializing LoRa...");
  int attempts = 0;
  while (!LoRa.begin(LORA_FREQUENCY) && attempts < LORA_MAX_ATTEMPTS) {
    Serial.println("LoRa init failed, retrying...");
    blinkLED(2, 200);
    vTaskDelay(pdMS_TO_TICKS(10000));
    attempts++;
  }

  if (attempts == LORA_MAX_ATTEMPTS) {
    Serial.println("LoRa init failed permanently. Blinking indefinitely...");
    while (true) {
      blinkLED(1, 200);
    }
  }

  Serial.println("LoRa started successfully!");
  blinkLED(3, 100);

  LoRa.setSpreadingFactor(LORA_SPREADING);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  LoRa.enableCrc();

  loraMutex = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(taskSendHello, "SendHello", 3072, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskReceive, "Receive", 3072, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskMonitorStack, "StackMonitor", 2048, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
