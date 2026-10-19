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
using namespace std;

//handling our files 
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


//using alpha dev alone for small sorts 

// puts two numbers in order (a <= b) without an if statement
void condSwap(int& a, int& b) {
    bool r = a < b;
    int tmp = r ? a : b;
    b = r ? b : a;
    a = tmp;
}

// AlphaDev's swap move 
void partialSwap(int& a, int& b, int& c) {
    bool r = c < a;
    int tmp = r ? c : a;
    c = r ? a : c;
    r = tmp < b;
    a = r ? a : b;
    b = r ? b : tmp;
}

void sort3(int& a, int& b, int& c) {
    condSwap(b, c);
    partialSwap(a, b, c);
}

void sort4(int& a, int& b, int& c, int& d) {
    condSwap(a, c);
    condSwap(b, d);
    condSwap(a, b);
    condSwap(c, d);
    condSwap(b, c);
}

void sort5(int& a, int& b, int& c, int& d, int& e) {
    condSwap(a, b);
    condSwap(d, e);
    partialSwap(c, d, e);
    condSwap(b, e);
    partialSwap(a, c, d);
    partialSwap(b, c, d);
}

// sorts a group of 1-5 numbers with the matching AlphaDev routine
void sortSmall(int arr[], int n) {
    if (n == 2) condSwap(arr[0], arr[1]);
    else if (n == 3) sort3(arr[0], arr[1], arr[2]);
    else if (n == 4) sort4(arr[0], arr[1], arr[2], arr[3]);
    else if (n == 5) sort5(arr[0], arr[1], arr[2], arr[3], arr[4]);
}


//Using the helper algorithms

// joins two sorted parts arr[left..mid-1] and arr[mid..right-1]
void merge(vector<int>& arr, vector<int>& temp, int left, int mid, int right) {
    int i = left, j = mid, k = left;
    while (i < mid && j < right) {
        if (arr[i] <= arr[j]) temp[k++] = arr[i++];
        else temp[k++] = arr[j++];
    }
    while (i < mid) temp[k++] = arr[i++];
    while (j < right) temp[k++] = arr[j++];
    for (int t = left; t < right; t++)
        arr[t] = temp[t];
}

int depthLimit(int n) {
    int depth = 0;
    for (int m = n; m > 1; m /= 2)
        depth++;
    return 2 * depth;
}

//AlphaDev + QuickSort

void alphaDevQuickOnly(int arr[], int n, int depth) {
    while (n > 5) {
        if (depth == 0) {                   // safety net: heapsort to prevent worst case scenario 
            make_heap(arr, arr + n);
            sort_heap(arr, arr + n);
            return;
        }
        depth--;

        sort3(arr[0], arr[n / 2], arr[n - 1]);   // median-of-3 pivot with AlphaDev
        int pivot = arr[n / 2];

        int i = 0, j = n - 1;               // partition
        while (i <= j) {
            while (arr[i] < pivot) i++;
            while (arr[j] > pivot) j--;
            if (i <= j) {
                swap(arr[i], arr[j]);
                i++;
                j--;
            }
        }

        int leftSize = j + 1;               // recurse on smaller side, loop on bigger
        int rightSize = n - i;
        if (leftSize < rightSize) {
            alphaDevQuickOnly(arr, leftSize, depth);
            arr = arr + i;
            n = rightSize;
        } else {
            alphaDevQuickOnly(arr + i, rightSize, depth);
            n = leftSize;
        }
    }
    sortSmall(arr, n);                      // 2-5 elements left: AlphaDev finishes
}

void alphaDevQuickSort(vector<int>& arr) {
    alphaDevQuickOnly(arr.data(), arr.size(), depthLimit(arr.size()));
}


//AlphaDev + Merge sort
//  AlphaDev sorts every group of 4, then groups are merged
//  4 -> 8 -> 16 -> ... until the whole array is sorted.

void alphaDevMergeSort(vector<int>& arr) {
    int n = arr.size();

    for (int i = 0; i < n; i += 4)
        sortSmall(arr.data() + i, min(4, n - i));

    vector<int> temp(n);
    for (int width = 4; width < n; width *= 2)
        for (int left = 0; left < n - width; left += 2 * width)
            merge(arr, temp, left, left + width, min(left + 2 * width, n));
}


//  TIMING
//  Sorts a fresh copy of the original numbers 'runs' times and
//  returns the median time in milliseconds.

double timeSort(void (*sortFunction)(vector<int>&), vector<int>& original, int runs, bool& correct) {
    vector<double> times;
    correct = true;
    for (int r = 0; r < runs; r++) {
        vector<int> copy = original;        // same numbers every run, every algorithm

        auto start = chrono::high_resolution_clock::now();
        sortFunction(copy);
        auto end = chrono::high_resolution_clock::now();

        times.push_back(chrono::duration<double, milli>(end - start).count());
        if (!is_sorted(copy.begin(), copy.end()))
            correct = false;
    }
    sort(times.begin(), times.end());
    return times[runs / 2];                 // the median (middle value)
}

//  MAIN PROGRAM

int main() {
    vector<int> numbers = readFromFile("random_numbers.txt");
    if (numbers.empty()) {
        cout << "No numbers loaded. Run the generator program first." << endl;
        return 1;
    }
    cout << "Loaded " << numbers.size() << " numbers" << endl;

    const int RUNS = 5;
    cout << "Each algorithm runs " << RUNS << " times, median time shown\n\n";

    // list of algorithms to compare: a name and the function to call
    string names[] = {
        "1. AlphaDev + QuickSort",
        "2. AlphaDev + Merge sort"
    };
    void (*functions[])(vector<int>&) = {
        alphaDevQuickSort,
        alphaDevMergeSort
    };

    // save results to a CSV file so they can be graphed in Excel
    ofstream csv("alphadev_comparison.csv");
    csv << "algorithm,median_ms\n";

    for (int k = 0; k < 2; k++) {
        bool correct;
        double ms = timeSort(functions[k], numbers, RUNS, correct);

        cout << names[k];
        for (int s = names[k].size(); s < 28; s++) cout << " ";   // line up the columns
        cout << ms << " ms";
        cout << (correct ? "   (sorted correctly)" : "   ERROR: not sorted") << endl;

        csv << names[k] << "," << ms << "\n";
    }

    csv.close();
    cout << "\nResults saved to alphadev_comparison.csv" << endl;
    return 0;
}