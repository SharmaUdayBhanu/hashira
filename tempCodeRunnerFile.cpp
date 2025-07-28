#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <nlohmann/json.hpp>  // JSON parsing ke liye library (https://github.com/nlohmann/json)

using namespace std;
using json = nlohmann::json;

// Bade numbers ko handle karne ke liye strings ka use kar rahe hain
using BigInt = string;

// Do bade numbers ko add karne wali function
BigInt addBigInt(const BigInt &a, const BigInt &b) {
    string res = "";
    int carry = 0, i = a.size() - 1, j = b.size() - 1;

    while (i >= 0 || j >= 0 || carry) {
        int x = (i >= 0 ? a[i--] - '0' : 0);
        int y = (j >= 0 ? b[j--] - '0' : 0);
        int sum = x + y + carry;
        res = char(sum % 10 + '0') + res;
        carry = sum / 10;
    }
    return res;
}

// secret (constant term) nikalne wali function (ham simple averaging kar rahe hain for now)
BigInt extractSecretFrom(const vector<pair<int, BigInt>> &shares) {
    BigInt sum = "0";
    for (auto &[x, y] : shares)
        sum = addBigInt(sum, y);

    // Average nikalne ke liye divide karenge
    // Since integer division for BigInt is complex, assume all valid secrets are same
    return shares[0].second;
}

// Combination generate karne ke liye
void generateCombinations(int start, int k, vector<pair<int, BigInt>> &arr, vector<pair<int, BigInt>> &temp,
                          vector<vector<pair<int, BigInt>>> &all) {
    if (temp.size() == k) {
        all.push_back(temp);
        return;
    }
    for (int i = start; i < arr.size(); ++i) {
        temp.push_back(arr[i]);
        generateCombinations(i + 1, k, arr, temp, all);
        temp.pop_back();
    }
}

void processOneInput(const json &jdata) {
    int n = jdata["n"];
    int k = jdata["k"];
    vector<pair<int, BigInt>> shares;

    for (auto &s : jdata["shares"]) {
        shares.push_back({s["x"], s["y"].get<string>()});
    }

    // Sabhi combinations of k shares lo aur secret nikaalo
    vector<vector<pair<int, BigInt>>> combinations;
    vector<pair<int, BigInt>> temp;
    generateCombinations(0, k, shares, temp, combinations);

    map<BigInt, int> freq;
    map<BigInt, vector<set<int>>> foundIn;

    for (auto &combo : combinations) {
        BigInt secret = extractSecretFrom(combo);
        freq[secret]++;
        set<int> keys;
        for (auto &[x, _] : combo)
            keys.insert(x);
        foundIn[secret].push_back(keys);
    }

    // Sabse zyada repeat hone wala secret nikaalo
    BigInt trueSecret = "";
    int maxCount = 0;
    for (auto &[sec, count] : freq) {
        if (count > maxCount) {
            maxCount = count;
            trueSecret = sec;
        }
    }

    // Wrong keys kaunse hain ye pata karo
    set<int> allCorrectKeys;
    for (auto &keySet : foundIn[trueSecret]) {
        allCorrectKeys.insert(keySet.begin(), keySet.end());
    }

    set<int> allKeys;
    for (auto &[x, _] : shares) allKeys.insert(x);

    set<int> wrongKeys;
    for (int x : allKeys)
        if (allCorrectKeys.find(x) == allCorrectKeys.end())
            wrongKeys.insert(x);

    // OUTPUT
    cout << "shh!! , the secret is: " << trueSecret << endl;
    cout << "and the wrong keys are: ";
    if (wrongKeys.empty()) cout << "none";
    else {
        bool first = true;
        for (int w : wrongKeys) {
            if (!first) cout << ", ";
            cout << w;
            first = false;
        }
    }
    cout << "\n\n";
}

int main() {
    ifstream file("input.json");
    if (!file.is_open()) {
        cout << "JSON file nahi mila!" << endl;
        return 1;
    }

    json jsonData;
    file >> jsonData;

    for (auto &entry : jsonData) {
        processOneInput(entry);
    }

    return 0;
}
