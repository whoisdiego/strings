#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include "memprof.h"
using namespace std;
using ll = long long;


string getBook(const string& bookname){
  ifstream libro(bookname);
  stringstream ss;
  ss << libro.rdbuf();
  return ss.str();
}

pair<ll, ll> buscar(const string& libro, const vector<int>& SA, const string& target){
  ll n = libro.size(), m = target.size();
  ll inicio, fin;
  ll lw = 0, hi = n;

  while(lw < hi){
    ll mid = lw + (hi-lw)/2;
    if(libro.compare(SA[mid], m, target) < 0){
      lw = mid + 1;
    } else hi = mid;
  }
  inicio = lw;
  hi = n;

  while(lw < hi){
    ll mid = lw + (hi-lw)/2;
    if(libro.compare(SA[mid], m, target) <= 0){
      lw = mid + 1;
    } else hi = mid;
  }

  fin = lw;

  return make_pair(inicio, fin);
}


class SubstrRank {
  public:
  int left_rank, right_rank, index;
  SubstrRank(int lr, int rr, int idx): left_rank(lr), right_rank(rr), index(idx) {}
};



vector<int> make_ranks(const vector<SubstrRank>& substr_rank, int n){
  int r = 1;
  vector<int> rank;
  rank.assign(n, -1);
  rank[substr_rank[0].index] = r;
  for(int i = 1; i< n; i++){
    if(substr_rank[i].left_rank != substr_rank[i-1].left_rank ||
			substr_rank[i].right_rank != substr_rank[i-1].right_rank) r++;
    rank[substr_rank[i].index] = r;
  }
  return rank;
}

vector<int> suffix_array(const string& s){
  int n = s.size();
  vector<SubstrRank> substr_rank;

  auto comp = [](const SubstrRank& a, const SubstrRank& b){
    if (a.left_rank != b.left_rank) return a.left_rank < b.left_rank;
    return a.right_rank < b.right_rank;
  };

  for(int i = 0; i<n; i++){
    substr_rank.push_back(SubstrRank((unsigned char)s[i], (i + 1 < n ? (unsigned char)s[i+1]: 0), i));
  }

  sort(substr_rank.begin(), substr_rank.end(), comp);

  int l = 2;
  while(l<n){
    vector<int> rank = make_ranks(substr_rank, n);

    for(int i = 0; i<n; i++){
      substr_rank[i].left_rank = rank[i];
      substr_rank[i].right_rank = ( i + l < n  ? rank[i+l] : 0);
      substr_rank[i].index = i;
    }

    l *=2;
    sort(substr_rank.begin(), substr_rank.end(), comp);
  }
  vector<int> SA(n);

  for(int i = 0; i<n; i++){
    SA[i] = substr_rank[i].index;
  }
  return SA;
}


int main(){
  string booksname = "frank.txt";

  string libro;
  {
    memprof::Etapa e("leer archivo");
    libro = getBook(booksname);
  }

  vector<int> SA;
  {
    memprof::Etapa e("suffix_array");
    SA = suffix_array(libro);
  }

  string palabra = "monster";
  {
    memprof::Etapa e("buscar");
    pair<ll, ll> r = buscar(libro, SA, palabra);
    cout << "Apariciones de " << palabra << ": " << r.second - r.first << "\n";
    for (ll i = r.first; i < r.second && i < r.first + 10; i++) cout << SA[i] << " ";
    cout << "\n";
  }

  memprof::resumen();
}