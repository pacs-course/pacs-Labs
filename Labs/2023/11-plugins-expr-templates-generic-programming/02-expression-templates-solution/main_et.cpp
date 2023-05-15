#include <cassert>
#include <chrono>
#include <iostream>
#include <vector>

template <class T, class Container = std::vector<T>>
class MyVector
{
public:
  // MyVector with initial size.
  MyVector(const size_t &n)
    : m_data(n)
  {}

  // MyVector with initial size and value.
  MyVector(const size_t &n, const double &initial_value)
    : m_data(n, initial_value)
  {}

  // Constructor for underlying container.
  MyVector(const Container &other)
    : m_data(other)
  {}

  // Assignment operator for MyVector of different type.
  template <class T2, class V2>
  MyVector &
  operator=(const MyVector<T2, V2> &other)
  {
    assert(size() == other.size());

    for (size_t i = 0; i < m_data.size(); ++i)
      m_data[i] = other[i];

    return (*this);
  }

  size_t
  size() const
  {
    return m_data.size();
  }

  T
  operator[](const size_t &i) const
  {
    return m_data[i];
  }

  T &
  operator[](const size_t &i)
  {
    return m_data[i];
  }

  const Container &
  data() const
  {
    return m_data;
  }

  Container &
  data()
  {
    return m_data;
  }

private:
  Container m_data;
};

// Element-wise sum.
template <class T, class Op1, class Op2>
class MyVectorAdd
{
public:
  MyVectorAdd(const Op1 &a, const Op2 &b)
    : m_op1(a)
    , m_op2(b)
  {
    assert(a.size() == b.size());
  }

  T
  operator[](const size_t &i) const
  {
    return m_op1[i] + m_op2[i];
  }

  size_t
  size() const
  {
    return m_op1.size();
  }

private:
  const Op1 &m_op1;
  const Op2 &m_op2;
};

// Element-wise product.
// One of the drawbacks of this implemenation is that there is
// a lot of boilerplate and repeated code between `Add` and `Mul`
// Actually we could do better, see for instance 
// https://devtut.github.io/cpp/expression-templates.html
template <class T, class Op1, class Op2>
class MyVectorMul
{
public:
  MyVectorMul(const Op1 &a, const Op2 &b)
    : m_op1(a)
    , m_op2(b)
  {
    assert(a.size() == b.size());
  }

  T
  operator[](const size_t &i) const
  {
    return m_op1[i] * m_op2[i];
  }

  size_t
  size() const
  {
    return m_op1.size();
  }

private:
  const Op1 &m_op1;
  const Op2 &m_op2;
};

// Function template for the sum operator.
template <class T, class V1, class V2>
MyVector<T, MyVectorAdd<T, V1, V2>>
operator+(const MyVector<T, V1> &a, const MyVector<T, V2> &b)
{
  return MyVector<T, MyVectorAdd<T, V1, V2>>(
    MyVectorAdd<T, V1, V2>(a.data(), b.data()));
}

// Function template for the multiplication operator.
template <class T, class V1, class V2>
MyVector<T, MyVectorMul<T, V1, V2>>
operator*(const MyVector<T, V1> &a, const MyVector<T, V2> &b)
{
  return MyVector<T, MyVectorMul<T, V1, V2>>(
    MyVectorMul<T, V1, V2>(a.data(), b.data()));
}

// A program that evaluates (x + x + y * y).
int
main(int argc, char **argv)
{
  constexpr size_t N = 1e8;

  MyVector<double> x(N, 5.4);
  MyVector<double> y(N, 10.3);

  // With expression templates.
  {
    auto start = std::chrono::system_clock::now();

    MyVector<double> result(N);
    result = x + x + y * y;

    auto stop = std::chrono::system_clock::now();

    std::cout << "Time elapsed (with expression templates): "
              << std::chrono::duration_cast<std::chrono::milliseconds>(stop -
                                                                       start)
                   .count()
              << "ms" << std::endl;
  }

  // Without expression templates, extra copies are needed...
  {
    auto start = std::chrono::system_clock::now();
    {
      MyVector<double> result(N);

      MyVector<double> tmp1(N);
      MyVector<double> tmp2(N);

      tmp1 = x + x;
      tmp2 = y * y;

      result = tmp1 + tmp2;
    }
    auto stop = std::chrono::system_clock::now();

    std::cout << "Time elapsed (without expression templates): "
              << std::chrono::duration_cast<std::chrono::milliseconds>(stop -
                                                                       start)
                   .count()
              << "ms" << std::endl;
  }

  return 0;
}
