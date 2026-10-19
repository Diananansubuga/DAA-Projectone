
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
    int n = 1000000;
    unsigned int seed = random_device{}();
    cout << "Seed: " << seed << endl;      // record this so the run can be repeated
    mt19937 gen(seed);

    // Random: any value from 0 to 1,000,000
    uniform_int_distribution<int> dist(0, 1000000);
    vector<int> randomData(n);
    for (int i = 0; i < n; i++)
        randomData[i] = dist(gen);
    saveToFile(randomData, "random.txt");

    // I sorted a copy of the random data, then swapped 1% of positions
    vector<int> nearlySorted = randomData;
    sort(nearlySorted.begin(), nearlySorted.end());
    uniform_int_distribution<int> pos(0, n - 1);
    for (int i = 0; i < n / 100; i++)              // 10,000 random swaps
        swap(nearlySorted[pos(gen)], nearlySorted[pos(gen)]);
    saveToFile(nearlySorted, "nearly_sorted.txt");

    // 3. the random numbers are organized form largest to smallest
    vector<int> reverseData = randomData;
    sort(reverseData.begin(), reverseData.end());
    reverse(reverseData.begin(), reverseData.end());
    saveToFile(reverseData, "reverse.txt");

    // only values 0 - 1000 so we have repeated values 
    uniform_int_distribution<int> smallDist(0, 1000);
    vector<int> duplicates(n);
    for (int i = 0; i < n; i++)
        duplicates[i] = smallDist(gen);
    saveToFile(duplicates, "duplicates.txt");

    cout << "All 4 datasets created." << endl;
    return 0;
}