// Project 1 - Modern sorting algorithms on 100,000,000 random numbers

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <iomanip>
using namespace std;

const int N = 100000000;
const int RUNS = 3;      // time = average of 3 runs

long long comps = 0;     // number of comparisons
bool countOn = false;    // only count during the counting run
int depth = 0;           // current recursion depth
int maxDepth = 0;        // deepest recursion reached
long long extraBytes = 0; // extra memory for TwinArray

bool lessThan(int x, int y) {
    if (countOn) comps++;
    return x < y;
}

void goDeeper() {
    depth++;
    if (depth > maxDepth) maxDepth = depth;
}


// - shared helpers -

void insertionSort(vector<int>& a, int lo, int hi) {
    for (int i = lo + 1; i < hi; i++) {
        int x = a[i];
        int j = i - 1;
        while (j >= lo && lessThan(x, a[j])) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = x;
    }
}

void siftDown(vector<int>& a, int lo, int root, int n) {
    while (true) {
        int child = 2 * root + 1;
        if (child >= n) break;
        if (child + 1 < n && lessThan(a[lo + child], a[lo + child + 1])) child++;
        if (lessThan(a[lo + root], a[lo + child])) {
            swap(a[lo + root], a[lo + child]);
            root = child;
        } else break;
    }
}

void heapSort(vector<int>& a, int lo, int hi) {
    int n = hi - lo;
    for (int i = n / 2 - 1; i >= 0; i--) siftDown(a, lo, i, n);
    for (int end = n - 1; end > 0; end--) {
        swap(a[lo], a[lo + end]);
        siftDown(a, lo, 0, end);
    }
}

// pivot is a[lo]. smaller values go left, values >= pivot go right.
// returns the final position of the pivot.
int partitionRight(vector<int>& a, int lo, int hi, bool& noSwaps) {
    int pivot = a[lo];
    int i = lo + 1, j = hi - 1;
    noSwaps = true;
    while (true) {
        while (i <= j && lessThan(a[i], pivot)) i++;
        while (i <= j && !lessThan(a[j], pivot)) j--;
        if (i > j) break;
        swap(a[i], a[j]);
        noSwaps = false;
        i++;
        j--;
    }
    swap(a[lo], a[j]);
    return j;
}

// - AlphaDev sort -
void cmpSwap(int& x, int& y) {
    if (countOn) comps++;
    int small = min(x, y);
    int big = max(x, y);
    x = small;
    y = big;
}

void sort3(vector<int>& a, int i) {
    cmpSwap(a[i], a[i + 1]);
    cmpSwap(a[i + 1], a[i + 2]);
    cmpSwap(a[i], a[i + 1]);
}

void sort4(vector<int>& a, int i) {
    cmpSwap(a[i], a[i + 1]);
    cmpSwap(a[i + 2], a[i + 3]);
    cmpSwap(a[i], a[i + 2]);
    cmpSwap(a[i + 1], a[i + 3]);
    cmpSwap(a[i + 1], a[i + 2]);
}

void sort5(vector<int>& a, int i) {
    cmpSwap(a[i], a[i + 1]);
    cmpSwap(a[i + 3], a[i + 4]);
    cmpSwap(a[i + 2], a[i + 4]);
    cmpSwap(a[i + 2], a[i + 3]);
    cmpSwap(a[i], a[i + 3]);
    cmpSwap(a[i], a[i + 2]);
    cmpSwap(a[i + 1], a[i + 4]);
    cmpSwap(a[i + 1], a[i + 3]);
    cmpSwap(a[i + 1], a[i + 2]);
}

void alphaDev(vector<int>& a, int lo, int hi) {
    while (true) {
        int n = hi - lo;
        if (n <= 1) return;
        if (n == 2) { cmpSwap(a[lo], a[lo + 1]); return; }
        if (n == 3) { sort3(a, lo); return; }
        if (n == 4) { sort4(a, lo); return; }
        if (n == 5) { sort5(a, lo); return; }

        // median of three becomes the pivot
        int mid = lo + n / 2;
        cmpSwap(a[lo], a[mid]);
        cmpSwap(a[mid], a[hi - 1]);
        cmpSwap(a[lo], a[mid]);
        swap(a[lo], a[mid]);

        bool noSwaps;
        int p = partitionRight(a, lo, hi, noSwaps);

        // recurse on the smaller side, loop on the bigger side
        goDeeper();
        if (p - lo < hi - p) {
            alphaDev(a, lo, p);
            lo = p + 1;
        } else {
            alphaDev(a, p + 1, hi);
            hi = p;
        }
        depth--;
    }
}

