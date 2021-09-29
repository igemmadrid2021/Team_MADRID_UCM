/**
* #########################
* # Humbold University zu Berlin
* + iGEM 2019 on Chlaydomonas cultivation
* MODIFIED BY
* # Complutense University of Madrid
* + iGEM 2021 : 4C_Fuels Project
*
* #########################
* # @version  0.2
* # @platform  arduino AT Mega 2560 
* # @clock  16mHz
* # @created  20190614
* # @changed  20210530
* # @user fmjorge99
      
* ############CODE OVERVIEW (Top -> Down Compiling) #############
*
* # Variable Declaration
* # Basic Functions definition (void)
* # Initialization (Setup)
*     - Starting Serial Port
*     - Arduino pin selction (Input / Output)
*     - Testing LEDs (ON / OFF)
* # Main Program
*     - Program Functions Definition 
*     - Execution of Looping code (void loop())
*   
*  The code so far just works as a turbidostat to grow cells at constant OD680 / OD720.
*  OD measurements are taken and sent via serial port to the computer every X milliseconds defined in the "Interval" Variable.
*  
* #########################
*/

//#####################################################################
// general import declaration
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


//#####################################################################

// PCB Wiring Analog Inputs from Sensors to Arduino

int sensorPinCurrent680  = A1;    // current used via shunt in mV/

int sensorPinCurrent720  = A3;    // current used via shunt in mV/

// Temp. Sensor DS18B20 in PWM 2

// PCB Wiring to Arduino (OUTPUTS)
 
 int redPin   = 3;
 int greenPin = 4;
 int bluePin  = 7;
 int whitePin = 6;
 int pumpIn = 11;
 int pumpOut = 9;

 int airValve = 8;
 int sensorPinTempSense = 2;

 int LED680 = 12;
 int LED720  = 13;

 int Heater = 10;

//#####################################################################

//  VARIABLES (To perform calculations)

int analogRead680 = 0;     // opt101 current value

int analogRead720 = 0;     // opt101 current value

int inByte = 0;         // incoming serial byte

int cycleNumber = 0; // number of measurments

float temp = 0; // temperature record as a float variable for decimals
float OD680 = 0;
float OD720 = 0;

int brightnessWhite = 0; // value between 0 und 255 – for intensity of white LEDs
int brightnessRed = 0; // value between 0 und 255 – for intensity of red LEDs
int brightnessGreen = 0; // value between 0 und 255 – for intensity of green LEDs
int brightnessBlue = 0; // value between 0 und 255 – for intensity of blue LEDs

int airValveStatus = 0; // Initial position of Air Control Solenoid

//#####################################################################

// member variables - timing

unsigned long previousMillis = 0;   // will store last time
const long interval = 300000;   // 5 minutes x 60 sec * 1000 milliseconds 
const int pumpTime = 4000; // 

const int T_value = 37;  // Desired value of Temperature to control -> CHANGE THE TEMPERATURE SETPOINT HERE
const int T_tol = 0.2; // Tolerance for temperature T_value ± T_tol is admitted -> CHANGE ONLY TO ADJUST THE Temp control Behavior

/* Parameters for OD Measurement. Obtained After Opt101 Calibration 
REPLACE THIS VALUES BY YOUR OWN ONES. 
OD Calculation Equation: OD = A*LN[analogRead(mv)]+B */

const float OD680_A = -196.1;
const float OD680_B = 21.083;
const float OD720_A = 0;
const float OD720_B = 0;


//########################## FUNCTION DEFINITION ###########################################

/*AUXILIARY FUNCTIONS
Modify the setup code with a call to the following functions in order to perform maintenance or setup tasks. */

void fill_chamber() {
  analogWrite(pumpIn,250);
  delay(90000);
  analogWrite(pumpIn,0);
  }

void empty_chamber() {
  analogWrite(pumpOut,250);
  delay(90000);
  analogWrite(pumpOut,0);
  }

void pipe_cleaning() {
 analogWrite(pumpIn,250);
 delay(80000);
 analogWrite(pumpOut,250);
 delay(180000);
 analogWrite(pumpIn,0);
 analogWrite(pumpOut,250);
}

/**
############LIGHT FUNCTIONS######################*/

/* shift the value to the external module ( MOSFET )*/
void setColor() {
  analogWrite(redPin, brightnessRed);
  analogWrite(greenPin,brightnessGreen);
  analogWrite(bluePin, brightnessBlue);
  analogWrite(whitePin, brightnessWhite);
  
  analogWrite(airValve, airValveStatus);
  
  delay(100);
}

/* set white, red and blue to 255 and green to 100; */

 void setAllLLightOn() {
    brightnessWhite = 255;
    brightnessRed = 255;
    brightnessGreen = 100;
    brightnessBlue = 255;
    airValveStatus = 0;
    setColor();
    delay(100);
 }

 /**
  * set white, red ,blue & green to 0;
  */
  
  void setLightOff() {
     brightnessWhite = 0;
     brightnessRed = 0;
     brightnessGreen = 0;
     brightnessBlue = 0;
     airValveStatus = 0;
     setColor();
     delay(100);
     
  }


