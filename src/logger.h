/*
 * 2016 Copyright (c) Seeed Technology Inc.  All right reserved.
 * Author:Baozhu Zuo <zuobaozhu@gmail.com>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
*/



#ifndef _DNS_LOGGER_H
#define	_DNS_LOGGER_H

#include <fstream>
#include <sstream>
#include <string>

#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


namespace dns {
//#define FASTERCONFIG_DEBUG
#ifdef FASTERCONFIG_DEBUG
#define MSG(fmt, args...) cout << ("FASTERCONFIG: " fmt, ## args) << endl;
#else
#define MSG(fmt, args...) { }
#endif

typedef struct _debug_conf {
    int debuglevel;      /**< @brief Debug information verbosity */
    int log_stderr;      /**< @brief Output log to stdout */
    int log_syslog;      /**< @brief Output log to syslog */
    int syslog_facility; /**< @brief facility to use when using syslog for logging */
} debugconf_t;


#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
/** Used to output messages.
 * The messages will include the filename and line number, and will be sent to syslog if so configured in the config file 
 * @param level Debug level
 * @param format... sprintf like format string
 */
#define debug(level, format...) logger->_debug(__FILENAME__, __LINE__, level, format)



/**
 *  Logger is a helper class that allows to trace text messages to a log file.
 *  It is a single instance class or also known as Singleton.
 */
class Logger {
public:
    /**
     *  Instanciates the one and only Logger object.
     *  @return A reference to the one and only Logger object.
     */
    static Logger& instance() throw ();

    /**
     *  Trace the message text to the log file.
     *  @param text Message text to log.
     */
    void trace(const char* text) throw();

    /**
     *  Trace the message text to the log file.
     *  @param text Message text to log.
     */
    void trace(std::string& text) throw();

    /**
     *  Trace the message text to the log file.
     *  @param text Message text to log.
     */
    void trace(std::ostringstream& text) throw();

    /**
     *  Trace the error message to the log file.
     *  @param text Error message to log.
     */
    void error(const char* text) throw();

    /**
     *  Trace the error message to the log file.
     *  @param text Error message to log.
     */
    void error(std::string& text) throw();

    /** @internal */
    void _debug(const char *, int, int, const char *, ...);


    
protected:
    /**
     *  Constructor.
     *  Creates the one and only Logger object.
     */
    Logger();

    /**
     *  Destructor
     */
    ~Logger() { _file.close(); }

private:
    static Logger* _instance;
    static std::ofstream _file;
    static debugconf_t debugconf;
};
}
#endif	/* _DNS_LOGGER_H */

