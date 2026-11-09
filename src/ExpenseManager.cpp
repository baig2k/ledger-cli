#include "ExpenseManager.h"
#include "sqlite3.h"
#include <iostream>
using namespace std;

// Helper 1: SQLite calls this automatically for every row found in listExpenses()
// values[0] = id, values[1] = category, values[2] = amount, values[3] = note
int printExpenseRow(void* data, int col_count, char** values, char** col_names) {
    (void)col_count; (void)col_names;

    // Convert the generic void* pointer back into an int* pointer
    int* row_num = (int*)data;

    cout << *row_num << "   | #"
         << values[0] << "  | "
         << values[1] << "          | Rs. "
         << values[2] << "      | "
         << values[3] << "\n";

    // Increase the row counter by 1 for the next row
    (*row_num)++;
    return 0;
}

// Helper 2: SQLite calls this automatically for every category total in showSummary()
// values[0] = category, values[1] = SUM(amount)
int printSummaryRow(void* not_used, int col_count, char** values, char** col_names) {
    (void)not_used; (void)col_count; (void)col_names;

    cout << values[0] << "          | Rs. " << values[1] << "\n";
    return 0;
}

// Private Helper: Runs any SQL command that modifies data (CREATE, INSERT, UPDATE, DELETE)
bool ExpenseManager::runQuery(string sql) {
    sqlite3* db;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        cout << "[Error] Could not open database file!\n";
        return false;
    }

    char* error_msg = 0;
    int status = sqlite3_exec(db, sql.c_str(), 0, 0, &error_msg);

    if (status != SQLITE_OK) {
        cout << "[SQL Error] " << error_msg << "\n";
        sqlite3_free(error_msg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return true;
}

// Constructor: Ensures the 'expenses' table exists on startup
ExpenseManager::ExpenseManager(const char* path) {
    db_path = path;

    string create_table_sql =
        "CREATE TABLE IF NOT EXISTS expenses ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "category TEXT, "
        "amount INTEGER, "
        "note TEXT);";

    runQuery(create_table_sql);
}

// 1. CREATE
bool ExpenseManager::addExpense(string category, int amount, string note) {
    string sql = "INSERT INTO expenses (category, amount, note) VALUES ('" +
                 category + "', " + to_string(amount) + ", '" + note + "');";

    if (runQuery(sql)) {
        cout << "[OK] Added expense: Rs. " << amount << " (" << category << " - " << note << ")\n";
        return true;
    }
    return false;
}

// 2. READ ALL
void ExpenseManager::listExpenses() {
    sqlite3* db;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) return;

    cout << "\nNo. | ID  | Category     | Amount     | Note\n";
    cout << "----+-----+--------------+------------+----------------------------\n";

    string sql = "SELECT id, category, amount, note FROM expenses;";
    char* error_msg = 0;
    int row_number = 1; // Starts at 1 every time we print the table

    // Pass &row_number as the 4th argument so printExpenseRow can use and increment it!
    sqlite3_exec(db, sql.c_str(), printExpenseRow, &row_number, &error_msg);

    if (row_number == 1) {
        cout << "(No expenses recorded yet)\n";
    }

    if (error_msg != 0) {
        cout << "[SQL Error] " << error_msg << "\n";
        sqlite3_free(error_msg);
    }

    cout << "\n";
    sqlite3_close(db);
}
// 3. READ SUMMARY (Grouped by Category)
void ExpenseManager::showSummary() {
    sqlite3* db;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) return;

    cout << "\n=== Spending Summary by Category ===\n";
    cout << "Category     | Total Spent\n";
    cout << "-------------+----------------\n";

    // GROUP BY groups identical categories together; SUM(amount) adds their prices!
    string sql = "SELECT category, SUM(amount) FROM expenses GROUP BY category;";
    char* error_msg = 0;

    sqlite3_exec(db, sql.c_str(), printSummaryRow, 0, &error_msg);

    if (error_msg != 0) {
        cout << "[SQL Error] " << error_msg << "\n";
        sqlite3_free(error_msg);
    }

    cout << "\n";
    sqlite3_close(db);
}

// 4. UPDATE
bool ExpenseManager::updateExpense(int id, int new_amount) {
    string sql = "UPDATE expenses SET amount = " + to_string(new_amount) +
                 " WHERE id = " + to_string(id) + ";";

    if (runQuery(sql)) {
        cout << "[OK] Updated expense #" << id << " to Rs. " << new_amount << "\n";
        return true;
    }
    return false;
}

// 5. DELETE
bool ExpenseManager::deleteExpense(int id) {
    string sql = "DELETE FROM expenses WHERE id = " + to_string(id) + ";";

    if (runQuery(sql)) {
        cout << "[OK] Deleted expense #" << id << "\n";
        return true;
    }
    return false;
}

// 6. FILTER BY CATEGORY (Using SQL WHERE)
void ExpenseManager::filterByCategory(string category) {
    sqlite3* db;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) return;

    cout << "\n=== Expenses in Category: " << category << " ===\n";
    cout << "No. | ID  | Category     | Amount     | Note\n";
    cout << "----+-----+--------------+------------+----------------------------\n";

    // SQL WHERE filters rows on disk before sending them to C++
    string sql = "SELECT id, category, amount, note FROM expenses WHERE category = '" + category + "';";
    char* error_msg = 0;
    int row_number = 1;

    sqlite3_exec(db, sql.c_str(), printExpenseRow, &row_number, &error_msg);

    if (row_number == 1) {
        cout << "(No expenses found in this category)\n";
    }

    if (error_msg != 0) {
        cout << "[SQL Error] " << error_msg << "\n";
        sqlite3_free(error_msg);
    }

    cout << "\n";
    sqlite3_close(db);
}