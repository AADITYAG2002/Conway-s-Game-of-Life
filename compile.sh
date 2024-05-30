#!/bin/bash
# script for compiling

if [ -z $1 ]; then
    echo "Usage: ./compile <CMainFile> (no extension)"
    exit
fi

# Verify no extensions were entered 
if [ ! -e "$1.c" ]; then 
    echo "Error, $1.c not found." 
    echo "Note, do not enter file extensions." 
    exit 
fi 

# Compile
gcc -Wall -fdiagnostics-color=always -g -o $1 $1.c -l ncursesw
