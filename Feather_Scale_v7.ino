#include <Adafruit_NAU7802.h> //v1.0.8
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h> //1.12.4
#include <Adafruit_ST7789.h> //v1.11.0
#include "RTClib.h" //v2.1.4
//#include <SD.h> //v1.3.0
#include "SdFat.h" //v2.3.0, I am not sure if this is the same version used in Feather Pressure profile v5
#include "Adafruit_MAX1704X.h" //v1.0.3

/* v0; initial attempt. This is a copy of v15 of the Feather Grinder but instead of using M4, using ESP32S3, 2/24/2026.
version tracking. Arduino 2.3.4
Board support file esp32 by Espressif Systems 2.0.17
v1. wiring Adalogger wing completed, and adding program to read RTC and SD. Looks like SdFat.h was used instead of SD.h
when I was last use the ESP32 with Adalogger, so I will update this accordingly in this version.
v2. reading SD card is working now, I am able to read data from Grinder.set and store it in the program.
v3. updating GUI to look nice!
v4. writing SD card to store new set points and entering calibration value for NAU7802.
v5. updating some term by making ratio setting and g_in as settings, the rests are calculated., adding flow rate calculation using easy method "low filter"
v6. adding 2nd flow rate calculation using moving window average, Gemini recommendation for more stable reading. and removing flow rate calc from v5.
v7. add time display from seconds to mm:ss, better for pour over while s.ms is better for espresso
*/
int Version=7;

// to get the board in upload mode sometime you have to hold and press D0 and then press Reset, and release both togeter
// lately unless the code has something bad, the board will restart in bootload mode, you will be required to fix the code first
// if you can't fix, go to the last stable version of the code and put the board in upload mode, and it should allow the ino. file to go in and you are back
// to the old fersion.
//you should see something like /dev/cu.usbmodem14101 in the PORT setting
//of you see other than that or something like /dev/cu.Bluetooth-incoming-port then you will not be able to
//download!

#define SD_CS_PIN 10 // if using the feather esp32-s3 use 10 for Chip Select Pin

//defining display
Adafruit_ST7789 display = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//defining battery monitor
Adafruit_MAX17048 maxlipo;

//definne A to D unit as nau
Adafruit_NAU7802 nau; 

// define real time clock, commented out for now
RTC_PCF8523 rtc;

// define file for data storage for later, commented out for now
#define SD_FAT_TYPE 1
File myFile;
SdFat SD;
int pointer=0, settingmax=2, reading_index=0;
String data;
String dataarray[2]; //use pointermax for size of array, this depends on how many settings you need to use.

int timesetting=180000,ratiosetting;

float target_g,g_in,end_g;
const int WINDOW_SIZE = 10;
float flowHistory[WINDOW_SIZE]={0};
int historyIndex = 0;

//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

uint32_t delayMS=100;
const int ledPin= 13;
long previousMillis = 0;


volatile int Button0Pressed,Button1Pressed,Button2Pressed;

