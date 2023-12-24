#include <Arduino.h>
#include "DHT_Async.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FirebaseESP32.h>
#include <Firebase_ESP_Client.h>
#define DHT_SENSOR_TYPE DHT_TYPE_22
#define DHT_SENSOR_PIN 22
DHT_Async dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);
//Provide the token generation process info this will mostly help us debug the progress and the state of token generation
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#define API_KEY "AIzaSyBU7Tn2EOD03z5eOseQ2rbKoBzbS3waS9w"
#define DATABASE_URL "https://esp32-dd238-default-rtdb.europe-west1.firebasedatabase.app/"
FirebaseData firebaseData;


const char* SSID = "kokinetwork-2G";
const char* PASSWORD = "0887588455";
#define DEVICE_ID "dev-1"

#define LOCATION "Living Room"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
FirebaseJson Tempreature_json;
FirebaseJson Humidity_json;
FirebaseJson DateTaken_json;
String path="";//base path
String temp_path="";
String hum_path="";
String tim_path="";

// Sensor data
float humidity=0.0;
float temperature=0.0;
std::string formattedDate="0.0.0.0";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
void FireBase_init(){
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    /* Assign the API key (required) */
    config.api_key = API_KEY;
    auth.user.email = "esp32-dev@abv.bg";
    auth.user.password =  "987456321k";
    /* Assign the RTDB URL */
    config.database_url = DATABASE_URL;

    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    Serial.print("Signing in ... ");

    /* Sign up */

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    config.max_token_generation_retry = 5;

    Firebase.begin(&config, &auth);
    Serial.println(" firebase begin");
    Serial.println( Firebase.ready());
    Serial.println( "WAITING FOR FIREBASE TOKEN GENERATION");
    while (!Firebase.ready()){ Serial.print("..");delay(300);    };


    path += auth.token.uid.c_str(); //<- user uid
    path+="/";

    // determining the paths

    path+=LOCATION;
    temp_path=path+"/temperature";
    hum_path=path+"/humidity";
    tim_path=path+"/timetaken";
}
void Json_init()
{
    Tempreature_json.add("Device ID",DEVICE_ID);
    Tempreature_json.add("Value",temperature);

    Humidity_json.add("Device ID",DEVICE_ID);
    Humidity_json.add("Value",humidity);
    DateTaken_json.add("Device ID",DEVICE_ID);
    DateTaken_json.add("Value",formattedDate);
}

void setup() {

    Serial.begin(115200);
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    timeClient.begin();
    timeClient.setTimeOffset(7200);
    FireBase_init();
    Json_init();
    Serial.println(path);
    Serial.println(temp_path);
    Serial.println(hum_path);
    Serial.println(tim_path);
    Serial.println(String(millis()));




}

static bool measure_environment(float *temperature, float *humidity) {
    static unsigned long measurement_timestamp = millis();
    if (millis() - measurement_timestamp > 4000ul) {
        if (dht_sensor.measure(temperature, humidity)) {
            measurement_timestamp = millis();
            return (true);
        }
    }
    return (false);
}

void loop() {
    float temperature;
    float humidity;
    if (measure_environment(&temperature, &humidity)) {
        while(!timeClient.update()) {
            timeClient.forceUpdate();
        }
        String formattedDate = timeClient.getFormattedTime();
        String data =  formattedDate + "," + String(temperature, 1) + "," + String(humidity, 1);
        Serial.println(data);
        Tempreature_json.set("Value",temperature);
        Humidity_json.set("Value",humidity);
        DateTaken_json.set("Value",formattedDate);

        Firebase.updateNode(fbdo, temp_path , Tempreature_json);
        Firebase.updateNode(fbdo, hum_path , Humidity_json);
        Firebase.updateNode(fbdo, tim_path , DateTaken_json);


    }

    delay(4000ul);

}
