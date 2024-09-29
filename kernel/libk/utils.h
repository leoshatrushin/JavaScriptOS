#pragma once

#define ROUNDUP2N(a, n) (((a) + (n) - 1) & ~((n) - 1))
#define ROUNDDOWN2N(a, n) ((a) & ~((n) - 1))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