/**
 * set white, red and blue to your wished intensity and color
 MODIFY HERE THE PARAMETERS */
 
 void setGrowLight() {
    brightnessWhite = 245;
    brightnessRed = 255;
    brightnessGreen = 50;
    brightnessBlue = 255;
    
    airValveStatus = 255;
    setColor();
 }


/**
############SYSTEM TEST FUNCTION###############*/

void test_PBR() {

// LED Lights Check

// led check, on and off
  Serial.println("running all led to ON");
  setAllLLightOn();
  delay(2000);
  Serial.println("running all led to OFF");
  setLightOff();
  delay(2000);

  // COLOR CHECK
  Serial.println("RED");
  analogWrite(redPin,255);
  delay(2000);
  analogWrite(redPin,0);
  Serial.println("GREEN");

  analogWrite(greenPin,255);
  delay(2000);
  analogWrite(greenPin,0);
  Serial.println("BLUE");

  analogWrite(bluePin,255);
  delay(2000);
  analogWrite(bluePin,0);
  Serial.println("WHITE");

  analogWrite(whitePin,255);
  delay(2000);
  Serial.println("running all led to OFF");
  setLightOff();
  delay(2000);

  // PUMPS CHECK

  Serial.println("Switching ON INPUT PUMP");
  Serial.println("");
  analogWrite(pumpIn,255);
  delay(4000);
  analogWrite(pumpIn,0);
  Serial.println("");
  Serial.println("Switching ON INPUT PUMP");
  analogWrite(pumpOut,255);
  delay(4000);
  analogWrite(pumpOut,0);

  // OD_Measurement

          setLightOff(); // Lights could be switched Off for routine OD measurements... (not-serial printed)
        
         // put ir led on

           Serial.println("Switching IR LEDS ON");
          
           delay(1000);
           analogWrite(LED680,255);
           analogWrite(LED720,255);
           delay(1000);
           Serial.println("Aeration OFF");
           
           analogWrite(airValve,0);
           delay(4000);    // give the sensor some time, to swing in
        
         // measure density with opt101 sensor 

           Serial.println("OD Measurement (mV)");
           Serial.println("680 Read");
           Serial.println(analogRead(sensorPinCurrent680));
           Serial.println("OD_680");
           OD680=(OD680_A*log(analogRead(sensorPinCurrent680))+OD680_B);
           Serial.println(OD680);
           Serial.println("720 Read");
           OD720=(OD720_A*log(analogRead(sensorPinCurrent720))+OD720_B);
           Serial.println(OD720);
        
         // switch ir led OFF
           Serial.println("Switching IR LEDS OFF");
           Serial.println("");
           analogWrite(LED680,0);
           analogWrite(LED720,0); 
        
           delay(1000);   // wait, till measruing is done
           Serial.println("Aeration ON");
           Serial.println("");
           analogWrite(airValve,255);
           setGrowLight();

   // Temp_Measurement
   measure_T();
   Serial.println(temp);
   Serial.println("");
   
}

/**
* prepare init variables and INPUT/OUTPUT procedure 
*/
 
//#################### SETUP (Running only once) ####################


