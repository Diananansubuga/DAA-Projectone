// Project 1 - Classic sorting algorithms, n = 1,000,000
// Merge sort, Heap sort, QuickSort
// Datasets: random, nearly sorted, reverse sorted, duplicates (values 0-999)

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
using namespace std;

const int N = 1000000;
const int RUNS = 5;      // time = average of 5 runs

long long comps = 0;     // number of comparisons
bool countOn = false;    // only count during the counting run
int depth = 0;           // current recursion depth
int maxDepth = 0;        // deepest recursion reached
long long extraBytes = 0; // extra memory (merge sort temp array)

bool lessThan(int x, int y) {
    if (countOn) comps++;
    return x < y;
}

void goDeeper() {
    depth++;
    if (depth > maxDepth) maxDepth = depth;
}


// - Merge sort -

void merge(vector<int>& a, vector<int>& temp, int lo, int mid, int hi) {
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (lessThan(a[j], a[i])) temp[k++] = a[j++];
        else temp[k++] = a[i++];
    }
    while (i < mid) temp[k++] = a[i++];
    while (j < hi) temp[k++] = a[j++];
    for (int x = lo; x < hi; x++) a[x] = temp[x];
}

void mergeSortRec(vector<int>& a, vector<int>& temp, int lo, int hi) {
    if (hi - lo < 2) return;
    int mid = lo + (hi - lo) / 2;
    goDeeper();
    mergeSortRec(a, temp, lo, mid);
    mergeSortRec(a, temp, mid, hi);
    depth--;
    merge(a, temp, lo, mid, hi);
}

void mergeSort(vector<int>& a) {
    vector<int> temp(a.size());
    extraBytes = (long long)a.size() * sizeof(int);
    mergeSortRec(a, temp, 0, a.size());
}


// - Heap sort -
void siftDown(vector<int>& a, int root, int n) {
    while (true) {
        int child = 2 * root + 1;
        if (child >= n) break;
        if (child + 1 < n && lessThan(a[child], a[child + 1])) child++;
        if (lessThan(a[root], a[child])) {
            swap(a[root], a[child]);
            root = child;
        } else break;
    }
}

void heapSort(vector<int>& a) {
    int n = a.size();
    for (int i = n / 2 - 1; i >= 0; i--) siftDown(a, i, n);
    for (int end = n - 1; end > 0; end--) {
        swap(a[0], a[end]);
        siftDown(a, 0, end);
    }
}


// - QuickSort -

int lomuto(vector<int>& a, int lo, int hi) {
    int mid = lo + (hi - lo) / 2;
    swap(a[mid], a[hi]);
    int pivot = a[hi];
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        if (lessThan(a[j], pivot)) {
            i++;
            swap(a[i], a[j]);
        }
    }
    swap(a[i + 1], a[hi]);
    return i + 1;
}

void quickSortRec(vector<int>& a, int lo, int hi) {
    if (lo >= hi) return;
    int p = lomuto(a, lo, hi);
    goDeeper();
    quickSortRec(a, lo, p - 1);
    quickSortRec(a, p + 1, hi);
    depth--;
}

void quickSort(vector<int>& a) {
    quickSortRec(a, 0, a.size() - 1);
}


// - testing -

void runTest(string name, void (*sortFn)(vector<int>&), const vector<int>& original) {
    vector<int> work;

    // timing runs (counting switched off)
    double total = 0;
    for (int r = 0; r < RUNS; r++) {
        work = original;
        auto start = chrono::high_resolution_clock::now();
        sortFn(work);
        auto end = chrono::high_resolution_clock::now();
        total += chrono::duration<double, milli>(end - start).count();
    }
    bool sorted = is_sorted(work.begin(), work.end());

    // one counting run
    work = original;
    comps = 0;
    depth = 0;
    maxDepth = 0;
    extraBytes = 0;
    countOn = true;
    sortFn(work);
    countOn = false;

    cout << left << setw(16) << name
         << right << setw(12) << fixed << setprecision(2) << total / RUNS
         << setw(16) << comps
         << setw(12) << maxDepth
         << setw(14) << setprecision(2) << extraBytes / (1024.0 * 1024.0)
         << setw(10) << (sorted ? "yes" : "NO") << endl;
}

// datasets 
// Every dataset starts from seed 42, so the numbers are the same in every run
// and the same in the modern and classic files.
// Only one dataset is kept in memory at a time to save RAM.

vector<int> makeData(string type) {
    mt19937 gen(42);
    vector<int> a(N);

    if (type == "Duplicates") {
        uniform_int_distribution<int> small(0, 999);   // only 1000 different values
        for (int i = 0; i < N; i++) a[i] = small(gen);
        return a;
    }

    uniform_int_distribution<int> dist(0, N - 1);
    for (int i = 0; i < N; i++) a[i] = dist(gen);

    if (type == "Nearly sorted") {
        sort(a.begin(), a.end());
        for (int s = 0; s < N / 100; s++) {      // swap 1% of the numbers
            int x = gen() % N;
            int y = gen() % N;
            swap(a[x], a[y]);
        }
    }
    if (type == "Reverse sorted") {
        sort(a.begin(), a.end(), greater<int>());
    }
    return a;
}

void printHeader(string type) {
    cout << "\nDataset: " << type << ", n = " << N << ", time = average of " << RUNS << " runs\n";
    cout << left << setw(16) << "Algorithm"
         << right << setw(12) << "Time (ms)"
         << setw(16) << "Comparisons"
         << setw(12) << "Max depth"
         << setw(14) << "Extra (MB)"
         << setw(10) << "Sorted" << endl;
    cout << string(80, '-') << endl;
}

int main() {
    string types[4] = {"Random", "Nearly sorted", "Reverse sorted", "Duplicates"};

    for (int t = 0; t < 4; t++) {
        vector<int> original = makeData(types[t]);
        printHeader(types[t]);
        runTest("Merge sort", mergeSort, original);
        runTest("Heap sort", heapSort, original);

        // QuickSort with Lomuto partition is O(n^2) when there are many equal values,
        // so at 100M duplicates it would take hours so we shall not run the QUOCK SORT here
        if (types[t] == "Duplicates" && N >= 100000000)
            cout << left << setw(16) << "QuickSort" << "  skipped: O(n^2) on duplicates, would take hours" << endl;
        else
            runTest("QuickSort", quickSort, original);
    }

    return 0;
}