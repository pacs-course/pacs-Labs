void func() {}
int func(int x) { return x; }
double func(const int x, double y) { return y; }

int main() {
  func();
  return 0;
}