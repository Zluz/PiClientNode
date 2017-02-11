
// #include "Arduino.h"


/* Constants */

const static char* FILE_LOG  = "ARDUINO/ARDUINO.LOG";
static char* SD_DIRS = "ARDUINO/CONFIG/";
//const static String COMMA          PROGMEM = ", ";
//const static String OP_SET         PROGMEM = "/set";
//const static String OP_SEND        PROGMEM = "/send";
//const static String OP_MODE        PROGMEM = "/mode";
//const static String OP_READ        PROGMEM = "/read";
//const static String OP_WRITE       PROGMEM = "/write";

//const static String FIELD_HOSTNAME PROGMEM = "hostname";
//const static String FIELD_HOST_IP  PROGMEM = "host_ip";
//const static String FIELD_INTERVAL PROGMEM = "interval";
//const static String FIELD_TIME     PROGMEM = "time";

//const static char VERSION[] PROGMEM = "20160906_001";
//const static char COMMA[] PROGMEM = {", "};
//const static char OP_SET[] PROGMEM = "/set";
//const static char OP_READ[] PROGMEM = {"/read"};
//const static char OP_SEND[] PROGMEM = {"/send"};
//const static char FIELD_HOSTNAME[] PROGMEM = {"hostname"};
//const static char FIELD_HOST_IP[] PROGMEM = {"host_ip"};
//const static char FIELD_INTERVAL[] PROGMEM = {"interval"};
//const static char FIELD_TIME[] PROGMEM = {"time"};




// replace  strMessge
//   with   iMsgCode, strMsgText

#define MSG_SCHEDULE_DISABLED			1001

#define MSG_STAR_HOSTNAME_SET			1002
#define MSG_STAR_IP_SET				1003
#define MSG_INTERVAL_SET			1004
#define MSG_INTERVAL_INVALID_VALUE		1005
#define MSG_TIME_OFFSET_SET			1006
#define MSG_TIME_OFFSET_INVALID_VALUE	        1007

#define MSG_SET_PIN_MODE_SUCCESS		1008	// text is pin
#define MSG_SET_PIN_MODE_INVALID_MODE		1009
#define MSG_SET_PIN_MODE_INVALID_PIN		1010
#define MSG_SET_PIN_MODE_SUCCESS		1011

#define MSG_WRITE_DIGITAL_SUCCESS		1012	// text is pin
#define MSG_WRITE_DIGITAL_INVALID_VALUE		1013

#define MSG_WRITE_PWM_SUCCESS			1014	// text is pin
#define MSG_WRITE_PWM_INVALID_VALUE		1015

#define MSG_WRITE_PIN_INVALID_MODE		1016
#define MSG_WRITE_PIN_INVALID_PIN		1017

#define MSG_INVALID_VARIABLE			1101
// #define MSG_OP_SEND				1102
// #define MSG_OP_SEND_FAST_SUCCESS		1104

#define MSG_SEND_FAILED_TO_CONNECT		1105
#define MSG_SEND_FAILED_NO_STAR_HOST	        1108
#define MSG_SEND_FAILED_NO_CLIENT		1106
#define MSG_SEND_SUCCESS			1107

#define MSG_OP_UNKNOWN				1100

#define MSG_OP_READ_SUCCESS			1103
#define MSG_OP_REBOOT_QUEUED			1109
#define MSG_OP_SHOW_LOG_SUCCESS			1110
#define MSG_OP_SHOW_LOG_ERR_NO_SD		1111


#define ERR_CLIENT_NOT_CONNECTED                3000


#define SEND_CODE_NODE_INIT         2000
#define SEND_CODE_SCHEDULED         2001
#define SEND_CODE_DIGITAL_CHANGE    2002
#define SEND_CODE_ANALOG_CHANGE     2003
#define SEND_CODE_TRIGGER           2004
#define SEND_CODE_REQUESTED         2005
#define SEND_CODE_NOT_SUPPORTED     2006




