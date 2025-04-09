/////////turbidity/////////
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 12
#define DESTINATION_ADDRESS 5
RF22Router rf22(MY_ADDRESS);

//turbidity sensor
#define TurbiditySensorPin A2
#define LED_GREEN     8
#define LED_RED       7

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

long randNumber;
boolean successful_packet = false;
int max_delay=3000;
int sensorVal = 0;
unsigned long time = millis();


void setup() {
  Serial.begin(9600);
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Turbidity:");
  // turbidity setup
  pinMode(TurbiditySensorPin,INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  
  // tx setup
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
  sensorVal = analogRead(TurbiditySensorPin);
  randomSeed(sensorVal);// (μία μόνο φορά μέσα στην setup)
}

void loop() {
  if (millis()- time > 5000U)
  {
    //read sensor value
    sensorVal = analogRead(TurbiditySensorPin);// read the input on analog pin 2:
    //float voltage = sensorValue; // * (5.0 / 1024.0); // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
    Serial.print("Turbidity value: "); Serial.println(sensorVal); // print out the value you read:
    /////////////////////
    lcd.setCursor(10,0);
    lcd.print("     ");
    lcd.setCursor(10, 0);
    lcd.print(sensorVal);
    ////////////////////
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
          lcd.setCursor(0, 1);
          lcd.print("                ");
          lcd.setCursor(0, 1);
          lcd.print("Send failed");
        Blink(LED_RED, 50, 3); //blink LED 3 times, 50ms between blinks  
        randNumber=random(100,max_delay);
        Serial.println(randNumber);
        delay(randNumber);
      }
      else
      {
        successful_packet = true;
        Serial.println("sendtoWait Succesful");
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        lcd.print("Send Succesful");
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
