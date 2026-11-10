#pragma once
// perfilador de memoria sencillo, incluir en un solo archivo cpp
// mide el heap de c++ con un operator new propio y el pico real del proceso con el sistema operativo
#include <cstdio>
#include <cstdlib>
#include <cstddef>
#include <new>
#include <chrono>

#if defined(_WIN32)
  #ifndef NOMINMAX
  #define NOMINMAX
  #endif
  #include <windows.h>
  #include <psapi.h>
  #if defined(_MSC_VER)
    #pragma comment(lib, "psapi.lib")
  #endif
#else
  #include <sys/resource.h>
#endif

namespace memprof {

// actual es lo que esta reservado ahora, pico es el maximo alcanzado
inline size_t actual = 0;
inline size_t pico = 0;
inline size_t total = 0;
inline size_t cantidad = 0;
inline const auto inicio = std::chrono::steady_clock::now();

inline double mb(size_t b) { return b / (1024.0 * 1024.0); }

inline void sumar(size_t n) {
  actual += n;
  total += n;
  cantidad++;
  if (actual > pico) pico = actual;
}

inline void restar(size_t n) { actual -= n; }

// pico de memoria fisica que el sistema operativo le dio al proceso
inline size_t pico_rss() {
#if defined(_WIN32)
  PROCESS_MEMORY_COUNTERS pmc;
  if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) return pmc.PeakWorkingSetSize;
  return 0;
#elif defined(__APPLE__)
  rusage r;
  getrusage(RUSAGE_SELF, &r);
  return (size_t)r.ru_maxrss;
#else
  rusage r;
  getrusage(RUSAGE_SELF, &r);
  return (size_t)r.ru_maxrss * 1024;
#endif
}

// mide una etapa, al crearse guarda el estado y al destruirse imprime el resultado
// reinicia el pico al entrar para saber el pico propio de la etapa
struct Etapa {
  const char* nombre;
  size_t base, pico_previo;
  std::chrono::steady_clock::time_point t0;

  Etapa(const char* n)
      : nombre(n), base(actual), pico_previo(pico), t0(std::chrono::steady_clock::now()) {
    pico = actual;
  }

  ~Etapa() {
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    std::fprintf(stderr,
                 "[etapa] %-22s %9.1f ms | heap final %8.2f MB | pico heap en etapa %8.2f MB | pico RSS %8.2f MB\n",
                 nombre, ms, mb(actual), mb(pico), mb(pico_rss()));
    // el pico global debe conservar el maximo de todo
    if (pico_previo > pico) pico = pico_previo;
  }
};

// linea suelta para ver el estado en un punto, por ejemplo dentro de una recursion
inline void punto(const char* nombre, int dato, long long n) {
  std::fprintf(stderr, "[punto] %-12s %3d | n %9lld | heap actual %8.2f MB | pico heap %8.2f MB\n",
               nombre, dato, n, mb(actual), mb(pico));
}

inline void resumen() {
  double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio).count();
  std::fprintf(stderr, "\n[resumen]\n");
  std::fprintf(stderr, "  tiempo total              %.2f s\n", s);
  std::fprintf(stderr, "  pico heap c++             %.2f MB\n", mb(pico));
  std::fprintf(stderr, "  pico RSS del proceso      %.2f MB\n", mb(pico_rss()));
  std::fprintf(stderr, "  total pedido acumulado    %.2f MB en %zu reservas\n", mb(total), cantidad);
  std::fprintf(stderr, "  heap sin liberar          %.2f MB\n", mb(actual));
}

}  // namespace memprof

// reemplazo global de new y delete, guarda el tamano en una cabecera de 16 bytes
// estas funciones no pueden ser inline, por eso este archivo se incluye una sola vez
void* operator new(std::size_t n) {
  void* p = std::malloc(n + 16);
  if (!p) throw std::bad_alloc();
  *(std::size_t*)p = n;
  memprof::sumar(n);
  return (char*)p + 16;
}

void operator delete(void* p) noexcept {
  if (!p) return;
  char* base = (char*)p - 16;
  memprof::restar(*(std::size_t*)base);
  std::free(base);
}

void operator delete(void* p, std::size_t) noexcept { operator delete(p); }