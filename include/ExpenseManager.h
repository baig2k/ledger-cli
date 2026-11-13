#ifndef EXPENSE_MANAGER_H
#define EXPENSE_MANAGER_H

#include <string>
using namespace std;

// 1. Struct representing a single expense inside our RAM Max-Heap
struct ExpenseItem {
    int id;
    string category;
    int amount;
    string note;

    ExpenseItem() {
        id = 0;
        category = "";
        amount = 0;
        note = "";
    }

    ExpenseItem(int i, string cat, int amt, string nt) {
        id = i;
        category = cat;
        amount = amt;
        note = nt;
    }
};

// 2. Dynamic Array-Based Max-Heap (Priority Queue ordered by 'amount')
class ExpenseMaxHeap {
private:
    ExpenseItem* heap_arr;
    int capacity;
    int size;

    void resizeHeap();
    void swapItems(int idx1, int idx2);

public:
    ExpenseMaxHeap(int initial_cap = 16);
    ~ExpenseMaxHeap();

    void insert(int id, string category, int amount, string note);
    ExpenseItem extractMax();
    bool isEmpty();
    int getSize();
};

class ExpenseManager {
private:
    const char* db_path;
    bool runQuery(string sql);

public:
    ExpenseManager(const char* path);

    bool addExpense(string category, int amount, string note);
    void listExpenses();
    void showSummary();
    bool updateExpense(int id, int new_amount);
    bool deleteExpense(int id);
    void filterByCategory(string category);

    // NEW: Uses an in-memory Max-Heap to extract the Top K largest expenses!
    void showTopExpenses(int k);
};

#endif