// Include the required libraries
#include <WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <HardwareSerial.h>
#include "DHT.h"

// Define wifi and io config
#define WIFI_SSID "NamPhu"
#define WIFI_PASS "11032002"
#define IO_USERNAME "lexuanbach"
#define IO_KEY "aio_ljDF89cmvjQg0yuyp2MATUAdXtuu"
// Define the UART pins
#define RX_PIN 16
#define TX_PIN 17
// Define DHT11
#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Create an instance of the AdafruitIO_WiFi class
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Create a feed
AdafruitIO_Feed *temp = io.feed("project-iot.temperature");
AdafruitIO_Feed *humi = io.feed("project-iot.humidity");
AdafruitIO_Feed *fan = io.feed("project-iot.fan");
AdafruitIO_Feed *autofan = io.feed("project-iot.auto");
AdafruitIO_Feed *status = io.feed("project-iot.status");

// Create a Serial object
HardwareSerial SerialUART(1);

String DataToMicro = "0";
String prevDataToMicro = "0";
String fanstt = "0";
String autofanstt = "0";
String ctrlocal = "0";

void handleMessagefan(AdafruitIO_Data *data) {
  Serial.print("Received fan from ada <- ");
  Serial.println(data->value());
  fanstt = String(data->value());
}

void handleMessageauto(AdafruitIO_Data *data) {
  Serial.print("Received autofan from ada <- ");
  Serial.println(data->value());
  autofanstt = String(data->value());
}

void ReceiveFromAda( void *pvParameters );
void SendToAda( void *pvParameters );

void ReceiveFromAda(void *pvParameters){
  (void) pvParameters;
  for (;;) {
    io.run();
  }
}

void SendToAda(void *pvParameters){
  (void) pvParameters;
  for (;;)
  {
    // Receive data via UART
    if (SerialUART.available() > 0) {
      String receivedData = SerialUART.readStringUntil('\r');
      Serial.println("Received from microbit: " + receivedData);
      if(receivedData == "1"){
        Serial.println("The fan is turned on.");
      }else{
        Serial.println("The fan is turned off.");
      }
      status->save(receivedData.toInt());
    }
    // Get environment data from dht11 sensor
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    Serial.print("Current humidity: ");
    Serial.println(h);
    Serial.print("Current temperature: ");
    Serial.println(t);

    // Set condition to auto control fan
    if(t > 25 || h > 60){
      ctrlocal = "1";
    }else{
      ctrlocal = "0";
    }
  
    // Send the value to Adafruit IO
    temp->save(t);
    humi->save(h);
    delay(10000);
  }
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to Adafruit
  io.connect();

  // Message handler
  fan->onMessage(handleMessagefan);
  autofan->onMessage(handleMessageauto);

  // Wait for connection
  while(io.status() < AIO_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Adafruit IO...");
  }
  Serial.println("Connected to Adafruit IO");

  dht.begin();

  // Configure UART
  SerialUART.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  // Set up tasks to run independently.
  xTaskCreate(
    ReceiveFromAda
    ,  "ReceiveFromAda"   // Name
    ,  4096               // Stack size
    ,  NULL
    ,  2                  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
  
  xTaskCreate(
    SendToAda
    ,  "SendToAda"    // Name
    ,  4096           // Stack size
    ,  NULL
    ,  1              // Priority
    ,  NULL );

  // Other setup code...
  autofan->get();
  fan->get();
  if(autofanstt == "0"){
    DataToMicro = fanstt;
    if(DataToMicro != prevDataToMicro){
      prevDataToMicro = DataToMicro;
      // Send data via UART
      SerialUART.println(DataToMicro);
      Serial.println("Sended to microbit:" + DataToMicro);
    }
  }else{
    DataToMicro = ctrlocal;
    if(DataToMicro != prevDataToMicro){
      prevDataToMicro = DataToMicro;
      // Send data via UART
      SerialUART.println(DataToMicro);     
      Serial.println("Sended to microbit:" + DataToMicro);
    }
  }
}

void loop() {

  if(autofanstt == "0"){
    DataToMicro = fanstt;
    if(DataToMicro != prevDataToMicro){
      prevDataToMicro = DataToMicro;
      // Send data via UART
      SerialUART.println(DataToMicro);
      Serial.println("Sended to microbit: " + DataToMicro);
    }
  }else{
    DataToMicro = ctrlocal;
    if(DataToMicro != prevDataToMicro){
      prevDataToMicro = DataToMicro;
      // Send data via UART
      SerialUART.println(DataToMicro);
      Serial.println("Sended to microbit: " + DataToMicro);
    }
  }
  // Other loop code...
}
