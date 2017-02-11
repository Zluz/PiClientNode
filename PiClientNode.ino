/****************************************
  EthernetNode
  
  Arduino Mega 2560
Sketch uses 46,130 bytes (18%) of program storage space. Maximum is 253,952 bytes.
Global variables use 1,658 bytes (20%) of dynamic memory, leaving 6,534 bytes for local variables. Maximum is 8,192 bytes.

  Arduino UNO - OUTDATED
Sketch uses 29,602 bytes (91%) of program storage space. Maximum is 32,256 bytes.
Global variables use 621 bytes (30%) of dynamic memory, leaving 1,427 bytes for local variables. Maximum is 2,048 bytes.

 ****************************************/

/* Select device. UNO program must fit in space. MEGA adds SD card support. */

//#define DEFINE_DEVICE_UNO
#define DEFINE_DEVICE__UNO__BARE
//#define DEFINE_DEVICE__MEGA__BARE
//#define DEFINE_DEVICE__MEGA__NETWORK


/* Included libraries */
#include "DHT.h"
#include <SPI.h>
#include <EEPROM.h>
#include <avr/pgmspace.h>
#include "Arduino.h"
#include <RGBdriver.h>


const static String VERSION  = "20170211_0304";


#if defined( DEFINE_DEVICE__MEGA__NETWORK )
#include <SD.h>
#include <Ethernet.h>
#endif

/*---GPIO-pin-assignment-----------*\

    PIN_10: 01 - DHT11

    PIN_22: 01 - RGB Driver CLK
    PIN_23: 01 - RGB Driver DIO
    PIN_24: 02 - RGB Driver CLK
    PIN_25: 02 - RGB Driver DIO
    PIN_26: 03 - RGB Driver CLK
    PIN_27: 03 - RGB Driver DIO
    PIN_28: 04 - RGB Driver CLK
    PIN_29: 04 - RGB Driver DIO
    
    PIN_30: 01 - Flammable Gas detect
    PIN_31: 02 - Flammable Gas detect
    
    PIN_32: 02 - DHT11
    PIN_33: 03 - DHT11
    PIN_34: 04 - DHT11
    PIN_35: 05 - DHT11
    PIN_36: 06 - DHT11
\*---------------------------------*/




// DHT options
//#define PIN_DHT 2     // what digital pin we're connected to
//#define DHT_TYPE DHT11   // DHT 11
DHT dht_01( 10, DHT11 );
DHT dht_02( 32, DHT11 );
DHT dht_03( 33, DHT11 );
#define MAX_DHT_DEV 3 // 3 DHT devices

RGBdriver rgbdriver_01( 22, 23 );
RGBdriver rgbdriver_02( 24, 25 );
RGBdriver rgbdriver_03( 26, 27 );
RGBdriver rgbdriver_04( 28, 29 );
#define MAX_RGB_DEV  4 // 4 RGB devices


void initPins() {
  dht_01.begin();
  dht_02.begin();
  dht_03.begin();
}


// SD card options
#define PIN_SD 4

//const static byte byteActivityLED = 13;
const static byte byteActivityLED = 0;

//long FILE_LOG_VIEW_SIZE = 262144; // 256 K
long FILE_LOG_VIEW_SIZE = 4096; // 4 K

const static long MANDATORY_UPTIME_REBOOT = 
//        86400000; // milliseconds in a day
//        3600000; // hour, 3600s/hr
//        2147483647; // max long, ~24.8 days
        -1; // disable



/* Globals */


byte byteFailedAttempts = 0;
unsigned long lSuccessAttempts = 0;
unsigned long lLastSendTime = 0;
String strLastFailedMessage = "";

