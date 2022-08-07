#include  <ESP8266WiFi.h>
#include  "FirebaseArduino.h"
#include  <ArduinoJson.h>

/* Define variable of Firebase and WiFi */
#define   FIREBASE_HOST "air-quality-tracking-3709a-default-rtdb.firebaseio.com" 
#define   FIREBASE_AUTH "C4qM5IhcqzdCbHG5JbyJUVh43XmCvrJFHSclcQML"   //Không dùng xác thực nên không đổi
#define   WIFI_SSID     "DPEE"   //Thay wifi và mật khẩu
#define   WIFI_PASSWORD "cbvldt2020"

FirebaseData firebaseData;
FirebaseData ledData;
FirebaseJson json;

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
void loop() {
  float randomNumber = random(500);
  Serial.println(randomNumber);
  
  String number = Firebase.pushFloat("Value", randomNumber);
  if(Firebase.failed()){
    Serial.println(Firebase.error());
  }
  delay(1000);


//  String number = String(randomNumber);
//  Firebase.pushString("value", number);
//  if (Firebase.failed()) 
//    {
// 
//      Serial.print("pushing failed:");
//      Serial.println(Firebase.error()); 
//      return;
//  }
//  delay(1000);

  
//  int i;
//  for(int i=0; i< 100; i++){
//     Firebase.setFloat("value: ", i);
//  }
//  if (Firebase.failed()) {
//    Serial.print("setting /number failed:");
//    Serial.println(Firebase.error());
//    return;
//  }
//  Serial.println(i);
//  delay(200);
}
