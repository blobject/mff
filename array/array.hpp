// file: array/array.hpp
// by  : jooh@cuni.cz
// for : nprg041
// lic.: mit

////////////////////////////////////////////////////////////////////////
// preproc

#include <cassert>
#include <iostream>

#define YOU_HAVE_IMPLEMENTED_ITERATORS 1
#define ACTIVATE_THIS_FOR_SEEING_THE_CORRECT_RESULT 0

#define DEBUG_ON 0
#define DEFAULT_MAGNITUDE 32
#define NULT (size_t)NULL

////////////////////////////////////////////////////////////////////////
// declaration

template <typename T>
class Array
{

public:
  // constructors
  Array();
  Array(size_t n);
  Array(size_t n, const T& v);

  // operators
  T operator[](size_t i) const;
  T& operator[](size_t i);

  // accessors
  size_t size() const;
  size_t capacity() const;
  bool empty() const;
  T back() const;
  T front() const;
  T& back();
  T& front();

  // mutators
  void reserve(size_t n);
  void resize(size_t n);
  void push_back(const T& v);
  void pop_back();
  void clear();
  void swap(Array<T>& a);
  void append(Array<T>& a);

  // bonus
  ~Array();
  Array(const Array<T>& a);
  Array(Array<T>&& a);
  Array<T>& operator=(const Array<T>&);
  Array<T>& operator=(Array<T>&& a);
  T const* begin() const;
  T const* end() const;
  T* begin();
  T* end();

private:
  T* array_;         // head of actual array
  size_t capacity_;  // declared but not assigned
  size_t size_;      // assigned
};

////////////////////////////////////////////////////////////////////////
// debug

template<typename T>
void
dbg(std::string msg, size_t oc, size_t nc, size_t os, size_t ns, T a)
{
  std::cout << msg << ": ";
  // capacity change
  if (oc != NULT || nc != NULT) std::cout << "c ";
  if (oc != NULT)               std::cout << oc;
  if (oc != NULT && nc != NULT) std::cout << "->";
  if (nc != NULT)               std::cout << nc;
  if (oc != NULT || nc != NULT) std::cout << "; ";
  // size change
  if (os != NULT || ns != NULT) std::cout << "s ";
  if (os != NULT)               std::cout << os;
  if (os != NULT && ns != NULT) std::cout << "->";
  if (ns != NULT)               std::cout << ns;
  if (os != NULT || ns != NULT) std::cout << "; ";
  // array address
  if (a)                        std::cout << "a " << a;
  std::cout << std::endl;
}

////////////////////////////////////////////////////////////////////////
// Array constructors

template<typename T>
Array<T>::Array()
{
  array_ = new T[DEFAULT_MAGNITUDE];
  capacity_ = DEFAULT_MAGNITUDE;
  size_ = 0;

#if DEBUG_ON
  dbg("Array()", NULT, capacity_, NULT, size_, array_);
#endif
}

template<typename T>
Array<T>::Array(size_t n)
{
  assert(n >= 0);

  array_ = new T[n];
  size_ = n;

  size_t c = 1;
  while (c < n)
    c *= 2;
  capacity_ = c;

  for (int i = 0; i < n; i++)
    array_[i] = T {};

#if DEBUG_ON
  std::ostringstream s;
  s << "Array(" << n << ")";
  dbg(s.str(), NULT, capacity_, NULT, size_, array_);
#endif
}

template<typename T>
Array<T>::Array(size_t n, const T& v)
{
  assert(n >= 0);

  array_ = new T[n];
  size_ = n;

  size_t c = 1;
  while (c < n)
    c *= 2;
  capacity_ = c;

  for (int i = 0; i < n; i++)
    array_[i] = v;

#if DEBUG_ON
  std::ostringstream s;
  s << "Array(" << n << ", v)";
  dbg(s.str(), NULT, capacity_, NULT, size_, array_);
#endif
}

////////////////////////////////////////////////////////////////////////
// Array operators

template<typename T>
T
Array<T>::operator[](size_t i) const
{
  assert((i >= 0) && (i < size_));

  return array_[i];
}

template<typename T>
T&
Array<T>::operator[](size_t i)
{
  assert((i >= 0) && (i < size_));

  return array_[i];
}

////////////////////////////////////////////////////////////////////////
// Array accessors

template<typename T>
size_t
Array<T>::size() const
{
  return size_;
}

template<typename T>
size_t
Array<T>::capacity() const
{
  return capacity_;
}

template<typename T>
bool Array<T>::empty() const
{
  return size_ == 0;
}

template<typename T>
T
Array<T>::back() const
{
  return array_[size_ - 1];
}

template<typename T>
T
Array<T>::front() const
{
  return array_[0];
}

template<typename T>
T&
Array<T>::back()
{
  return array_[size_ - 1];
}

template<typename T>
T&
Array<T>::front()
{
  return array_[0];
}

////////////////////////////////////////////////////////////////////////
// Array mutators

