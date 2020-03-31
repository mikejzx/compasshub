
/*
	log.c
	Contains definitions for info-logging system.
*/

// Don't include the PCH since it's for C++.
#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Initialise the logger.
void log_initialise(void)
{
	// Get the log level string
#if LOG_LEVEL == LOG_LEVEL_MINIMAL
	#define S_LOG_STR_MODE "MINIMAL"
#elif LOG_LEVEL == LOG_LEVEL_VERBOSE
	#define S_LOG_STR_MODE "VERBOSE"
#else
	#define S_LOG_STR_MODE "(UNKNOWN)"
#endif

	// Get program name string.
#ifdef LOG_PROGRAM_NAME
	#define S_LOG_STR_NAME LOG_PROGRAM_NAME
#else
	#define S_LOG_STR_NAME "UNTITLED-PROJECT"
#endif

	// Overwrite the log file and replace with the header.
	FILE* f = fopen(LOG_FILE_PATH, "w");
	fprintf(f, "==== %s LOG BEGIN, [Log Mode: %s] ====\n", S_LOG_STR_NAME, S_LOG_STR_MODE);
	fclose(f);

	log_infofln("Logger initialised.");

	// Undefine local defines.
#undef S_LOG_STR_MODE
#undef S_LOG_STR_NAME
}

// Log a formatted info string with variadic arguments.
void log_infofln(const char* format, ...)
{
	// Variadic arguments stored here.
	va_list args;
	
	// Get arguments after 'format' parameter in param list.
	va_start(args, format);
	
	// Open the log file in append mode and write.
	FILE* f = fopen(LOG_FILE_PATH, "a");
	fprintf(f, LOG_PREFIX_INFO); // Write prefix.
	vfprintf(f, format, args);   // Write the args.
	fprintf(f, "\n");            // Write a NL.
	// Close the file
	fclose(f);

	// Needed to allow normal return.
	va_end(args);
}

// Used a bunch of times so it's in a macro now.
// log_infofln has a commented version of this.
// (This get's undef'd at the EOF.)
#define S_LOG_FUNC_TEMPLATE(prefix)\
	va_list args;\
	va_start(args, format);\
	FILE* f = fopen(LOG_FILE_PATH, "a");\
	fprintf(f, prefix);\
	vfprintf(f, format, args);\
	fprintf(f, "\n");\
	fclose(f);\
	va_end(args);\

// Generate all the other functions using the macro.

// Errors.
void log_errfln(const char* format, ...)
{
	S_LOG_FUNC_TEMPLATE(LOG_PREFIX_ERR);	
}

// Warnings.
void log_warnfln(const char* format, ...)
{
	S_LOG_FUNC_TEMPLATE(LOG_PREFIX_WARN);
}

// Debug messages.
void log_dbugfln(const char* format, ...)
{
	S_LOG_FUNC_TEMPLATE(LOG_PREFIX_DBUG);	
}

// Fatal.
void log_fatalfln(const char* format, ...)
{
	S_LOG_FUNC_TEMPLATE(LOG_PREFIX_FATAL);
}

// Undefine the 'template' macro used for logging.
#undef S_LOG_FUNC_TEMPLATE
