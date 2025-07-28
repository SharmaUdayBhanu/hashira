#include <bits/stdc++.h>
#include "json.hpp" // nlohmann/json library
using namespace std;
using json = nlohmann::json;

// Function to convert string number of any base to decimal
long long convertToDecimal(const string& num, int base) {
    long long result = 0;
    for (char digit : num) {
        int value;
        if (isdigit(digit)) value = digit - '0';
        else value = tolower(digit) - 'a' + 10; // for base > 10
        result = result * base + value;
    }
    return result;
}

// Function to manually solve constant term (c) using Gaussian Elimination or direct combination test (when k=3)
long long solveForC(vector<pair<long long, long long>>& points, int k) {
    // Try all combinations of k points
    for (int i = 0; i < points.size(); i++) {
        for (int j = i + 1; j < points.size(); j++) {
            for (int l = j + 1; l < points.size(); l++) {
                vector<pair<long long, long long>> sample = {points[i], points[j], points[l]};

                // Interpolate c using simple method
                // Assume polynomial: f(x) = a*x^2 + b*x + c
                long long x1 = sample[0].first, y1 = sample[0].second;
                long long x2 = sample[1].first, y2 = sample[1].second;
                long long x3 = sample[2].first, y3 = sample[2].second;

                long long denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
                if (denom == 0) continue;

                // Solve using basic formula for constant term
                long long a = ((y1*(x2 - x3) - y2*(x1 - x3) + y3*(x1 - x2)));
                if (a % denom != 0) continue;
                long long c = a / denom; // approximate constant term

                return c;
            }
        }
    }
    return -1; // if not found
}

int main() {
    ifstream file("input.json");
    if (!file.is_open()) {
        cerr << "Failed to open input.json" << endl;
        return 1;
    }

    json j;
    file >> j;

    // Wrap input cases into array if only one case
    vector<json> testcases;
    if (j.is_array()) testcases = j;
    else testcases.push_back(j);

    for (auto& test : testcases) {
        int n = test["keys"]["n"];
        int k = test["keys"]["k"];

        vector<pair<long long, long long>> points;
        vector<int> wrong_keys;

        // Go through all root entries
        for (auto& el : test.items()) {
            if (el.key() == "keys") continue;
            int x = stoi(el.key());
            int base = stoi(el.value()["base"].get<string>());
            string y_str = el.value()["value"];
            long long y = convertToDecimal(y_str, base);
            points.push_back({x, y});
        }

        long long c = solveForC(points, k);

        cout << "shh!! , the secret is: " << c << endl;
        cout << "And the wrong keys were: ";
        for (auto& p : points) {
            if (p.second != c) {
                cout << "(" << p.first << ", " << p.second << ") ";
            }
        }
        cout << "\n\n";
    }
    return 0;
}
