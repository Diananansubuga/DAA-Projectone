
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <new>
using namespace std;


long long comparisons = 0;
bool countOn = false;

bool less_than(int a, int b) {
    if (countOn) comparisons++;
    return a < b;
}

//  Recursion depth 
int currentDepth = 0;
int maxDepth = 0;

void enterCall() {
    currentDepth++;
    if (currentDepth > maxDepth) maxDepth = currentDepth;
}
void leaveCall() {
    currentDepth--;
}

//  Heap memory 
size_t heapNow = 0;     
size_t heapPeak = 0;    

void* operator new(size_t size) {
    char* block = (char*)malloc(size + 16);
    if (!block) throw bad_alloc();
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



//  MERGE SORT 

void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    // Create temp vectors
    vector<int> L(n1), R(n2);

    // Copy data to temp vectors L[] and R[]
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0;
    int k = left;

    // Merge the temp vectors back into arr[left..right]
    while (i < n1 && j < n2) {
        if (!less_than(R[j], L[i])) {      // same as L[i] <= R[j]
            arr[k] = L[i];
            i++;
        }
        else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if there are any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if there are any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void mergeSort(vector<int>& arr, int left, int right) {
    if (left >= right)
        return;
    enterCall();

    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);

    leaveCall();
}

void runMergeSort(vector<int>& arr) {
    mergeSort(arr, 0, arr.size() - 1);
}

//  HEAP SORT 
// To heapify a subtree rooted with node i
void heapify(vector<int>& arr, int n, int i) {
    enterCall();

    int largest = i;            // Initialize largest as root
    int l = 2 * i + 1;          // left index
    int r = 2 * i + 2;          // right index

    // If left child is larger than root
    if (l < n && less_than(arr[largest], arr[l]))
        largest = l;

    // If right child is larger than largest so far
    if (r < n && less_than(arr[largest], arr[r]))
        largest = r;

    // If largest is not root
    if (largest != i) {
        swap(arr[i], arr[largest]);
        heapify(arr, n, largest);   // Recursively heapify the affected sub-tree
    }

    leaveCall();
}

void heapSort(vector<int>& arr) {
    int n = arr.size();

    // Build heap (rearrange vector)
    for (int i = n / 2 - 1; i >= 0; i--)
        heapify(arr, n, i);

    // One by one extract an element from heap
    for (int i = n - 1; i > 0; i--) {
        swap(arr[0], arr[i]);       // Move current root to end
        heapify(arr, i, 0);         // Call max heapify on the reduced heap
    }
}

//  QUICKSORT (Using the middle element as pivot)

int partition(vector<int>& arr, int low, int high) {
    // choose the middle element as pivot and move it to the end
    int mid = low + (high - low) / 2;
    swap(arr[mid], arr[high]);
    int pivot = arr[high];

    // index of smaller element
    int i = low - 1;

    // move all elements smaller than the pivot to the left side
    for (int j = low; j <= high - 1; j++) {
        if (less_than(arr[j], pivot)) {
            i++;
            swap(arr[i], arr[j]);
        }
    }

    // move pivot after smaller elements and return its position
    swap(arr[i + 1], arr[high]);
    return i + 1;
}

void quickSort(vector<int>& arr, int low, int high) {
    if (low < high) {
        enterCall();

        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);

        leaveCall();
    }
}

void runQuickSort(vector<int>& arr) {
    quickSort(arr, 0, arr.size() - 1);
}

//  FILE READING

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

//  MEASURING ONE ALGORITHM ON ONE DATASET

struct Result {
    double medianMs;
    long long comparisons;
    size_t extraHeapBytes;
    int maxDepth;
    bool correct;
};

Result measure(void (*sortFunction)(vector<int>&), vector<int>& original, int runs) {
    Result res;
    res.correct = true;
    res.extraHeapBytes = 0;

    //1. Time and heap memory 
    countOn = false;
    vector<double> times;
    for (int r = 0; r < runs; r++) {
        vector<int> copy = original;        // fresh unsorted copy

        size_t heapBefore = heapNow;
        heapPeak = heapNow;

        auto start = chrono::high_resolution_clock::now();
        sortFunction(copy);
        auto end = chrono::high_resolution_clock::now();

        size_t extra = heapPeak - heapBefore;
        if (extra > res.extraHeapBytes) res.extraHeapBytes = extra;

        times.push_back(chrono::duration<double, milli>(end - start).count());
        if (!is_sorted(copy.begin(), copy.end())) res.correct = false;
    }
    sort(times.begin(), times.end());
    res.medianMs = times[runs / 2];

    // 2. Comparisons and recursion depth 
    vector<int> copy = original;
    comparisons = 0;
    maxDepth = 0;
    currentDepth = 0;
    countOn = true;
    sortFunction(copy);
    countOn = false;
    res.comparisons = comparisons;
    res.maxDepth = maxDepth;

    return res;
}


//  MAIN PROGRAM

int main() {
    string datasets[] = { "random_100m.txt", "nearly_sorted_100m.txt", "reverse_100m.txt", "duplicates_100m.txt" };
    string names[] = { "Merge sort", "Heap sort", "QuickSort" };
    void (*functions[])(vector<int>&) = { runMergeSort, heapSort, runQuickSort };
    const int RUNS = 5;

    ofstream csv("classic_results_100m.csv");
    csv << "algorithm,dataset,n,median_ms,comparisons,extra_heap_bytes,max_recursion_depth\n";

    for (int d = 0; d < 4; d++) {
        vector<int> data = readFromFile(datasets[d]);
        if (data.empty()) continue;

        cout << "\n Dataset: " << datasets[d] << " (" << data.size() << " numbers)\n";
        cout << "Algorithm     Time (ms)     Comparisons     Extra heap (KB)   Recursion depth\n";
        

        for (int a = 0; a < 3; a++) {
            Result r = measure(functions[a], data, RUNS);

            cout << names[a];
            for (int s = names[a].size(); s < 14; s++) cout << " ";
            cout << r.medianMs << "       " << r.comparisons << "       "
                 << r.extraHeapBytes / 1024 << "               " << r.maxDepth
                 << (r.correct ? "" : "   ERROR: not sorted") << endl;

            csv << names[a] << "," << datasets[d] << "," << data.size() << ","
                << r.medianMs << "," << r.comparisons << ","
                << r.extraHeapBytes << "," << r.maxDepth << "\n";
        }
    }

    csv.close();
    cout << "\nResults saved to classic_results_100m.csv" << endl;
    return 0;
}

