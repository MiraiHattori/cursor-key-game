#!/bin/sh
g++ -std=c++14 -Wall -Wextra -Wconversion ./cursor.cpp -o ./Cursor -pthread
./Cursor
