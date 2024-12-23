#define _POSIX_C_SOURCE 200809L

#include <janet.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

static int compare(const void *a, const void *b) {
  double diff = *(double *) a - *(double *) b;
  return (diff > 0) - (diff < 0);
}

static double quantile(double *data, int32_t n, double p) {
  if (n == 1)
    return data[0];

  qsort(data, n, sizeof(double), compare);
  double position = p * (n - 1);
  size_t index = (size_t) position;

  if (index + 1 < n)
    return data[index] + (position - index) * (data[index + 1] - data[index]);
  else
    return data[index];
}

static int64_t time_function(JanetFunction *f) {
  struct timespec start, stop;

  clock_gettime(CLOCK_MONOTONIC, &start);
  janet_call(f, 0, NULL);
  clock_gettime(CLOCK_MONOTONIC, &stop);

  return (stop.tv_sec - start.tv_sec) * 1e9 + (stop.tv_nsec - start.tv_nsec);
}

static Janet do_timing(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 4);

  JanetFunction *f = janet_getfunction(argv, 0),
                *empty_fn = janet_getfunction(argv, 1);
  int32_t times = janet_getinteger(argv, 2),
          warmup = janet_getinteger(argv, 3);

  JanetArray *results = janet_array(2);

  // We use a multi-pass algorithm to first calculate the mean using
  // Neumaier summation followed by the IQR and the sample
  // variance.
  double *diffs = janet_smalloc(times * sizeof(double));

  double sum = 0.0, k = 0.0;
  for (int i = 0; i < times + warmup; i++) {
    double d = (double) (time_function(f) - time_function(empty_fn));
    if (i < warmup)
      continue;

    double tmp = sum + d;
    k += (fabs(sum) >= fabs(d)) ? (sum - tmp) + d : (d - tmp) + sum;

    sum = tmp;
    diffs[i - warmup] = d;
  }

  double mean = (sum + k) / (double) times;
  janet_array_push(results, janet_wrap_number(mean));

  // Calculate IQR, 0.25 - 0.75 quartiles
  janet_array_push(results, janet_wrap_number(quantile(diffs, times, 0.25)));
  janet_array_push(results, janet_wrap_number(quantile(diffs, times, 0.75)));

  // For only a single sample leave the standard deviation undefined
  if (times == 1) {
    janet_array_push(results, janet_wrap_number(INFINITY));
    goto finish;
  }

  // Sample variance with Bessel's correction
  sum = k = 0.0;
  for (int i = 0; i < times; i++) {
    double sq_deviation = pow(diffs[i] - mean, 2);

    double tmp = sum + sq_deviation;
    k += (fabs(sum) >= fabs(sq_deviation)) ? (sum - tmp) + sq_deviation
         : (sq_deviation - tmp) + sum;

    sum = tmp;
  }

  double stddev = sqrt((sum + k) / (double) (times - 1));
  janet_array_push(results, janet_wrap_number(stddev));

  goto finish;

 finish:
  janet_sfree(diffs);
  return janet_wrap_array(results);
}

static const JanetReg cfuns[] = {
  {"nstime-function", do_timing, "Internal function to benchmark two thunks with clock_gettime"},
  {NULL, NULL, NULL}
};

JANET_MODULE_ENTRY(JanetTable *env) {
  janet_cfuns(env, "timing", cfuns);
}
