/*
  2016 Copyright (c) Seeed Technology Inc.  All right reserved.
  Author:Baozhu Zuo
  suli is designed for the purpose of reusing the high level implementation
  of each libraries for different platforms out of the hardware layer.
  suli2 is the new reversion of original suli. There're lot of improvements upon
  the previous version. Currently, it can be treated as the internal strategy for
  quick library development of seeed. But always welcome the community to
  follow the basis of suli to contribute grove libraries.
  The MIT License (MIT)
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
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
#define FASTERCONFIG_DEBUG
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

extern debugconf_t debugconf;

/** Used to output messages.
 * The messages will include the filename and line number, and will be sent to syslog if so configured in the config file 
 * @param level Debug level
 * @param format... sprintf like format string
 */
#define debug(level, format...) logger._debug(__FILE__, __LINE__, level, format)



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
    debugconf_t debugconf;
protected:
    /**
     *  Constructor.
     *  Creates the one and only Logger object.
     */
    Logger() {}

    /**
     *  Destructor
     */
    ~Logger() { _file.close(); }

private:
    static Logger* _instance;
    static std::ofstream _file;

};
}
#endif	/* _DNS_LOGGER_H */

