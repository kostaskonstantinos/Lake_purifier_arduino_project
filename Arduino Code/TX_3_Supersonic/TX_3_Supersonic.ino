#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 14
#define DESTINATION_ADDRESS 5
RF22Router rf22(MY_ADDRESS);

#define echoPin A2 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin A3 //attach pin D3 Arduino to pin Trig of HC-SR04
#define LED_GREEN     8
#define LED_RED       7

long duration = 0; // variable for the duration of sound wave travel
int distance = 0; // variable for the distance measurement
int sensorVal = 0;
long randNumber;
boolean successful_packet = false;
int max_delay=3000;
unsigned long time = millis();

void setup() {
  Serial.begin(9600); // // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");

  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  
  if (!rf22.init())
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(434.0))
    Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM);
  //1,2,5,8,11,14,17,20 DBM
  //rf22.setModemConfig(RF22::OOK_Rb40Bw335  );
  rf22.setModemConfig(RF22::GFSK_Rb125Fd125);
  //modulation

  // Manually define the routes for this network
  rf22.addRouteTo(DESTINATION_ADDRESS, DESTINATION_ADDRESS);
  sensorVal = analogRead(echoPin);
  randomSeed(sensorVal);// (μία μόνο φορά μέσα στην setup)
}

void loop() {
    if (millis() - time > 5000U)
    { 
    
    // Clears the trigPin condition
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
    // Displays the distance on the Serial Monitor
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // send measurement
    sensorVal = distance;
    char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
    uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
    memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
    memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
    sprintf(data_read, "%d", sensorVal);
    data_read[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
    memcpy(data_send, data_read, RF22_ROUTER_MAX_MESSAGE_LEN);
  
    successful_packet = false;
    while (!successful_packet)
    {
  
      if (rf22.sendtoWait(data_send, sizeof(data_send), DESTINATION_ADDRESS) != RF22_ROUTER_ERROR_NONE)
      {
        Serial.println("sendtoWait failed");
        Blink(LED_RED, 50, 3); //blink LED 3 times, 50ms between blinks
        randNumber=random(100,max_delay);
        Serial.println(randNumber);
        delay(randNumber);
      }
      else
      {
        successful_packet = true;
        Serial.println("sendtoWait Succesful");
        Blink(LED_GREEN, 50, 3); //blink LED 3 times, 50ms between blinks
      }
    }
    time = millis();
  }
}

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
