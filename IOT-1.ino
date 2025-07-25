#define BLYNK_TEMPLATE_ID "Your_template_id"
#define BLYNK_TEMPLATE_NAME "NAME"
#define BLYNK_AUTH_TOKEN "your_token"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

BlynkTimer timer;

const char* ssid = "testwifi";
const char* password = "123456789";

const char* accountSID  = "ssid";
const char* authToken  = "token";
const char* fromPhoneNumber = "from";
const char* toPhoneNumber = "to";

WiFiClientSecure client;
String twilioURL = "https://api.twilio.com/2010-04-01/Accounts/" + String(accountSID) + "/Messages.json";

#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
const int gasSensorPin = A0;

#define AC_1 D5
#define AC_2 D6
#define EXHAUST D7
#define LED_GAS_ALERT D8

ESP8266WebServer server(80);

// Virtual Pins
#define ALERT_VPIN V0
#define LED_VPIN V1
#define TEMP_VPIN V2
#define HUMIDITY_VPIN V3
#define GAS_VPIN V4
#define FRESHNESS_VPIN V5
#define AC2_VPIN V6
#define EXHAUST_VPIN V7
#define AC1_VPIN V9

// Device statuses
bool ac1Status = false;
bool ac2Status = false;
bool exhaustStatus = false;

float temperature = 0.0;
float humidity = 0.0;

void sendSMS(String message) {
  HTTPClient http;
  http.begin(client, twilioURL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "To=" + String(toPhoneNumber) + "&From=" + String(fromPhoneNumber) + "&Body=" + message;
  http.setAuthorization(accountSID, authToken);

  int httpResponseCode = http.POST(postData);
  if (httpResponseCode > 0) {
    Serial.println("SMS sent successfully");
    Serial.println(http.getString());
  } else {
    Serial.print("Error sending SMS: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void sendSensorData() {
  int gasValue = analogRead(gasSensorPin);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
    digitalWrite(AC_1, HIGH);

  static bool gasSpoilSent = false;

  // AC_2 auto control
  if (temperature >= 35.0 && !ac2Status) {
    digitalWrite(AC_2, HIGH);
    ac2Status = true;
    Blynk.virtualWrite(AC2_VPIN, 1);
    sendSMS("AC_2 ON");
  } else if (temperature < 35.0 && ac2Status) {
    digitalWrite(AC_2, LOW);
    ac2Status = false;
    Blynk.virtualWrite(AC2_VPIN, 0);
    sendSMS("AC_2 OFF");
  }

  // Exhaust auto control
  if (humidity > 50 && !exhaustStatus) {
    digitalWrite(EXHAUST, HIGH);
    exhaustStatus = true;
    Blynk.virtualWrite(EXHAUST_VPIN, 1);
    sendSMS("EXHAUST ON");
  } else if (humidity <= 50 && exhaustStatus) {
    digitalWrite(EXHAUST, LOW);
    exhaustStatus = false;
    Blynk.virtualWrite(EXHAUST_VPIN, 0);
    sendSMS("EXHAUST OFF");
  }

  // AC_1 manual control
  
  // Gas Alert
  if (gasValue >= 250 && !gasSpoilSent) {
    Blynk.logEvent("gas_detected", "Gas Detected: " + String(gasValue));
    sendSMS("SPOIL");
    gasSpoilSent = true;
    digitalWrite(LED_GAS_ALERT, HIGH);
  } else if (gasValue < 250) {
    gasSpoilSent = false;
    digitalWrite(LED_GAS_ALERT, LOW);
  }

  // Freshness
  String freshness;
  if (gasValue < 160 && temperature < 30 && humidity < 50) {
    freshness = "Fresh";
  } else if (gasValue < 210 && temperature < 35) {
    freshness = "Slightly Spoiled";
  } else {
    freshness = "Spoiled";
  }

  // Alert Message
  String alertMsg = "";
  if (gasValue > 250) alertMsg += "⚠ Methane Detected\n";
  if (temperature >= 35.0) alertMsg += "🔥 High Temp\n";
  if (humidity >= 75.0) alertMsg += "💧 High Humidity\n";
  bool danger = (alertMsg.length() > 0);
  if (danger) Blynk.logEvent("gas_alert", alertMsg);

  // Send to Blynk
  Blynk.virtualWrite(TEMP_VPIN, temperature);
  Blynk.virtualWrite(HUMIDITY_VPIN, humidity);
  Blynk.virtualWrite(GAS_VPIN, gasValue);
  Blynk.virtualWrite(FRESHNESS_VPIN, freshness);
  Blynk.virtualWrite(ALERT_VPIN, danger ? alertMsg : "✅ All OK");
  Blynk.virtualWrite(V8, gasValue);
}



BLYNK_WRITE(AC2_VPIN) {
  bool ac2State = param.asInt();
  digitalWrite(AC_2, ac2State);
  ac2Status = ac2State;
  if (ac2State) {
    Blynk.logEvent("ac2_on", "AC_2 manually turned ON");
    sendSMS("AC_2 ON (manual)");
  } else {
    Blynk.logEvent("ac2_off", "AC_2 manually turned OFF");
    sendSMS("AC_2 OFF (manual)");
  }
}

BLYNK_WRITE(EXHAUST_VPIN) {
  bool exhaustState = param.asInt();
  digitalWrite(EXHAUST, exhaustState);
  exhaustStatus = exhaustState;
  if (exhaustState) {
    Blynk.logEvent("exhaust_on", "Exhaust manually turned ON");
    sendSMS("EXHAUST ON (manual)");
  } else {
    Blynk.logEvent("exhaust_off", "Exhaust manually turned OFF");
    sendSMS("EXHAUST OFF (manual)");
  }
}

void handleRoot() {
  server.send(200, "text/html", "<h1>Food Freshness Monitor</h1><p>Use Blynk or API to check data.</p>");
}

void handleData() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  int gas = analogRead(gasSensorPin);
  String json = "{\"temp\":" + String(temperature, 2) + ",\"humidity\":" + String(humidity, 2) + ",\"gas\":" + String(gas) + "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected. IP: " + WiFi.localIP().toString());

  client.setInsecure();  // For ESP8266 without CA cert
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  // Web Server
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();

  // Device Setup
  pinMode(AC_1, OUTPUT);
  pinMode(AC_2, OUTPUT);
  pinMode(EXHAUST, OUTPUT);
  pinMode(LED_GAS_ALERT, OUTPUT);
  digitalWrite(AC_1, HIGH);
  timer.setInterval(5000L, sendSensorData);
}

// This function is called whenever V1 changes in Blynk app
BLYNK_WRITE(LED_VPIN) {
  int value = param.asInt(); // Get 1 or 0 from button
  digitalWrite(AC_1, value); // Set LED accordingly
}
void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();
}