void setup() {  

//Opening Aeration
analogWrite(airValve,255);

// gpio - input // Arduino pin selction 

  pinMode(sensorPinCurrent680, INPUT); // analog1

  pinMode(sensorPinCurrent720, INPUT); // analog3

  pinMode(sensorPinTempSense, INPUT); 

  // gpio - output
  
  pinMode(whitePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  pinMode(pumpIn, OUTPUT);
  pinMode(pumpOut, OUTPUT);

  pinMode(airValve, OUTPUT);

  pinMode(LED680, OUTPUT);
  pinMode(LED720, OUTPUT);

  pinMode(Heater, OUTPUT);

  // Serial initialization for monitoring on a computer 
  
  Serial.begin(9600);
  while (!Serial) {
    ; // getReadyStateLike
  }

  delay(2222);  // time to get setup done 
  
  if (Serial.available() == 0 && Serial.availableForWrite() > 0) { // ready for serial output
    //Serial.available()
    Serial.print("SERIAL["); 
    Serial.print(Serial.available()); 
    Serial.print("|");
    Serial.print(Serial.availableForWrite());
    Serial.print("|");
    Serial.print(Serial.getTimeout());
    Serial.println("]");
    delay(Serial.getTimeout()*1.41);

  }

//####################

  // Setup log header
  
  Serial.println("INIT[CycleTime (ms),OD_680,OD_720,Temp.]");  

  Serial.print("INTERVAL (ms),");// first line in log for interval
  Serial.print(interval);
  Serial.println(",");
  
  Serial.print("INPUTPORT["); 
  Serial.print(millis());
  Serial.print(",");

  Serial.print(OD680); //680 Read
  Serial.print(",");
 
  Serial.print(OD720); //720 Read
  Serial.print(",");

  measure_T();
  Serial.print(temp); //
  Serial.println("]"); 

  

Serial.print("UPDATING_every (ms) [");
Serial.print(interval);
Serial.println("]");

  Serial.println("SETUP[done]");

  Serial.println("DATA_LOG[CycleNº,OD_680,OD_720,Temp.]"); 

 // 
}



//#####################################################################
// PROGRAM TO EXECUTE STARTS HERE
//#####################################################################


/** * read values from wired inputs and send data via serial to host computer, for logging of values */


/*#PROGRAM FUNCTIONS DEFINITION#*/

/* ############## OD Measurement with IR LED + Opt101 ################### */
  
void measureDensity(){

  setLightOff(); // Lights could be switched Off for routine OD measurements... (not-serial printed)

 // put ir led on
    
   analogWrite(LED680,255);
   analogWrite(LED720,255);
   analogWrite(airValve,0);
   delay(1000);    // give the sensor some time, to swing in

 // measure density with opt101 sensor 
     
   analogRead680 = analogRead(sensorPinCurrent680);
   analogRead720 = analogRead(sensorPinCurrent720);
   OD680=(OD680_A*log(analogRead680)+OD680_B);
   OD720=(OD720_A*log(analogRead720)+OD720_B);

 // switch ir led OFF

   analogWrite(LED680,0);
   analogWrite(LED720,0); 

   delay(1000);   // wait, till measruing is done
     
 
}

/* ############## Turbidostat Pump Control ################### */

void checkDensityAndPump(){

  setGrowLight();
  //Serial.print("analogRead680: ");
  //Serial.println(analogRead680);
  
  if ( OD680 <= 1.2 ) {  // ADJUST THE OD680 to the desired value for the turbidostat.
    
    Serial.println("pumping chamber  IN/OUT");
    // stop cycle and pump medium out
    // when medium is exported, refill with fresh medium
    
    digitalWrite(pumpOut, HIGH);
    digitalWrite(pumpIn, HIGH);
    delay(pumpTime); 
    digitalWrite(pumpIn, LOW);
    digitalWrite(pumpOut, LOW);
    delay(4000);    // wait for some time
       
  }
}

// Temperature measurement // 

void measure_T() {
  analogWrite(Heater,0);
  sensors.requestTemperatures(); // Send the command to get temperatures
  sensors.getTempCByIndex(0); // Display last recorded temperature in ºC
  temp = sensors.getTempCByIndex(0); // Display last recorded temperature in ºC)
  temperature_control();
  }

// Sending Measurements to the computer via Serial // 

void doTheLogging() { //logging of sensor values
    
    // get incoming byte:
   
    inByte = Serial.read(); // start serial monitor
    
    //#####################################################################  
    
    measure_T();
    measureDensity();
    //checkDensityAndPump();
    
    //#####################################################################
  
    // send sensor values:
   
    Serial.print(">DATA: [");
    Serial.print(cycleNumber);
    Serial.print(",");
    
    Serial.print(OD680);
    Serial.print(",");
    Serial.print(OD720);
    Serial.print(",");
    Serial.print(temp); // temp senseor
    Serial.println("]");
    
    cycleNumber = cycleNumber+1; // number of measurments  +1
  } 
  
// Checking if the interval between Serial communicated measurements has been reached // 

void checkTimeAndLog(){ //
  
  unsigned long currentMillis = millis(); // get runtime since setup()
  if ((currentMillis - previousMillis) >= interval) { // If the elapsed time exceeds the specified interval for serial communication...
 
    
    previousMillis = currentMillis; // save current time
    delay(500);
    doTheLogging();  // Execute readings and Serial Print of Values
    delay(500);
  }
  
  else {
    setGrowLight();
  }
 }

/* SIMPLE ON / OFF Temperature Control */

/* !! IMPORTANT: Heater values can be adjusted depending on the length and resistance of your heating element */ 

void temperature_control(){

analogWrite(Heater, 0);
  sensors.requestTemperatures(); // Send the command to get temperatures
  sensors.getTempCByIndex(0); // Display last recorded temperature in ºC
  temp = sensors.getTempCByIndex(0); // Display last recorded temperature in ºC)
if (temp >= (T_value+T_tol)) { 
 
 analogWrite(Heater, 0);      
  }
  else {
          if (temp > T_value) { 
            analogWrite(Heater, 130); 
            }
          else {

            // If temperature is close to the setpoint heating becomes milder
            
                  if (temp >= (T_value-T_tol)) { 
                   
                     analogWrite(Heater, 50); 
                }
               else {
               
               analogWrite(Heater, 160); 
        }
        }
  }
  }

/** LOOPING PROGRAM HERE. All the used methods has been declared before*/

void loop() {
  setGrowLight(); // GrowLights and Aeration are switched On  if lights has been previously switched off this will restore lights to ON.
  checkTimeAndLog();  // If measurement interval is reached, OD-check is performed and values are printed.
  temperature_control(); // Temperature is permanently controlled in each loop iteration.
  
}
