#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include "fauxmoESP.h"

#define FOOD_RELEASE_PIN        18
#define BALL_RELEASE_PIN        21
#define BALL_SENSOR_PIN         19
#define MOTOR_CONTROL_PIN       5

#define FOOD_OPEN               150
#define FOOD_CLOSED             0

/*
 * #define WIFI_SSID ""
 * #define WIFI_PASS ""
 */
#define WIFI_SSID ""
#define WIFI_PASS ""

#define DEVICE_ID               "Ball Thrower"

bool enable_game;
bool launch_ball;

fauxmoESP fauxmo;
Servo myservo;

int treat_count;

void wifiSetup() {

    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);
    
    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    
    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();
    
    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());

}

void setup() {

    Serial.begin(115200);
    
    pinMode(MOTOR_CONTROL_PIN, OUTPUT);
    pinMode(BALL_RELEASE_PIN, OUTPUT);
    pinMode(BALL_SENSOR_PIN, INPUT_PULLUP);
    
    digitalWrite(BALL_RELEASE_PIN, LOW);
    digitalWrite(MOTOR_CONTROL_PIN, LOW);

    enable_game = false;
    launch_ball = false;
    treat_count = 3;
    
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    
    myservo.setPeriodHertz(50);
    myservo.attach(FOOD_RELEASE_PIN, 1000, 2000);
    
    myservo.write(FOOD_CLOSED);

    wifiSetup();

    fauxmo.createServer(true);
    fauxmo.setPort(80);
    fauxmo.enable(true);
    fauxmo.addDevice(DEVICE_ID);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");

        if (strcmp(device_name, DEVICE_ID)==0) {
            if(state) {
                enable_game = true;
                launch_ball = true;
            } else {
                enable_game = false;
                launch_ball = false;
                
                digitalWrite(BALL_RELEASE_PIN, LOW);
                digitalWrite(MOTOR_CONTROL_PIN, LOW);
            }
        }

    });
    
    Serial.println(" INITIALISED");
}

void loop() {
  
    fauxmo.handle();

    if(enable_game) {
        if(launch_ball) {
            launch_ball = false;

            digitalWrite(MOTOR_CONTROL_PIN, HIGH);
            delay(2000);
            
            Serial.println(" BALL RELEASE");
            digitalWrite(BALL_RELEASE_PIN, HIGH);
            delay(500);
            digitalWrite(BALL_RELEASE_PIN, LOW);
            Serial.println(" BALL RELEASE END");
            
            delay(500);
            digitalWrite(MOTOR_CONTROL_PIN, LOW);
        }

        if(!digitalRead(BALL_SENSOR_PIN)) {
            
            Serial.println(" BALL SENSOR TRIGGERED");

            if(treat_count == 3)
            {
                treat_count = 0;
                Serial.println(" GIVE TREAT");

                delay(1000);
                myservo.write(FOOD_OPEN);
                delay(210); 
                myservo.write(FOOD_CLOSED);
                delay(3000); 
            }
            
            treat_count++;
            launch_ball = true;
        }
    }
}
