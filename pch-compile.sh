#!/bin/bash
#g++ -x c++-header -c ./src/pch.h -o ./src/pch.h.gch
g++ ./src/pch.h
echo "Compiled pre-compiled header."
