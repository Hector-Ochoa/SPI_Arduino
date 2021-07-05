 /*
  Datalogger used to record data from the TI PGA970EVM board.
The circuit:
  Using the SparkFun SD card Shield attached to SPI bus as follows:
    MOSI - pin 11
    MISO - pin 12
    CLK - pin 13
    SS - pin 8
  PGA970EVM Board
    SS - pin 4        - Requires Voltage Divider resistor values used are 4.7k and 5.6k
    MOSI - pin 11     - Requires Voltage Divider resistor values used are 4.7k and 5.6k
    MISO - pin 12
    CLK - pin 13      - Requires Voltage Divider resistor values used are 4.7k and 5.6k
  RED LED used for Error Messaging.
    LED - pin 1
LED Error Code:
  250ms ON, 250ms OFF         - No SD Card detected.
  2 Short Blinks, Long Off    - File Open Error, Requires Restart.
*/

//Pin Configs
#define SD_SS_PIN 8                         //Need to check the value on the shield
#define PGA970_SS_PIN 4                     //Randomally picked this value
#define RED_ERROR_LED 3

//Configure Settings for PGA970
#define PGA970_SPI_SPEED 1000000
#define PGA970_SPI_ORDER MSBFIRST
#define PGA970_SPI_MODE SPI_MODE0
#define PGA970_DATA_SIZE 3                  //Number of bytes to be transferred at a time.

//PGA970 Commands
#define PGA970_CMD_READ_BUFF 0x008000       //Read the PGA970 SPI Buffer                0b000 00000100 0 XXXXXXXX 0000
#define PGA970_CMD_READ_STATUS 0x00A000     //Read the PGA970 SPI Status                0b000 00000101 0 XXXXXXXX 0000
#define PGA970_CMD_WRITE_BUFF 0x011000      //Write to the PGA970 SPI Buffer
#define PGA970_CMD_START_TX 0x011AA0        //Command to load data to SPI Buffer        0b000 00001000 1 10101010 0000

//SD Card Configs
#define FILE_NAME "Data.txt"

//Debug Mode comment out define statement to remove debug mode.
#define DEBUG_MODE

#include <SPI.h>
#include <SD.h>

SPISettings PGA970Settings(PGA970_SPI_SPEED, PGA970_SPI_ORDER, PGA970_SPI_MODE);
char PGA970Buffer[PGA970_DATA_SIZE];
char dataString[17];                   //Extra for Null terminator and the '.' or voltage


void setup() {
  pinMode(RED_ERROR_LED,OUTPUT);
  
#ifndef DEBUG_MODE 
  while(!SD.begin(SD_SS_PIN)){
    digitalWrite(RED_ERROR_LED, HIGH);
    delay(250);
    digitalWrite(RED_ERROR_LED, LOW);
    delay(250);
  }
#endif

#ifdef DEBUG_MODE
  Serial.begin(9600);
#endif

//The final value in the char array MUST be Null to serve as a Null Terminator so the array can be easily converted to a String object. This can be set here, to save time, because it should NEVER be changed.
  dataString[16] = 0x00;
  pinMode(PGA970_SS_PIN,OUTPUT);
  digitalWrite(PGA970_SS_PIN, HIGH);  
  SPI.begin();     
}

void loop(){
  PGA970Buffer[0]= PGA970_CMD_START_TX;
  PGA970Buffer[1]= PGA970_CMD_START_TX >> 8;
  PGA970Buffer[2]= PGA970_CMD_START_TX >> 16;           //Load command into the buffer to be sent. The buffer is a char array, bytes beyond the first byte should be ignored.
  
  
  SPI.beginTransaction(PGA970Settings);
  digitalWrite(PGA970_SS_PIN, LOW);                     //SPI is active low Slave Select.
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);
  //Serial.println("hello");
