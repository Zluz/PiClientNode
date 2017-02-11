
//char cTimestamp[20];
//char* cTimestamp = malloc( 9 );

//char* cTimestamp = malloc( 9 );
char cTimestamp[20];
//char* cFilename = "1234567890123456789012345678901234567890";
//char cFilename[80];

void log( const String strText ) {
#if defined( DEFINE_DEVICE__MEGA__NETWORK )
  File fileLog = SD.open( FILE_LOG, FILE_WRITE );
  if ( fileLog ) {
    
    // 86400000
    // 12345678
    unsigned long time = millis();
    sprintf( cTimestamp, "%08u", millis() );

    long lTime = millis();
    long lBig = lTime / 1000;
    long lSmall = lTime % 1000;
    sprintf( cStringBuffer, "%05u", lBig );
    fileLog.print( cStringBuffer );
    fileLog.print( F(".") );
    sprintf( cStringBuffer, "%03u", lSmall );
    fileLog.print( cStringBuffer );
 
//    fileLog.print( cTimestamp );
//    fileLog.print( formatTimeValue( millis() ) );
    
    fileLog.print( F(" ") );
    fileLog.println( strText );
    fileLog.close();
  }
#endif
  Serial.print( F( "log> " ) );
  Serial.println( strText );
}



