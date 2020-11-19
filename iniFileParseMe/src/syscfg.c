/*
 * $Id: syscfg.c,v 1.1 2014/08/13 06:11:13 hyao Exp $
 *
 *      Copyright (c) 2003 Anta Systems.
 *      All rights reserved.
 *      Use of copyright notices does not imply publication.
 *
 *      This Document contains Confidential Information or Trade Secrets,
 *      or both, which are the property of Anta Systems.
 *      This document may not be copied, reproduced, reduced to any
 *      electronic medium or machine readable form or otherwise duplicated
 *      and the information herein may not be used, disseminated or
 *      otherwise disclosed, except with the prior written consent of
 *      Anta Systems.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <syscfg.h>
#include <ctype.h>

/******************************************************************
 * config file parsing
 */

#define MAX_LINE_SIZE           512

typedef enum {
    CONFIGSTATUS_OK = 0,
    CONFIGSTATUS_ENVIRON_NOT_SET,
    CONFIGSTATUS_FILE_NOT_EXIST,
    CONFIGSTATUS_FILE_NOT_OPEN,
    CONFIGSTATUS_STR_TOO_LONG,
    CONFIGSTATUS_SECTION_TOO_LONG,
    CONFIGSTATUS_NOT_FOUND,
    CONFIGSTATUS_ILLEGAL_SYNTAX
} eConfigStatus;

#if 0
static char *_vErrorStr[] = {
    (char *) "OK",
    (char *) "Enviroment variable CONFIG_FILE not set",
    (char *) "Configuration file does not exist",
    (char *) "Configuration file was not opened",
    (char *) "Configuration parameter or its value too long",
    (char *) "Number of section exceeds",
    (char *) "Configuration parameter not found",
    (char *) "Illegal syntax in configuration file"
};
#endif

typedef struct {
#define ATTRIBUTE_NAME_SIZE 50
    char name[ATTRIBUTE_NAME_SIZE + 1];
#define ATTRIBUTE_VALUE_SIZE (MAX_LINE_SIZE - ATTRIBUTE_NAME_SIZE)
    char value[ATTRIBUTE_VALUE_SIZE + 1];
} tAttribute;

typedef struct {
#define SECTION_NAME_SIZE 50
    char name[SECTION_NAME_SIZE + 1];
#define MAX_ATTRIBUTE           50
    int currentAttribute;
    tAttribute attribute[MAX_ATTRIBUTE];
} tSection;

typedef struct {
#define MAX_FILE_NAME_SIZE      120
    char fileName[MAX_FILE_NAME_SIZE + 1];
    FILE *cfp;
    eConfigStatus status;
    int currentLine;
    int currentSection;
#define MAX_SECTIONS            25
    tSection section[MAX_SECTIONS];
} tSysConfig;

static tSysConfig _vConfig = { "", NULL, CONFIGSTATUS_FILE_NOT_OPEN, 0, 0 };
static pthread_mutex_t _vConfigLock = PTHREAD_MUTEX_INITIALIZER;

#define CURRENT_LINE            _vConfig.currentLine
#define CURRENT_SECTION_INDEX   _vConfig.currentSection
#define CURRENT_ATTRIBUTE_INDEX _vConfig.section[CURRENT_SECTION_INDEX].currentAttribute

#define CURRENT_SECTION         _vConfig.section[CURRENT_SECTION_INDEX]
#define CURRENT_ATTRIBUTE       CURRENT_SECTION.attribute[CURRENT_ATTRIBUTE_INDEX]

#define CURRENT_SECTION_NAME    CURRENT_SECTION.name
#define CURRENT_ATTRIBUTE_NAME  CURRENT_ATTRIBUTE.name
#define CURRENT_ATTRIBUTE_VALUE CURRENT_ATTRIBUTE.value

#define SECTION_NAME(n)         _vConfig.section[n].name

static void
_filterComment(char *p_str_ptr)
{

    char *ptr;

    ptr = strstr(p_str_ptr, "//");
    if (ptr != NULL) {
        *ptr = 0;
    } else if ((strlen(p_str_ptr) > 0) && (p_str_ptr[strlen(p_str_ptr) - 1] == '\n')) {
        p_str_ptr[strlen(p_str_ptr) - 1] = '\0';    // filter the trailing \n
        if ((strlen(p_str_ptr) > 0) && (p_str_ptr[strlen(p_str_ptr) - 1] == '\r')) {
            p_str_ptr[strlen(p_str_ptr) - 1] = '\0';    // filter the trailing \r
        }
    } else if ((strlen(p_str_ptr) > 0) && (p_str_ptr[strlen(p_str_ptr) - 1] == '\r')) {
        p_str_ptr[strlen(p_str_ptr) - 1] = '\0';    // filter the trailing \r
    }
}

static int
_isBlankLine(char *p_line_ptr)
{

    while (*p_line_ptr) {
        if (*p_line_ptr != '\n' && !isblank(*p_line_ptr)) {
            return 0;
        } else {
            p_line_ptr++;
        }
    }

    return 1;
}

