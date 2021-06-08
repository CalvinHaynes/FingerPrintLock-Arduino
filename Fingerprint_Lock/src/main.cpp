#include <Arduino.h>

#include <SoftwareSerial.h>
#include <FPC1020.h>
#include <Servo.h>
#include <Wire.h>
#include <SPI.h>

extern unsigned char l_ucFPID;   //User ID
extern unsigned char rBuf[192];  //Receive return data

//UART interface init
#define sw_serial_rx_pin 2       //Connect this pin to TX on the FPC1020*
#define sw_serial_tx_pin 3       //Connect this pin to RX on the FPC1020

//create a Fingerprint object
SoftwareSerial swSerial(sw_serial_rx_pin, sw_serial_tx_pin);   //Fingerprint serial (RX, TX)
FPC1020 Finger(&swSerial);

//create servo object to control a servo
Servo chx_servo;  

//user_mode settings
USER_MODE user_mode = ADMINISTRATOR_MODE;


/*--------Function prototype--------*/
void SearchMode(USER_MODE user_mode);
void FingerAdministration(void);
void SystemRun(void);


void setup() 
{
  Serial.begin(115200);

  pinMode(4, INPUT);    //connect this pin to T on the FPC1020 (touch detection signal) 
  chx_servo.attach(9);  //attaches the servo on pin 9 to the servo object

  Serial.println("============== Fingerprint Lock by Calvin Haynes ================");
}


void loop()
{
  SearchMode(user_mode);
}


void FingerAdministration(void)
{
  unsigned int User_ID = 0;
  unsigned char incomingNub;
  // unsigned int  matchUserID = 0;
  unsigned char rtf = 0;

  while(1)
  {
    Serial.println("============== Menu ================");
    Serial.println("Add a New User ----------------- 1");
    Serial.println("Fingerprint Matching --------------- 2");
    Serial.println("Get User Number and Print All User ID ------ 3 ");
    Serial.println("Delete Assigned User --------- 4");
    Serial.println("Delete All User ---------- 5");
    Serial.println("============== End =================");
                                                                                        
    unsigned char  MODE = 0;
    
    while(Serial.available()<=0);
        
    MODE = Serial.read()-0x30;
        
    switch(MODE)
    {
      case 0:  // Null
        break;
          
      case 1:   // Fingerprint Input and Add a New User
          MODE = 0;
          User_ID = 0;
              
          Serial.println("Please input the new user ID (0 ~ 99).");

          while(Serial.available()<=0);
          delay(100);

          incomingNub = Serial.available();

          for(char i=incomingNub; i>=1; i--)
          {
            User_ID = User_ID + (Serial.read()-0x30)*pow(10,(i-1));
          }
              
          Serial.println("Add Fingerprint, please put your finger on the Fingerprint Sensor.");
          
          rtf = Finger.Enroll(User_ID);
              
          if(rtf == TRUE) 
          { 
            Serial.print("Success, your User ID is: "); 
            Serial.println( User_ID , DEC);
          }
          
          else if (rtf == FALSE) 
          {
            Serial.println("Failed, please try again.");
          }
          
          else if( rtf == ACK_USER_OCCUPIED)
          {
            Serial.println("Failed, this User ID alread exsits.");
          }
              
          else if( rtf == ACK_USER_EXIST)
          {
            Serial.println("Failed, this fingerprint alread exsits.");
          }
              
          delay(2000);

          break;
         
      case 2:  // Fingerprint Matching
        MODE = 0 ;                                   
        
        Serial.println("Match Fingerprint, please put your finger on the Sensor.");

        if( Finger.Search())
        {
          Serial.print("Success, your User ID is: "); 
          Serial.println( l_ucFPID, DEC);
        }
        
        else
        {
          Serial.println("Failed, please try again.");
        }							
        
        break;
         
      case 3:   // Print all user ID
        MODE = 0;

        if(Finger.PrintUserID())
        {
          Serial.print("Number of Fingerprint User is:"); 
          unsigned char UserNumb;
          UserNumb = (l_ucFPID-2)/3;
    
          Serial.println(UserNumb,DEC);
          Serial.println("Print all the User ID:"); 
          
          for(char i = 0; i < UserNumb; i++)
          {
            Serial.println(rBuf[12+i*3],DEC);
          }
        }
             
        else 
        {
          Serial.println("Print User ID Fail!");
        }
        
        delay(1000);
        
        break;
             
      case 4:   // Delete Assigned User ID
        MODE = 0;
        User_ID = 0;

        Serial.println("Please input the user ID(0 ~ 99) you want to delecte.");

        while(Serial.available()<=0);
        delay(100);

        incomingNub = Serial.available();
        
        for(char i=incomingNub; i>=1; i--)
        {
          User_ID = User_ID + (Serial.read()-0x30)*pow(10,(i-1));
        }
             
        if(Finger.Delete(User_ID)) 
        {
          Serial.println("Delete Fingerprint User Success!"); 
        }

        else
        {
          Serial.println("Delete Fingerprint User Fail!");
        }
        
        delay(1000);

        break;
         
      case 5:  // Delete All User ID
        MODE = 0;
        unsigned char DeleteFlag = 0;
            
        Serial.println("Delete All Users, Y/N ?");
            
        for(unsigned char i=200; i>0; i--)//wait response info
        {
          delay(20);

          if(Serial.available()>0)
          {
            DeleteFlag = Serial.read();
            break;
          }
        }
                
        if(DeleteFlag == 'Y'||'y')
        {
          if(Finger.Clear())
          {
            Serial.println("Delete All Fingerprint User Success!");
          }

          else
          {
            Serial.println("Delete All Fingerprint User Fail!");
          }
        }

        delay(500);

        break;
    }
  }
}


void SystemRun(void)
{
  // variable to store the servo position
  int pos = 0;  

  Serial.println("Match Fingerprint, please put your finger on the Sensor.");

  if(Finger.Search())
  {
    Serial.print("Success, your User ID is: "); 
    Serial.println(l_ucFPID, DEC);

    for(pos = 0 ;pos <= 180 ;pos ++)
    {
      chx_servo.write(pos);
      delay(15);
    }
    
    delay(60000);
    
    for(pos = 180 ;pos <= 0 ;pos --)
    {
      chx_servo.write(pos);
      delay(15);
    }
    
  }
  
  else
  {
    Serial.println("Failed, please try again.");
  }

  delay(1000);					
}


void SearchMode(USER_MODE user_mode)
{
    if (user_mode == DEFAULT_MODE)
    {
      SystemRun();
    }

    if(user_mode == ADMINISTRATOR_MODE)
    {
      FingerAdministration();
    }
}