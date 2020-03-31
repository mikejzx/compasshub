#ifndef COH_PCH_H
#define COH_PCH_H

/*
 * This is meant to be a pre-compiled header file, but 
 * as it stands, it's just a regular header.
 */

// We include our defines so that it is inside every damn file.
#include "defines.h"

// C++ includes.
#include <cctype>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// C Includes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// *nix Includes:
#include <signal.h>

// Local Includes:
#include "logger/log.h"
#include "util.h"

// Libraries
#include <ncurses.h>               // Ncurses TUI
#include "rapidjson/document.h"    // RapidJSON
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "httplib/httplib.h"       // HTTP library.

#endif
