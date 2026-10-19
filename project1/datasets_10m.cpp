#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
using namespace std;

void saveToFile(vector<int>& arr, string filename) {
    ofstream out(filename);
    for (int i = 0; i < arr.size(); i++)
        out << arr[i] << "\n";
    out.close();
    cout << "Saved " << arr.size() << " numbers to " << filename << endl;
}

int main() {
    int n = 100000000;                      // 100 million
    unsigned int seed = random_device{}();
    cout << "Seed: " << seed << endl;      // record this so the run can be repeated
    mt19937 gen(seed);

    // 1. Random: any value from 0 to 100,000,000
    uniform_int_distribution<int> dist(0, 100000000);
    vector<int> randomData(n);
    for (int i = 0; i < n; i++)
        randomData[i] = dist(gen);
    saveToFile(randomData, "random_100m.txt");

    // 2. Nearly sorted: sort a copy, then swap 1% of positions (1,000,000 swaps)
    vector<int> sortedData = randomData;
    sort(sortedData.begin(), sortedData.end());
    randomData.clear();
    randomData.shrink_to_fit();             // free memory we no longer need

    vector<int> nearlySorted = sortedData;
    uniform_int_distribution<int> pos(0, n - 1);
    for (int i = 0; i < n / 100; i++)
        swap(nearlySorted[pos(gen)], nearlySorted[pos(gen)]);
    saveToFile(nearlySorted, "nearly_sorted_100m.txt");
    nearlySorted.clear();
    nearlySorted.shrink_to_fit();

    // 3. Reverse sorted: largest to smallest
    reverse(sortedData.begin(), sortedData.end());
    saveToFile(sortedData, "reverse_100m.txt");
    sortedData.clear();
    sortedData.shrink_to_fit();

    // 4. Duplicates: only values 0 - 1000, so each value appears ~100,000 times
    uniform_int_distribution<int> smallDist(0, 1000);
    vector<int> duplicates(n);
    for (int i = 0; i < n; i++)
        duplicates[i] = smallDist(gen);
    saveToFile(duplicates, "duplicates_100m.txt");

    cout << "All 4 datasets created." << endl;
    return 0;
}