void setup() {  
  Serial.begin(9600); // not really necessary for the actual product but always useful to include Serial for troubleshooting.
  
  //setting up TFT
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  display.init(135, 240); // Init ST7789 240x135
  display.setRotation(3);
  display.fillScreen(ST77XX_BLACK);
  display.setTextWrap(true);
  display.setCursor(0, 0);display.setTextColor(ST77XX_WHITE);display.setTextSize(1);

  //Serial.println(F("ST7789 TFT Initialized"));
  display.println(F("ST7789 TFT Initialized"));
 
  while (!maxlipo.begin()) {
    //Serial.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    display.println(F("Couldnt find Adafruit MAX17048?\nMake sure a battery is plugged in!"));
    delay(2000);
  }

  //Serial.print(F("Found MAX17048"));Serial.print(F(" with Chip ID: 0x"));Serial.println(maxlipo.getChipID(), HEX);
  display.print(F("Found MAX17048"));display.print(F(" with Chip ID: 0x"));display.println(maxlipo.getChipID(), HEX);

  // standard Feather front push button setups
  pinMode(0, INPUT_PULLUP);pinMode(1, INPUT_PULLDOWN);pinMode(2, INPUT_PULLDOWN); 
  attachInterrupt(digitalPinToInterrupt(0), DetectButton0, FALLING);
  attachInterrupt(digitalPinToInterrupt(1), DetectButton1, RISING);
  attachInterrupt(digitalPinToInterrupt(2), DetectButton2, RISING);

  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected
  pinMode(ledPin, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

  display.print("Scale v");display.print(Version);display.println(" by AM");display.println("Feb 2025, Maple Grove, MN, USA");
  display.println("Adafruit FeatherESP32S3 Reverse TFT");
  Serial.println("NAU7802");
  display.println("NAU7802");

  if (! nau.begin()) {
    Serial.println("Failed to find NAU7802");
    display.println("Failed to find NAU7802");
  }
  Serial.println("Found NAU7802");
  display.println("Found NAU7802");

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
    display.println("Failed to calibrate internal offset, retrying!");
    delay(1000);
  }
  Serial.println("Calibrated internal offset");
  display.println("Calibrated internal offset");

  while (! nau.calibrate(NAU7802_CALMOD_OFFSET)) {
    Serial.println("Failed to calibrate system offset, retrying!");
    display.println("Failed to calibrate system offset, retrying!");
    delay(1000);
  }
  Serial.println("Calibrated system offset");
  display.println("Calibrated system offset");
  

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    display.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  rtc.start();


  if (!SD.begin(SD_CS_PIN)) {  //CS Pin for Feather is define up above 
    Serial.println("SD Card initialization failed!");
    display.println("SD Card initialization failed!");
  }
  Serial.println("SD Card initialization done.");
  display.println("SD Card initialization done.");
 //  up to here it works so continue here tomorrow. So it can start
  if (SD.exists("Grindset.txt")) {
    Serial.println("Grindset.txt exists.");
    display.println("Grindset.txt exists.");
  } else {
    Serial.println("Grindset.txt doesn't exist.");
    display.println("Grindset.txt doesn't exists.");
    Serial.println("Creating Grindset.txt");
    display.println("Creating Grindset.txt");

    myFile = SD.open("Grindset.txt", FILE_WRITE);
    myFile.close();
  }
/*
  if (SD.exists("Grindset.txt")) {
    Serial.println("Grindset.txt creation confirmed.");
    display.println("Grindset.txt creation confirmed.");

  } else {
    Serial.println("Grindset.txt creation failed.");
    display.println("Grindset.txt creation failed.");
  }
  */
  
  myFile = SD.open("Grindset.txt");
  if (myFile) {
    Serial.println("Grindset.txt:");
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    myFile.close();
  } else {
    Serial.println("error opening Grindset.txt");
  }

  // this section read the Grindset.txt and read the first 2 rows
  // and stored it into String arrays
  // data is stored in the following order : 
  // array[0] = target weight, 1st row in the text file
  // array [1] = target time, 2nd row in the text file


  myFile = SD.open("Grindset.txt"); // very important to have this SD.open before calling myFile.available!
  while (myFile.available() && pointer<settingmax) {
    Serial.print("reading content of myFile, pointer value is");Serial.println(pointer);
    char c= myFile.read();
    if (c=='\r') {
      continue;
      data.concat(c);
    } 
    else if ( c == '\n')
    {
       Serial.println("end of line read, store in array");
      dataarray[pointer]=data;
      data=""; pointer++;
    }
    else {
      data.concat(c);
    }
  }

  while (myFile.available()){
    //read 1 char at a time and store it in char array for tft.print
    dataarray[reading_index]=myFile.read();
    display.print(dataarray[reading_index]);
    reading_index++;
  }
  reading_index=0;
  myFile.close();

  Serial.print("Read from SD 1st row: ");Serial.println(dataarray[0]);
  Serial.print("Read from SD 2nd row: ");Serial.println(dataarray[1]);
  g_in=dataarray[0].toFloat();
  ratiosetting=dataarray[1].toInt();
  target_g=g_in*ratiosetting;
  Serial.print("Assign array 0 to setting g_in: ");Serial.println(g_in);
  Serial.print("Assign array 1 to setting ratiosetting: ");Serial.println(ratiosetting);
  Serial.print("End weight = ratio setting x g_in: ");Serial.println(target_g);
  delay(3000);
  display.fillScreen(ST77XX_BLACK);

} // end setup



