#include <bits/stdc++.h>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

// ---- Minimal BigInt class ----

struct BigInt {
    static const int BASE = 1000000000;
    static const int BASE_DIGITS = 9;

    vector<int> a; // little endian, each element < BASE
    bool negative;

    BigInt(): negative(false) {}
    BigInt(long long v) { *this = v; }
    BigInt(const string &s) { read(s); }

    void operator=(long long v) {
        negative = v < 0;
        if(negative) v = -v;
        a.clear();
        while(v > 0) {
            a.push_back(v % BASE);
            v /= BASE;
        }
        trim();
    }

    void read(const string &s) {
        negative = false;
        a.clear();
        int pos = 0;
        while(pos < (int)s.size() && (s[pos] == '-' || s[pos] == '+')) {
            if(s[pos] == '-') negative = !negative;
            pos++;
        }
        for(int i = (int)s.size()-1; i >= pos; i -= BASE_DIGITS) {
            int x = 0;
            int start = max(pos, i - BASE_DIGITS + 1);
            for(int j = start; j <= i; j++)
                x = x*10 + (s[j]-'0');
            a.push_back(x);
        }
        trim();
    }

    void trim() {
        while(!a.empty() && a.back() == 0)
            a.pop_back();
        if(a.empty())
            negative = false;
    }

    void print() const {
        if(negative && !isZero()) cout << '-';
        if(a.empty()) {
            cout << 0;
            return;
        }
        cout << a.back();
        for(int i=(int)a.size()-2; i>=0; i--)
            cout << setw(BASE_DIGITS) << setfill('0') << a[i];
    }

    bool isZero() const {
        return a.empty();
    }

    bool operator<(const BigInt &b) const {
        if(negative != b.negative)
            return negative;
        if(a.size() != b.a.size())
            return a.size() * (negative ? -1 : 1) < b.a.size() * (negative ? -1 : 1);
        for(int i = (int)a.size() - 1; i >= 0; i--) {
            if(a[i] != b.a[i])
                return a[i] * (negative ? -1 : 1) < b.a[i] * (negative ? -1 : 1);
        }
        return false;
    }

    bool operator==(const BigInt &b) const {
        return negative == b.negative && a == b.a;
    }

    bool operator>(const BigInt &b) const {
        return b < *this;
    }
    bool operator<=(const BigInt &b) const {
        return !(b < *this);
    }
    bool operator>=(const BigInt &b) const {
        return !(*this < b);
    }

    BigInt operator-() const {
        BigInt res = *this;
        if(!res.isZero())
            res.negative = !res.negative;
        return res;
    }

    BigInt abs() const {
        BigInt res = *this;
        res.negative = false;
        return res;
    }

    BigInt operator+(const BigInt &b) const {
        if(negative == b.negative) {
            BigInt res = add_abs(*this, b);
            res.negative = negative;
            return res;
        }
        if(abs() >= b.abs())
            return sub_abs(*this, b);
        else
            return sub_abs(b, *this);
    }

    BigInt operator-(const BigInt &b) const {
        return *this + (-b);
    }

    BigInt operator*(const BigInt &b) const {
        BigInt res;
        res.negative = negative != b.negative;
        res.a.assign(a.size() + b.a.size(), 0);
        for(size_t i = 0; i < a.size(); i++) {
            long long carry = 0;
            for(size_t j = 0; j < b.a.size() || carry; j++) {
                long long cur = res.a[i + j] + (long long)a[i] * (j < b.a.size() ? b.a[j] : 0) + carry;
                res.a[i + j] = int(cur % BASE);
                carry = cur / BASE;
            }
        }
        res.trim();
        return res;
    }

    // Division by int (long long fits inside int for denominators)
    pair<BigInt,int> divmod(int v) const {
        long long rem = 0;
        BigInt res;
        res.a.resize(a.size());
        for(int i = (int)a.size() - 1; i >= 0; i--) {
            long long cur = a[i] + rem * (long long)BASE;
            res.a[i] = (int)(cur / v);
            rem = cur % v;
        }
        res.negative = negative;
        res.trim();
        return {res, (int)rem};
    }

    BigInt operator/(int v) const {
        return divmod(v).first;
    }

