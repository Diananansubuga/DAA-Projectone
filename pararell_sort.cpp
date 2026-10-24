// Project 1 - Bonus: Parallel merge sort with OpenMP
// Same merge sort as classic_1M.cpp, but the two halves are sorted
// on different CPU cores at the same time.
// Data: the random dataset (seed 42), n = 1,000,000 and 100,000,000.
// Tested with 1 thread, 2, 4, ... up to all the threads your CPU has.
//
// Compile: g++ -O2 -std=c++17 -fopenmp parallel_sort.cpp -o parallel_sort.exe
// Run:     parael_sort.exe

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
#include <omp.h>
using namespace std;

const int RUNS = 3;            // time = median of 3 runs
const int CUTOFF = 100000;     // parts smaller than this are sorted on one core


//  serial merge sort (same as classic) 

void merge(vector<int>& a, vector<int>& temp, int lo, int mid, int hi) {
    int i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        if (a[j] < a[i]) temp[k++] = a[j++];
        else temp[k++] = a[i++];
    }
    while (i < mid) temp[k++] = a[i++];
    while (j < hi) temp[k++] = a[j++];
    for (int x = lo; x < hi; x++) a[x] = temp[x];
}

void mergeSortRec(vector<int>& a, vector<int>& temp, int lo, int hi) {
    if (hi - lo < 2) return;
    int mid = lo + (hi - lo) / 2;
    mergeSortRec(a, temp, lo, mid);
    mergeSortRec(a, temp, mid, hi);
    merge(a, temp, lo, mid, hi);
}


//  parallel merge sort 
// Each half becomes an OpenMP "task" that any free core can pick up.
// Small parts are not split further, because starting a task costs time.

void parallelRec(vector<int>& a, vector<int>& temp, int lo, int hi) {
    if (hi - lo < CUTOFF) {
        mergeSortRec(a, temp, lo, hi);
        return;
    }
    int mid = lo + (hi - lo) / 2;

    #pragma omp task shared(a, temp)
    parallelRec(a, temp, lo, mid);

    #pragma omp task shared(a, temp)
    parallelRec(a, temp, mid, hi);

    #pragma omp taskwait          // wait for both halves before merging
    merge(a, temp, lo, mid, hi);
}

void parallelMergeSort(vector<int>& a, int threads) {
    vector<int> temp(a.size());
    omp_set_num_threads(threads);
    #pragma omp parallel
    {
        #pragma omp single        // one thread starts, the others pick up tasks
        parallelRec(a, temp, 0, a.size());
    }
}


//  testing 

double timeSort(const vector<int>& original, int threads, bool& sorted) {
    vector<double> times;
    vector<int> work;
    for (int r = 0; r < RUNS; r++) {
        work = original;
        auto start = chrono::high_resolution_clock::now();
        parallelMergeSort(work, threads);
        auto end = chrono::high_resolution_clock::now();
        times.push_back(chrono::duration<double, milli>(end - start).count());
    }
    sorted = is_sorted(work.begin(), work.end());
    sort(times.begin(), times.end());
    return times[RUNS / 2];       // median
}

int main() {
    int maxThreads = omp_get_max_threads();
    cout << "Your CPU has " << maxThreads << " threads\n";

    // thread counts to test: 1, 2, 4, 8, ... and the maximum
    vector<int> threadList;
    for (int t = 1; t < maxThreads; t *= 2) threadList.push_back(t);
    threadList.push_back(maxThreads);

    int sizes[2] = {1000000, 100000000};
    for (int s = 0; s < 2; s++) {
        int n = sizes[s];

        mt19937 gen(42);
        uniform_int_distribution<int> dist(0, n - 1);
        vector<int> original(n);
        for (int i = 0; i < n; i++) original[i] = dist(gen);

        cout << "\nRandom data, n = " << n << ", time = median of " << RUNS << " runs\n";
        cout << left << setw(10) << "Threads"
             << right << setw(14) << "Time (ms)"
             << setw(12) << "Speedup"
             << setw(10) << "Sorted" << endl;
        cout << string(46, '-') << endl;

        double oneThread = 0;
        for (int t : threadList) {
            bool sorted;
            double ms = timeSort(original, t, sorted);
            if (t == 1) oneThread = ms;
            cout << left << setw(10) << t
                 << right << setw(14) << fixed << setprecision(2) << ms
                 << setw(11) << setprecision(2) << oneThread / ms << "x"
                 << setw(10) << (sorted ? "yes" : "NO") << endl;
        }
    }
    return 0;
}