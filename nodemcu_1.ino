#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <FirebaseArduino.h>
#include <BlynkSimpleEsp8266.h>
#include <time.h>
#include <TimeLib.h>


// WiFi credentials
const char* ssid = "KOST 572";
const char* password = "Tuentrem";

// Firebase credentials
#define FIREBASE_HOST "gas-karbon-0-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "ILsixEgOj88ZOQAoGB4w48YV4N3atcgwO9Fqcq84"

// Blynk credentials
char auth[] = "U2aS9gLgA3c3-UFywwiFDkZNTjGG3SS4";

// ThingSpeak credentials
const char* server = "api.thingspeak.com";
const char* apiKey = "C2YKLAF85BT0LQ0X";

#define FIREBASE_PATH "/logs" // Path Firebase untuk menyimpan data logs

// Pin for MQ-7 sensor
const int sensorPin = A0;

WiFiClient client;

// Time offset for Jakarta (in seconds)
const int timeZoneOffset = 14390;  

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  ThingSpeak.begin(client);

//  Initial Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  // Set time from NTP server
  configTime(timeZoneOffset, 0, "pool.ntp.org");

  // Wait for time to be set
  while (!time(nullptr)) {
    delay(1000);
    Serial.println("Waiting for time synchronization...");
  }
  Serial.println("Time synchronized!");
}

 long RL = 1000; // 1000 Ohm
 long Ro = 1000; // 830 ohm


void loop() {

 // Get current time
  time_t now = time(nullptr) + timeZoneOffset;
  struct tm *timeinfo = localtime(&now);

  // Format time as string
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);


  // Read sensor value
  int sensorValue = analogRead(sensorPin);

  float VRL= sensorValue*5.00/1024;  // mengubah nilai ADC ( 0 - 1023 ) menjadi nilai voltase ( 0 - 5.00 volt )
  Serial.print("VRL : ");
  Serial.print(VRL);
  Serial.println(" volt");
  
  float Rs = ( 5.00 * RL / VRL ) - RL;
  Serial.print("Rs : ");
  Serial.print(Rs);
  Serial.println(" Ohm");
  
  float ppm = 100 * pow(Rs / Ro,-1.53); // ppm = 100 * ((rs/ro)^-1.53);
  Serial.print("CO : ");
  Serial.print(ppm);
  Serial.println(" ppm");
  Serial.println(" ");
  
  if(ppm > 60){
    Firebase.setString( "/Kondisi", "Gas level sangat bahaya");
  } else if (ppm > 30){
    Firebase.setString( "/Kondisi", "Gas level bahaya");
  } else {
    Firebase.setString( "/Kondisi", "Gas level normal");
  }
  
  // Membuat path Firebase untuk menyimpan data logs
  String firebasePath = "/logs/" + String(timestamp);

   Firebase.setFloat("/ppm", ppm);
   Firebase.setFloat("/VRL", VRL);
   Firebase.setFloat("/Rs", Rs);
   Firebase.setString("/datetime", timestamp);
  
  // Menambahkan data ke Firebase
  Firebase.setFloat(firebasePath + "/ppm", ppm);
  Firebase.setFloat(firebasePath + "/VRL", VRL);
  Firebase.setFloat(firebasePath + "/Rs", Rs);

    if(ppm > 60){
    Firebase.setString( firebasePath + "/Kondisi", "Gas level sangat bahaya");
  } else if (ppm > 30){
    Firebase.setString( firebasePath + "/Kondisi", "Gas level bahaya");
  } else {
    Firebase.setString( firebasePath + "/Kondisi", "Gas level normal");
  }

  
//  if (Firebase.success()) {
//    Serial.println("Data added to Firebase logs");
//  } else {
//    Serial.println("Failed to add data to Firebase");
//    Serial.println(Firebase.error());
//  }



  Blynk.virtualWrite(V0, ppm);

  // Update ThingSpeak channel with sensor value
  ThingSpeak.writeField(2200757, 1, ppm, apiKey);

  delay(500); // Delay 1 detik antara pengiriman data
  

  
  
}
