#include <Arduino.h>
#include <http_client.h>
#include <led_ctrl.h>
#include <log.h>
#include <lte.h>
#include <TinyGPSPlus.h>
#include "pitches.h"
 

// -----------------------------------------------------------------------
// parameter
#define SEND_INTERVAL         3 // send interval in seconds
#define NET_INTERVAL          1 // send interval in seconds

#define SPACE_LAT   0.0015
#define SPACE_LOT   0.0015

#define home_lat    34.XXXXXXXXXXXXXX
#define home_lot    135.XXXXXXXXXXXXXX

#define school_lat  34.XXXXXXXXXXXXXX
#define school_lot  135.XXXXXXXXXXXXXX

#define URL_GET_PARAMETER "/ENEBULAR_NODE_NAME/?lat=%f&lot=%f"
#define HOST_SERVER       "lcdpXXX.enebular.com"
// -----------------------------------------------------------------------


#define PIN_SPEAKER A1
#define GPSSerial Serial2

static const int tempo = 130;
static const float duration = 60000 / tempo * 4;

int melody[] = {
  NOTE_G5, NOTE_FS5, NOTE_G5, 0 
};

int noteDurations[] = {
  8, 8, 8, 8
};


TinyGPSPlus gps;


#define IMG_TYPE_HOME     1
#define IMG_TYPE_SCHOOL   2
#define IMG_TYPE_NO_MOVE  3
#define IMG_TYPE_MOVE     4

float g_flat = home_lat;
float g_flot = home_lot;

float g_arflat[3];
float g_arflot[3];

int g_cnt_gps = 0;
int g_place = 0;
int g_cnt_net = 0;

//-------------------------------------------------------------
//playTone
//-------------------------------------------------------------
void playTone()
{
  for (int i = 0; i<sizeof(melody) / sizeof(int); i++) {
    int noteDuration = duration / noteDurations[i];
    tone(PIN_SPEAKER, melody[i], noteDuration);
    delay(noteDuration);
    noTone(PIN_SPEAKER);
  }
}


//-------------------------------------------------------------
//ConvertGps
//-------------------------------------------------------------
float ConvertGps(float f)
{
  float deg = (int)f / 100;
  float min = f - deg * 100;
   
  return deg + min / 60;
}


//-------------------------------------------------------------
//SendGpsDataFunction
//-------------------------------------------------------------
void SendGpsDataFunction(float flat, float flot)
{
  Log.info("SendGpsDataFunction\n");

  if (!HttpClient.configure(HOST_SERVER, 443, true)) {
    Log.info(F("Failed to configure https client\r\n"));
    return;
  }

  char buffer[60];
  snprintf(buffer, sizeof(buffer), URL_GET_PARAMETER, flat, flot);
  Log.info(buffer);

  HttpResponse response = HttpClient.get(buffer);

  Log.infof(F("GET -status code: %u, data size: %u\r\n"),
            response.status_code,
            response.data_size);
}


//-------------------------------------------------------------
//GpsDataFunction
//-------------------------------------------------------------
int GpsDataFunction(float flat, float flot)
{
  int ret = 0;
  // 自宅近く
  if (abs(flat - home_lat) < SPACE_LAT &&
      abs(flot - home_lot) < SPACE_LOT) {
    ret = IMG_TYPE_HOME;
    Log.info("HOME, ");
  }
  // 学校近く
  else if (abs(flat - school_lat) < SPACE_LAT &&
           abs(flot - school_lot) < SPACE_LOT) {
    ret = IMG_TYPE_SCHOOL;
    Log.info("SCHOOL, ");
  }  
  // 動いていない
  else if (abs(flat - g_flat) < SPACE_LAT &&
           abs(flot - g_flot) < SPACE_LOT) {
    ret = IMG_TYPE_NO_MOVE;
    Log.info("NO MOVE, ");
  }

  // Now Parameter
  g_flat = flat;
  g_flot = flot;

  g_arflat[g_cnt_gps] = flat;
  g_arflot[g_cnt_gps] = flot;

  g_cnt_gps++;
  if (g_cnt_gps >= 3) {
    g_cnt_gps = 0;
  }  
  
  Log.infof("GPS=%f,%f\n", g_flat, g_flot);

  return ret;
}


//-------------------------------------------------------------
//Setup
//-------------------------------------------------------------
void setup() 
{
  Log.begin(115200);
  Log.setLogLevel(LogLevel::INFO);

  LedCtrl.begin();
  LedCtrl.startupCycle();

  // Speaker
  pinMode(PIN_SPEAKER,OUTPUT);  //setting up buzzer pin as output

  // GPS
  GPSSerial.swap(1);
  GPSSerial.begin(9600);
  GPSSerial.println("Hello, world?");

  Log.infof("TinyGPSPlus v. %d\n", TinyGPSPlus::libraryVersion());

  // Start LTE modem and connect to the operator
  if (!Lte.begin()) {
    Log.error(F("Failed to connect to the operator"));
  } else {
    Log.infof(F("Connected to operator: %s\r\n"), Lte.getOperator().c_str());
  }
  
  Log.info("-- START --");
}


//-------------------------------------------------------------
//loop
//-------------------------------------------------------------
void loop() 
{
  while (GPSSerial.available() > 0) {
    char c = GPSSerial.read();
    gps.encode(c);
/*    
    if (gps.time.isValid()) {      
      int hour = gps.time.hour() + 9;
      int min  = gps.time.minute();
      int sec  = gps.time.second();
      //Log.infof("TIME=%d:%d.%d\n", hour, min, sec);
    }
*/    
    if (gps.location.isUpdated()){
      float fLat = gps.location.lat();
      float flot = gps.location.lng();

      int ret = GpsDataFunction(fLat, flot);
      if (g_place != ret) {
        if(IMG_TYPE_NO_MOVE != ret) {
          g_place = ret;
          SendGpsDataFunction(fLat, flot);
        } else {
          Log.infof("SAME PLACE\r\n");
        }
        
        //playTone();
      }      
      delay(SEND_INTERVAL * 1000);
    }
  }
}
