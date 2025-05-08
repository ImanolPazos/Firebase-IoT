#include <Arduino.h>
#include <WiFi.h>
#include "time.h"
#include <Firebase_ESP_Client.h>
// Sensor

#include <DHT.h>
#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Firebase helpers
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Timestamp
String getLocalTimeISO();
String getLocalTimeUNIX();

// NTP
#define NTP_SERVER "pool.ntp.org"
#define NTP_GMT_OFFSET_SEC 0
#define NTP_DAYLIGHT_OFFSET_SEC 7200



// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Global variables
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

//LED
#define LED_1 23
#define LED_2 22
#define LED_R 17
#define LED_G 18
#define LED_B 19


void setup()
{
  // Initialize Serial
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(333);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize NTP
  configTime(NTP_GMT_OFFSET_SEC, NTP_DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  // Configure Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Firebase signup ok!");
    signupOK = true;
  }
  else
  {
    Serial.println("Firebase signup failed!");
  }

  // Assign callback function for token generation task
  config.token_status_callback = tokenStatusCallback;

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  //Sensor 
  dht.begin();
  //LED1
  pinMode(LED_1, OUTPUT);
  //LED2
  pinMode(LED_2, OUTPUT);
  //LEDR
  pinMode(LED_R, OUTPUT);
  //LEDG
  pinMode(LED_G, OUTPUT);
  //LEDB
  pinMode(LED_B, OUTPUT);
}

void loop ()
{
  static int intValue = 0;
  float floatValue = 0.0;
  static bool boolValue = true;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 60000 || sendDataPrevMillis == 0))
  {
    // Firebase is ready, we are signup and 10 seconds has passed
    // Save current time
    sendDataPrevMillis = millis();
    // Print time
    String timestamp_unix = getLocalTimeUNIX();
    Serial.println("UNIX: " + timestamp_unix);

    String timestamp_iso = getLocalTimeISO();
    Serial.println("ISO: " + timestamp_iso);
    //SENSOR
    // Humedad
    float Humedad = dht.readHumidity ();
    Serial.print("Humedad");
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/" + timestamp_unix +"/humedad", Humedad))
    {
      Serial.println("OK");
      Serial.println("  PATH: " + fbdo.dataPath());
      Serial.println("  TYPE: " + fbdo.dataType());
      Serial.print("  VALUE: ");
      Serial.println(dht.readHumidity());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    // Temperatura
    float Temperatura = dht.readTemperature ();
    Serial.print("Temperartura");
    if (Firebase.RTDB.setFloat(&fbdo, "Sensor/" + timestamp_unix +"/temperatura", Temperatura))
    {
      Serial.println("OK");
      Serial.println("  PATH: " + fbdo.dataPath());
      Serial.println("  TYPE: " + fbdo.dataType());
      Serial.print("  VALUE: ");
      Serial.println(dht.readTemperature());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    //Confort
    String Confort = "";
    Serial.print("Confort");
    if (Temperatura>20&Humedad>40)
    {
      if (Firebase.RTDB.setString(&fbdo, "Sensor/" + timestamp_unix +"/Confort", "Si"))
    {
      digitalWrite(LED_2, HIGH);
    }
    else 
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    }
    else { if (Firebase.RTDB.setString(&fbdo, "Sensor/" + timestamp_unix +"/Confort", "NO"))
    {
      digitalWrite(LED_2, LOW);
    }
        else 
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    }  
    // Hora
    Serial.print("Hora");
    if (Firebase.RTDB.setString(&fbdo, "Sensor/" + timestamp_unix +"/timestamp", timestamp_iso))
    {
      Serial.println("OK");
      Serial.println("  PATH: " + fbdo.dataPath());
      Serial.println("  TYPE: " + fbdo.dataType());
      Serial.print("  VALUE: ");
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
  }

   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0))
   {
//ACTUADOR
    // Write sample float
    Serial.print("FLOAT WRITE ");
    if (Firebase.RTDB.setFloat(&fbdo, "test/float", floatValue))
    {
      Serial.println("OK");
      Serial.println("  PATH: " + fbdo.dataPath());
      Serial.println("  TYPE: " + fbdo.dataType());
      Serial.print("  VALUE: ");
      Serial.println(fbdo.floatData());
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    // LEDRGB
    int red = 0;
    int green = 0;
    int blue = 0;
    Serial.print("read/int: ");
    if (Firebase.RTDB.getInt(&fbdo,"actuador/rgb", &red))
    {
      analogWrite(LED_R, red);
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.getInt(&fbdo,"actuador/rgb", &green))
    {
        analogWrite(LED_G, green); 
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    if (Firebase.RTDB.getInt(&fbdo,"actuador/rgb", &blue))
    {
      analogWrite(LED_B, blue);
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    // Encender/Apagar Led
    bool LED = false;
    Serial.print("actuador/LED: ");
    if (Firebase.RTDB.getBool(&fbdo, "actuador/LED", &LED))
    {
      if (LED=true)
    {
      digitalWrite(LED_1, HIGH);
    }
    else
    {
      digitalWrite(LED_1, LOW);
    }
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }
    
    // Read sample string
    String stringRead = "";
    Serial.print("read/string: ");
    if (Firebase.RTDB.getString(&fbdo, "read/string", &stringRead))
    {
      Serial.println(stringRead);
    }
    else
    {
      Serial.println("FAILED");
      Serial.println("  REASON: " + fbdo.errorReason());
    }

    
  }
   }
String getLocalTimeISO()
{
  struct tm timeinfo;
  char buffer[20];

  // Get local time
  if(!getLocalTime(&timeinfo))
  {
    return "NTP Error!";
  }

  // Obtain ISO 8601 timestamp
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &timeinfo);
  return String(buffer);
}

String getLocalTimeUNIX()
{
  struct tm timeinfo;

  // Get local time
  if(!getLocalTime(&timeinfo))
  {
    return "NTP Error!";
  }

  // Obtain UNIX timestamp
  return String(mktime(&timeinfo));
}
