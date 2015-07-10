#include <SPI.h>
#include "AD5781.h"
#include "constants.h"

char msg [110];
int ndx = 0; 
char receivedChars[100] = {'\0'};   // an array to store the received data
bool newData = false;
const char endMarker = '\n';
bool debug = false;

void setup()
{
    // Setup Serial Bus
    Serial.begin(115200);


    pinMode(DAC_RESET, OUTPUT); 
    digitalWrite(DAC_RESET,1); 

    pinMode(DAC_LDAC,  OUTPUT); 
    digitalWrite(DAC_LDAC,0); 

    pinMode(DAC_CLR,   OUTPUT); 
    digitalWrite(DAC_CLR,1); 

    for (int i=0; i<20; i++) {
        pinMode(SYNC[i], OUTPUT); 
        digitalWrite(SYNC[i],HIGH); 
    }

    for (int i=0; i<4; i++)
        pinMode(ADR[i], INPUT); 

    // Configure DAC Reset
    pinMode(DAC_RESET, OUTPUT);

    digitalWrite(DAC_RESET,HIGH);
    delayMicroseconds(1000000);
    digitalWrite(DAC_RESET,LOW);
    delayMicroseconds(1000000);
    digitalWrite(DAC_RESET,HIGH);
    delayMicroseconds(1000000);

    // Configure SPI
    SPI.begin();
    SPI.setDataMode(SPI_MODE1);
    SPI.setBitOrder(MSBFIRST);
    SPI.setClockDivider(21);

    // Set Resolution for analog Read and Write for Arduino Built-in DAC/ADC
    analogReadResolution(12);

    initialize_board();
}

void loop() 
{
    if (!newData)
        serialReadLine(); 
    if (newData) {
        processSerialString(); 
        newData=false; 
    }
}

void serialReadLine () 
{
    char rc; 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();
        if (rc!='\n') {
            receivedChars[ndx] = rc;
            ndx++;
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
            break; 
        }
    }
}

int processSerialString() 
{
    char* token=strtok(receivedChars, " ,");
    if (strstr(token, "id?")) {
      Serial.println("LB3 Piezo Driver:");
      Serial.println("  HID#P01, SID#P01");
      Serial.println("  Driver 1 to 4 are 18-bits (1 to 262144) from -10V to 10V");
      Serial.println("Commands:");
      Serial.println("  id?  #Identification");
      Serial.println("  rst  #Reset");
      Serial.println("  1 10 #Set driver #1 to step 10");
    }
    else if (strstr(token, "dbg")) {
      debug=1;
      Serial.println("Debug Mode On");
    }
    else if (strstr(token, "rst")) {
      Serial.println("Resetting");
      reset_board();
      Serial.println("Reset Complete");
    }
    else {
      int idac = strtol(token, NULL, 0);
      if (strstr(token, "?")) {
        Serial.print("Reading Driver ");
        Serial.print(idac);
        Serial.print("\n"); 
        Serial.println(AD5781::read(0x1,(idac+3)));
      }
      else {
        token=strtok(NULL, " ,");
        int dac_counts = strtol(token, NULL, 0); 
    
        if (idac>=1 && idac<=4 && dac_counts >=1 && dac_counts <= 262144) {
            Serial.print("Setting Driver "); 
            Serial.print(idac); 
            Serial.print(" to Step "); 
            Serial.print(dac_counts); 
            Serial.print("\n"); 
    
            AD5781::setDAC((dac_counts-1), (idac+3)); 
    
            Serial.print("Set Complete");
            if (debug == true) {
              Serial.print(" (");
              Serial.print(AD5781::read(0x1,(idac+3)));
              Serial.print(")");
            }
            Serial.print("\n");
        }
        else {
          Serial.println("Invalid Command");
        }
      }
    }
}

void initialize_board () {
    debug = false;
    AD5781::init();
    //AD5781_setDAC(0);      // Start
    //AD5781_setDAC(65535);  // 1st Quarter
    //AD5781_setDAC(131071); // 2nd Quarter
    //AD5781_setDAC(131072); // Zero Value 
    //AD5781_setDAC(196607); // 3rd Quarter
    //AD5781_setDAC(262143); // End
    delayMicroseconds(1000000);
    for (int i=0; i<20; i++)
    {
        AD5781::setDAC(131071, i);  // Zero Value
    }
}

void reset_board () {
    // Configure DAC Reset
    //pinMode(DAC_RESET, OUTPUT);

    digitalWrite(DAC_RESET,HIGH);
    delayMicroseconds(1000000);
    digitalWrite(DAC_RESET,LOW);
    delayMicroseconds(1000000);
    digitalWrite(DAC_RESET,HIGH);
    delayMicroseconds(1000000);

    // Configure SPI
    //SPI.begin();
    //SPI.setDataMode(SPI_MODE1);
    //SPI.setBitOrder(MSBFIRST);
    //SPI.setClockDivider(21);

    initialize_board();
}

