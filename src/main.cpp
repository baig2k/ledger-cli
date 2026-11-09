#include <iostream>
#include <string>
#include "ExpenseManager.h"
using namespace std;

void printHelp() {
    cout << "\n=== Ledger CLI - Expense Tracker Commands ===\n";
    cout << "  add <category> <amount> <note>  : Record a new expense\n";
    cout << "  list                            : View all expenses (with row numbers & IDs)\n";
    cout << "  summary                         : View total spent per category (SQL GROUP BY)\n";
    cout << "  filter <category>               : View expenses in one category (SQL WHERE)\n";
    cout << "  update <id> <new_amount>        : Change the amount of expense #id\n";
    cout << "  delete <id>                     : Delete expense #id\n\n";
}

// Converts digit strings like "450" into int 450 using ASCII math
int textToInt(const char text[]) {
    if (text[0] == '\0') return -1;

    int result = 0;
    int i = 0;

    while (text[i] != '\0') {
        if (text[i] < '0' || text[i] > '9') {
            return -1;
        }
        result = (result * 10) + (text[i] - '0');
        i++;
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printHelp();
        return 0;
    }

    ExpenseManager ledger("data/ledger.db");
    string command = argv[1];

    // 1. ADD: ./build/ledger.exe add <category> <amount> <note>
    if (command == "add") {
        if (argc < 5) {
            cout << "[Error] Usage: ledger add <category> <amount> <note>\n";
            return 1;
        }
        int amount = textToInt(argv[3]);
        if (amount < 0) {
            cout << "[Error] Amount must be a valid positive number!\n";
            return 1;
        }
        ledger.addExpense(argv[2], amount, argv[4]);
    }
    // 2. LIST: ./build/ledger.exe list
    else if (command == "list") {
        ledger.listExpenses();
    }
    // 3. SUMMARY: ./build/ledger.exe summary
    else if (command == "summary") {
        ledger.showSummary();
    }
    // 4. FILTER: ./build/ledger.exe filter <category>
    else if (command == "filter") {
        if (argc < 3) {
            cout << "[Error] Usage: ledger filter <category>\n";
            return 1;
        }
        ledger.filterByCategory(argv[2]);
    }
    // 5. UPDATE: ./build/ledger.exe update <id> <new_amount>
    else if (command == "update") {
        if (argc < 4) {
            cout << "[Error] Usage: ledger update <id> <new_amount>\n";
            return 1;
        }
        int id = textToInt(argv[2]);
        int new_amount = textToInt(argv[3]);
        if (id <= 0 || new_amount < 0) {
            cout << "[Error] Please provide a valid ID and amount!\n";
            return 1;
        }
        ledger.updateExpense(id, new_amount);
    }
    // 6. DELETE: ./build/ledger.exe delete <id>
    else if (command == "delete") {
        if (argc < 3) {
            cout << "[Error] Usage: ledger delete <id>\n";
            return 1;
        }
        int id = textToInt(argv[2]);
        if (id <= 0) {
            cout << "[Error] Please provide a valid ID!\n";
            return 1;
        }
        ledger.deleteExpense(id);
    }
    else {
        cout << "[Error] Unknown command: '" << command << "'\n";
        printHelp();
        return 1;
    }

    return 0;
}