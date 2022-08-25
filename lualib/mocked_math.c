#include "math.h"
#include <stdint.h>

double acos(double x) { return 0; }

double asin(double x) { return 0; }

double atan2(double x, double y) { return 0; }

double ceil(double x) { return 0; }

double cos(double x) { return 0; }

double cosh(double x) { return 0; }

double exp(double x) { return 0; }

double fabs(double x) {
  union {
    double f;
    uint64_t i;
  } u = {x};
  u.i &= -1ULL / 2;
  return u.f;
}

double log(double x) { return 0; }

double log2(double x) { return 0; }

double log10(double x) { return 0; }

double pow(double x, double y) { return 0; }

double sin(double x) { return 0; }

double sinh(double x) { return 0; }

double sqrt(double x) { return 0; }

double tan(double x) { return 0; }

double tanh(double x) { return 0; }
