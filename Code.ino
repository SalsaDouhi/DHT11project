#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define DHTTYPE DHT11
const char *ssid = "Fibre_MarocTelecom_2.4G";
const char *password = "EhLbfkJL";
const char *serverName = "http://127.0.0.1:8000/api/post";
// const char* serverName = "https://salsabil.pythonanywhere.com/api";
#define dht_dpin 12
DHT dht(dht_dpin, DHTTYPE);
// Telegram Bot Details
const char *botToken = "7824644172:AAH86j9IgZDUHXA-QrbNM5F-xBcjdgKMV00";
const char *chatID = "6210219140";
const char *telegramAPI = "https://api.telegram.org";
// Temperature Range
const float minTemp = 2.0;
const float maxTemp = 10.0;
void setup()
{
  dht.begin();
  Serial.begin(9600);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  Serial.print("Current humidity = ");
  Serial.print(humidity);
  Serial.print("%  ");
  Serial.print("temperature = ");
  Serial.print(temperature);
  Serial.println("C  ");
  // verification de temperature
  if (temperature < minTemp || temperature > maxTemp)
  {
    sendTelegramNotification(humidity, temperature);
  }
  // Envoi des valeurs au serveur Web
  WiFiClient client;
  HTTPClient http;
  DynamicJsonDocument jsonDoc(200); // Utilisation de DynamicJsonDocument pour la sérialisation JSON
  jsonDoc["temp"] = temperature;
  jsonDoc["hum"] = humidity;
  String jsonStr;
  serializeJson(jsonDoc, jsonStr);

  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");
  Serial.println(jsonStr);
  int httpResponseCode = http.POST(jsonStr);

  if (httpResponseCode > 0)
  {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();

  // Attente de 10 secondes avant de lire les valeurs suivantes
  delay(1200000);
}
void sendTelegramNotification(float humidity, float temperature)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFiClientSecure client;
    client.setInsecure(); // For HTTPS requests without certificate verification

    HTTPClient http;
    String message = "Alert: Temperature is out of range!\n";
    message += "Current Temperature: " + String(temperature) + "°C\n";
    message += "Current Humidity: " + String(humidity) + "%";

    String url = String(telegramAPI) + "/bot" + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;

    http.begin(client, url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      Serial.print("Telegram Message Sent. Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error sending message: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi not connected. Unable to send Telegram message.");
      
  }
}
