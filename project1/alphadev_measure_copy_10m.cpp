// APLHA DEV SORTING ALGORITHM
//  AlphaDev only sorts 3, 4 or 5 numbers at a specific time we are going to use host alorithms
//  . In this program i am going to test  AlphaDev with two hosts (QuickSort and
//  Merge sort), each with and without Insertion sort:
//  Loads 1,000,000 numbers from random_numbers.txt.
//  Each algorithm sorts a fresh copy of the SAME numbers 5 times,
//  and the median time is reported.

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>
#include <cstdlib>
#include <new>
using namespace std;


long long comparisons = 0;

template <bool COUNT>
bool lessThan(int a, int b) {
    if (COUNT) comparisons++;
    return a < b;
}

int maxDepth = 0;

size_t heapNow = 0;     
size_t heapPeak = 0;    
void* operator new(size_t size) {
    char* block = (char*)malloc(size + 16);     
    *(size_t*)block = size;                    
    heapNow += size;
    if (heapNow > heapPeak) heapPeak = heapNow;
    return block + 16;
}

void operator delete(void* ptr) noexcept {
    if (!ptr) return;
    char* block = (char*)ptr - 16;
    heapNow -= *(size_t*)block;
    free(block);
}

void operator delete(void* ptr, size_t) noexcept {
    operator delete(ptr);
}

vector<int> readFromFile(string filename) {
    vector<int> arr;
    ifstream in(filename);
    if (!in) {
        cout << "Could not open " << filename << endl;
        return arr;
    }
    int x;
    while (in >> x)
        arr.push_back(x);
    return arr;
}



//  AlphaDev + QuickSort (Introsort with AlphaDev routines)

// puts two numbers in order (a <= b) without an if statement
template <bool COUNT>
void condSwap(int& a, int& b) {
    bool r = lessThan<COUNT>(a, b);
    int tmp = r ? a : b;
    b = r ? b : a;
    a = tmp;
}

// AlphaDev's swap move
template <bool COUNT>
void partialSwap(int& a, int& b, int& c) {
    bool r = lessThan<COUNT>(c, a);
    int tmp = r ? c : a;
    c = r ? a : c;
    r = lessThan<COUNT>(tmp, b);
    a = r ? a : b;
    b = r ? b : tmp;
}

template <bool COUNT>
void alphaSort3(int& a, int& b, int& c) {
    condSwap<COUNT>(b, c);
    partialSwap<COUNT>(a, b, c);
}

template <bool COUNT>
void alphaSort4(int& a, int& b, int& c, int& d) {
    condSwap<COUNT>(a, c);
    condSwap<COUNT>(b, d);
    condSwap<COUNT>(a, b);
    condSwap<COUNT>(c, d);
    condSwap<COUNT>(b, c);
}

template <bool COUNT>
void alphaSort5(int& a, int& b, int& c, int& d, int& e) {
    condSwap<COUNT>(a, b);
    condSwap<COUNT>(d, e);
    partialSwap<COUNT>(c, d, e);
    condSwap<COUNT>(b, e);
    partialSwap<COUNT>(a, c, d);
    partialSwap<COUNT>(b, c, d);
}

template <bool COUNT>
void alphaSortSmall(int arr[], int n) {
    if (n == 2) condSwap<COUNT>(arr[0], arr[1]);
    else if (n == 3) alphaSort3<COUNT>(arr[0], arr[1], arr[2]);
    else if (n == 4) alphaSort4<COUNT>(arr[0], arr[1], arr[2], arr[3]);
    else if (n == 5) alphaSort5<COUNT>(arr[0], arr[1], arr[2], arr[3], arr[4]);
}

