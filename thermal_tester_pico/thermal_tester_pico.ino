//Pico thermal resistance tester. Simple-Circuit 2023
volatile long analog_avg;    //global variables
volatile long analog_avg2;
volatile float watts[1000];
volatile float deltac[1000];
volatile unsigned long t;

void setup() {
 pinMode(15,OUTPUT);         //pin-15 drives the emitter follower
 digitalWrite(15,0);
 Serial.begin(460800);       //set your serial terminal to 460800 baud 
 analogReadResolution(12);   //set the ADC to read 3.3V/4096 = 0.81mV resolution
}

void readADC(void){
      analogRead(A0);
      analog_avg = 0;
      for (int j = 0; j<64; j++) analog_avg += analogRead(A0);  //average 64 readings to reduce noise
      analog_avg =  analog_avg >> 6;                          //shift to 12-bit value 
      analogRead(A1);
      analog_avg2 = 0;
      for (int j = 0; j<8; j++) analog_avg2 += analogRead(A1);  //average 8 readings to reduce noise
      analog_avg2 =  analog_avg2 >> 3;   
}


void loop() {
  char c;
  int i;
  float mw, ref, ref2, degc;
   c=' ';
   digitalWrite(15,0);                 //turn heating off
   while (c!='r'){                     //start the test when an 'r' is typed
     while (Serial.available() == 0);
     c = Serial.read();
   }  
     Serial.println("mw, delta_C");  
      readADC();
      ref2 = 0.403*analog_avg;       //diode cool reference
      digitalWrite(15,1);            //start heating
      delay(100);                    //heat for 100ms
      readADC();                     //measure the heating power
      mw = analog_avg*(analog_avg2-analog_avg)*0.0000433; 
      digitalWrite(15,0);           //drop the heating
      delay(5);                     //wait for system to settle
      readADC();                    //measure the diode voltage
      t = millis() + 100;           //initialize loop timer
      digitalWrite(15,1);           //turn on heating
      ref = analog_avg*0.403;       //save the diode measure starting point
      degc = ref - analog_avg*0.403;
      watts[0] = mw;                //log the first values
      deltac[0] = degc;
      Serial.print(mw);             //print the data
      Serial.print(", ");
      Serial.println(degc);             
            
     for (i=1; i<500; i++){      
      while (millis() < t);         //sample every 100ms
      t = t + 100;
      readADC();                    //measure heating power
      mw = analog_avg*(analog_avg2-analog_avg)*0.0000433;
      watts[i] = mw;
      digitalWrite(15,0);           //drop the power and mesure diode voltage 
      delay(5);
      readADC();
      digitalWrite(15,1);          //turn heat on
      degc = ref - analog_avg*0.403;
      deltac[i] = degc;
      watts[i] = mw;              //log the data
      Serial.print(mw);           //print the data
      Serial.print(", ");
      Serial.println(degc);
     }
     
     digitalWrite(15,0);          //all done, turn of heating
           
     for (i=500; i<1000; i++){     
      while (millis() < t); 
      t = t + 100;
      readADC();                 //read the diode voltage as it cools  
      mw = analog_avg*(analog_avg2-analog_avg)*0.000065;
      readADC();
      degc = ref2 - analog_avg*0.403;
      deltac[i] = degc;          //log the data
      watts[i] = mw;
      Serial.print(mw);          //print the data
      Serial.print(", ");
      Serial.println(degc);
     }
     mw = 0.0;                   //calculate the average power
     for (i=1; i<500; i++) mw = mw + watts[i];
     mw = mw / 525.26;
     degc = 1000.0 * deltac[499] / mw;   //calculate the thermal resistance
     Serial.print(degc,0);               //print it
     Serial.print("C/W, ");
     mw = (deltac[501] - deltac[999])*0.37 + deltac[999];
     for (i=501 ; i<999; i++){           //find the thermal time constant
      if (deltac[i]<=mw){
        Serial.print((i-501)*0.100,1);   //print it
        Serial.println("sec_TC");
        break;
      }
     }
}
