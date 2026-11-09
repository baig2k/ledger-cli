#!/bin/bash
g++ -Wall -Wextra -I./include src/main.cpp src/ExpenseManager.cpp build/sqlite3.o -o build/ledger.exe
