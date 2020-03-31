#ifndef RT_LOG_H
#define RT_LOG_H

#include "../defines.h"

/*
	log.h
	This header contains declarations for writing to a log file.
	Used for debugging purposes.
*/

// Macro definitions.
#define LOG_LEVEL_MINIMAL 0
#define LOG_LEVEL_VERBOSE 1

// Change this based on what log level is needed. (Moved to defines.h)
//#define LOG_LEVEL LOG_LEVEL_VERBOSE
//#define LOG_FILE_PATH "./compassview.log"

// Clear macros if log level doesn't approve of them.
#if LOG_LEVEL == LOG_LEVEL_MINIMAL
	// Minimal, remove "info"s and "dbug"s.
	#define LOG_INFO(...)
	#define LOG_DBUG(...)
#elif LOG_LEVEL == LOG_LEVEL_VERBOSE
	// Verbose, 
	#define LOG_INFO(...) log_infofln(__VA_ARGS__)
	#define LOG_DBUG(...) log_dbugfln(__VA_ARGS__)
#endif
// Always leave warnings and errors defined.
#define LOG_WARN(...) log_warnfln(__VA_ARGS__)
#define LOG_ERROR(...) log_errfln(__VA_ARGS__)
#define LOG_FATAL(...) log_fatalfln(__VA_ARGS__)

// Prefixes for log messages.
#define LOG_PREFIX_BASE_INFO "[INFO] "
#define LOG_PREFIX_BASE_WARN "[WARN] "
#define LOG_PREFIX_BASE_ERR "[ERROR] "
#define LOG_PREFIX_BASE_DBUG "[DEBUG] "
#define LOG_PREFIX_BASE_FATAL "==== [FATAL!] ==== "

// If we allow colour coding, and on *NIX, insert escape codes.
// No Windows support just yet.
#ifdef LOG_COLOUR_ENABLED
	#ifdef PLATFORM_UNIX
		#define LOG_PREFIX_INFO  "\033[00;39m" LOG_PREFIX_BASE_INFO
		#define LOG_PREFIX_WARN  "\033[00;93m" LOG_PREFIX_BASE_WARN
		#define LOG_PREFIX_ERR   "\033[00;91m" LOG_PREFIX_BASE_ERR
		#define LOG_PREFIX_DBUG  "\033[00;95m" LOG_PREFIX_BASE_DBUG
		#define LOG_PREFIX_FATAL "\033[101m" LOG_PREFIX_BASE_FATAL
	#endif
#else
	#define LOG_PREFIX_INFO  LOG_PREFIX_BASE_INFO
	#define LOG_PREFIX_WARN  LOG_PREFIX_BASE_WARN
	#define LOG_PREFIX_ERR   LOG_PREFIX_BASE_ERR
	#define LOG_PREFIX_DBUG  LOG_PREFIX_BASE_DBUG
	#define LOG_PREFIX_FATAL LOG_PREFIX_BASE_FATAL
#endif

// Declarations here.
void log_initialise(void);          // Initialise the logger.
void log_infofln(const char*, ...); // Log info.
void log_errfln (const char*, ...); // Log error.
void log_warnfln(const char*, ...); // Log warning
void log_dbugfln(const char*, ...); // Log debug message.
void log_fatalfln(const char*, ...); // Log fatal.

#endif