//next are all the customizable variables some are used to display default settings or parameters
int n = 1; float nmax=10.0;
float offset_values=1099.92; // update this after manual calibration 976.97
float valtotal=0;
float average_val=0;
float displayed_g=0.0, frozen_g=0.0,previous_g=0.0;
float nau_adjusted_g=0.0;
float stored_nau_adjusted_g=0.0;
int timebase=0,weightbase=0,modecounter=1,changecounter=0,ready,updatetime=0, onetime,increase=0,decrease=0;
unsigned long time_at_tar;  

int displayedtime,frozentime,previoustime=0; //30000ms = 30s default timer
int time_in_s,time_in_m,time_in_s_after_m;
char modechar='g',changechar='T';
int rectangle_x_start=0,rectangle_y_start=24;
int rectangle_x_width=127, rectangle_y_width=40;
int relaypin = 11;

int f2r1y=0,f2r2y=16,f2r3y=32,f2r7y=96,f2r8y=112,f2r9y=128;
int f3r1y=0,f3r2y=24,f3r3y=48,f3r4y=72,f3r5y=96;


// loop start
void loop() {
  float val = nau.read();
  pinMode(relaypin, OUTPUT);  
  float cellVoltage = maxlipo.cellVoltage();
  float oldcellVoltage=0;
  if (isnan(cellVoltage)) {
    Serial.println("Failed to read cell voltage, check battery is connected!");
    delay(2000);
    return;
  }

  
 
  if (oldcellVoltage!=cellVoltage){ // if the battery voltage change value then update the display,. otherwise no update.
    oldcellVoltage = cellVoltage;  
    if (cellVoltage<3.3){
      display.setTextColor(ST77XX_RED);
    }
    else {
      display.setTextColor(ST77XX_GREEN);
    } 
    display.setCursor(0,f2r1y);display.setTextSize(2);
    display.print("Batt:");  
    display.fillRect(75,f2r1y,240,14,ST77XX_BLACK);
    display.setCursor(75,f2r1y);
    display.print(cellVoltage, 1); display.print(" V,");
    display.print(maxlipo.cellPercent(), 0); display.println(" %");
  }


  //DateTime now=rtc.now();
  
  while (! nau.available()) {
    delay(1);
  }
  while (n<=nmax) {
    valtotal=valtotal+nau.read()/offset_values;
    n=n+1;
    delay(5);
  }
  average_val=valtotal/nmax;
  n=1;
  valtotal=0.0;
 
  nau_adjusted_g=average_val;
  Serial.print("Read "); Serial.print(val/offset_values,1); Serial.print(" average = ");Serial.print(average_val,1);
  Serial.print(" actual grams = "); Serial.println(nau_adjusted_g,1);
   
  displayed_g=nau_adjusted_g-stored_nau_adjusted_g;
  
  Serial.print("displayed_g ");Serial.print(displayed_g);
  
  if (displayed_g>0.20 && displayed_g<=target_g && ready==1 && (weightbase==1 || timebase==1)){
    updatetime=1;
    while (onetime==1) {
     time_at_tar=millis();onetime=0;}
    
  }
 
  if (updatetime==1 && ready==1) { //if TAR is pressed and time is updating
    displayedtime=int(millis())-int(time_at_tar);frozentime=displayedtime;
    frozen_g=displayed_g;
    update1(displayedtime,displayed_g,displayedtime-previoustime,displayed_g-previous_g);
  }
  else if (updatetime==0 && ready==0) { // if TAR is not present/pressed and time is not updating, just update the weight, but not increment the time
    //update1(frozentime,frozen_g); disabling this and switching with the else below to see if this allow update of weight without TAR (ready=1)
    update1(frozentime,displayed_g,frozentime-previoustime,displayed_g-previous_g);
  }
  else  { // if it does not meet any the conditions aboved, do not update the weight and the time
    //update1(frozentime,displayed_g); disabling this since it make sense 
    update1(frozentime,frozen_g,frozentime-previoustime,frozen_g-previous_g);
  }
     
  Serial.print("displayedtime ");Serial.print(displayedtime/1000);Serial.print(" ");Serial.println(int(64-(displayed_g/target_g)*rectangle_y_start));
  /*
  display.drawLine(rectangle_x_start,64-rectangle_y_start,64,64-rectangle_y_start,1); //display line representing target_g
  display.drawLine(int(timesetting/1000),rectangle_y_start,int(timesetting/1000),64,1); //display line representing timesetting
  
  if ((displayedtime/1000)<=rectangle_x_width && (64-(displayed_g/target_g)*rectangle_y_start)>=rectangle_y_start){
    display.drawPixel(int(displayedtime/1000),int(64-(displayed_g/target_g)*rectangle_y_start),1);//displaying trace/pixel
  }
  */
  display.fillRect(0,f2r2y,240,f2r3y-1,ST77XX_BLACK);
  display.setCursor(0,f2r2y); display.setTextSize(2);
  display.print("1:");display.print(ratiosetting,1);display.print(" ");
  target_g=g_in*ratiosetting;end_g=target_g+g_in;
  display.print(g_in,0);display.print("g + ");display.print(target_g,0);display.println("g H2O");//display.print(end_g);
  display.print("Mode:");display.print(modechar);display.println(changechar);

  Serial.print("displayed_g ");Serial.print(displayed_g);Serial.print(" target_g ");Serial.print(target_g,1);Serial.print(" Update=");Serial.print(updatetime);
  Serial.print(" Ready=");Serial.print(ready);Serial.print(" Onetime=");Serial.println(onetime);
  
  /*
  if (displayed_g>=target_g && weightbase==1 && ready==1){
    frozen_g=displayed_g;
    update1(frozentime,frozen_g,frozentime-previoustime,frozen_g-previous_g);
    relaycontrol();
  }

  if (displayedtime>=timesetting && timebase==1 && ready==1){
    frozen_g=displayed_g;
    update1(frozentime,frozen_g,frozentime-previoustime,frozen_g-previous_g);
    //relaycontrol();
  }
*/

  if (Button0Pressed) {
    if (weightbase==1 && increase==1){
      g_in=g_in+1;
    }
    else if (timebase==1 && increase==1){
        ratiosetting=ratiosetting+1;
    }
    else if (weightbase==1 && decrease==1){
      g_in=g_in-1;
    }
    else if (timebase==1 && decrease==1){
      ratiosetting=ratiosetting-1;
    }
    else if (increase==0 && decrease==0) {
      Serial.println("TAR");
      stored_nau_adjusted_g=nau_adjusted_g;
      displayedtime=0;
      time_at_tar=millis();
      ready=1; onetime=1;frozentime=0;updatetime=0;
      //cleangrapharea();
      updatesettings(g_in,ratiosetting);
    }
    Button0Pressed=0;
  }
  
  if (Button1Pressed) {
    Serial.print("1 ");Serial.print(changecounter);Serial.print(" ");
    if (changecounter==0){
      changecounter=1; Serial.println(changecounter);
    }
    else if (changecounter==1){
      changecounter=2; Serial.println(changecounter);
    }
    else if (changecounter==2){
      changecounter=0; Serial.println(changecounter);
    }
    Button1Pressed=0;
  }

  if (Button2Pressed) {
    Serial.print("2 ");Serial.print(modecounter);Serial.print(" ");
    if (modecounter==0){
      modecounter=1; Serial.println(modecounter);
    }
    else if (modecounter==1){
      modecounter=2;Serial.println(modecounter);
    }
    else if (modecounter==2){
      modecounter=0;Serial.println(modecounter);
    }
    Button2Pressed=0;
  }

  switch (modecounter) {
    case (0):
      weightbase=0;timebase=0;modechar=' ';break;
    case (1):
      weightbase=1;timebase=0;modechar='g';break;
    case (2):
      weightbase=0;timebase=1;modechar='r';break;
  }
  switch (changecounter) {
    case (0):
      increase=0;decrease=0;changechar='T';break;
    case (1):
      increase=1;decrease=0;changechar='+';break;
    case (2):
      increase=0;decrease=1;changechar='-';break;
  }

  display.setTextSize(2);display.setCursor(0,f2r7y);display.fillRect(0,f2r8y-1,240,14,ST77XX_BLACK);
  display.print("End ratio 1:");display.print(displayed_g/g_in,1);

  if (ready==1) {
    display.setTextSize(2);display.setTextColor(ST77XX_BLUE);
    display.setCursor(0,f2r8y);display.fillRect(0,f2r8y,240,14,ST77XX_BLACK);
    display.print("TARED");
    //display.fillCircle(220,f2r9y,3,1);
  }
  else {
    display.setTextSize(2);display.setTextColor(ST77XX_RED);
    display.setCursor(0,f2r8y);display.fillRect(0,f2r8y,240,14,ST77XX_BLACK);
    display.print("Press D0 to TARE");
    //display.fillCircle(220,f2r9y,3,0);
  }
  previoustime=displayedtime;
  previous_g=displayed_g;

  //display.fillScreen(ST77XX_BLACK);
  //display.fillRect(0,0,rectangle_x_width,rectangle_y_start,0); // instead of using clearDisplay, drawing a black rectangle over the area that need to be cleared is easy
  // this allow some section to be cleared and some not, the area where the Pixel is drawn is not cleared so it create nice trace of the weight
  delay(delayMS);



}