void alphaDevSort(vector<int>& a) {
    alphaDev(a, 0, a.size());
}


// - PDQSort (simplified) -

int partitionLeft(vector<int>& a, int lo, int hi) {
    int pivot = a[lo];
    int i = lo + 1, j = hi - 1;
    while (true) {
        while (i <= j && !lessThan(pivot, a[i])) i++;
        while (i <= j && lessThan(pivot, a[j])) j--;
        if (i > j) break;
        swap(a[i], a[j]);
        i++;
        j--;
    }
    swap(a[lo], a[j]);
    return j;
}

// gives up after 8 moves
bool partialInsertionSort(vector<int>& a, int lo, int hi) {
    int moves = 0;
    for (int i = lo + 1; i < hi; i++) {
        int x = a[i];
        int j = i - 1;
        while (j >= lo && lessThan(x, a[j])) {
            a[j + 1] = a[j];
            j--;
            moves++;
        }
        a[j + 1] = x;
        if (moves > 8) return false;
    }
    return true;
}

void pdq(vector<int>& a, int lo, int hi, int badAllowed) {
    while (true) {
        int n = hi - lo;
        if (n < 24) {
            insertionSort(a, lo, hi);
            return;
        }

        int mid = lo + n / 2;
        if (lessThan(a[mid], a[lo])) swap(a[mid], a[lo]);
        if (lessThan(a[hi - 1], a[mid])) swap(a[hi - 1], a[mid]);
        if (lessThan(a[mid], a[lo])) swap(a[mid], a[lo]);
        swap(a[lo], a[mid]);

        // pivot equal to the value before this part -> many duplicates
        if (lo > 0 && !lessThan(a[lo - 1], a[lo])) {
            int p = partitionLeft(a, lo, hi);
            lo = p + 1;
            continue;
        }

        bool noSwaps;
        int p = partitionRight(a, lo, hi, noSwaps);
        int leftSize = p - lo;
        int rightSize = hi - p - 1;

        if (leftSize < n / 8 || rightSize < n / 8) {
            badAllowed--;
            if (badAllowed <= 0) {
                heapSort(a, lo, hi);
                return;
            }
        } else if (noSwaps) {
            if (partialInsertionSort(a, lo, p) && partialInsertionSort(a, p + 1, hi)) return;
        }

        goDeeper();
        pdq(a, lo, p, badAllowed);
        depth--;
        lo = p + 1;
    }
}

void pdqSort(vector<int>& a) {
    pdq(a, 0, a.size(), (int)log2(a.size()));
}


// - TwinArray Sort -
// Two arrays of size k (k = max - min + 1):
//   value[i] holds the number, count[i] holds how many times it appears.
// Numbers are placed by index, so the sort itself does no comparisons.
// (Finding min and max does use comparisons, and they are counted.)

void twinArraySort(vector<int>& a) {
    int mn = a[0], mx = a[0];
    for (int x : a) {
        if (lessThan(x, mn)) mn = x;
        if (lessThan(mx, x)) mx = x;
    }
    int k = mx - mn + 1;
    vector<int> value(k, 0);
    vector<int> count(k, 0);
    extraBytes = 2LL * k * sizeof(int);

    for (int x : a) {
        value[x - mn] = x;
        count[x - mn]++;
    }

    int pos = 0;
    for (int i = 0; i < k; i++)
        for (int c = 0; c < count[i]; c++)
            a[pos++] = value[i];
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

int main() {
    // same seed = same numbers every time
    mt19937 gen(42);
    uniform_int_distribution<int> dist(0, N - 1);
    vector<int> original(N);
    for (int i = 0; i < N; i++) original[i] = dist(gen);

    cout << "Random data, n = " << N << ", time = average of " << RUNS << " runs\n\n";
    cout << left << setw(16) << "Algorithm"
         << right << setw(12) << "Time (ms)"
         << setw(16) << "Comparisons"
         << setw(12) << "Max depth"
         << setw(14) << "Extra (MB)"
         << setw(10) << "Sorted" << endl;
    cout << string(80, '-') << endl;

    runTest("AlphaDev sort", alphaDevSort, original);
    runTest("PDQSort", pdqSort, original);
    runTest("TwinArray Sort", twinArraySort, original);

    return 0;
}