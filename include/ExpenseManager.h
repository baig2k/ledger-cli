#ifndef EXPENSE_MANAGER_H
#define EXPENSE_MANAGER_H

#include <string>
using namespace std;

class ExpenseManager {
private:
    const char* db_path;

    // Helper: Opens the DB, runs a non-SELECT query, and closes the DB
    bool runQuery(string sql);

public:
    ExpenseManager(const char* path);

    // 1. CREATE
    bool addExpense(string category, int amount, string note);

    // 2. READ (All rows)
    void listExpenses();

    // 3. READ (Grouped category totals calculated by SQL)
    void showSummary();

    // 4. UPDATE
    bool updateExpense(int id, int new_amount);

    // 5. DELETE
    bool deleteExpense(int id);
    
    void filterByCategory(string category);
};

#endif