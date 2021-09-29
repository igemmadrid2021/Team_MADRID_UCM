
int pumpIn = 11;
int pumpOut = 9;

int LED680 = 6;
int LED720  = 7
;


void fill_chamber(){
  Serial.print("Filling Chamber for 20 seconds"); 
  analogWrite(pumpIn,255);
  delay(75000);
  digitalWrite(pumpIn,LOW);
  }

void dilution(){
  Serial.println("Pumping IN/OUT Chamber for 10 seconds TAKE SAMPLE FIRST!!!!!"); 
  delay(3000);
  analogWrite(pumpOut,255);
  delay(3000);
  analogWrite(pumpIn,255);
  delay(10000);
  
  analogWrite(pumpOut,0);
  analogWrite(pumpIn,0);
  }

void OD_measure(){
   digitalWrite(LED680,HIGH);
     digitalWrite(LED720,HIGH);

     Serial.println("");
     Serial.println("Performing 5 Measurements");
     for (int i = 0; i <= 5; i++) {
     Serial.print("OD680 = ");
     Serial.print(analogRead(A1));
     Serial.print("|");
     Serial.print("OD720 =  ");
     Serial.print(analogRead(A3));
     Serial.println(" "); 
     delay(1500);
  }

     digitalWrite(LED680,LOW);
     digitalWrite(LED720,LOW);
     
  }
 

void setup() {
  // put your setup code here, to run once:


//* USE THE FOLLOWING CODE FOR THE INITIAL CHAMBER FILLING.

/*
Serial.print("Filling Chamber for 20 seconds"); 
fill_chamber() 
*//


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
    pinMode(A1,INPUT);
    pinMode(A3,INPUT);
    
  
}}



void loop() {
  // put your main code here, to run repeatedly:

    // OD_measure()
    
     Serial.println("3 seconds... for dilution");
     delay(3000);
     dilution();    
      }