/*--------------------------------------------------------------------
 * this function open the configuration file. it returns
 *    NO_ERROR
 *    MAJOR ERROR (ENVIRON_NOT_SET, FILE_NOT_EXIST, FILE_NOT_OPEN)
 *    CRITICAL ERROR (STR_TOO_LONG, ILLEGAL_SYNTAX)
 *    MINOR ERROR (NOT_FOUND)
*/
static eConfigErr
_openConfigFile()
{
    char *fname_ptr, filename[MAX_FILE_NAME_SIZE + 1];

    if ((fname_ptr = getenv("ANTA_CONFIG_BASE_DIR")) == 0)
        fname_ptr = DEFAULT_BASE_DIR;

    strcpy(filename, fname_ptr);
    strcat(filename, CONFIG_FILE);

    printf("filename;%s\n", filename);

    if ((_vConfig.cfp = fopen(filename, "r")) == NULL) {
        _vConfig.status = CONFIGSTATUS_FILE_NOT_EXIST;
        return MAJOR_ERROR;
    }

    _vConfig.status = CONFIGSTATUS_OK;
    strncpy(_vConfig.fileName, filename, MAX_FILE_NAME_SIZE);

    return NO_ERROR;
}

static eConfigErr
parseAttribute()
{
    char buffer[MAX_LINE_SIZE], *from_ptr, *to_ptr;
    int linesize, n;

    while (feof(_vConfig.cfp) == 0 && CURRENT_ATTRIBUTE_INDEX < MAX_ATTRIBUTE) {
        if (fgets(buffer, MAX_LINE_SIZE, _vConfig.cfp) == 0)
            break;

        CURRENT_LINE++;

        _filterComment(buffer);
        if (_isBlankLine(buffer))
            continue;

        /* hit next section header, stop. */
        if (buffer[0] == '[') {

            linesize = strlen(buffer);
            // _filterComment kills \r\n, that's why fgets failed at next time
            ungetc('\n', _vConfig.cfp);
            while (linesize--)
                ungetc(buffer[linesize], _vConfig.cfp);

            break;
        }

        /* if no equal sign, syntax error */
        if (strchr(buffer, '=') == NULL) {
            _vConfig.status = CONFIGSTATUS_ILLEGAL_SYNTAX;
            return CRITICAL_ERROR;
        }

        for (n = 0, from_ptr = buffer, to_ptr = CURRENT_ATTRIBUTE_NAME; !isblank(*from_ptr) && *from_ptr != '=';) {
            if (n > ATTRIBUTE_NAME_SIZE) {
                _vConfig.status = CONFIGSTATUS_STR_TOO_LONG;
                return CRITICAL_ERROR;
            } else {
                *to_ptr++ = *from_ptr++;
                n++;
            }
        }
        *to_ptr = 0;

        while (*from_ptr == '=' || isblank(*from_ptr))
            from_ptr++;

        for (n = 0, to_ptr = CURRENT_ATTRIBUTE_VALUE; !isblank(*from_ptr) && *from_ptr && *from_ptr != '\n';) {
            if (n > ATTRIBUTE_VALUE_SIZE) {
                _vConfig.status = CONFIGSTATUS_STR_TOO_LONG;
                return CRITICAL_ERROR;
            } else {
                *to_ptr++ = *from_ptr++;
                n++;
            }
        }
        *to_ptr = 0;

        // there should not be trailing nonblank characters
        while (*from_ptr) {
            if (!isblank(*from_ptr) && *from_ptr != '\n') {
                _vConfig.status = CONFIGSTATUS_ILLEGAL_SYNTAX;
                return CRITICAL_ERROR;
            } else
                from_ptr++;
        }

        CURRENT_ATTRIBUTE_INDEX++;
    }

    _vConfig.status = CONFIGSTATUS_OK;
    return NO_ERROR;
}

