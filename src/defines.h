#ifndef COH_DEFINES_H
#define COH_DEFINES_H

/*
 * defines.h
 * - List of defines used throughout the project.
 */

// Project defines.
#define COH_PROGRAM_NAME        "CompassHub"
#define COH_PROGRAM_NAME_LOWER  "compasshub"
#define COH_PROGRAM_NAME_UPPER  "COMPASSHUB"
#define COH_PROGRAM_VERSION     "v0.0.2 SNAPSHOT 20w14a"
#define COH_PROGRAM_CREDIT      "Created by Michael Skec"

// Enable this to use sample data.
// #define COH_USE_SAMPLE_DATA

// Status defines and strings.
#define COH_STATUS_LOGGEDOFF 0
#define COH_STATUS_READDISK 1
#define COH_STATUS_LOGGEDIN 2
#define COH_SZ_STATUS_BASE "Status: "
#define COH_SZ_STATUS_LOGGEDOFF (COH_SZ_STATUS_BASE "Logged off")
#define COH_SZ_STATUS_READDISK  (COH_SZ_STATUS_BASE "Re-using cookies from disk")
#define COH_SZ_STATUS_LOGGEDIN  (COH_SZ_STATUS_BASE "Logged in")

// Caching defines.
#define COH_CACHE_DIR "./" COH_PROGRAM_NAME_LOWER "-cache/"
#define COH_CACHE_DELIM "\x1D"
#define COH_CACHE_RETRV_DATE_FORMAT "%04u-%02u-%02u %02u:%02u"

// Retreival defines.
#define COH_SZ_RETR_PROMPT "Press R to refresh."
#define COH_SZ_RETR_LAST "Retrieved "

// Window manager defines
#define COH_SZ_LOADING "Loading..."
#define COH_SZ_NOEVENTS "No events this day"
#define COH_WND_HEADER_TEXT (COH_PROGRAM_NAME " - " COH_PROGRAM_VERSION)

// Window manager indices.
#define COH_WND_IDX_EVENTS 0
#define COH_WND_IDX_MAIN   1
#define COH_WND_IDX_FOOTER 2
#define COH_WND_IDX_HEADER 3
#define COH_WND_IDX_STATUS 4

// Window manager, main window stuff.
#define COH_WND_MAIN_TITLE_OFFSY 2    // Y Offset of the date title.
#define COH_WND_MAIN_PERIODS_OFFSY 4  // Y Offset of the actual periods.
#define COH_WND_MAIN_PERIODS_OFFSX 12 // X Offset of ''    ''      ''

// Colour pairs
#define COH_COL_BG_STATUS COLOR_BLACK
#define COH_COL_HEADER 1           // Header colour pair.
#define COH_COL_FOOTER 2           // Footer colour pair.
#define COH_COL_MAIN 3             // Main colour pair.
#define COH_COL_STATUS_LI 4        // Status bar: Logged in colour.
#define COH_COL_STATUS_LD 5        // Status bar: Loaded from disk colour.
#define COH_COL_STATUS_LO 6        // Status bar: Logged out colour.
#define COH_COL_PERIOD_NORMAL 7    // Period: Normal
#define COH_COL_PERIOD_NORMAL_B 11 // Period: (Black foregr.) Normal
#define COH_COL_PERIOD_RCHANG 8    // Period: Room change.
#define COH_COL_PERIOD_RCHANG_B 12 // Period: (Black foregr.) Room change.
#define COH_COL_PERIOD_SUBST  9    // Period: Substitute teacher.
#define COH_COL_PERIOD_SUBST_B  13 // Period: (Black foregr.) Substitute teacher.
#define COH_COL_PERIOD_CANCEL 10   // Period: Cancelled
#define COH_COL_PERIOD_CANCEL_B 13 // Period: (Black foregr.) Cancelled
#define COH_COL_PERIOD_FG_STRIDE 4 // Use this to convert between blackfg and whitefg colours.
#define COH_WND_STRETCH -1         // Set this to stretch along axis.

// Path defines.
#define COH_COOKIE_JAR_PATH "./" COH_PROGRAM_NAME_LOWER ".cookiejar"
#define COH_PREFS_FILE_PATH "./" COH_PROGRAM_NAME_LOWER ".prefs"

// Preferences.
#define COH_PREF_NAME_HOSTNAME "hostname"
#define COH_PREF_NAME_PLOGIN "login"
#define COH_PREF_NAME_PAUTH "auth"
#define COH_PREF_NAME_PTT "tt"
#define COH_PREF_NAME_PLOGOFF "logoff"
#define COH_PREF_MODE_ALIASES_BEGIN "aliases_begin"
#define COH_PREF_MODE_ALIASES_END "aliases_end"

// Logger (log.h, log.c)
#define LOG_LEVEL LOG_LEVEL_VERBOSE
#define LOG_PROGRAM_NAME COH_PROGRAM_NAME_UPPER
#define LOG_FILE_PATH "./" COH_PROGRAM_NAME_LOWER ".log"
#define LOG_COLOUR_ENABLED

#endif
