#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <RF22.h>
#include <RF22Router.h>

#define MY_ADDRESS 13
#define DESTINATION_ADDRESS 5
RF22Router rf22(MY_ADDRESS);
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows

//tds sensor
#define TdsSensorPin A1
#define LED_GREEN     8
#define LED_RED       7
#define VREF 5.0             // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;


int sensorVal = 0;
long randNumber;
boolean successful_packet = false;
int max_delay=3000;
unsigned long time = millis();


void setup() {
  Serial.begin(9600);
  //lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("TDS:");
   // tds setup
  pinMode(TdsSensorPin,INPUT);
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
  sensorVal = analogRead(TdsSensorPin);
  randomSeed(sensorVal);// (μία μόνο φορά μέσα στην setup)
}

void loop() {
  
   //store multiple measurements in array to compute average value
   static unsigned long analogSampleTimepoint = millis();
   if(millis() - analogSampleTimepoint > 240U)     //every 240 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }
   
   // calculate and send final measurement   
   if(millis() - time > 4800U)
   {
      // calculate final tdsValue
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");

      // send measurement
   
      sensorVal = tdsValue;
      char data_read[RF22_ROUTER_MAX_MESSAGE_LEN];
      uint8_t data_send[RF22_ROUTER_MAX_MESSAGE_LEN];
      memset(data_read, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
      memset(data_send, '\0', RF22_ROUTER_MAX_MESSAGE_LEN);
      sprintf(data_read, "%d", sensorVal);
      data_read[RF22_ROUTER_MAX_MESSAGE_LEN - 1] = '\0';
      memcpy(data_send, data_read, RF22_ROUTER_MAX_MESSAGE_LEN);
    /////////////////////
    lcd.setCursor(5,0);
    lcd.print("     ");
    lcd.setCursor(5, 0);
    lcd.print(sensorVal);
  ////////////////////
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
          Serial.println("send Succesful");
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

int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
