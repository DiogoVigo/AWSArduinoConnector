#include <FS.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>

// Amazon Endpoint
const char* Amazon_Endpoint = "xxxxxxxxxx-ats.iot.eu-central-1.amazonaws.com";

// WiFi Credentials
const char* ssid = "ssid";
const char* password = "password";

//Thing details
const String thingID  = "ESPTry3";
const String SensorType = "Sensor_ESP8266";

// Topics
String topic = thingID + "/" + SensorType;
const char* tpc = topic.c_str();

//---------- Shadow Topics ------------
const String getShadowTopic          = "$aws/things/" + thingID + "/shadow/get";
const String getShadowAcceptedTopic    = "$aws/things/" + thingID + "/shadow/get/accepted";
const String getShadowRejectedTopic    = "$aws/things/" + thingID + "/shadow/get/rejected";

const String updateShadowTopic       = "$aws/things/" + thingID + "/shadow/update";
const String updateShadowAcceptedTopic = "$aws/things/" + thingID + "/shadow/update/accepted";
const String updateShadowRejectedTopic = "$aws/things/" + thingID + "/shadow/update/rejected";

const String DeleteShadowTopic       = "$aws/things/" + thingID + "/shadow/delete";
const String DeleteShadowAcceptedTopic = "$aws/things/" + thingID + "/shadow/delete/accepted";
const String DeleteShadowRejectedTopic = "$aws/things/" + thingID + "/shadow/delete/rejected";

const String updateShadowDocTopic    = "$aws/things/" + thingID + "/shadow/update/documents";
//------------------------------------------

long lastReconnectAttempt = 0;
long lastMsg = 0;
int test_para = 2000;
unsigned long startMills;

WiFiClientSecure wifiClient;
PubSubClient client(Amazon_Endpoint, 8883, wifiClient);


// Load Certificates from File System (SPIFFS)
// Certificates and keys must be in .der formar
// openssl x509 -outform der -in xxxxxxxx-certificate.pem.crt -out cert.der
// openssl rsa -outform der -in xxxxxxxx-private.pem.key -out private.der
// openssl x509 -outform der -in AmazonRootCA1.pem -out ca.der
void loadcerts() {

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Load Thing Certificate file
  File cert = SPIFFS.open("/cert.der", "r");
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  else
    Serial.println("Success to open cert file");

  delay(1000);

  // Set thing Certificate
  if (wifiClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");

  // Load Thing private key file
  File private_key = SPIFFS.open("/private.der", "r");
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");

  delay(1000);

  // Set thing private key
  if (wifiClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");

  // Load Amazon CA file
  File ca = SPIFFS.open("/ca.der", "r");
  if (!ca) {
    Serial.println("Failed to open ca ");
  }
  else
    Serial.println("Success to open ca");
  delay(1000);

  // Set Amazon CA file
  if (wifiClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    Serial.println("ca failed");

}

// Using SNTP (Simple Network Time Protocol) verifies if
// the TLS certificates are currently valid.
// Without this server won't accept the certificates.
void getTime() {
  configTime(8 * 3600, 0, "de.pool.ntp.org");
  time_t now = time(nullptr);
  while (now < 1000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
}

// Callback function to receive data from AWS IoT
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
    Serial.print((char)payload[i]);

  Serial.println();
}

// Establishes connection to Amazon endpoint
boolean reconnect()
{
  if (!client.connected()) {
    if (client.connect(thingID.c_str())) {
      Serial.println("Connected to Endpoint");
      client.setCallback(callback);
      if (client.subscribe("test")) {
        Serial.println("Subscribed to test");
      }
    } else {
      Serial.print("Something went wrong. RC=");
      Serial.println(client.state());
    }
  }
  return client.connected();
}

// Establishes Wi-Fi Connection
void wifi_connect()
{
  if (WiFi.status() != WL_CONNECTED) {
    // WIFI
    Serial.println("Connecting to: "); Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
    }
    Serial.println("Successful connected");
    Serial.println("IP address: "); Serial.println(WiFi.localIP());
  }
}

// Setup for the sensor
void setup()
{
  Serial.begin(115200);

  wifi_connect();
  delay(500);

  getTime();
  delay(500);

  loadcerts();
  delay(200);

  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.println("topic:");
  Serial.println(topic);
}

//Main loop
void loop()
{
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      long now = millis();
      Serial.println("Lost Connection to AWS");
      if (now - lastReconnectAttempt > 2000) {
        lastReconnectAttempt = now;
        if (reconnect()) {
          lastReconnectAttempt = 0;
        }
      }
    }
    else {
      Serial.println("Connected to AWS");
      long now = millis();
      String payload = "{\"Data\":";
      payload += 1;
      payload += "}";
      sendmqttMsg(tpc, payload);
      delay(5000);
      client.loop();
    }
  }
  else {
    wifi_connect();
  }

}

// Sends message to endpoint on a specific topic
void sendmqttMsg(const char* topictosend, String payload)
{

  if (client.connected()) {
    Serial.print("Sending payload: "); Serial.print(payload); 
    unsigned int msg_length = payload.length();
    byte* p = (byte*)malloc(msg_length);
    memcpy(p, (char*)payload.c_str(), msg_length);

    if (client.publish(topictosend, p, msg_length)) {
      Serial.println("Published");
      free(p);
    } else {
      Serial.println("Publish failed");
      free(p);
    }
  }
}
