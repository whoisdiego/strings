#include <unordered_map>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include "memprof.h"
using namespace std;
using ll = long long;

// lee todo el archivo y lo guarda en una sola string
string getBook(const string& bookname){
  ifstream libro(bookname);
  stringstream ss;
  ss << libro.rdbuf();
  return ss.str();
}

// busqueda binaria sobre el suffix array, regresa el rango inicio y fin
// todos los sufijos que empiezan con target quedan juntos en ese rango
// si inicio es igual a fin, el patron no existe
pair<ll, ll> buscar(const string& libro, const vector<int>& SA, const string& target){
  // en sais el SA tiene una posicion mas que el libro por el simbolo final
  ll n = SA.size(), m = target.size();
  ll inicio, fin;
  ll lw = 0, hi = n;

  // primer sufijo que es mayor o igual que target
  while(lw < hi){
    ll mid = lw + (hi-lw)/2;
    if(libro.compare(SA[mid], m, target) < 0){
      lw = mid + 1;
    } else hi = mid;
  }
  inicio = lw;
  hi = n;

  // primer sufijo que es estrictamente mayor que target
  while(lw < hi){
    ll mid = lw + (hi-lw)/2;
    if(libro.compare(SA[mid], m, target) <= 0){
      lw = mid + 1;
    } else hi = mid;
  }

  fin = lw;

  return make_pair(inicio, fin);
}

// cuenta cuantas veces aparece cada simbolo y calcula donde empieza y termina
// su cubeta en el suffix array, cada simbolo ocupa un rango continuo
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

  // tipo de cada sufijo, S si es menor que el siguiente, L si es mayor
  // el ultimo siempre es S por el simbolo del final
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

  // un LMS es un sufijo S que tiene una L antes
  // se colocan al final de su cubeta sin ordenar bien
  // LMS guarda donde termina la subcadena de cada LMS, o sea el siguiente LMS
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

  // induccion de los L, de izquierda a derecha, se colocan al inicio de su cubeta
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

  // induccion de los S, de derecha a izquierda, se colocan al final de su cubeta
  for (int i = n - 1; i > 0; i--) {
    if (SA[i] > 0) {
      if (t[SA[i] - 1] == 'S') {
        int symbol = T[SA[i] - 1];
        int revoffset = ++freq[symbol];

        SA[buckets[symbol].second - revoffset] = SA[i] - 1;
      }
    }
  }

  // le damos un nombre a cada subcadena LMS en el orden en que quedaron
  // si dos subcadenas son iguales reciben el mismo nombre
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

  // cadena reducida, los nombres en el orden en que aparecen en el texto
  // SApIdx guarda la posicion original de cada LMS
  vector<int> names, SApIdx;

  for (int i = 0; i < n; i++) {
    if (namesp[i] != -1) {
      names.push_back(namesp[i]);
      SApIdx.push_back(i);
    }
  }

  vector<int> reducedSA;

  // si hay nombres repetidos hay que resolver la cadena reducida recursivamente
  // si todos son distintos el orden sale directo de los nombres
  if (name < (int)names.size() - 1) {
    reducedSA = sais(names);
  }
  else {
    reducedSA.assign(names.size(), -1);

    for (int i = 0; i < (int)names.size(); i++) {
      reducedSA[names[i]] = i;
    }
  }

  // con los LMS ya bien ordenados se colocan de nuevo en sus cubetas
  // y se repiten las inducciones para obtener el suffix array final
  fill(SA.begin(), SA.end(), -1);
  freq.clear();

  for (int i = (int)reducedSA.size() - 1; i >= 0; i--) {
    int pos = SApIdx[reducedSA[i]];

    int revoffset = ++freq[T[pos]];
    SA[buckets[T[pos]].second - revoffset] = pos;
  }

  freq.clear();

  // induccion final de los L
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

  // induccion final de los S
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

// convierte la string a vector de enteros y le agrega al final el simbolo que pide sais
// cada byte se sube en uno para que el 0 sea unico y el menor de todos, el orden no cambia
vector<int> aVector(const string& s) {
  vector<int> T;
  T.reserve(s.size() + 1);
  for (char c : s) T.push_back((unsigned char)c + 1);
  T.push_back(0);
  return T;
}

int main() {
    string libro;
    {
      memprof::Etapa e("leer archivo");
      libro = getBook("frank.txt");
    }

    vector<int> T;
    {
      memprof::Etapa e("string a vector");
      T = aVector(libro);
    }

    vector<int> SA;
    {
      memprof::Etapa e("sais");
      SA = sais(T);
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