void update1(int time,float weight,int deltaTime, float deltaWeight) {
    float currentFlowRate=0,smoothedFlowRate=0,alpha=0.15;//alpha is smoothing factor, lower is smoother but slower, range recommendation 0.05 to 0.3
    display.setTextSize(3);
    display.setCursor(0,f3r3y);display.fillRect(0,f3r3y,240,f3r5y-1,ST77XX_BLACK);

    time_in_s=int(time/1000); 
    time_in_m=int(time_in_s/60); 
    time_in_s_after_m=time_in_s%60;

    //display.print(time/1000,1);
    display.print(time_in_m);display.print("m:");
    display.print(time_in_s_after_m);display.print("s");
  
    display.setCursor(120,f3r3y);
    if (weight>0.5) {
      display.print(weight,1);display.println("g");
    }
    else {
      display.print("0g");
    }

    currentFlowRate=deltaWeight/((deltaTime/1000.0)+.01);
    display.setCursor(0,f3r4y);
    if (getSmoothedFlow(currentFlowRate)>1 && ready==1) {
      display.print(getSmoothedFlow(currentFlowRate),1);display.print("g/s");
    }
    else {
      display.print("0 g/s");
    }
}


float getSmoothedFlow(float newRate){
  flowHistory[historyIndex]=newRate;

  historyIndex++; 
  if (historyIndex>=WINDOW_SIZE){
    historyIndex=0;
  }

  float sum = 0;
  for (int i=0;i < WINDOW_SIZE;i++){
    sum += flowHistory[i];
  }
  return (sum/(float)WINDOW_SIZE);
}

