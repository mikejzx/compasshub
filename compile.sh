#!/bin/bash

# Colour format codes.
C_RESET='\033[0m'
C_RED='\033[01;31m'
C_GRN='\033[01;32m'
C_YLW='\033[01;33m'
C_CYN='\033[01;36m'

# Check timestamp for snapshot builds.
# Remove this if not building "snapshots".
# snapsh_year=$(date +"%y")
# snapsh_week=$(($(date +"%U")+1))
# snapsh_timestr=''$snapsh_year'w'$snapsh_week'a'
# snapsh_in_file="$(grep "SNAPSHOT" src/defines.h | cut -b 50- | cut -b 1-6)"
# if [[ ! "$snapsh_timestr" == "$snapsh_in_file" ]]; then
# 	echo -e "${C_YLW}Snapshot time is incorrect. Fixing now...${C_RESET}"
# 	sed -i 's/'$snapsh_in_file'/'$snapsh_timestr'/' ./src/defines.h
# fi

# Compile
time make
cd bin
./compasshub
cd ..

# Binary terminated. Ask if user wants to view log.
echo -e "${C_CYN}Program terminated. Press RETURN to VIEW THE LOG, or CTRL+C to abort.${C_RESET}"
read
less -R ./bin/compasshub.log
echo -e "${C_CYN}Log was viewed.${C_RESET}"
cd ..
