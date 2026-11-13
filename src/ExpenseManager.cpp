#include "ExpenseManager.h"
#include "sqlite3.h"
#include <iostream>
#include <cstdlib>
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

// ============================================================================
// MAX-HEAP IMPLEMENTATION (Dynamic Array Priority Queue)
// ============================================================================

ExpenseMaxHeap::ExpenseMaxHeap(int initial_cap) {
    capacity = initial_cap;
    size = 0;
    heap_arr = new ExpenseItem[capacity];
}

ExpenseMaxHeap::~ExpenseMaxHeap() {
    delete[] heap_arr;
}

void ExpenseMaxHeap::resizeHeap() {
    int new_cap = capacity * 2;
    ExpenseItem* new_arr = new ExpenseItem[new_cap];
    for (int i = 0; i < size; i++) {
        new_arr[i] = heap_arr[i];
    }
    delete[] heap_arr;
    heap_arr = new_arr;
    capacity = new_cap;
}

void ExpenseMaxHeap::swapItems(int idx1, int idx2) {
    ExpenseItem temp = heap_arr[idx1];
    heap_arr[idx1] = heap_arr[idx2];
    heap_arr[idx2] = temp;
}

bool ExpenseMaxHeap::isEmpty() {
    return size == 0;
}

int ExpenseMaxHeap::getSize() {
    return size;
}

// SIFT-UP (Heapify-Up): Places new item at the end and bubbles it up if larger than parent
void ExpenseMaxHeap::insert(int id, string category, int amount, string note) {
    if (size == capacity) {
        resizeHeap();
    }

    // 1. Place the new expense at the very bottom-right leaf of the tree
    int current = size;
    heap_arr[current] = ExpenseItem(id, category, amount, note);
    size++;

    // 2. Bubble up: While current item's amount > parent's amount, swap them!
    while (current > 0) {
        int parent = (current - 1) / 2;
        if (heap_arr[current].amount > heap_arr[parent].amount) {
            swapItems(current, parent);
            current = parent;
        } else {
            break; // Max-Heap property is satisfied!
        }
    }
}

// SIFT-DOWN (Heapify-Down): Removes root (max item) and sinks last item down to proper spot
ExpenseItem ExpenseMaxHeap::extractMax() {
    if (size == 0) return ExpenseItem();

    // 1. Save the maximum item sitting at index 0
    ExpenseItem max_item = heap_arr[0];

    // 2. Move the last item in the heap to index 0 and reduce size
    heap_arr[0] = heap_arr[size - 1];
    size--;

    // 3. Sink down: Swap with the larger of its two children until it's >= both children
    int current = 0;
    while (true) {
        int left_child  = (2 * current) + 1;
        int right_child = (2 * current) + 2;
        int largest     = current;

        if (left_child < size && heap_arr[left_child].amount > heap_arr[largest].amount) {
            largest = left_child;
        }
        if (right_child < size && heap_arr[right_child].amount > heap_arr[largest].amount) {
            largest = right_child;
        }

        if (largest != current) {
            swapItems(current, largest);
            current = largest;
        } else {
            break; // In the right spot!
        }
    }

    return max_item;
}

// SQLite Callback: Pushes every database row into our Max-Heap in RAM
int loadRowIntoHeap(void* data, int col_count, char** values, char** col_names) {
    (void)col_count; (void)col_names;

    ExpenseMaxHeap* heap = (ExpenseMaxHeap*)data;

    int id     = atoi(values[0]);
    string cat = values[1];
    int amount = atoi(values[2]);
    string nt  = values[3];

    heap->insert(id, cat, amount, nt);
    return 0;
}

// 7. TOP K EXPENSES: Extracts the K largest expenses using our Max-Heap
void ExpenseManager::showTopExpenses(int k) {
    sqlite3* db;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) return;

    ExpenseMaxHeap max_heap;
    string sql = "SELECT id, category, amount, note FROM expenses;";
    char* error_msg = 0;

    // Pass &max_heap so loadRowIntoHeap inserts every row into our Max-Heap!
    sqlite3_exec(db, sql.c_str(), loadRowIntoHeap, &max_heap, &error_msg);
    sqlite3_close(db);

    if (max_heap.isEmpty()) {
        cout << "\n(No expenses recorded yet)\n\n";
        return;
    }

    cout << "\n=== Top " << k << " Highest Expenses (via RAM Max-Heap) ===\n";
    cout << "Rank | ID  | Amount     | Category     | Note\n";
    cout << "-----+-----+------------+--------------+----------------------------\n";

    int rank = 1;
    while (rank <= k && !max_heap.isEmpty()) {
        ExpenseItem top = max_heap.extractMax();
        cout << "#" << rank << "   | #"
             << top.id << "  | Rs. "
             << top.amount << "      | "
             << top.category << "          | "
             << top.note << "\n";
        rank++;
    }
    cout << "\n";
}