    int operator%(int v) const {
        return divmod(v).second;
    }

private:
    static BigInt add_abs(const BigInt &a, const BigInt &b) {
        BigInt res;
        int carry = 0;
        int n = max((int)a.a.size(), (int)b.a.size());
        for(int i = 0; i < n || carry; i++) {
            int cur = carry;
            if(i < (int)a.a.size()) cur += a.a[i];
            if(i < (int)b.a.size()) cur += b.a[i];
            if(cur >= BASE) {
                carry = 1;
                cur -= BASE;
            } else carry = 0;
            res.a.push_back(cur);
        }
        return res;
    }

    static BigInt sub_abs(const BigInt &a, const BigInt &b) {
        // assume a >= b (abs)
        BigInt res;
        int carry = 0;
        for(int i = 0; i < (int)a.a.size(); i++) {
            int cur = a.a[i] - carry - (i < (int)b.a.size() ? b.a[i] : 0);
            if(cur < 0) {
                cur += BASE;
                carry = 1;
            } else carry = 0;
            res.a.push_back(cur);
        }
        res.trim();
        res.negative = a.negative;
        return res;
    }
};

// Decode a string with given base into BigInt
BigInt decode(const string& value, int base) {
    static const string digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    BigInt res = 0;
    for(char ch : value) {
        int digit = (int)digits.find(tolower(ch));
        if(digit < 0 || digit >= base) {
            cerr << "Invalid digit '" << ch << "' for base " << base << endl;
            exit(1);
        }
        res = res * base + digit;
    }
    return res;
}

// Integer-only Lagrange interpolation at x=0
BigInt lagrange_interpolate_at_zero(const vector<pair<BigInt, BigInt>> &points) {
    int k = (int)points.size();
    BigInt secret = 0;
    for(int j=0; j<k; j++) {
        BigInt numerator = points[j].second; // y_j
        BigInt denominator = 1;
        for(int m=0; m<k; m++) {
            if(m == j) continue;
            numerator = numerator * (-points[m].first);
            denominator = denominator * (points[j].first - points[m].first);
        }

        // denominator should fit in long (small integers from differences)
        long long denom64 = 1;
        bool denom_neg = false;
        for(int m=0; m<k; m++) {
            if(m == j) continue;
            BigInt diff = points[j].first - points[m].first;
            if(diff.negative) {
                denom_neg = !denom_neg;
                diff = diff.abs();
            }
            // Extract integer from BigInt (should fit in 64-bit)
            long long val = 0;
            for(int z = (int)diff.a.size()-1; z >= 0; z--) {
                val = val * BigInt::BASE + diff.a[z];
            }
            denom64 *= val;
        }
        if(denom64 == 0) {
            cerr << "Zero denominator!" << endl;
            exit(1);
        }
        if(denom_neg) denom64 = -denom64;

        auto divmod = numerator.divmod((int)(denom64));
        if(divmod.second != 0) {
            cerr << "Non exact division encountered in Lagrange interpolation!" << endl;
            exit(1);
        }
        secret = secret + divmod.first;
    }
    return secret;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<string> files = {"testcase1.json", "testcase2.json"};

    for(const auto &filename : files) {
        ifstream fin(filename);
        if(!fin.is_open()) {
            cerr << "Failed to open " << filename << endl;
            return 1;
        }
        json j;
        fin >> j;

        int n = j["keys"]["n"];
        int k = j["keys"]["k"];

        vector<pair<BigInt, BigInt>> points;

        vector<int> keys_available;
        for (auto it = j.begin(); it != j.end(); ++it) {
            if(it.key() != "keys") {
                keys_available.push_back(stoi(it.key()));
            }
        }
        sort(keys_available.begin(), keys_available.end());

        int count = 0;
        for(auto x : keys_available) {
            if(count >= k) break;
            auto root = j[to_string(x)];
            int base = stoi(root["base"].get<string>());
            string val = root["value"];
            BigInt yval = decode(val, base);
            points.push_back({BigInt(x), yval});
            count++;
        }

        if((int)points.size() != k) {
            cerr << "Insufficient points collected from JSON\n";
            return 1;
        }

        BigInt secret = lagrange_interpolate_at_zero(points);
        secret.print();
        cout << "\n";
    }

    return 0;
}
