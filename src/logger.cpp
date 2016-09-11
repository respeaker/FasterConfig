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


#include "logger.h"
#include <iostream>
using namespace dns;
using namespace std;

// static
Logger* Logger::_instance = 0;
ofstream Logger::_file("/tmp/fasterconfig.log", ios::out|ios::trunc);
debugconf_t Logger::debugconf;


Logger::Logger() { 
debugconf.debuglevel = LOG_INFO;
debugconf.log_stderr = 1;
debugconf.log_syslog = 0;
debugconf.syslog_facility = 0;
}
Logger& Logger::instance() throw() {

    if (_instance == 0) {
        _instance = new Logger();
    }

    return *_instance;
}

void Logger::trace(const char* text) throw() {

    _file << " ## " << text << endl;
}

void Logger::trace(std::string& text) throw() {

    trace(text.data());
}

void Logger::trace(std::ostringstream& stream) throw() {

    string text = stream.str();
    trace(text.data());
}

void Logger::error(const char* text) throw() {

    _file << " !! " << text << endl;
}

void Logger::error(std::string& text) throw() {

    trace(text.data());
}
/** @internal
Do not use directly, use the debug macro */
void Logger::_debug(const char *filename, int line, int level, const char *format, ...)
{
    char buf[28];
    va_list vlist;
    time_t ts;
    sigset_t block_chld;

    time(&ts);

    if (10 >= level) {
        sigemptyset(&block_chld);
        sigaddset(&block_chld, SIGCHLD);
        sigprocmask(SIG_BLOCK, &block_chld, NULL);
        
        if (level <= LOG_WARNING) {
            fprintf(stderr, "[%d][%.24s][%u](%s:%d) ", level, ctime_r(&ts, buf), getpid(),
                filename, line);
            va_start(vlist, format);
            vfprintf(stderr, format, vlist);
            va_end(vlist);
            fputc('\n', stderr);
        } else if (debugconf.log_stderr) {
            fprintf(stderr, "[%d][%.24s][%u](%s:%d) ", level, ctime_r(&ts, buf), getpid(),
                filename, line);
            va_start(vlist, format);
            vfprintf(stderr, format, vlist);
            va_end(vlist);
            fputc('\n', stderr);
        }

        if (debugconf.log_syslog) {
            openlog("fasterconfig", LOG_PID, debugconf.syslog_facility);
            va_start(vlist, format);
            vsyslog(level, format, vlist);
            va_end(vlist);
            closelog();
        }
        
        sigprocmask(SIG_UNBLOCK, &block_chld, NULL);
    }
}