template<typename T>
void
Array<T>::reserve(size_t n)
{
  assert(n >= 0);

  if (n <= capacity_) {
#if DEBUG_ON
    std::cout << "forgo reserve()" << std::endl;
#endif
    return;
  }

#if DEBUG_ON
  size_t oc = capacity_;
#endif

  size_t c = 1;
  while (c < n)
    c *= 2;
  capacity_ = c;

  T* a = new T[capacity_];

  for (int i = 0; i < size_; i++)
    a[i] = array_[i];
  if (array_)
    delete[] array_;

  array_ = a;

#if DEBUG_ON
  std::ostringstream s;
  s << "reserve(" << n << ")";
  dbg(s.str(), oc, capacity_, NULT, size_, array_);
#endif
}

template<typename T>
void
Array<T>::resize(size_t n)
{
  assert(n >= 0);

#if DEBUG_ON
  size_t oc = capacity_;
  size_t os = size_;
#endif
  T* a = new T[n];

  if (n > capacity_)
    reserve(n);
  for (int i = 0; i < std::min(size_, n); i++)
    a[i] = array_[i];
  for (int i = size_; i < n; i++)
    a[i] = T {};
  if (array_)
    delete[] array_;

  array_ = a;
  size_ = n;

#if DEBUG_ON
  std::ostringstream s;
  s << "resize(" << n << ")";
  dbg(s.str(), oc, capacity_, os, size_, array_);
#endif
}

template<typename T>
void
Array<T>::push_back(const T& v)
{
#if DEBUG_ON
  size_t oc = capacity_;
  size_t os = size_;
#endif

  if (size_ >= capacity_)
    reserve(size_ + 1);

  array_[size_] = v;
  ++size_;

#if DEBUG_ON
  dbg("push_back(v)", oc, capacity_, os, size_, array_);
#endif
}

template<typename T>
void
Array<T>::pop_back()
{
#if DEBUG_ON
  size_t os = size_;
#endif

  --size_;

#if DEBUG_ON
  dbg("pop_back()", NULT, capacity_, os, size_, array_);
#endif
}

template<typename T>
void
Array<T>::clear()
{
  if (array_)
    delete[] array_;
  array_ = new T[size_];
  for (int i = 0; i < size_; i++)
    array_[i] = T {};

#if DEBUG_ON
  dbg("clear()", NULT, capacity_, NULT, size_, array_);
#endif
}

template<typename T>
void
Array<T>::swap(Array<T>& a)
{
  T* tmpa = a.array_;
  size_t tmpc = a.capacity_;
  size_t tmps = a.size_;
  a.array_ = array_;
  a.capacity_ = capacity_;
  a.size_ = size_;
  array_ = tmpa;
  capacity_ = tmpc;
  size_ = tmps;

#if DEBUG_ON
  dbg("swap(a)", a.capacity_, capacity_, a.size_, size_, &a);
#endif
}

template<typename T>
void
Array<T>::append(Array<T>& a)
{
#if DEBUG_ON
  size_t oc = capacity_;
  size_t os = size_;
#endif

  for (int i = 0; i < a.size_; i++) {
    push_back(a[i]);
  }

#if DEBUG_ON
  dbg("append(a)", oc, capacity_, os, size_, array_);
#endif
}

////////////////////////////////////////////////////////////////////////
// Array: bonus

//// rule of five

// destructor
template<typename T>
Array<T>::~Array()
{
  if (array_)
    delete[] array_;

#if DEBUG_ON
  dbg("~Array()", NULT, NULT, NULT, NULT, array_);
#endif
}

// copy constructor
template<typename T>
Array<T>::Array(const Array<T>& a)
{
  capacity_ = a.capacity_;
  size_ = a.size_;
  array_ = new T[size_];
  for (int i = 0; i < size_; i++) {
    if (!a.array_[i])
      array_[i] = T {};
    else
      array_[i] = a.array_[i];
  }

#if DEBUG_ON
  dbg("Array(cp)(a)", NULT, capacity_, NULT, size_, array_);
#endif
}

// move constructor
template<typename T>
Array<T>::Array(Array<T>&& a)
{
  capacity_ = a.capacity_;
  size_ = a.size_;
  array_ = a.array_;
  std::__exchange(a.array_, nullptr);

#if DEBUG_ON
  dbg("Array(mv)(a)", oc, capacity_, os, size_, array_);
#endif
}

// copy assignment
template<typename T>
Array<T>&
Array<T>::operator=(const Array<T>& a)
{
#if DEBUG_ON
  size_t oc = capacity_;
  size_t os = size_;
#endif

  Array<T> tmp(a);
  tmp.swap(*this);
  return *this;

#if DEBUG_ON
  dbg("operator=(cp)(a)", oc, capacity_, os, size_, array_);
#endif
}

// move assignment
template<typename T>
Array<T>&
Array<T>::operator=(Array<T>&& a)
{
#if DEBUG_ON
  size_t oc = capacity_;
  size_t os = size_;
#endif

  std::swap(capacity_, a.capacity_);
  std::swap(size_, a.size_);
  std::swap(array_, a.array_);
  return *this;

#if DEBUG_ON
  dbg("operator=(mv)(a)", oc, capacity_, os, size_, array_);
#endif
}

//// iterators

template<typename T>
T const*
Array<T>::begin() const {
  return array_;
}

template<typename T>
T const*
Array<T>::end() const {
  return array_ + size_;
}

template<typename T>
T*
Array<T>::begin() {
  return array_;
}

template<typename T>
T*
Array<T>::end() {
  return array_ + size_;
}

