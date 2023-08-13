void func() {}
extern "C" { void another_func() {} }

int main() {
  func();
  return 0;
}