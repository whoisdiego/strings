#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
using namespace std;


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

vector<int> suffix_array(string s){
  int n = s.size();
  vector<SubstrRank> substr_rank;

  auto comp = [](const SubstrRank& a, const SubstrRank& b){
    if (a.left_rank != b.left_rank) return a.left_rank < b.left_rank;
    return a.right_rank < b.right_rank;
  };

  for(int i = 0; i<n; i++){
    substr_rank.push_back(SubstrRank((int)s[i], (i + 1 == n ? (int)s[i+1]: 0), i));
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
  vector<int> SA = suffix_array("mississippi");
  for(int x: SA) cout<< x<< " ";
}





