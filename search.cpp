// Project 1 - Search algorithms, n = 1,000,000
// Chosen Algorithm: Binary search, Exponential search, M-way tree (M = 10)
// Data: the random dataset (seed 42, same as sorting), sorted first.
// Queries: 1000 random numbers (seed 7). Some are in the data, some are not.
//
// Compile: g++ -O2 -std=c++17 search_1M.cpp -o search_1M.exe
// Run:     search_1M.exe

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <iomanip>
using namespace std;

const int N = 1000000;
const int QUERIES = 1000;
const int REPEAT = 100;   // each query is repeated 100 times and the time divided by 100,
                          // because one search is too fast for the clock to measure alone
const int ROUNDS = 5;     // each query is timed in 5 rounds, fastest round is kept

long long comps = 0;      // comparisons for the current query
bool countOn = false;
volatile int zero = 0;    // stops the compiler from skipping the repeated searches

bool lessThan(int x, int y) {
    if (countOn) comps++;
    return x < y;
}

bool isEqual(int x, int y) {
    if (countOn) comps++;
    return x == y;
}


// ---------------- Binary search ----------------
// Look at the middle, then keep only the half that can contain x.

bool binarySearch(const vector<int>& a, int x) {
    int lo = 0, hi = a.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (isEqual(a[mid], x)) return true;
        if (lessThan(x, a[mid])) hi = mid - 1;
        else lo = mid + 1;
    }
    return false;
}

bool binaryRange(const vector<int>& a, int x, int lo, int hi) {
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (isEqual(a[mid], x)) return true;
        if (lessThan(x, a[mid])) hi = mid - 1;
        else lo = mid + 1;
    }
    return false;
}


// ---------------- Exponential search ----------------
// Jump 1, 2, 4, 8, ... until we pass x, then binary search that last jump.

bool exponentialSearch(const vector<int>& a, int x) {
    int n = a.size();
    if (isEqual(a[0], x)) return true;
    int i = 1;
    while (i < n && !lessThan(x, a[i])) i *= 2;    // while a[i] <= x
    return binaryRange(a, x, i / 2, min(i, n - 1));
}

// M-way tree (M = 10) 
//**How it works: 
// Each node has up to 10 children and 9 keys.
// The bottom level points to blocks of 9 numbers in the sorted array,
// so the tree only stores the "signposts", not a second copy of the data.
// Built from the bottom up: blocks -> nodes of 10 -> nodes of 10 -> ... -> root.

const int M = 10;
const int BLOCK = M - 1;   // 9 numbers per block

struct Node {
    int count;             // number of children
    int keys[M - 1];       // keys[j] = smallest value in child j+1
    int child[M];          // index of child node, or block number at the bottom
    bool bottom;           // true if the children are blocks of the array
};

vector<Node> tree;
int root = 0;

void buildTree(const vector<int>& a) {
    tree.clear();
    int n = a.size();
    int blocks = (n + BLOCK - 1) / BLOCK;

    vector<int> ids, firsts;      // children of the level being built
    for (int b = 0; b < blocks; b++) {
        ids.push_back(b);
        firsts.push_back(a[b * BLOCK]);
    }

    bool bottom = true;
    while (ids.size() > 1 || bottom) {
        vector<int> newIds, newFirsts;
        for (int i = 0; i < (int)ids.size(); i += M) {
            Node nd;
            nd.count = min(M, (int)ids.size() - i);
            nd.bottom = bottom;
            for (int j = 0; j < nd.count; j++) {
                nd.child[j] = ids[i + j];
                if (j > 0) nd.keys[j - 1] = firsts[i + j];
            }
            tree.push_back(nd);
            newIds.push_back(tree.size() - 1);
            newFirsts.push_back(firsts[i]);
        }
        ids = newIds;
        firsts = newFirsts;
        bottom = false;
    }
    root = ids[0];
}