/*
void relaycontrol() {
    display.fillRect(0,f2r9y,240,14,ST77XX_BLACK);display.setTextSize(2);
    display.setCursor(0,f2r9y);display.print("Target reached");
    //turn relay on to open contact
    digitalWrite(relaypin, HIGH);
    Serial.println("Target reached");
    display.setTextSize(2);display.setCursor(0,f2r7y);display.fillRect(0,f2r8y-1,240,14,ST77XX_BLACK);
    display.print("End ratio 1:");display.print(displayed_g/g_in,1);
    TimeoutTimer(30);
    //delay(30000); // keep open contact for 30s; // maybe good to make the delay time a parameter that can be updated from the UI
    //then display a timer that counts down
    digitalWrite(relaypin, LOW);
    display.fillRect(0,f2r9y,240,14,ST77XX_BLACK);
    display.setCursor(0,f2r9y);
    Serial.println("");
    updatetime = 0; // stop updating timer;
    ready=0; // require TAR to be push again  
}
*/

void cleangrapharea() {
  // function to clean only graph area
  //display.fillRect(rectangle_x_start,rectangle_y_start,64*2,rectangle_y_width,0);
}


void updatesettings(float g_in,int ratiosetting) {
  // write new settings to SD card
  //SD.remove("Grindset.txt");
  //myFile.close();
  myFile = SD.open("Grindset.txt", O_WRONLY | O_CREAT | O_TRUNC);
  myFile.println(g_in);
  myFile.println(ratiosetting);
  myFile.close();
}

/*
void TimeoutTimer (int timeout) {
  while (timeout>=0) {
    Serial.println(timeout);
    display.setTextSize(2);
    display.setCursor(0,f2r8y);
    display.fillRect(0,f2r8y,240,f2r9y-1,ST77XX_BLACK);
    display.print(timeout);
    timeout=timeout-1;
    delay(1000);
    //display.fillRect(rectangle_x_start+64,rectangle_y_start,64,rectangle_y_width,0);
  }
}
*/

void DetectButton0() {
  Button0Pressed = 1;
}

void DetectButton1() {
  Button1Pressed = 1;
}

void DetectButton2() {
  Button2Pressed = 1;
}