const static long MAX_PIN = 13; // pins 0 and 1 are reserved. 2 to 13 are usable [on Uno].
// output pin modes   0=undefined, 1=read mode, 2=write digital, 3=write analog
//                           x  x  2  3  4  5  6  7  8  9 10 11 12 13
static byte arrPinMode[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static byte arrPinLast[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


//static char arrFloatToStr[16];

char sIDRead[7];

String strStarHost;
unsigned int iInterval;
unsigned long lNextSend;

//EthernetServer server(80);

unsigned long lTime;

////char* cTimestamp = malloc( 9 );
//char cTimestamp[20];
////char* cFilename = "1234567890123456789012345678901234567890";
//char cFilename[80];

const static long BUFFER_SIZE = 80;
static char cStringBuffer[ BUFFER_SIZE ];

//File fileLog;



#include "DefineMessages.h"
//#include "CommSerial.h"
//#include "Logging.h"
#include "Utilities.h"

/* Functions */



void log( const String strText ) {
  Serial.print( F("[LOG] " ) );
  Serial.println( strText );
} // if no Logging.h, use this..

void sendSerial( const String strText ) {
  Serial.print( F("[SERIAL] ") );
  Serial.println( strText );
}

//String printTimeValue( EthernetClient client,
//                       const long lTime ) {
//  /* 14 is mininum width, 3 is precision; float value is copied onto str_temp*/
//  float fTime = ((float)lTime) / 1000;
////  dtostrf( fTime, 11, 3, &arrFloatToStr[0] );
//  dtostrf( fTime, 11, 3, arrFloatToStr );
////  sprintf( arrFloatToStr, "%s F", arrFloatToStr );
//  client.print( arrFloatToStr );
//  client.print( F(" s") );
//}


//String printTimeValue( EthernetClient client,
//                       const unsigned long lTime ) {
//  client.print( ((float)lTime) / 1000 );
//  client.print( F(" s") );
//
////  client.print( lTime );
////  client.print( F(" ms") );
//}


String formatTimeValue( const unsigned long lTime ) {
  String strText;
  long lBig = lTime / 1000;
  long lSmall = lTime % 1000;
  sprintf( cStringBuffer, "%07u", lBig );
  strText = String( cStringBuffer ) + F(".");
  sprintf( cStringBuffer, "%03u", lSmall );
  strText = strText + String( cStringBuffer );

  return strText;
}


String formatTimeValue_print( const unsigned long lTime ) {
  String strText;
  long lBig = lTime / 1000;
  long lSmall = lTime % 1000;
  sprintf( cStringBuffer, "%07u", lBig );
  strText = String( cStringBuffer ) + F(".");
  
  Serial.print( strText );

  sprintf( cStringBuffer, "%03u", lSmall );
  strText = strText + String( cStringBuffer );
  
  Serial.print( String( cStringBuffer ) );
  strText = " (" + strText + ")";

  return strText;
}





long getSystemTime() {
  if ( lTime>0 ) {
    return lTime + millis();
  } else {
    return 0;
  }
}



String getSerialNumber() {
  for (int i=0; i<6; i++) {
    sIDRead[i] = EEPROM.read(i);
  }
  String strSerialNumber = String( sIDRead );
  return strSerialNumber;
}


String getVersion() {
  return VERSION;
}



int sendAtom( int iSendCode ) {
  int iResult;
  String strMessage = buildSendMessage( iSendCode );
//  if ( Serial ) {
    iResult = sendAtomSerial( iSendCode, strMessage );
//  } else {
//    iResult = sendAtomHTTP( iSendCode, strMessage );
//  }
  return iResult;
}


String build_Start( int iSendCode ) {
  String str = "{";
  str += "\"code\":";
  str += String( iSendCode );
  
  return str;
}

String build_End() {
  return "}";
}

String build_Digital() {
  String str = "";
  for ( int iD = 2; iD < 14; iD++ ) {
    int iValue = digitalRead( iD );
    str += ",\"D";
    str += String( iD );
    str += "\"=";
    str += String( iValue );
  }
  return str;
}

String build_Analog() {
  String str = "";
  for ( int iA = 0; iA < 6; iA++ ) {
    int iValue = analogRead( iA );
    str += ",\"A";
    str += String( iA );
    str += "\"=";
    str += String( iValue );
  }
  return str;
}

String build_DeviceInfo() {
  String str;
  
  str += ",\"SerNo\"=\"";
  str += getSerialNumber();
  str += "\",\"Ver\"=\"";
  str += getVersion();
  str += "\",\"Mem\"=";
  str += String( freeRam() );

  str += ",\"millis\"=";
  str += String( millis() );
  str += ",\"lTime\"=";
  str += String( lTime );
  str += ",\"iInterval\"=";
  str += String( iInterval );

  return str;
}

String buildSendMessage( int iSendCode ) {
  String str = "";
  
  str += build_Start( iSendCode );
  str += build_DeviceInfo();
  str += build_Analog();
  str += build_Digital();
  
  str += buildDHTSnippet( dht_01, "01" );
  str += buildDHTSnippet( dht_02, "02" );
  str += buildDHTSnippet( dht_03, "03" );

  str += build_End();
  
  return str;
}

//String buildSendMessageDHT11( DHT dht, String strSuffix ) {
//
//  String str = "";
//  // DHT11: temperature and humidity
//
//  // Reading temperature or humidity takes about 250 milliseconds!
//  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
////  // Read temperature as Celsius (the default)
//  // Read temperature as Fahrenheit (isFahrenheit = true)
//  float fTemperature = dht.readTemperature(true);
////  float fTemperature = dht.readTemperature();
//  float fHumidity = dht.readHumidity();
//
//  // Check if any reads failed and exit early (to try again).
//  if ( isnan(fHumidity) || isnan(fTemperature) ) {
////    str += "&Temp=NA&Humid=NA";
//  } else {
//    str += "&Temp" + strSuffix + "=";
//    str += String( fTemperature );
//    str += "&Humid" + strSuffix + "=";
//    str += String( fHumidity );
//  }
//
//  return str;
//}

String buildDHTSnippet( DHT dht,
                        String strSuffix ) {
  
//  dht01.read11( 13 );
  
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float fTemperature = dht.readTemperature(true);
//  float fTemperature = dht.readTemperature();
  float fHumidity = dht.readHumidity();

  String str = "";
//  String strSuffix = "01";

  // Check if any reads failed and exit early (to try again).
  if ( isnan(fHumidity) || isnan(fTemperature) ) {
//    str += "&Temp=NA&Humid=NA";
  } else {
    str += ",\"Temp" + strSuffix + "\"=";
    str += String( fTemperature );
    str += ",\"Humid" + strSuffix + "\"=";
    str += String( fHumidity );
  }

  return str;
}




int sendAtomSerial( int iSendCode,
                    String strMessage ) {
  log( F("--> sendAtomSerial()") );
  
//  Serial.println( F("--> sendAtom()") );
  
  // light up activity LED
  if ( byteActivityLED>0 ) { digitalWrite( byteActivityLED, HIGH ); }

  Serial.print( strMessage );

  byteFailedAttempts = 0;
  lSuccessAttempts = lSuccessAttempts + 1;
  
  // turn off activity LED
  if ( byteActivityLED>0 ) { digitalWrite( byteActivityLED, LOW ); }

  // return
//  String strResult = "Atom sent, response: " + strResponse;
//  Serial.println( F("<-- sendAtom(), normal") );
//  Serial.print( F("    sendAtom(), response = ") );
//  Serial.println( strResponse );
//  return strResult;
  log( F("<-- sendAtomSerial(), success") );
  return MSG_SEND_SUCCESS;
}





int processRequestSet( String strName,
                       String strValue,
                       boolean bAllowSave ) {

  log( "Request to set: " + strName + " to " + strValue );                         
                         


//    String strMessage = F("(no message)");
    String strMsgText = "";
    int iMsgCode = 0;
//    boolean bSendFullResponse = true;
//    int iResponseMessageFormat = 2; // 1 = brief, 2 = full, 3 = showlog
   
      if ( false ) {
  
//      Serial.println( "(command is to set)" );

      } else if ( strName.equals( F("interval") ) ) {

//        if ( bAllowSave ) {
//          saveConfig( "interval", strValue );
//        }

        String strInterval = strValue;
        strInterval.trim();
        int iValue = strInterval.toInt();
        
        if ( strInterval.equals( F("0") ) ) {
          
          iInterval = 0;
          scheduleSend( 0 );
//          strMessage = "Schedule send disabled.";
          iMsgCode = MSG_SCHEDULE_DISABLED;
          
        } else if ( iValue>0 ) {
  
          iInterval = iValue;
          scheduleSend( getSystemTime() );
//          strMessage = "interval set to " + String( iValue ) + ".";
          iMsgCode = MSG_INTERVAL_SET;

        } else {
          
//          strMessage = "Invalid interval value: \"" + strValue + "\".";
          iMsgCode = MSG_INTERVAL_INVALID_VALUE;
          strMsgText = strValue;
          
        }
        
      } else if ( strName.equals( F("time") ) ) {

log( "Setting time: strValue = " + strValue );
        
        String strTime = strValue;
//log( "Setting time: strTime (1) = \"" + strTime + "\"" );
        strTime.trim();
//log( "Setting time: strTime (2) = \"" + strTime + "\"" );
        long lValue = strTime.toInt();

//log( "Setting time: strTime.toInt().." );

        String strTest = String( lValue );

//log( "Setting time: strTest = " + strTest );
        
//log( "Setting time: lValue = " + lValue );
        if ( lValue>0 ) {
//log( "Setting time (lValue>0)" );
          
          const long lRunningTime = millis();
          lTime = lValue - lRunningTime;
//          strMessage = "Time offset set to " + String( lTime ) + ".";
          iMsgCode = MSG_TIME_OFFSET_SET;
          strMsgText = strTime;

          scheduleSend( getSystemTime() );
          
        } else {
log( "Setting time (invalid time?)" );
//          strMessage = "Invalid time value: \"" + strValue + "\".";
          iMsgCode = MSG_TIME_OFFSET_INVALID_VALUE;
          strMsgText = strValue;
        }
        
      } else {
//          strMessage = "Invalid variable: \"" + strName + "\".";
        iMsgCode = MSG_INVALID_VARIABLE;
        strMsgText = strName;
      }


  return iMsgCode;
}

void setRGBDriver(  RGBdriver rgb,
                    long lR,
                    long lG,
                    long lB ) {
Serial.print( "setRGBDriver( *, " + String(lR) + ", " + String(lG) + ", " + String(lB) + " )" );                      
  rgb.begin();
  rgb.SetColor( lR, lG, lB );
  rgb.end();
}

//int processRequest( EthernetClient client ) {
int processRequestSerial( String strRequest ) {

    /* pull the request from the stream */
    
//    // light up activity LED
//    digitalWrite( byteActivityLED, HIGH );

//    String strRequest;
//    boolean bRead = true;
//    while ( bRead ) {
//
//      if ( client.available() ) {
//        const int i = client.read();
//        if ( -1==i ) {
//          bRead = false;
//        } else {
//          char c = (char) i;
//          strRequest = strRequest + c;
//        }
//      }
//      
//      if ( strRequest.indexOf( F("HTTP") )>-1 ) {
//        bRead = false;
//      }
//    }

//    // turn off activity LED
//    digitalWrite( byteActivityLED, LOW );
    
    /* extract info the request */
    
    const String strOriginal = strRequest;
    
    String strTemp;
    String strMessage = "";
    
    pop( strRequest, " " );
    strRequest = pop( strRequest, "HTTP" );
    strRequest.trim();

    log( "Incoming request: \"" + strRequest + "\"" );
    
    String strCommand = strRequest + "/";
    const int iPosQMark = strCommand.indexOf( F(":") );
    const int iPosSlash = strCommand.indexOf( F("/"), 2 );
    if ( iPosQMark<0 || iPosQMark>iPosSlash ) {
      strCommand = strCommand.substring( 0, iPosSlash );
    } else {
      strTemp = strCommand;
      strCommand = pop( strTemp, ":" );
    }
    strCommand.toLowerCase();

    log( "\tstrCommand=" + strCommand );
    
    String strParams = strRequest;
    strParams = strParams.substring( strCommand.length() + 1 );

//    log( "strParams=" + strParams );

    String strName = strParams;
    strTemp = strName;
    strName = pop( strTemp, F("=") );
    strName.toLowerCase();

    log( "\tstrName=" + strName );
    
    String strValue = strParams;
    if ( strValue.indexOf( F("=") )>-1 ) {
      pop( strValue, "=" );
    } else {
      strValue = "";
    }

    log( "\tstrValue=" + strValue );
    
//    #if defined( DEFINE_DEBUG )
//      Serial.print( F("strRequest = ") );
//      Serial.println( strRequest );
//      Serial.print( F("strCommand = ") );
//      Serial.println( strCommand );
//      Serial.print( F("strParams = ") );
//      Serial.println( strParams );
//      Serial.print( F("strName = ") );
//      Serial.println( strName );
//      Serial.print( F("strValue = ") );
//      Serial.println( strValue );
//    #endif

   
    /* process the changes requested */ 
    
//    String strMessage = F("(no message)");
    String strMsgText = "";
    int iMsgCode = 0;
//    boolean bSendFullResponse = true;
    int iResponseMessageFormat = 2; // 1 = brief, 2 = full, 3 = showlog
   
    if ( strCommand.equals( F("set") ) ) {
//    if ( strCommand.equals( OP_SET ) ) {

  
      log( F("Command: set") );
      
      
      iMsgCode = processRequestSet( strName, strValue, true );
      
      
      
      
    } else if ( strCommand.equals( F("send") ) ) {
//    } else if ( strCommand.equals( OP_SEND ) ) {

      log( F("Command: send") );
  
//      Serial.println( F("(request to send atom)") );
      
      iMsgCode = sendAtomSerial( SEND_CODE_REQUESTED, "<respond to send request>" );
      //strMsgText = "";

      if ( strName.equals( "fast" ) ) {
        iResponseMessageFormat = 1;
      }

//      strMessage = "Request to send atom, result: " + strResult;
//      strMsgText = strResult;

    } else if ( strCommand.equals( F("mode") ) ) {

      log( F("Command: mode") );

      const long lPin = strName.toInt();
      if ( lPin>1 && lPin<=MAX_PIN ) {
        const long lValue = strValue.toInt();
        if ( lValue>0 && lValue<6 ) {
          arrPinMode[lPin] = lValue;
          
          if ( 1==lValue ) {         // mode 1 (read)
            pinMode( lPin, INPUT );
          } else if ( 2==lValue ) {  // mode 2 (write, digital)
            pinMode( lPin, OUTPUT );
          } else if ( 3==lValue ) {  // mode 3 (write, PWM)
            pinMode( lPin, OUTPUT );
            
//          } else if ( 4==lValue ) {  // mode 4: DHT11
//            dht_01.begin();
//
//          } else if ( 5==lValue ) {  // mode 5: RGBDriver
//            rgbdriver_01.begin();
          }

          strMessage = "Pin " + strName + " set to mode " + strValue;
          iMsgCode = MSG_SET_PIN_MODE_SUCCESS;
          strMsgText = strName;
        } else {
          strMessage = "Failed to set pin mode. Invalid mode: " + strValue;
//          strMessage = "Invalid mode: " + strValue;
          iMsgCode = MSG_SET_PIN_MODE_INVALID_MODE;
          strMsgText = strValue;
        }
      } else {
        strMessage = "Failed to set pin mode. Invalid pin: " + strName;
//        strMessage = "Invalid pin: " + strName;
        iMsgCode = MSG_SET_PIN_MODE_INVALID_PIN;
        strMsgText = strName;
      }
           
    } else if ( strCommand.equals( F("write_rgb") ) ) {

      log( F("Command: write_rgb") ); // format: write_rgb:1=0,128,255
                                      //              pin -^ ^-^^^-^^^- RGB values

      const long lDev = strName.toInt();
      if ( lDev>0 && lDev<=MAX_RGB_DEV ) {

        const String strR = pop( strValue, F(",") );
        const String strG = pop( strValue, F(",") );
        const String strB = strValue;

        const long lR = strR.toInt();
        const long lG = strG.toInt();
        const long lB = strB.toInt();

        if ( 1==lDev ) {
          setRGBDriver( rgbdriver_01, lR, lG, lB );
        } else if ( 2==lDev ) {
          setRGBDriver( rgbdriver_02, lR, lG, lB );
        } else if ( 3==lDev ) {
          setRGBDriver( rgbdriver_03, lR, lG, lB );
        } else if ( 4==lDev ) {
          setRGBDriver( rgbdriver_04, lR, lG, lB );
        }

      } else {
        strMessage = "Failed to set device. Invalid device: " + strName;
//        strMessage = "Invalid pin: " + strName;
        iMsgCode = MSG_SET_PIN_MODE_INVALID_PIN;
        strMsgText = strName;
      }
            
    } else if ( strCommand.equals( F("write") ) ) {
//    } else if ( strCommand.equals( OP_WRITE ) ) {

      log( F("Command: write") );

      const long lPin = strName.toInt();
      if ( lPin>1 && lPin<=MAX_PIN ) {
        
        const long lValue = strValue.toInt();
        const byte byteMode = arrPinMode[lPin];

//      log( F("  write: lValue = " ) + String( lValue ) );
//      log( F("  write: byteMode = " ) + String( byteMode ) );
        
        if ( 2==byteMode ) { // write digital
          if ( lValue<2 ) {
            digitalWrite( lPin, lValue );
            iMsgCode = MSG_WRITE_DIGITAL_SUCCESS;
            strMsgText = strName;
          } else {
            strMessage = "Attempt to write failed: value is invalid for digital pin: " + strValue;
//            strMessage = "Value is invalid for digital pin: " + strValue;
            iMsgCode = MSG_WRITE_DIGITAL_INVALID_VALUE;
            strMsgText = strValue;
          }
        } else if ( 3==byteMode ) { // write PWM
          if ( lValue<1024 ) {
            analogWrite( lPin, lValue );
            iMsgCode = MSG_WRITE_PWM_SUCCESS;
            strMsgText = strName;
          } else {
            strMessage = "Attempt to write failed: value is invalid for PWM pin: " + strValue;
//            strMessage = "Value is invalid for PWM pin: " + strValue;
            iMsgCode = MSG_WRITE_PWM_INVALID_VALUE;
            strMsgText = strValue;
          }
        } else {
          strMessage = "Attempt to write failed: Pin is invalid mode: " + byteMode;
//          strMessage = "Pin is in invalid mode: " + byteMode;
            iMsgCode = MSG_WRITE_PIN_INVALID_MODE;
            strMsgText = String( byteMode );
        }

      } else {
        strMessage = "Failed to write to pin. Invalid pin: " + strName;
//        strMessage = "Invalid pin: " + strName;
        iMsgCode = MSG_WRITE_PIN_INVALID_PIN;
        strMsgText = strName;
      }
                  
    } else if ( strCommand.equals( F("read") ) ) {
//    } else if ( strCommand.equals( String( OP_READ ) ) ) {

      log( F("Command: read") );

      strMessage = buildSendMessage( MSG_OP_READ_SUCCESS );
      sendSerial( strMessage );
      
//      Serial.println( F("(command is to read)") );

//      strMessage = F("Read request recognized.");
      iMsgCode = MSG_OP_READ_SUCCESS;
//      strMsgText = ""; // nothing to say
      
    } else if ( strCommand.equals( F("showlog") ) ) {

      log( F("Command: showlog") );

    } else if ( strCommand.equals( F("reboot") ) ) {

      log( F("Command: reboot") );

      Serial.println( F("Command: reboot") );

//      strMessage = F("Read request recognized.");
      iMsgCode = MSG_OP_REBOOT_QUEUED;
//      strMsgText = ""; // nothing to say
      
    } else {

      log( F("Unknown command") );
      
//      Serial.println( F("(command is unknown)") );
      
//      strMessage = "Unknown command: \"" + strCommand + "\".";
//      strMessage = "Unknown command: \"" + strCommand + "\", available commands:\n";
//          + OP_SET + " (" + FIELD_HOSTNAME + COMMA
//              + FIELD_HOST_IP + COMMA + FIELD_INTERVAL + COMMA + FIELD_TIME + F(")") + COMMA
//          + OP_READ + COMMA 
//          + OP_SEND;
      iMsgCode = MSG_OP_UNKNOWN;
      strMsgText = strCommand;
    }
    
    
    if ( strMessage!="" ) {
      log( "strMessage = " + strMessage );
    }
    
    
    /* write the response back to the client */
    
    
    
    // light up activity LED
    if ( byteActivityLED>0 ) { digitalWrite( byteActivityLED, HIGH ); }

//    Serial.println( "Sending response back to client.." );

    // send a standard http response header
//    client.println( F("HTTP/1.1 200 OK" ) );
//    client.println( F("Connection: close" ) );  // the connection will be closed after completion of the response
//    client.println("Refresh: 1");  // refresh the page automatically every 5 sec
//    client.println();
    
    if ( 1==iResponseMessageFormat ) {

      log( F("Sending brief message response") );
      
//      client.println( F("Content-Type: text/plain" ) );
//      client.println( F("Connection: close" ) );  // the connection will be closed after completion of the response
//      client.println();
//      client.print( F("iMsgCode: " ) );
//      client.println( String( iMsgCode ) );
      
//      sendHTTPHTMLHeader( client );
//      client.print( F("<tt>iMsgCode: " ) );
//      client.println( String( iMsgCode ) );
//      client.print( F("</tt></html>" ) );

//      sendHTTPTextHeader( client );
//      client.print( F("iMsgCode: " ) );
//      client.println( String( iMsgCode ) );
      
    } else if ( 3==iResponseMessageFormat ) {
//
//      log( F("Sending file log contents as response") );
//      
////      sendHTTPTextHeader( client );
//
//#if defined( DEFINE_DEVICE_MEGA )
////      File fileLogRead = SD.open( FILE_LOG, FILE_READ );
////      if ( fileLogRead ) {
////        while ( fileLogRead.available() ) {
////          const char c = fileLogRead.read();
////          client.print( c );
////        }
////        fileLogRead.close();
////      }
//
//
//      File fh = SD.open( FILE_LOG );
//      if (fh) {
//       
//        long lSize = fh.size();
//        if ( lSize > FILE_LOG_VIEW_SIZE ) {
//          long lPos = lSize - FILE_LOG_VIEW_SIZE;
//          fh.seek( lPos );
//
//          boolean bScan = fh.available();
//          char c;
//          while ( bScan ) {
//            c = fh.read();
//            bScan = ( c!='\n' && fh.available() );
//          }
//        }
//        
//        byte clientBuf[64];
//        int clientCount = 0;
// 
//        while ( fh.available() ) {
//          clientBuf[clientCount] = fh.read();
//          clientCount++;
// 
//          if(clientCount > 63) {
////            Serial.println("Packet");
//            client.write(clientBuf,64);
//            clientCount = 0;
//          }
//        }
//     
//        if (clientCount > 0) {
//          client.write( clientBuf, clientCount );
//        }
//
//        fh.close();
//      } else {
////        Serial.println("file open failed");
//          client.print( F("Failed to open file.") );
//      }
//      
//#else
//      client.println( F( "No log file available." ) );
//#endif

    } else /*if ( 2==iResponseMessageFormat )*/ {

      log( F("Sending complete response") );
//
////      sendHTTPHTMLHeader( client );
//
////      client.println( F("Content-Type: text/html" ) );
////
////      client.println( F("Connection: close" ) );  // the connection will be closed after completion of the response
////      client.println();
////
////      client.println( F("<!DOCTYPE HTML>" ) );
////      client.println( F("<html>" ) );
//      
//      client.println( F("<font face='verdana'><table border='1' cellpadding='4'>" ) );
//      
//      printSection( client, F("Configuration") );
//      printNameValue( client, F("Serial Number"), getSerialNumber() );
//      
//  //    printNameValue( client, F("MAC Address"), getMACAddress() );
//      // print mac address
//      client.print( F( "<tr><td colspan='2'>MAC Address</td><td><tt>" ) );
//      printMACAddress( client );
//      client.print( F( "</tt></td></tr>" ) );    
//  
//  //    client.println( printNameValue( "MAC Address", macPlanet ) );
//      printNameValue( client, F("Sketch Version"), getVersion() );
//  
//      printNameValue( client, F("SRAM"), String( freeRam() ) + F(" bytes") );
//      
//  
//      client.print( F( "<tr><td colspan='3' align='center'> Star Host </td></tr>" ) );    
//  
//      printNameValue( client, F("Hostname"), F("hostname"), strStarHost );
//      // print star host IP
//      client.print( F( "<tr><td>Host IP</td><td><tt>host_ip</tt></td><td><tt>" ) );
//      printStarIP( client );
//      client.print( F( "</tt></td></tr>" ) );    
//  
//      printNameValue( client, F("Send, total success"), String( lSuccessAttempts ) );
//      printNameValue( client, F("Send, recent failed"), String( byteFailedAttempts ) );
//      printNameValue( client, F("Send, last failed message"), strLastFailedMessage );
//      
////      printNameValue( client, F("Last success time"), String( lLastSendTime ) );
//      printNameValue( client, F("Last success time"), formatTimeValue( lLastSendTime ) );
//  //    client.print( F( "<tr><td colspan='2'> Last success time </td><td><tt>" ) );
//  //    printTimeValue( client, lLastSendTime );
//  //    client.println( F( "</tt>\n</td></tr>" ) );    
//  
//      client.print( F( "<tr><td colspan='3' align='center'>Time (in ms)</td></tr>" ) );
//      
////      printNameValue( client, F("Running time"), String( millis() ) );
//      printNameValue( client, F("Running time"), formatTimeValue( millis() ) );
//  //    client.print( F( "<tr><td> Running time </td><td><tt> time </tt></td><td><tt>" ) );
//  //    printTimeValue( client, millis() );
//  //    client.println( F( "</tt>\n</td></tr>" ) );
//      
////      printNameValue( client, F("System time"), F("time"), String( getSystemTime() ) );
//      printNameValue( client, F("System time"), F("time"), formatTimeValue( getSystemTime() ) );
//  //    client.print( F( "<tr><td colspan='2'> System time </td><td><tt>" ) );
//  //    printTimeValue( client, getSystemTime() );
//  //    client.println( F( "</tt>\n</td></tr>" ) );    
//          
////      printNameValue( client, F("Send interval"), F("interval"), String( iInterval ) );
//      printNameValue( client, F("Send interval"), F("interval"), formatTimeValue( iInterval ) );
////      printNameValue( client, F("Scheduled send"), String( lNextSend ) );
//      printNameValue( client, F("Scheduled send"), formatTimeValue( lNextSend ) );
//
//
//#if defined( DEFINE_DEVICE_MEGA )
//      printSection( client, F("Storage") );
//      
//      String strStatus;
//
////      Sd2Card card;
////      SdVolume volume;
////      
////      if ( card.init( SPI_HALF_SPEED, PIN_SD ) ) {
////        switch (card.type()) {
////          case SD_CARD_TYPE_SD1:
////            strStatus = F("SD1");
////            break;
////          case SD_CARD_TYPE_SD2:
////            strStatus = F("SD2");
////            break;
////          case SD_CARD_TYPE_SDHC:
////            strStatus = F("SDHC");
////            break;
////          default:
////            strStatus = F("Unknown");
////        }        
////        printNameValue( client, F("SD card type"), strStatus );
////        
////        if ( volume.init( card ) ) {
////          
////          uint32_t volsize;
////          volsize = volume.blocksPerCluster() * volume.clusterCount() * 512;
////          printNameValue( client, F("Volume size"), String( volsize ) + " bytes" );
////          volsize = volsize / 1024 / 1024;
////          printNameValue( client, F("Volume size"), String( volsize ) + " MB" );
////          
////        } else {
////          printNameValue( client, F("Volume"), F("Init failed") );
////        }
////
////      } else {
////        printNameValue( client, F("SD card"), F("Init failed") );
////      }
//
//      
//      File fileLog = SD.open( FILE_LOG );
//      if (fileLog) {
//        long lSize = fileLog.size();
//        lSize = lSize / 1024;
//        strStatus = String( lSize ) + " KB";
//        fileLog.close();
//      } else {
//          strStatus = F("File could not be opened");
//      }
//      printNameValue( client, F("Log file size"), strStatus );
//
//#endif  
//      
//      printSection( client, F("Request Details") );
//      printNameValue( client, F("strOriginal"), strOriginal );
//      printNameValue( client, F("strRequest"), strRequest );
//      printNameValue( client, F("strCommand"), strCommand );
//      printNameValue( client, F("strParams"), strParams );
//      printNameValue( client, F("strName"), strName );
//      printNameValue( client, F("strValue"), strValue );
//      
//      printSection( client, F("Results") );
//  //    printNameValue( client, F("strMessage"), strMessage );
//      client.print( F( "<tr><td>Message</td><td>" ) );
//      client.println( String( iMsgCode ) );
//      client.println( F( "</td><td>" ) );
//      client.println( strMsgText );
//      client.println( F( "</td></tr>" ) );    
//      
//  //    client.println( printSection( "Data Points" ) );
//      printSection( client, F("Data Points") );
//      // output the value of each analog input pin
//      client.print( F( "<tr><td colspan='3' align='center'>Analog Inputs</td></tr>" ) );    
//      for ( int iA = 0; iA < 6; iA++ ) {
//        int iValue = analogRead( iA );
//        client.print( F( "<tr><td colspan='2'>" ) );
//        client.print( F( "Analog Input " ) );
//        client.print( String( iA ) );
//        client.print( F( "</td><td><tt>" ) );
//        client.print( String( iValue ) );
//        client.println( F( "</tt></td></tr>" ) );
//      }
//      client.print( F( "<tr><td colspan='3' align='center'>Digital Inputs</td></tr>" ) );    
//      for ( int iD = 2; iD <= MAX_PIN; iD++ ) {
//        int iValue = digitalRead( iD );
//        client.print( F( "<tr><td>Digital Input" ) );
//        client.print( String( iD ) );
//        client.print( F( "</td><td>" ) );
//        const byte byteMode = arrPinMode[iD];
//        if ( 0==byteMode ) {
//          client.print( F("0:undef") );
//        } else if ( 1==byteMode ) {
//          client.print( F("1:input") );
//        } else if ( 2==byteMode ) {
//          client.print( F("2:out digital") );
//        } else if ( 3==byteMode ) {
//          client.print( F("3:out PWM") );
//        } else {
//          client.print( String( byteMode ) );
//          client.print( F(":INVALID") );
//        }
//        client.print( F( "</td><td><tt>" ) );
//        client.print( String( iValue ) );
//        client.println( F( "</tt></td></tr>" ) );
//      }
//  
//      client.println( F("</table></font>" ) );
//    
//      client.println( F("</html>" ) );
    }
    
//  Serial.println( "Response sent completely." );

    
    
  // turn off activity LED
  if ( byteActivityLED>0 ) { digitalWrite( byteActivityLED, LOW ); }
  
  log( "Output code: " + String( iMsgCode ) );
  
  return iMsgCode;
}



void scheduleSend( long lTimeReference ) {
  if ( iInterval>0 && lTimeReference>0 ) {
    lNextSend = lTimeReference + iInterval;
  } else {
    lNextSend = 0;
  }
}


boolean hasDataChanged() {
  
  boolean bResult = false;
  
  for ( int iD = 2; iD <= MAX_PIN; iD++ ) {
    const byte byteMode = arrPinMode[iD];
    if ( 1==byteMode ) { // digital input
    
      const byte byteValue = digitalRead( iD );
      const byte byteLast = arrPinLast[iD];

      if ( byteValue != byteLast ) {
        arrPinLast[iD] = byteValue;
        bResult = true;
      }    
    }
  }
  return bResult;
}














String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete





















void setup() {
  // put your setup code here, to run once:
//  Serial.begin(9600);
  Serial.begin(115200);
  inputString.reserve(200);
  if ( byteActivityLED>0 ) { pinMode( byteActivityLED, OUTPUT ); }

#if defined( DEFINE_DEVICE_MEGA )
  if ( !SD.begin( PIN_SD ) ) {
    Serial.print( F( "Failed to initialize SD card." ) );
  } else {
    if ( !SD.exists( SD_DIRS ) ) {
      SD.mkdir( SD_DIRS );
    }
  }
//#else
//  fileLog = 0;
#endif  
  
  log( F("-----------------------------------------------------------") );
  log( F("--> setup()") );

  //TODO remove  
  Serial.println( formatTimeValue( millis() ) );

//  dht02.begin();
//  dht03.begin();
//  dht04.begin();

  sendAtomSerial( SEND_CODE_NODE_INIT, "<SEND_CODE_NODE_INIT>" );

  initPins();

  log( F("<-- setup()") );
}







/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      processRequestSerial( inputString );
      inputString = "";
      stringComplete = true;
    } else if (inChar == ';') {
      processRequestSerial( inputString );
      inputString = "";
      stringComplete = true;
    } else {
      inputString += inChar;
    }
  }
}





