
#include <Arduino_JSON.h>


#include "FS.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include <SPI.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include "time.h"

#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false
double temp;
const char* ssid = "Beachhouse";
const char* password = "jacobandkyle69";
const String endpoint = "https://api.openweathermap.org/data/2.5/weather?lat=33.983841&lon=-118.451424&appid=";
const String key = "5d1f0d27a524847686682aea32f0409d";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -14400;
const int   daylightOffset_sec = -10800;
struct tm timeinfo;



#define TFT_GREY 0x5AEB

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library
char currentPage;
char resetpg;
float sx = 0, sy = 1, mx = 1, my = 0, hx = -1, hy = 0;    // Saved H, M, S x & y multipliers
float sdeg=0, mdeg=0, hdeg=0;
uint16_t osx=120, osy=120, omx=120, omy=120, ohx=120, ohy=120;  // Saved H, M, S x & y coords
uint16_t x0=0, x1=0, yy0=0, yy1=0;
uint32_t targetTime = 0;  
// for next 1 second timeout
int cacti1X = 220;
int dinoJumpedY = 218;
const char* token;


static uint8_t conv2d(const char* p); // Forward declaration needed for IDE 1.6.x
uint8_t hh=conv2d(__TIME__), mm=conv2d(__TIME__+3), ss=conv2d(__TIME__+6);  // Get H, M, S from compile time
//uint8_t hh, mm, ss=;  // Get H, M, S from compile time


bool initial = 1;

byte omm = 99, oss = 99;
byte xcolon = 0, xsecs = 0;
unsigned int colour = 0;
void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  hh = (uint8_t)atoi(timeHour);
  Serial.println(timeHour);
  char timeMins[3];
  strftime(timeMins,3, "%M", &timeinfo);
  mm = (uint8_t)atoi(timeMins);
  Serial.println(timeMins);
  char timeSecs[3];
  strftime(timeSecs,3, "%S", &timeinfo);
  ss = (uint8_t)atoi(timeSecs);
  Serial.println(timeSecs);
}
void setup(void) {
    Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  touch_calibrate();
  initWiFi();
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status
 
    HTTPClient http;
    http.useHTTP10(true);
    http.begin(endpoint + key); //Specify the URL
    int httpCode = http.GET();  //Make the request
 
    if (httpCode > 0) { //Check for the returning code
 
        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);
        JSONVar myObject = JSON.parse(payload);
        //Serial.println(myObject["main"]["temp"]);
        temp = myObject["main"]["temp"];
        
        temp = 1.8 * (temp - 273)+32;
        Serial.println(temp);


//        JSONVar keys = myObject.keys();
//    
//      for (int i = 0; i < keys.length(); i++) {
//        JSONVar value = myObject[keys[i]];
//        Serial.print(keys[i]);
//        Serial.print(" = ");
//        Serial.println(value);
//      }
      }
 
    else {
      Serial.println("Error on HTTP request");
    }
 
    http.end(); //Free the resources
  }
 configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  
  //tft.fillScreen(TFT_BLACK);
  //tft.fillScreen(TFT_RED);
  //tft.fillScreen(TFT_GREEN);
  //tft.fillScreen(TFT_BLUE);
  //tft.fillScreen(TFT_BLACK);
  drawClock();
//  tft.fillScreen(TFT_GREY);
//  
//  tft.setTextColor(TFT_WHITE, TFT_GREY);  // Adding a background colour erases previous text automatically
//  
//  // Draw clock face
//  tft.fillCircle(120, 120, 118, TFT_BLUE);
//  tft.fillCircle(120, 120, 110, TFT_BLACK);
//
//  // Draw 12 lines
//  for(int i = 0; i<360; i+= 30) {
//    sx = cos((i-90)*0.0174532925);
//    sy = sin((i-90)*0.0174532925);
//    x0 = sx*114+120;
//    yy0 = sy*114+120;
//    x1 = sx*100+120;
//    yy1 = sy*100+120;
//
//    tft.drawLine(x0, yy0, x1, yy1, TFT_GREEN);
//  }
//
//  // Draw 60 dots
//  for(int i = 0; i<360; i+= 6) {
//    sx = cos((i-90)*0.0174532925);
//    sy = sin((i-90)*0.0174532925);
//    x0 = sx*102+120;
//    yy0 = sy*102+120;
//    // Draw minute markers
//    tft.drawPixel(x0, yy0, TFT_WHITE);
//    
//    // Draw main quadrant dots
//    if(i==0 || i==180) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
//    if(i==90 || i==270) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
//  }
//
//  tft.fillCircle(120, 121, 3, TFT_WHITE);
//
//  // Draw text at position 120,260 using fonts 4
//  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . - a p m
//  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
//  tft.drawString("WEATHER:",180,0,2);
//  tft.drawString("69° F",250,0,2);
  currentPage = '0';
  drawAppIcon();
  targetTime = millis() + 1000; 
  
}