//Ignore received Data, it will be dummy data.

  PollStatus();

  PGA970Buffer[0]= PGA970_CMD_READ_BUFF;                //Send command to continue to read the buffer. Might need to poll COM_TX_STATUS again. Received data will be first set of real data.
  PGA970Buffer[1]= PGA970_CMD_READ_BUFF >> 8;
  PGA970Buffer[2]= PGA970_CMD_READ_BUFF >> 16;
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);

  dataString[0] = PGA970Buffer[1];                      //This appears to be Noob code, however the code to insert the necessary ASCII characters into the string would balloon the code and slow it down.
  dataString[1] = '.';                                  //There is plenty of program memory and speed in more important.
  dataString[2] = PGA970Buffer[2];

  PollStatus();

  PGA970Buffer[0]= PGA970_CMD_READ_BUFF;                
  PGA970Buffer[1]= PGA970_CMD_READ_BUFF >> 8;
  PGA970Buffer[2]= PGA970_CMD_READ_BUFF >> 16;
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);

  dataString[3] = PGA970Buffer[1];                                                        
  dataString[4] = PGA970Buffer[2];

  PollStatus();

  PGA970Buffer[0]= PGA970_CMD_READ_BUFF;                //Send command to continue to read the buffer. Data will now be the temperature.
  PGA970Buffer[1]= PGA970_CMD_READ_BUFF >> 8;
  PGA970Buffer[2]= PGA970_CMD_READ_BUFF >> 16;
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);

  dataString[5] = PGA970Buffer[1];                                                        
  dataString[6] = PGA970Buffer[2];

//The 8 Characters of the LVDT Voltage have been read.
  dataString[8] = ',';

  PollStatus();

  PGA970Buffer[0]= PGA970_CMD_READ_BUFF;                
  PGA970Buffer[1]= PGA970_CMD_READ_BUFF >> 8;
  PGA970Buffer[2]= PGA970_CMD_READ_BUFF >> 16;
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);

  dataString[9] = PGA970Buffer[1];
  dataString[10] = '.';                                                      
  dataString[11] = PGA970Buffer[2];

  PollStatus();

  PGA970Buffer[0]= PGA970_CMD_READ_BUFF;                
  PGA970Buffer[1]= PGA970_CMD_READ_BUFF >> 8;
  PGA970Buffer[2]= PGA970_CMD_READ_BUFF >> 16;
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);

  dataString[12] = PGA970Buffer[1];
  dataString[13] = PGA970Buffer[2];

  PollStatus();

  PGA970Buffer[0]= PGA970_CMD_READ_BUFF;                //Sending Dummy command to read the buffer.
  PGA970Buffer[1]= PGA970_CMD_READ_BUFF >> 8;
  PGA970Buffer[2]= PGA970_CMD_READ_BUFF >> 16;
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);

  dataString[14] = PGA970Buffer[1];                                                        
  dataString[15] = PGA970Buffer[2];
  
#ifdef DEBUG_MODE
  Serial.println(String(dataString));
#endif

#ifndef DEBUG_MODE  
  File dataFile = SD.open(FILE_NAME, FILE_WRITE);
  if (dataFile) {
    dataFile.println(String(dataString));
    dataFile.close();
  }
  else {
    while(1){
      digitalWrite(RED_ERROR_LED, HIGH);
      delay(150);
      digitalWrite(RED_ERROR_LED, LOW);
      delay(150);
      digitalWrite(RED_ERROR_LED, HIGH);
      delay(150);
      digitalWrite(RED_ERROR_LED, LOW);
      delay(500);
    }
  }
#endif
}

void PollStatus(){
  do{
    PGA970Buffer[0]= PGA970_CMD_READ_STATUS;
    PGA970Buffer[1]= PGA970_CMD_READ_STATUS >> 8;
    PGA970Buffer[2]= PGA970_CMD_READ_STATUS >> 16;
    SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);
#ifdef DEBUG_MODE
      Serial.println(PGA970Buffer[1]);
#endif
  }while(PGA970Buffer[1] !=0x01);                       //Polling COM_TXRDY bit of COM_TX_STATUS Register
 
  PGA970Buffer[0]= PGA970_CMD_READ_BUFF;                //Sending command to read data results will be the COM_TX_STATUS again, so ignore.
  PGA970Buffer[1]= PGA970_CMD_READ_BUFF >> 8;
  PGA970Buffer[2]= PGA970_CMD_READ_BUFF >> 16;
  SPI.transfer(&PGA970Buffer,PGA970_DATA_SIZE);
//Ignore received Data, it will be dummy data.
}