bool mwaySearch(const vector<int>& a, int x) {
    int node = root;
    while (true) {
        const Node& nd = tree[node];
        int i = 0;
        while (i < nd.count - 1 && !lessThan(x, nd.keys[i])) i++;   // keys[i] <= x -> go right
        if (nd.bottom) {
            int lo = nd.child[i] * BLOCK;
            int hi = min(lo + BLOCK, (int)a.size());
            for (int k = lo; k < hi; k++) {
                if (isEqual(a[k], x)) return true;
                if (lessThan(x, a[k])) return false;
            }
            return false;
        }
        node = nd.child[i];
    }
}

//  testing 

void runTest(string name, bool (*searchFn)(const vector<int>&, int),
             const vector<int>& a, const vector<int>& queries, double extraMB) {
    // time: each query is timed in ROUNDS separate rounds and we keep its fastest round,
    // so a slow moment (another program, Windows) does not count as a "worst case"
    vector<double> best(QUERIES, 1e18);
    int sink = 0;
    for (int round = 0; round < ROUNDS; round++) {
        for (int q = 0; q < QUERIES; q++) {
            auto start = chrono::high_resolution_clock::now();
            for (int r = 0; r < REPEAT; r++) sink += searchFn(a, queries[q] + zero);
            auto end = chrono::high_resolution_clock::now();
            double ns = chrono::duration<double, nano>(end - start).count() / REPEAT;
            if (ns < best[q]) best[q] = ns;
        }
    }
    double totalNs = 0, worstNs = 0;
    for (int q = 0; q < QUERIES; q++) {
        totalNs += best[q];
        if (best[q] > worstNs) worstNs = best[q];
    }

    // comparisons: one counted search per query
    long long totalComps = 0, worstComps = 0;
    int found = 0;
    for (int q = 0; q < QUERIES; q++) {
        comps = 0;
        countOn = true;
        if (searchFn(a, queries[q])) found++;
        countOn = false;
        totalComps += comps;
        if (comps > worstComps) worstComps = comps;
    }

    cout << left << setw(20) << name
         << right << setw(10) << fixed << setprecision(1) << totalNs / QUERIES
         << setw(11) << worstNs
         << setw(10) << setprecision(1) << (double)totalComps / QUERIES
         << setw(10) << worstComps
         << setw(12) << setprecision(2) << extraMB
         << setw(8) << found << endl;
    if (sink == -1) cout << "";    // uses sink so the searches are not removed
}

int main() {
    // same data as the random sorting dataset, then sorted
    mt19937 gen(42);
    uniform_int_distribution<int> dist(0, N - 1);
    vector<int> a(N);
    for (int i = 0; i < N; i++) a[i] = dist(gen);
    sort(a.begin(), a.end());

    // 1000 random queries
    mt19937 qgen(7);
    vector<int> queries(QUERIES);
    for (int i = 0; i < QUERIES; i++) queries[i] = dist(qgen);

    // build the M-way tree (timed separately, it is done once)
    auto start = chrono::high_resolution_clock::now();
    buildTree(a);
    auto end = chrono::high_resolution_clock::now();
    double buildMs = chrono::duration<double, milli>(end - start).count();
    double treeMB = tree.size() * sizeof(Node) / (1024.0 * 1024.0);

    cout << "Search on sorted random data, n = " << N << ", " << QUERIES << " queries\n";
    cout << "M-way tree: M = " << M << ", " << tree.size() << " nodes, built in "
         << fixed << setprecision(2) << buildMs << " ms\n\n";

    cout << left << setw(20) << "Algorithm"
         << right << setw(10) << "Avg (ns)"
         << setw(11) << "Worst (ns)"
         << setw(10) << "Avg cmp"
         << setw(10) << "Worst cmp"
         << setw(12) << "Extra (MB)"
         << setw(8) << "Found" << endl;
    cout << string(81, '-') << endl;

    runTest("Binary search", binarySearch, a, queries, 0.0);
    runTest("Exponential search", exponentialSearch, a, queries, 0.0);
    runTest("M-way tree (M=10)", mwaySearch, a, queries, treeMB);

    return 0;
}