// level = how deep in the recursion we are (for measuring stack use)
template <bool COUNT>
void alphaDevQuick(int arr[], int n, int depth, int level) {
    if (COUNT && level > maxDepth) maxDepth = level;

    while (n > 5) {
        if (depth == 0) {                           
            make_heap(arr, arr + n, lessThan<COUNT>);
            sort_heap(arr, arr + n, lessThan<COUNT>);
            return;
        }
        depth--;

        alphaSort3<COUNT>(arr[0], arr[n / 2], arr[n - 1]);   
        int pivot = arr[n / 2];

        int i = 0, j = n - 1;                       
        while (i <= j) {
            while (lessThan<COUNT>(arr[i], pivot)) i++;
            while (lessThan<COUNT>(pivot, arr[j])) j--;
            if (i <= j) {
                swap(arr[i], arr[j]);
                i++;
                j--;
            }
        }

        int leftSize = j + 1;                       
        int rightSize = n - i;
        if (leftSize < rightSize) {
            alphaDevQuick<COUNT>(arr, leftSize, depth, level + 1);
            arr = arr + i;
            n = rightSize;
        } else {
            alphaDevQuick<COUNT>(arr + i, rightSize, depth, level + 1);
            n = leftSize;
        }
    }
    alphaSortSmall<COUNT>(arr, n);                  
}

template <bool COUNT>
void alphaDevSort(vector<int>& arr) {
    int n = arr.size();
    int log2n = 0;
    for (int m = n; m > 1; m /= 2) log2n++;
    alphaDevQuick<COUNT>(arr.data(), n, 2 * log2n, 1);
}


struct Result {
    double medianMs;
    long long comparisons;
    size_t extraHeapBytes;
    int maxDepth;
    bool correct;
};

// timedSort   = the version WITHOUT counting (for clean timing)
// countedSort = the version WITH counting (for comparisons and depth)
Result measure(void (*timedSort)(vector<int>&), void (*countedSort)(vector<int>&),
               vector<int>& original, int runs) {
    Result res;
    res.correct = true;
    res.extraHeapBytes = 0;

    // ---- 1. Execution time and heap memory ----
    vector<double> times;
    for (int r = 0; r < runs; r++) {
        vector<int> copy = original;            // fresh copy (allocated BEFORE measuring memory)

        size_t heapBefore = heapNow;            // memory in use before sorting
        heapPeak = heapNow;                     // reset the peak

        auto start = chrono::high_resolution_clock::now();
        timedSort(copy);
        auto end = chrono::high_resolution_clock::now();

        size_t extra = heapPeak - heapBefore;   // extra heap the sort needed
        if (extra > res.extraHeapBytes) res.extraHeapBytes = extra;

        times.push_back(chrono::duration<double, milli>(end - start).count());
        if (!is_sorted(copy.begin(), copy.end())) res.correct = false;
    }
    sort(times.begin(), times.end());
    res.medianMs = times[runs / 2];

    // ---- 2. Comparisons and recursion depth (one separate run) ----
    vector<int> copy = original;
    comparisons = 0;
    maxDepth = 0;
    countedSort(copy);
    res.comparisons = comparisons;
    res.maxDepth = maxDepth;
    if (!is_sorted(copy.begin(), copy.end())) res.correct = false;

    return res;
}


//  run alpha-dev on all programs

int main() {
    string datasets[] = { "random_100m.txt", "nearly_sorted_100m.txt", "reverse_100m.txt", "duplicates_100m.txt" };
    const int RUNS = 5;

    // one CSV file for all four datasets
    ofstream csv("alphadev_measurements_100m.csv");
    csv << "algorithm,dataset,n,median_ms,comparisons,extra_heap_bytes,max_recursion_depth\n";

    for (int d = 0; d < 4; d++) {
        vector<int> numbers = readFromFile(datasets[d]);
        if (numbers.empty()) {
            cout << "Skipping " << datasets[d] << endl;
            continue;
        }
        int n = numbers.size();

        Result r = measure(alphaDevSort<false>, alphaDevSort<true>, numbers, RUNS);

        cout << "\nAlphaDev sort on " << datasets[d] << " (" << n << " numbers)" << endl;
        cout << "Execution time (median of " << RUNS << " runs): " << r.medianMs << " ms" << endl;
        cout << "Number of comparisons:           " << r.comparisons << endl;
        cout << "Extra heap memory:               " << r.extraHeapBytes << " bytes" << endl;
        cout << "Max recursion depth (stack):     " << r.maxDepth << " levels" << endl;
        cout << (r.correct ? "Array is sorted correctly" : "ERROR: array is not sorted") << endl;

        csv << "AlphaDev sort," << datasets[d] << "," << n << "," << r.medianMs << ","
            << r.comparisons << "," << r.extraHeapBytes << "," << r.maxDepth << "\n";
    }

    csv.close();
    cout << "\nResults saved to alphadev_measurements_100m.csv" << endl;
    return 0;
}