long lCounter = 0;

void loop() {
  // put your main code here, to run repeatedly:
  lCounter = lCounter + 1;

  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;
  }


//  if ( 0==(lCounter % ( 10 * 10000 ) ) ) { // every 10 seconds..
////    Ethernet.maintain(); // do NOT use DHCP: not enough room for program
//    Serial.print( F("lCounter:") );
//    Serial.print( lCounter );
//    Serial.print( F(", millis():") );
//    Serial.println( formatTimeValue_print( millis() ) );
//  }


  /* check conditions to Send to Star */
  {
    boolean bUpdateTime = false;
  
    if ( hasDataChanged() ) {
      sendAtomSerial( SEND_CODE_DIGITAL_CHANGE, "<SEND_CODE_DIGITAL_CHANGE:digital change>" );
      
      String strMessage = build_Start( SEND_CODE_DIGITAL_CHANGE );
      strMessage += build_Digital();
      strMessage += build_End();
      sendSerial( strMessage );
      
//      bUpdateTime = true;
    }
  
    /* check to see if there is a Send scheduled */
    const long lTimeNow = getSystemTime();
    if ( (lTimeNow>0) && (lNextSend>0) && (lTimeNow>lNextSend) ) {
      
//      sendAtomSerial( SEND_CODE_SCHEDULED, "<SEND_CODE_SCHEDULED:send scheduled>" );


      String strMessage = buildSendMessage( SEND_CODE_SCHEDULED );
      sendSerial( strMessage );
      
      bUpdateTime = true; 
    }
 
    if ( bUpdateTime ) {
      scheduleSend( lTimeNow );
    }
  }
  
  
  /* check for mandatory reboot */
  if ( ( MANDATORY_UPTIME_REBOOT > 0 ) 
        && ( millis() >= MANDATORY_UPTIME_REBOOT ) ) {
    reboot();
  }
  
  

  // turn off activity LED
  if ( byteActivityLED>0 ) { digitalWrite( byteActivityLED, LOW ); }
}

