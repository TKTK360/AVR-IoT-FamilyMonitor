■電子パーツ
・Microchip AVR-IoT WA Development Board
・GPS Module:GYSFDMAXB

■Software
・Arduino

■Cloud
・Enebular
  Use cloud execution environment
  Save GPS values in data store
  Display location information on Openstreetmap based on data store values when viewing the client


--------------------------------------------------------------------
1. provision

Please obtain the encrypted root certificate based on the reference site and set it.

[Reference site]
https://www.mgo-tec.com/blog-entry-arduino-esp32-ssl-stable-root-ca.html/2

--------------------------------------------------------------------
2. AVIoT-Tracker.ino
 parameter custom edit
 
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


--------------------------------------------------------------------
3. enebular

This is achieved using the cloud execution environment, which is a Nebular service.
Two functions are implemented by branching the path for one URI assigned in the cloud execution environment.
The first is the storage process of GPS location information sent from Spresense.
The second process is to return HTML for viewing location information on a map from a smartphone.
The map uses Openstreetmap.
Please refer to Github for the flow reproduction file. To make it work, please create an Enebular account and upload.
Also, data store values and base GPS values may need to be rewritten depending on your usage situation.
