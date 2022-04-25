
#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <ClosedCube_HDC1080.h>
ClosedCube_HDC1080 hdc1080;
double lastT;
double lastH;

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128,32, &Wire);

#include "arduino_secrets.h" 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                 // your network key index number (needed only for WEP)

// varie 
int status = WL_IDLE_STATUS;
WiFiServer server(80);
int scrapeCount = 0;

// --------------------------------------------------------------- SETUP
void setup() {
  Serial.begin(9600);      // initialize serial communication

  // inizializzo sensore
  hdc1080.begin(0x40);
  Serial.print("Manufacturer ID=0x");
  Serial.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
  Serial.print("Device ID=0x");
  Serial.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device


  // inizializzo display
  Serial.print("OLED Setting up");
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.display();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

     display.clearDisplay();
    display.setCursor(0,0);
    display.print("Trying connection ");
    display.print("to:");
    display.println(ssid);
    display.setCursor(0,0);
    display.display(); // actually display all of the above
    
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status

  display.clearDisplay();
  display.setCursor(0,0);
  display.println("Connected to:");
  display.println(ssid);
  display.println(WiFi.localIP());
  display.println("Server Ready!");
  display.setCursor(0,0);
  display.display(); // actually display all of the above
}

// --------------------------------------------------------------- LOOP
void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/plain; version=0.0.4; charset=utf-8");
            client.println();

            lastT = hdc1080.readT();
            lastH = hdc1080.readH();
            scrapeCount = scrapeCount + 1;

            // the content of the HTTP response follows the header:
            client.print("# HELP generic_temp_c Temperatura rilevata da arduino.\n");
            client.print("# TYPE generic_temp_c gauge\n");
            client.print((String)"generic_temp_c " + lastT +"\n");
            
            client.print("# HELP generic_igrom_perc Umidita rilevata da arduino.\n");
            client.print("# TYPE generic_igrom_perc gauge\n");
            client.print((String)"generic_igrom_perc " + lastH +"\n");

            client.print("# HELP generic_arduino00_scrapeCount Contatore scraping su Arduino00.\n");
            client.print("# TYPE generic_arduino00_scrapeCount counter\n");
            client.print((String)"generic_arduino00_scrapeCount " + scrapeCount +"\n");
            
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("Scraping from:");
            display.println(client.remoteIP());
            
            display.print("T: ");
            display.print(lastT);
            display.print("c H:");
            display.print(lastH);
            display.println("%");

            display.print("scrape#");
            display.println(scrapeCount);
            
            display.setCursor(0,0);
            display.display(); // actually display all of the above
            

            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  delay(1000);
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
