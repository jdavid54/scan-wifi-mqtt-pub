/*
 *  This sketch demonstrates how to scan WiFi networks.
 *  The API is based on the Arduino WiFi Shield library, but has significant changes as newer WiFi functions are supported.
 *  E.g. the return value of `encryptionType()` different because more modern encryption is supported.
 */
#include "WiFi.h"
#include <PubSubClient.h>

static const uint8_t LED_BUILTIN = 8;     //LED on with LOW
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
uint8_t n = 0;

// Wifi
const char* ssid = "your_ssid";
const char* password = "your_password";
// MQTT
const char* mqtt_server = "your_broker";


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());

  Serial.println("");
  Serial.print("WiFi connected - ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish the 5 SSIDs ...
      char msg[MSG_BUFFER_SIZE];
      for (int i = 0; i < 5; ++i) {
        snprintf (msg, MSG_BUFFER_SIZE, "%1d - %32s: %4d CH%2d", i+1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), WiFi.channel(i));
        client.publish("ScanTopic", msg);
      }
      Serial.println("Sending message ...");
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void send_serial() {
  for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found

      Serial.printf("%2d",i + 1);
      Serial.print(" | ");
      Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
      Serial.print(" | ");
      Serial.printf("%4d", WiFi.RSSI(i));
      Serial.print(" | ");
      Serial.printf("%2d", WiFi.channel(i));
      Serial.print(" | ");
      switch (WiFi.encryptionType(i))
      {
      case WIFI_AUTH_OPEN:
          Serial.print("open");
          break;
      case WIFI_AUTH_WEP:
          Serial.print("WEP");
          break;
      case WIFI_AUTH_WPA_PSK:
          Serial.print("WPA");
          break;
      case WIFI_AUTH_WPA2_PSK:
          Serial.print("WPA2");
          break;
      case WIFI_AUTH_WPA_WPA2_PSK:
          Serial.print("WPA+WPA2");
          break;
      case WIFI_AUTH_WPA2_ENTERPRISE:
          Serial.print("WPA2-EAP");
          break;
      case WIFI_AUTH_WPA3_PSK:
          Serial.print("WPA3");
          break;
      case WIFI_AUTH_WPA2_WPA3_PSK:
          Serial.print("WPA2+WPA3");
          break;
      case WIFI_AUTH_WAPI_PSK:
          Serial.print("WAPI");
          break;
      default:
          Serial.print("unknown");
      }
      Serial.println();
      delay(10);
  }
}

void wifi_scan() {
// WiFi.scanNetworks will return the number of networks found.
    Serial.println("Scan start");
    digitalWrite(LED_BUILTIN, LOW);   // LED on
    n = WiFi.scanNetworks();
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(n);
        Serial.println(" networks found");
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        snprintf (msg, MSG_BUFFER_SIZE, "%32s: %4d CH%2d", WiFi.SSID(0).c_str(), WiFi.RSSI(0), WiFi.channel(0));
        send_serial();
    }
    Serial.println("");
    //digitalWrite(LED_BUILTIN, HIGH);    // LED off
    // Delete the scan result to free memory for code below.
    //WiFi.scanDelete();
    //return WiFi;
}

void setup()
{
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("Setup done");
}

void loop()
{
    wifi_scan(); 
    setup_wifi();
    reconnect();

    // Wait a bit before scanning again.
    delay(3000);
}
