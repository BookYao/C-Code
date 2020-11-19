/* * * * * * * * * * * * * * * * * * * * * * * *
*
* FileName: syscfg.h
*
* Author: 
*
* Time: 2014-08-07 11:27
*
* History:1.0.0.0 
* =============================================
*
* Description:
* get value from ini file 
*
* * * * * * * * * * * * * * * * * * * * * * * */

#ifndef SYSCONF_H
#define SYSCONF_H

#define CONFIG_MAX_LENGTH        80
#define DEFAULT_BASE_DIR        "/usr/local/opa/configs/"         /* file path  */
#define CONFIG_FILE             "sgip.conf"                       /* ini file   */

typedef enum {
  NO_ERROR = 1,
  MAJOR_ERROR = 0,
  CRITICAL_ERROR = -1,
  MINOR_ERROR = -2
} eConfigErr;

#ifdef __cplusplus
extern "C"
{

#endif

void sysconfReset(void);
eConfigErr sysconfGet(char *section, char *param, char *value);

#ifdef __cplusplus
}
#endif

#endif
