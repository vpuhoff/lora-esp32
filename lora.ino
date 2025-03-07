#include <SPI.h>
#include <LoRa.h>

#if defined(CONFIG_IDF_TARGET_ESP32S3)
  #define SS          10
  #define RST         14
  #define DIO0        9
  #define LED_BUILTIN 48
#elif defined(CONFIG_IDF_TARGET_ESP32)
  #define SS          15
  #define RST         14
  #define DIO0        4
  #define LED_BUILTIN 2
#else
  #error "Unsupported ESP32 variant"
#endif

SemaphoreHandle_t loraMutex;

void blinkLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(delayTime));
    digitalWrite(LED_BUILTIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(delayTime));
  }
}

void taskSendHello(void *parameter) {
  while (true) {
    if (xSemaphoreTake(loraMutex, pdMS_TO_TICKS(1000))) {
      LoRa.beginPacket();
      LoRa.print("Hello receiver!");
      LoRa.endPacket();
      Serial.println("Packet sent: Hello receiver!");
      blinkLED(3, 50);
      xSemaphoreGive(loraMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void taskReceive(void *parameter) {
  for (;;) {
    if (xSemaphoreTake(loraMutex, pdMS_TO_TICKS(1000))) {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        String incoming = "";
        while (LoRa.available()) {
          incoming += (char)LoRa.read();
        }
        incoming.trim();

        Serial.print("Received: ");
        Serial.println(incoming);

        if (incoming == "Hello receiver!") {
          Serial.println("Hello received! Sending ACK...");
          LoRa.beginPacket();
          LoRa.print("ACK");
          LoRa.endPacket();
          blinkLED(2, 100);
        }
        else if (incoming == "ACK") {
          Serial.println("ACK received!");
          blinkLED(2, 100);
        }
      }
      xSemaphoreGive(loraMutex);
    }
    vTaskDelay(pdMS_TO_TICKS(50));
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
  pinMode(LED_BUILTIN, OUTPUT);

  LoRa.setPins(SS, RST, DIO0);

  Serial.println("Initializing LoRa...");
  int attempts = 0;
  while (!LoRa.begin(433E6) && attempts < 5) {
    Serial.println("LoRa init failed, retrying...");
    blinkLED(2, 200);
    vTaskDelay(pdMS_TO_TICKS(10000));
    attempts++;
  }

  if (attempts == 5) {
    Serial.println("LoRa init failed permanently. Blinking indefinitely...");
    while (true) {
      blinkLED(1, 200);
    }
  }

  Serial.println("LoRa started successfully!");
  blinkLED(3, 100);

  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(125E3);
  LoRa.setCodingRate4(5);

  loraMutex = xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(taskSendHello, "SendHello", 3072, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskReceive, "Receive", 3072, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(taskMonitorStack, "StackMonitor", 2048, NULL, 1, NULL, 1);
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