void loop() {
  uint16_t t_x = 0, t_y = 0;
  bool pressed = tft.getTouch(&t_x, &t_y);
  if(currentPage == '0'){
    if((t_x > 245) && (t_x < 320) ){
      if((t_y > 190) && (t_y < 240)){
          tft.fillRect(245,190,75,50,TFT_WHITE);
          tft.init();
          tft.setRotation(1);
          resetpg = '0';
          currentPage = '1';
      }
    }
    if (targetTime < millis()) {
      targetTime += 1000;
      ss++;              // Advance second
      if (ss==60) {
        ss=0;
        mm++;            // Advance minute
        if(mm>59) {
          mm=0;
          hh++;          // Advance hour
          if (hh>23) {
            hh=0;
          }
        }
      }
  
      sdeg = ss*6;                  // 0-59 -> 0-354
      mdeg = mm*6+sdeg*0.01666667;  // 0-59 -> 0-360 - includes seconds
      hdeg = hh*30+mdeg*0.0833333;  // 0-11 -> 0-360 - includes minutes and seconds
      hx = cos((hdeg-90)*0.0174532925);    
      hy = sin((hdeg-90)*0.0174532925);
      mx = cos((mdeg-90)*0.0174532925);    
      my = sin((mdeg-90)*0.0174532925);
      sx = cos((sdeg-90)*0.0174532925);    
      sy = sin((sdeg-90)*0.0174532925);
  
      if (ss==0 || initial) {
        initial = 0;
        tft.drawLine(ohx, ohy, 120, 121, TFT_BLACK);
        ohx = hx*62+121;    
        ohy = hy*62+121;
        tft.drawLine(omx, omy, 120, 121, TFT_BLACK);
        omx = mx*84+120;    
        omy = my*84+121;
      }
  
        tft.drawLine(osx, osy, 120, 121, TFT_BLACK);
        osx = sx*90+121;    
        osy = sy*90+121;
        tft.drawLine(osx, osy, 120, 121, TFT_RED);
        tft.drawLine(ohx, ohy, 120, 121, TFT_WHITE);
        tft.drawLine(omx, omy, 120, 121, TFT_WHITE);
        tft.drawLine(osx, osy, 120, 121, TFT_RED);
  
      tft.fillCircle(120, 121, 3, TFT_RED);
  
  
      int xpos = 90;
      int ypos = 160; 
  
      if (omm != mm) { 
        omm = mm;
        // Draw hours and minutes
        if (hh < 10) xpos += tft.drawChar('0', xpos, ypos, 4); 
        xpos += tft.drawNumber(hh, xpos, ypos, 4);             
        xpos += tft.drawChar(':', xpos, ypos, 4);
        if (mm < 10) xpos += tft.drawChar('0', xpos, ypos, 4); 
        xpos += tft.drawNumber(mm, xpos, ypos, 4);             
        xsecs = xpos; 
  
  
      
  
      
    }

    
  }
  if(currentPage == '1'){
    if(resetpg == '0'){
    tft.fillScreen(TFT_BLACK);
    drawAppPage();
    }
    if((t_x > 0) && (t_x < 50) ){
      if((t_y > 0) && (t_y < 25)){
          drawClock();
          drawAppIcon();

          currentPage = '0';
      }
    }
    if((t_x > 50) && (t_x < 100) ){
      if((t_y > 50) && (t_y < 100)){
          resetpg = '0';
          currentPage = '2';
          
      }
    }
    if((t_x > 130) && (t_x < 190) ){
      if((t_y > 130) && (t_y < 190)){
          resetpg = '0';
          currentPage = '6';
          
      }
    }
    if((t_x > 40) && (t_x < 90) ){
      if((t_y > 140) && (t_y < 190)){
          resetpg = '0';
          currentPage = '7';
          
      }
    }
  }
  if(currentPage == '2'){
    if((t_x > 0) && (t_x < 50) ){
      if((t_y > 0) && (t_y < 25)){
        drawClock();
          drawAppIcon();
          resetpg = '0';
          currentPage = '1';

      }
    }
    delay(3);
    dinoRun();
    if(resetpg == '0'){
//     delay(2)
//    dinoRun();
    }
  }
  if(currentPage == '6'){
    if((t_x > 0) && (t_x < 50) ){
      if((t_y > 0) && (t_y < 25)){
        drawClock();
          drawAppIcon();
          resetpg = '0';
          currentPage = '1';

      }
    }
    if(resetpg == '0'){
    tft.fillScreen(TFT_BLACK);
    gateOpen();
    }
    if((t_x > 0) && (t_x < 100) ){
      if((t_y > 70) && (t_y < 170)){
          openGateAuth();

      }
    }
    if((t_x > 220) && (t_x < 320) ){
      if((t_y > 70) && (t_y < 170)){
          openGateCall();

      }
    }
    //gateOpen();
  }
    if(currentPage == '7'){
    if((t_x > 0) && (t_x < 50) ){
      if((t_y > 0) && (t_y < 25)){
        drawClock();
          drawAppIcon();
          resetpg = '0';
          currentPage = '1';

      }
    }
    if(resetpg == '0'){
    tft.fillScreen(TFT_BLACK);
    drawRemote();
    }
    if((t_x > 300) && (t_x < 320) ){
      if((t_y > 0) && (t_y < 20)){
        if(WiFi.status()== WL_CONNECTED){
    const String offAddress = "http://192.168.86.23:8060/keypress/PowerOff";
      HTTPClient http;
    
      http.begin(offAddress);
      
      int httpResponseCode = http.POST("gjh");
      String payload = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("HTTP Response: ");
      Serial.println(payload);
      JSONVar myObjectCode = JSON.parse(payload);
      http.end();
    
    }
      }
    }
    if((t_x > 270) && (t_x < 290) ){
      if((t_y > 0) && (t_y < 20)){
        if(WiFi.status()== WL_CONNECTED){
    const String onAddress = "http://192.168.86.23:8060/keypress/PowerOn";
      HTTPClient http;
    
      http.begin(onAddress);
      
      int httpResponseCode = http.POST("gjh");
      String payload = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("HTTP Response: ");
      Serial.println(payload);
      JSONVar myObjectCode = JSON.parse(payload);
      http.end();
    
    }
      }
    }
    if((t_x > 300) && (t_x < 320) ){
      if((t_y > 30) && (t_y < 50)){
        if(WiFi.status()== WL_CONNECTED){
    const String upAddress = "http://192.168.86.23:8060/keypress/VolumeUp";
      HTTPClient http;
    
      http.begin(upAddress);
      
      int httpResponseCode = http.POST("gjh");
      String payload = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("HTTP Response: ");
      Serial.println(payload);
      JSONVar myObjectCode = JSON.parse(payload);
      http.end();
    
    }
      }
    }
    if((t_x > 300) && (t_x < 320) ){
      if((t_y > 60) && (t_y < 80)){
        if(WiFi.status()== WL_CONNECTED){
    const String downAddress = "http://192.168.86.23:8060/keypress/VolumeDown";
      HTTPClient http;
    
      http.begin(downAddress);
      
      int httpResponseCode = http.POST("gjh");
      String payload = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("HTTP Response: ");
      Serial.println(payload);
      JSONVar myObjectCode = JSON.parse(payload);
      http.end();
    
    }
      }
    }
  }
    
}
void drawRemote(){
  tft.fillScreen(TFT_BLACK);
    tft.drawRect(0,0,50,25,TFT_WHITE);
    tft.drawString("back",10,5,2);
    tft.drawRect(300,0,20,20,TFT_WHITE);
    tft.drawString("Off",300,0,2);
    tft.drawRect(270,0,20,20,TFT_WHITE);
    tft.drawString("On",270,0,2);
    tft.drawRect(300,30,20,20,TFT_WHITE);
    tft.drawString("Vup",300,30,2);
    tft.drawRect(300,60,20,20,TFT_WHITE);
    tft.drawString("Vdn",300,60,2);
    resetpg = 1;
    
}
void drawClock(){
  tft.fillScreen(TFT_GREY);
  
  //tft.setTextColor(TFT_WHITE, TFT_GREY);  // Adding a background colour erases previous text automatically
  
  // Draw clock face
  tft.fillCircle(120, 120, 118, TFT_BLUE);
  tft.fillCircle(120, 120, 110, TFT_BLACK);

  // Draw 12 lines
  for(int i = 0; i<360; i+= 30) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*114+120;
    yy0 = sy*114+120;
    x1 = sx*100+120;
    yy1 = sy*100+120;

    tft.drawLine(x0, yy0, x1, yy1, TFT_GREEN);
  }

  // Draw 60 dots
  for(int i = 0; i<360; i+= 6) {
    sx = cos((i-90)*0.0174532925);
    sy = sin((i-90)*0.0174532925);
    x0 = sx*102+120;
    yy0 = sy*102+120;
    // Draw minute markers
    tft.drawPixel(x0, yy0, TFT_WHITE);
    
    // Draw main quadrant dots
    if(i==0 || i==180) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
    if(i==90 || i==270) tft.fillCircle(x0, yy0, 2, TFT_WHITE);
  }

  tft.fillCircle(120, 121, 3, TFT_WHITE);

  // Draw text at position 120,260 using fonts 4
  // Only font numbers 2,4,6,7 are valid. Font 6 only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : . - a p m
  // Font 7 is a 7 segment font and only contains characters [space] 0 1 2 3 4 5 6 7 8 9 : .
  tft.drawString("WEATHER:",180,0,2);
  String temptobeDisp="";
  temptobeDisp = String(temp,2);
  Serial.println(temptobeDisp);
  tft.drawString(temptobeDisp,250,0,2);
  tft.drawString("°F",295,0,2);
}
void drawAppIcon(){
  tft.drawRect(245,190,75,50,TFT_BLACK);
  tft.fillRect(250,195,65,40,TFT_YELLOW);
  tft.drawCentreString("APPS",280,205,2);
  
}
void drawAppPage(){
  tft.drawRect(0,0,50,25,TFT_WHITE);
  tft.drawString("back",10,5,2);
  tft.drawRect(40,50, 50, 50,TFT_WHITE);
  tft.drawString("DINO",50,65,2);
  tft.drawRect(130,50, 50, 50,TFT_WHITE);
  tft.drawString("Chess",140,65,2);
  tft.drawRect(210,50, 50, 50,TFT_WHITE);
  tft.drawString("Drone",220,65,2);
  tft.drawRect(40,140, 50, 50,TFT_WHITE);
  tft.drawString("TV",50,155,2);
  tft.drawRect(130,140, 50, 50,TFT_WHITE);
  tft.drawString("Gate",140,155,2);
  tft.drawRect(210,140, 50, 50,TFT_WHITE);
  tft.drawString("test",220,155,2);
  resetpg = '1';
}
void dinoRun(){
  uint16_t t_x = 0, t_y = 0;
  bool pressed = tft.getTouch(&t_x, &t_y);
  PROGMEM const unsigned char dino1[] = {
   0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0xd8, 0x1f, 0x00, 0xf8, 0x1f,
   0x00, 0xf8, 0x1f, 0x00, 0xf8, 0x1f, 0x00, 0xf8, 0x00, 0x00, 0xf8, 0x07,
   0x02, 0x7c, 0x00, 0x02, 0x7f, 0x00, 0x86, 0xff, 0x01, 0xce, 0x7f, 0x01,
   0xfe, 0x7f, 0x00, 0xfe, 0x7f, 0x00, 0xfc, 0x3f, 0x00, 0xf8, 0x3f, 0x00,
   0xf0, 0x1f, 0x00, 0xe0, 0x0f, 0x00, 0xc0, 0x0d, 0x00, 0xc0, 0x08, 0x00,
   0x40, 0x08, 0x00, 0xc0, 0x18, 0x00, 0x00, 0x00, 0x00
  };
  PROGMEM const unsigned char cacti1[] = {
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x18, 0x00, 0xe0, 0x01, 0x3c, 0x00,
   0xe0, 0x01, 0x3c, 0x00, 0xe0, 0x81, 0x3c, 0x00, 0xe0, 0xc1, 0x3d, 0x00,
   0xe0, 0xc9, 0x3d, 0x01, 0xe4, 0xdd, 0xbd, 0x03, 0xee, 0xdd, 0xbd, 0x03,
   0xee, 0xdd, 0xbd, 0x03, 0xee, 0xdd, 0xbd, 0x03, 0xee, 0xdd, 0xbf, 0x03,
   0xee, 0x9d, 0xbf, 0x03, 0xee, 0x1d, 0xff, 0x03, 0xfe, 0x0f, 0xfc, 0x01,
   0xfc, 0x07, 0xfc, 0x00, 0xf8, 0x01, 0x3c, 0x00, 0xe0, 0x01, 0x3c, 0x00,
   0xe0, 0x01, 0x3c, 0x00, 0xe0, 0x01, 0x3c, 0x00, 0xe0, 0x01, 0x3c, 0x00,
   0xe0, 0x01, 0x3c, 0x00, 0xe0, 0x01, 0x3c, 0x00, 0xf8, 0x05, 0x3f, 0x00,
   0x00, 0x00, 0x80, 0x00, 0x80, 0x00, 0x02, 0x00
  };
  tft.fillScreen(TFT_BLACK);
  int cacti1Y = 218;
  int dinoX = 50;
  int dinoY = 218;
  int dinoJumpedX = 50;
  tft.drawXBitmap(cacti1X,cacti1Y, cacti1, 26, 27, TFT_WHITE);
  if(pressed){
    for(int i=0 ; i <=10; i++){
      delay(1);
        tft.fillScreen(TFT_BLACK);

      tft.drawXBitmap(dinoJumpedX,dinoJumpedY, dino1, 23, 22, TFT_WHITE);
      dinoJumpedY = dinoJumpedY - 6;
      if(cacti1X >1){
    cacti1X = cacti1X - 5;
  }
  else{
    cacti1X = 220;
  }
    tft.drawXBitmap(cacti1X,cacti1Y, cacti1, 26, 27, TFT_WHITE);

    }
    for(int i=0 ; i <=10; i++){
      delay(1);
        tft.fillScreen(TFT_BLACK);

      tft.drawXBitmap(dinoJumpedX,dinoJumpedY, dino1, 23, 22, TFT_WHITE);
      dinoJumpedY = dinoJumpedY + 6;
      if(cacti1X >1){
    cacti1X = cacti1X - 5;

  }
  else{
    cacti1X = 220;
  }
    tft.drawXBitmap(cacti1X,cacti1Y, cacti1, 26, 27, TFT_WHITE);

    }
    if(dinoJumpedX+23>=cacti1X && dinoJumpedX<= cacti1X+26 && dinoJumpedY+22>cacti1Y){
        tft.fillScreen(TFT_WHITE);
        for(int i=0; i<=100; i++){
          tft.drawString("You Lose",140,120,4);
        }
        delay(1000);
         drawClock();
          drawAppIcon();
        resetpg = 0;
        currentPage = '0';
        
        
    }
  }
  else{
    tft.drawXBitmap(dinoX,dinoY, dino1, 23, 22, TFT_WHITE);
    if(dinoX+23>=cacti1X && dinoX<= cacti1X+26 && dinoY+22>cacti1Y){
        tft.fillScreen(TFT_WHITE);
        for(int i=0; i<=100; i++){
          tft.drawString("You Lose",140,120,4);
        }
       delay(1000);
        drawClock();
          drawAppIcon();
        resetpg = 0;
        currentPage = '0';
    }
  }
  resetpg = '1';
  if(cacti1X >1){
    cacti1X = cacti1X - 5;
  }
  else{
    cacti1X = 220;
  }
  
}
void openGateAuth(){
  
  if(WiFi.status()== WL_CONNECTED){
    const String gateAuthAddress = "https://auth.brivo.com/oauth/token?grant_type=refresh_token&refresh_token=eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJjcmVkIjoiNTExNjAxODMiLCJ1c2VyX25hbWUiOiJuL2EiLCJhdXRob3JpdGllcyI6WyJST0xFX0NBUkRfSE9MREVSIl0sImNsaWVudF9pZCI6ImQ1MTBkYTcwLWI2ODgtNDVjNi1hYWRlLWRiMjEyN2UwNWI0NSIsImFjY291bnRfb2JqZWN0X2lkIjoxOTA3NzMwNSwidXNlcl9pZCI6IjM2NTkxNzUxIiwic2NvcGUiOlsiYnJpdm8uYXBpIl0sImF0aSI6IjJiMWEzNTc0LWY0ZGEtNDlmOC1hNGI0LTA0OGQyMDhmNjE3NSIsImlzc3VlZCI6IjE2NTE2OTY1NjQ2MDAiLCJleHAiOjIyODI4MzQ1NjQsInNlcnZpY2VfdG9rZW4iOm51bGwsImp0aSI6IjAxNWRiOTE5LTEwY2QtNGYyMi1iYWI1LTM4N2MyNDg4ZjM3MCIsIndoaXRlX2xpc3RlZCI6dHJ1ZX0.a5okVZoaRTSejSQTLA8aLNl0DvxFItXLpfhAS6haL0sx6vns8btY7r0OBOnGiTBE8bFggnq7pRnaxU0hl8qRP1Wz-Rax9Uiw2EtADOThiT5miaOJXlkCsoTHL-zKHmYipCFRZVeGm-nAb-NFWbMpcZAgNqknN6ny267l8pEfP_Y";
      HTTPClient http;
    
      http.begin(gateAuthAddress);
      http.addHeader("Authorization", "Basic ZDUxMGRhNzAtYjY4OC00NWM2LWFhZGUtZGIyMTI3ZTA1YjQ1OjN0d3NnNEx6WjgybXN5NTA3Q0FXcXFPc0VZZ3p3Wldl");
      http.addHeader("Cookie", "AWSALB=4OA1lTRvVlIhqTggEadYAHnCoyI806nejnW12f0amwrA1xzUw2oLKpds7MI3ulg6IDqv3xEyu8kZdBucPCjqBZvpFPgk9rgJ2xgfyCFbRdJTypmlJNvxkH1W47cu; AWSALBCORS=4OA1lTRvVlIhqTggEadYAHnCoyI806nejnW12f0amwrA1xzUw2oLKpds7MI3ulg6IDqv3xEyu8kZdBucPCjqBZvpFPgk9rgJ2xgfyCFbRdJTypmlJNvxkH1W47cu");
      
      int httpResponseCode = http.POST("gjh");
      
      
      String payload = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("HTTP Response: ");
      Serial.println(payload);
      JSONVar myObjectCode = JSON.parse(payload);
      http.end();
      token = (myObjectCode["access_token"]);
      tft.drawString("Auth Call returned",40,20,4);
      tft.drawString(String(httpResponseCode),260,20,4);
      

    
    
    }
  
}
void openGateCall(){
  
  if(WiFi.status()== WL_CONNECTED){
    const String gateOpenAddress = "pi.brivo.com/api/access-points/20661623/unlock";
      HTTPClient http;
      String tokenFin = "Bearer ";
      
      tokenFin = tokenFin+token;
      Serial.println(tokenFin);

      
      http.begin(gateOpenAddress);
      http.addHeader("Authorization", tokenFin);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Connection", "keep-alive");
      http.addHeader("Host", "pi.brivo.com");
      http.addHeader("Content-Length", "143");
      int httpResponseCodeForSecond = http.POST("{\"credentialValue\":\"MDAwMDAwMDAwMDAwMDAwMDAwMDAwMDAzMGNhNDc3\",\"proximity\":{\"wifi\":[{\"bssid\":null,\"ssid\":null,\"active\":false,\"ssiddata\":null}]}}");
      String payload2 = http.getString();
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCodeForSecond);
      Serial.print("HTTP Response: ");
      Serial.println(payload2);
      http.end();
    }
  
}
void gateOpen(){
 
    tft.fillScreen(TFT_BLACK);
    tft.drawRect(0,0,50,25,TFT_WHITE);
    tft.drawString("back",10,5,2);
    tft.drawRect(0,70,100,100,TFT_WHITE);
    tft.drawString("AUTH",10,120,4);
    tft.drawRect(220,70,100,100,TFT_WHITE);
    tft.drawString("OPEN",230,120,4);
    resetpg = 1;
    
  
  
}

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  while(WiFi.status() == WL_CONNECTED){
    tft.drawString("Wifi is connected",40,120,4);
    delay(1000);
    break;
    
  }
}
void touch_calibrate()
{
  uint16_t calData[5];
  uint8_t calDataOK = 0;

  // check file system exists
  if (!SPIFFS.begin()) {
    Serial.println("Formating file system");
    SPIFFS.format();
    SPIFFS.begin();
  }

  // check if calibration file exists and size is correct
  if (SPIFFS.exists(CALIBRATION_FILE)) {
    if (REPEAT_CAL)
    {
      // Delete if we want to re-calibrate
      SPIFFS.remove(CALIBRATION_FILE);
    }
    else
    {
      File f = SPIFFS.open(CALIBRATION_FILE, "r");
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = SPIFFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
}