#include <Adafruit_NAU7802.h>
#include <Wire.h>
Adafruit_NAU7802 nau; //define the analog to digital unit as variable "nau"


void setup() {  
  Serial.begin(9600); // not really necessary for the actual product but always useful to include Serial for troubleshooting.
  
  Serial.println("NAU7802");
  

  if (! nau.begin()) {
    Serial.println("Failed to find NAU7802");
  }
  Serial.println("Found NAU7802");

  nau.setLDO(NAU7802_3V0); //set according to the NAU you purchase
  nau.setGain(NAU7802_GAIN_128); // higher gain more accurate measurement, for other selection check the example in Adafruit
  nau.setRate(NAU7802_RATE_10SPS);

  // Take 10 readings to flush out readings
  for (uint8_t i=0; i<10; i++) {
    while (! nau.available()) delay(1);
    nau.read();
  }

  while (! nau.calibrate(NAU7802_CALMOD_INTERNAL)) {
    Serial.println("Failed to calibrate internal offset, retrying!");
    delay(1000);
  }
  Serial.println("Calibrated internal offset");

  while (! nau.calibrate(NAU7802_CALMOD_OFFSET)) {
    Serial.println("Failed to calibrate system offset, retrying!");
    delay(1000);
  }
  Serial.println("Calibrated system offset");
}


//next are all the customizable variables some are used to display default settings or parameters
int nmax=100; // number of samples reading for average
int timer=10000; //time delay between steps
float known_average,zero_average, offset_value,known_weight=200.0,read_div_by_offset_average;
int calibrating=1;

void loop() {
  while (calibrating) {

  
  while (! nau.available()) {
    delay(1);
  }
  Serial.print("# of samples ");Serial.println(nmax);
  Serial.println("Step 1 starts, don't put anything on the scale");
  // step 1, don't put anything on the scale, the code will read average of reading without any weight put on
  zero_average=readaverage(nmax,1.0);
  Serial.print("Zero_average nau read ");Serial.print(zero_average);Serial.print(" ");
  Serial.print("Put known weight object (known weight =");Serial.print(known_weight);Serial.print("g) now. You have");Serial.print(timer/1000);Serial.println("s");  
  delay(timer); // wait 10s, for user to put known items on the scale
  // step 2, put a known weight and the code will read the average reading again
  Serial.println("Step 2 starts");
  known_average=readaverage(nmax,1.0);
  Serial.print("known_average nau read ");Serial.print(known_average);Serial.print(" ");
  offset_value=(known_average-zero_average)/known_weight;
  Serial.print("This is the offset values "); Serial.print(offset_value);
  Serial.println("transfer this number to the other grinder program later.");
  Serial.println("Next we test with the calculated offset values, you can put any other known weight and compare. If you leave it long enough you get average or just keep changing the item if you don't care about average reading");
  delay(timer);
  read_div_by_offset_average=readaverage(nmax,offset_value);
  Serial.print("average nau read/offset ");Serial.print(read_div_by_offset_average);Serial.print(" ");
  Serial.println("Another chance reading samples, after the reading reach 100 sample, the program will end, you just need to reset the microcontroller");
  delay(timer);
  read_div_by_offset_average=readaverage(nmax,offset_value);
  Serial.print("average nau read/offset ");Serial.print(read_div_by_offset_average);Serial.print(" ");
  Serial.print("Use this OFFSET value : ");Serial.println(offset_value);
  Serial.print("# of samples used to create average reading : ");Serial.println(nmax);
  Serial.print("Known weight entered in the code :  ");Serial.println(known_weight);
Serial.print("Known average read :  ");Serial.println(known_average);
Serial.print("Known zero read :  ");Serial.println(zero_average);

  calibrating=0;
} 
}

float readaverage (int nmax, float offset_value){
  int n=1;float valtotal=0.0,val;
  while (n<=nmax) {
    val=nau.read();
    valtotal=valtotal+(val/offset_value);
    Serial.print("Nau.read ");Serial.print(val/offset_value);Serial.print(" Sample# ");Serial.print(n);;Serial.print(" Total added ");Serial.println(valtotal);
    n=n+1;
    delay(500);
  }  
  return valtotal/nmax;
}
