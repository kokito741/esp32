#include "Adafruit_Sensor.h"
#include <Arduino.h>
#include "DHT.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>
#include <Firebase_ESP_Client.h>
#define DHT_SENSOR_TYPE DHT22
#define DHT_SENSOR_PIN 4
DHT dht=DHT(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
//Provide the token generation process info this will mostly help us debug the progress and the state of token generation
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#define API_KEY "AIzaSyBU7Tn2EOD03z5eOseQ2rbKoBzbS3waS9w"
#define DATABASE_URL "https://esp32-dd238-default-rtdb.europe-west1.firebasedatabase.app/"
FirebaseData firebaseData;
#include "addons/RTDBHelper.h"


const char* SSID = "kokinetwork-2G";
const char* PASSWORD = "0887588455";
#define DEVICE_ID "esp32-dev-1"

#define LOCATION "Living Room"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson Tempreature_json;
FirebaseJson Humidity_json;
FirebaseJson DateTaken_json;
String path="";//base path
String currentdata_path="";
String temp_path="";
String hum_path="";
String tim_path="";
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
float humidity=0.0;
float temperature=0.0;
std::string formattedDate="0.0.0.0";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
void FireBase_init(){
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
    config.api_key = API_KEY;
    auth.user.email = "esp32-dev@abv.bg";
    auth.user.password =  "987456321k";
    config.database_url = DATABASE_URL;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);
    Serial.print("Signing in ... ");
    config.token_status_callback = tokenStatusCallback;
    config.max_token_generation_retry = 5;

    Firebase.begin(&config, &auth);
    Serial.println(" firebase begin");
    Serial.println( Firebase.ready());
    Serial.println( "WAITING FOR FIREBASE TOKEN GENERATION");
    while (!Firebase.ready()){ Serial.print("..");delay(300);    };
    path += auth.token.uid.c_str(); //<- user uid
    path+="/";
    path+=LOCATION;
    path+="/";

}
void sendFloat(String path, float value){
    if (Firebase.setFloat(fbdo, path.c_str(), value)){
        Serial.print("Writing value: ");
        Serial.print (value);
        Serial.print(" on the following path: ");
        Serial.println(path);
        Serial.println("PASSED");
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
    }
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    dht.begin();
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    timeClient.begin();
    timeClient.setTimeOffset(7200);
    FireBase_init();
    Serial.println(currentdata_path);
    Serial.println(temp_path);
    Serial.println(hum_path);
    Serial.println(String(millis()));




}
void loop() {
    Serial.println("Delay finished");
    digitalWrite(LED_BUILTIN, HIGH);
    if (!Firebase.ready()) {
        Firebase.begin(&config, &auth);
        while (!Firebase.ready()) {
            delay(1000);
            Serial.println("Waiting for Firebase to initialize...");
        }
    }
    float temperature= dht.readTemperature();
    float humidity=dht.readHumidity();
    timeClient.forceUpdate();
    String weekDay = weekDays[timeClient.getDay()];
    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime ((time_t *)&epochTime);
    int monthDay = ptm->tm_mday;
    int currentMonth = ptm->tm_mon+1;
    String currentMonthName = months[currentMonth-1];
    int currentYear = ptm->tm_year+1900;
    String currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear)+" - " +  (timeClient.getHours() < 10 ? "0" : "") + String(timeClient.getHours())+"-" + (timeClient.getMinutes() < 10 ? "0" : "") + String(timeClient.getMinutes());
    Serial.print("Current date: ");
    Serial.println(currentDate);
    currentdata_path = path+currentDate;
    Serial.println(currentdata_path);
    temp_path=currentdata_path+"/temperature";
    hum_path=currentdata_path+"/humidity";
    sendFloat(temp_path, temperature);
    sendFloat(hum_path, humidity);

    digitalWrite(LED_BUILTIN, LOW);
    delay(40000);
}
