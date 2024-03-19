#include <stdio.h>
#include <assert.h>

/* t1.c */
int f(int a, int b) {
 return a + b;
}

int main() {
  int a,b;
  int r = f(a,b);
  assert (r==1);
  return 0;
}