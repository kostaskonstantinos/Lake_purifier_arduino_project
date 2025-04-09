#include <RF22.h>
#include <RF22Router.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
#include <Wire.h> 

#define NODE_ADDRESS_1 12
#define NODE_ADDRESS_2 13
#define NODE_ADDRESS_3 14

#define MY_ADDRESS 5
RF22Router rf22(MY_ADDRESS);

#define LED_YELLOW     7

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

void setup() {
  Serial.begin(9600);
  pinMode(LED_YELLOW, OUTPUT);
  
  if (!rf22.init())
    Serial.println("RF22 init failed");
  // Defaults after init are 434.0MHz, 0.05MHz AFC pull-in, modulation FSK_Rb2_4Fd36
  if (!rf22.setFrequency(434.0))
    Serial.println("setFrequency Fail");
  rf22.setTxPower(RF22_TXPOW_20DBM);
  //1,2,5,8,11,14,17,20 DBM
  rf22.setModemConfig(RF22::GFSK_Rb125Fd125);
  //modulation
  // Manually define the routes for this network
  rf22.addRouteTo(NODE_ADDRESS_1, NODE_ADDRESS_1);
  rf22.addRouteTo(NODE_ADDRESS_2, NODE_ADDRESS_2);
  rf22.addRouteTo(NODE_ADDRESS_3, NODE_ADDRESS_3);
  
  // monitor
  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("TURB"); 
  lcd.setCursor(6, 0);
  lcd.print("TDS");
  lcd.setCursor(11, 0);
  lcd.print("TRASH");
}

void loop() {
  uint8_t buf[RF22_ROUTER_MAX_MESSAGE_LEN];
  char incoming[RF22_ROUTER_MAX_MESSAGE_LEN];
  memset(buf, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  memset(incoming, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
  uint8_t len = sizeof(buf);
  uint8_t from;
  int received_value = 0;


  
  if (rf22.recvfromAck(buf, &len, &from))
  {
    buf[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
    memcpy(incoming, buf, RF22_ROUTER_MAX_MESSAGE_LEN);
    Serial.print("got request from : ");
    Serial.println(from, DEC);
    received_value = atoi((char*)incoming);
    Serial.println(received_value);
    Blink(LED_YELLOW, 50, 3); //blink LED 3 times, 50ms between blinks
    if(from==12)
    {
      lcd.setCursor(0, 1);
      lcd.print("     ");
      lcd.setCursor(0, 1);
      lcd.print(received_value);
    }
     if(from==13)
    {
      lcd.setCursor(6, 1);
      lcd.print("     ");
      lcd.setCursor(6, 1);
      lcd.print(received_value);
    }
     if(from==14)
    {
      lcd.setCursor(11, 1);
      lcd.print("     ");
      lcd.setCursor(11, 1);
      if(received_value <= 20)
        lcd.print("Oops!");
      else
        lcd.print("OK!");
    }
 //   delay(1000);
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