static eConfigErr
parseConfigFile()
{
    char buffer[MAX_LINE_SIZE], *to_ptr, *from_ptr;
    int n;
    eConfigErr status;

    if (_vConfig.cfp == NULL) {
        if ((status = _openConfigFile()) != NO_ERROR)
            return MAJOR_ERROR;
    }

    fseek(_vConfig.cfp, 0L, SEEK_SET);
    CURRENT_LINE = 0;
    CURRENT_SECTION_INDEX = 0;

    while (feof(_vConfig.cfp) == 0) {
        if (fgets(buffer, MAX_LINE_SIZE, _vConfig.cfp) == 0)
            break;

        CURRENT_LINE++;

        _filterComment(buffer);
        if (_isBlankLine(buffer))
            continue;

        /* find start of section name */
        if (buffer[0] != '[') {
            fclose(_vConfig.cfp);
            _vConfig.cfp = NULL;

            _vConfig.status = CONFIGSTATUS_ILLEGAL_SYNTAX;
            return CRITICAL_ERROR;
        }

        /* find end of section name */
        if (strchr(buffer, ']') == NULL) {
            fclose(_vConfig.cfp);
            _vConfig.cfp = NULL;

            _vConfig.status = CONFIGSTATUS_ILLEGAL_SYNTAX;
            return CRITICAL_ERROR;
        }

        /* record section */
        for (n = 0, from_ptr = &buffer[1], to_ptr = CURRENT_SECTION_NAME; *from_ptr != ']';) {
            if (isblank(*from_ptr))
                from_ptr++;
            else {
                if (n > SECTION_NAME_SIZE) {
                    fclose(_vConfig.cfp);
                    _vConfig.cfp = NULL;

                    _vConfig.status = CONFIGSTATUS_STR_TOO_LONG;
                    return CRITICAL_ERROR;
                } else {
                    n++;
                    *to_ptr++ = *from_ptr++;
                }
            }
        }
        *to_ptr = 0;

        // there should not be trailing nonblank characters
        while (*from_ptr) {
            if (!isblank(*from_ptr) && *from_ptr != '\n' && *from_ptr != ']') {
                fclose(_vConfig.cfp);
                _vConfig.cfp = NULL;

                _vConfig.status = CONFIGSTATUS_ILLEGAL_SYNTAX;
                return CRITICAL_ERROR;
            } else
                from_ptr++;
        }

        CURRENT_ATTRIBUTE_INDEX = 0;

        if ((status = parseAttribute()) != NO_ERROR) {
            fclose(_vConfig.cfp);
            _vConfig.cfp = NULL;

            return status;
        }

        CURRENT_SECTION_INDEX++;
        if (CURRENT_SECTION_INDEX >= MAX_SECTIONS)
        {
            _vConfig.status = CONFIGSTATUS_SECTION_TOO_LONG;
            return CRITICAL_ERROR;
        }
    }

    fclose(_vConfig.cfp);
    _vConfig.cfp = NULL;

    _vConfig.status = CONFIGSTATUS_OK;
    return NO_ERROR;
}

void
sysconfReset()
{

    CURRENT_SECTION_INDEX = 0;
}

/*
 * Name:       SysconfGet
 * Summary:    gets value of a configured parameter 'param' in section 'section'. the value is return to 'value'
 * Parameter:  [Section Head]
               parameter string
               buf of receive param value
 * Return:     error type of get value
 *
 * First Date: 2005.07.28 by Quan Wen
 * Update:     
 *
 */
eConfigErr
sysconfGet(char *p_section_ptr, char *p_param_ptr, char *p_value_str)
{
    eConfigErr status;
    int i, j;

    *p_value_str = 0;

    if (CURRENT_SECTION_INDEX == 0) {

        pthread_mutex_lock(&_vConfigLock);
        status = parseConfigFile();
        pthread_mutex_unlock(&_vConfigLock);

        if (status != NO_ERROR)
            return status;
    }

    for (i = 0; i < CURRENT_SECTION_INDEX; i++) {
        if (!strcmp(p_section_ptr, _vConfig.section[i].name)) {
            for (j = 0; j < _vConfig.section[i].currentAttribute; j++) {
                if (!strcmp(p_param_ptr, _vConfig.section[i].attribute[j].name)) {
                    strcpy(p_value_str, _vConfig.section[i].attribute[j].value);

                    _vConfig.status = CONFIGSTATUS_OK;
                    return NO_ERROR;
                }
            }

            _vConfig.status = CONFIGSTATUS_NOT_FOUND;
            return MINOR_ERROR;
        }
    }

    _vConfig.status = CONFIGSTATUS_NOT_FOUND;
    return MINOR_ERROR;
}

char *
SysconfBase()
{

    char *fname_ptr;

    if ((fname_ptr = getenv("ANTA_CONFIG_BASE_DIR")) == 0)
        fname_ptr = DEFAULT_BASE_DIR;

    return fname_ptr;
}

#if 0
/*----------------------------------------------------------------
 * returns error string indicating error of the sysconf
 */
static char *
_sysconfErrStr()
{
    static char error[80];

    switch (_vConfig.status) {
    case CONFIGSTATUS_OK:
        return 0;
    case CONFIGSTATUS_NOT_FOUND:
    case CONFIGSTATUS_ENVIRON_NOT_SET:
    case CONFIGSTATUS_FILE_NOT_EXIST:
    case CONFIGSTATUS_FILE_NOT_OPEN:
        sprintf(error, "%s", _vErrorStr[_vConfig.status]);
        break;

    case CONFIGSTATUS_ILLEGAL_SYNTAX:
    case CONFIGSTATUS_STR_TOO_LONG:
        sprintf(error, "%s in line %d.\n", _vErrorStr[_vConfig.status], CURRENT_LINE);
        break;
    default:
        return 0;
    }

    return error;
}
#endif

#ifdef TEST
main()
{
    eConfigErr status;
    char buffer[80];

    status = SysconfGet("SGIP", "serv_addr", buffer);
    printf("status = %d\n", status);
    if (status == NO_ERROR) {
        printf("%s---size(%d)\n", buffer, strlen(buffer));
    } else
        printf("Error: %s\n", _sysconfErrStr());
}
#endif
