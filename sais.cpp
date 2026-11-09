
#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
using namespace std;

unordered_map<int, pair<int, int>> getBuckets(const vector<int>& T) {
  map<int, int> freq;
  unordered_map<int, pair<int, int>> buckets;

  for (int c : T) freq[c]++;

  int start = 0;
  for (auto it = freq.begin(); it != freq.end(); ++it) {
    int c = (*it).first;
    buckets[c] = make_pair(start, start + freq[c]);
    start += freq[c];
  }

  return buckets;
}

vector<int> sais(const vector<int>& T) {
  int n = T.size();

  if (n == 0) return {};
  if (n == 1) return {0};

  vector<char> t(n, '_');
  t[n - 1] = 'S';

  for (int i = n - 1; i > 0; i--) {
    if (T[i - 1] == T[i]) t[i - 1] = t[i];
    else t[i - 1] = (T[i - 1] < T[i] ? 'S' : 'L');
  }

  unordered_map<int, pair<int, int>> buckets = getBuckets(T);

  unordered_map<int, int> freq, LMS;
  vector<int> SA(n, -1);
  int end = -1;

  for (int i = n - 1; i > 0; i--) {
    if (t[i] == 'S' && t[i - 1] == 'L') {
      int revoffset = ++freq[T[i]];
      SA[buckets[T[i]].second - revoffset] = i;

      if (end != -1) LMS[i] = end;
      end = i;
    }
  }
  LMS[n - 1] = n - 1;
  freq.clear();

  for (int i = 0; i < n; i++) {
    if (SA[i] >= 0 && SA[i] > 0) {
      if (t[SA[i] - 1] == 'L') {
        int symbol = T[SA[i] - 1];
        int offset = freq[symbol];

        SA[buckets[symbol].first + offset] = SA[i] - 1;
        freq[symbol] = offset + 1;
      }
    }
  }

  freq.clear();

  for (int i = n - 1; i > 0; i--) {
    if (SA[i] > 0) {
      if (t[SA[i] - 1] == 'S') {
        int symbol = T[SA[i] - 1];
        int revoffset = ++freq[symbol];

        SA[buckets[symbol].second - revoffset] = SA[i] - 1;
      }
    }
  }

  vector<int> namesp(n, -1);
  int name = 0;
  int prev = -1;

  for (int i = 0; i < n; i++) {
    if (SA[i] > 0 && t[SA[i]] == 'S' && t[SA[i] - 1] == 'L') {
      if (prev != -1) {
        int a = SA[prev];
        int b = SA[i];

        int lenA = LMS[a] - a;
        int lenB = LMS[b] - b;

        if (lenA != lenB || !equal(T.begin() + a, T.begin() + a + lenA, T.begin() + b)) name++;
      }
      prev = i;
      namesp[SA[i]] = name;
    }
  }

  vector<int> names, SApIdx;

  for (int i = 0; i < n; i++) {
    if (namesp[i] != -1) {
      names.push_back(namesp[i]);
      SApIdx.push_back(i);
    }
  }

  vector<int> reducedSA;

  if (name < (int)names.size() - 1) {
    reducedSA = sais(names);
  }
  else {
    reducedSA.assign(names.size(), -1);

    for (int i = 0; i < (int)names.size(); i++) {
      reducedSA[names[i]] = i;
    }
  }

  fill(SA.begin(), SA.end(), -1);
  freq.clear();

  for (int i = (int)reducedSA.size() - 1; i >= 0; i--) {
    int pos = SApIdx[reducedSA[i]];

    int revoffset = ++freq[T[pos]];
    SA[buckets[T[pos]].second - revoffset] = pos;
  }

  freq.clear();

  for (int i = 0; i < n; i++) {
    if (SA[i] >= 0 && SA[i] > 0) {
      if (t[SA[i] - 1] == 'L') {
        int symbol = T[SA[i] - 1];
        int offset = freq[symbol];

        SA[buckets[symbol].first + offset] = SA[i] - 1;
        freq[symbol] = offset + 1;
      }
    }
  }

  freq.clear();

  for (int i = n - 1; i > 0; i--) {
    if (SA[i] > 0) {
      if (t[SA[i] - 1] == 'S') {
        int symbol = T[SA[i] - 1];
        int revoffset = ++freq[symbol];

        SA[buckets[symbol].second - revoffset] = SA[i] - 1;
      }
    }
  }
  return SA;
}



int main() {
    string s = "GTCCCGATGTCATGTCAGGA$";
    vector<int> T;
    for (char c : s) {
      T.push_back((int)c);
    }

    vector<int> SA = sais(T);

    for (int x : SA) cout << x << " ";
    cout << '\n';
}
