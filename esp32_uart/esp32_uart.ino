// Include the required libraries
#include <WiFi.h>
#include <AdafruitIO_WiFi.h>
// #include <Arduino_FreeRTOS.h>
#include <HardwareSerial.h>

// Define the UART pins
#define WIFI_SSID "llapttop"
#define WIFI_PASS "12344321"
#define IO_USERNAME "lexuanbach"
#define IO_KEY "aio_yUqT04A8xmes8L3ycQY3hjHdXM2a"
#define RX_PIN 16
#define TX_PIN 17

// Create an instance of the AdafruitIO_WiFi class
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Create a feed
AdafruitIO_Feed *temp = io.feed("project-iot.temperature");
AdafruitIO_Feed *humi = io.feed("project-iot.humidity");
AdafruitIO_Feed *fan = io.feed("project-iot.fan");

// Create a Serial object
HardwareSerial SerialUART(1);

String DataToMicro = "0";

void handleMessage(AdafruitIO_Data *data) {
  Serial.print("received <- ");
  Serial.println(data->value());
  DataToMicro = String(data->value());
}

String store[4];

void separate (String strori, String *strs)
{
  int splitcount = 0;
  while (strori.length() > 0){
    int index = strori.indexOf('#');
    if (index == -1){
      strs[splitcount++] = strori;
      break;
    }else{
      strs[splitcount++] = strori.substring(0, index);
      strori = strori.substring(index + 1);
    }
  }
}

void ReceiveFromAda( void *pvParameters );
void SendToAda( void *pvParameters );

void ReceiveFromAda(void *pvParameters){
  (void) pvParameters;
  for (;;) {
    io.run();
    // Send data via UART

  }
}

void SendToAda(void *pvParameters){
  (void) pvParameters;
  for (;;)
  {
    // Receive data via UART
    if (SerialUART.available() > 0) {
      String receivedData = SerialUART.readStringUntil('\r');
      separate(receivedData, store);
      Serial.println("Received: " + receivedData);
    }

    float tempValue = random(20, 30);
    float humiValue = random(50, 60);

    // Send the temperature value to Adafruit IO
    temp->save(tempValue);
    humi->save(humiValue);
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
  fan->onMessage(handleMessage);

  // Wait for connection
  while(io.status() < AIO_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Adafruit IO...");
  }
  Serial.println("Connected to Adafruit IO");

  // Configure UART
  SerialUART.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

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
}

void loop() {
  SerialUART.println(DataToMicro + "\n");
  Serial.println("Sended");
  Serial.println(DataToMicro);
  delay(2000);
  // Other loop code...
}
