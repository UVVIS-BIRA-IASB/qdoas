/*
Qdoas is a cross-platform application for spectral analysis with the DOAS
algorithm.  Copyright (C) 2007  S[&]T and BIRA

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef _REFCOUNTPTR_H_GUARD
#define _REFCOUNTPTR_H_GUARD


// reference counted pointer
//
// These pointers provide reference counting for objects created on the
// heap. It is intended that  RefCountPtr<T> be used where 'T*' is the
// basic data type, and that RefCountConstPtr<T> be used where 'const T*'
// is the basic data type. This provides the expected semantics that
// const T* can be created and assigned to T*, but not visa-versa.
// Similarly, equality and inequality comparison between const T* and T*
// is supported.
// Note, this allows simultaneous const/non-const reference to the same
// data, just like normal pointers. The RefCountPtr can ensure it has
// sole access to the data be calling the detach method.

template<typename T> class RefCountPtr;
template<typename T> class RefCountConstPtr;

//------------------------------------------------------------

template<typename T>
class RefCountPtr {
 public:
  RefCountPtr();
  RefCountPtr(T *t);
  RefCountPtr(const RefCountPtr<T> &other);
  ~RefCountPtr();

  RefCountPtr<T>& operator=(const RefCountPtr<T> &rhs);

  void detach(void);

  T& operator*(void);
  const T& operator*(void) const;
  T* operator->(void);
  const T* operator->(void) const;

  bool operator==(const RefCountPtr &rhs) const;
  bool operator!=(const RefCountPtr &rhs) const;

  bool operator==(unsigned long rhs) const;
  bool operator!=(unsigned long rhs) const;

  friend class RefCountConstPtr<T>;

  bool operator==(const RefCountConstPtr<T> &rhs) const;
  bool operator!=(const RefCountConstPtr<T> &rhs) const;

 private:
  T *m_d;                     // data
  mutable unsigned int *m_rc; // reference count
};

//------------------------------------------------------------

template<typename T>
class RefCountConstPtr {
 public:
  RefCountConstPtr();
  RefCountConstPtr(const T *t);
  RefCountConstPtr(const RefCountConstPtr<T> &other);
  RefCountConstPtr(const RefCountPtr<T> &other);
  ~RefCountConstPtr();

  RefCountConstPtr<T>& operator=(const RefCountConstPtr<T> &rhs);
  RefCountConstPtr<T>& operator=(const RefCountPtr<T> &rhs);

  const T& operator*(void) const;
  const T* operator->(void) const;

  bool operator==(const RefCountConstPtr<T> &rhs) const;
  bool operator!=(const RefCountConstPtr<T> &rhs) const;

  bool operator==(unsigned long rhs) const;
  bool operator!=(unsigned long rhs) const;

  friend class RefCountPtr<T>;

  bool operator==(const RefCountPtr<T> &rhs) const;
  bool operator!=(const RefCountPtr<T> &rhs) const;

 private:
  const T *m_d;               // data
  mutable unsigned int *m_rc; // reference count
};

//------------------------------------------------------------

template<typename T>
RefCountPtr<T>::RefCountPtr() :
  m_d(NULL),
  m_rc(NULL)
{
}

template<typename T>
RefCountPtr<T>::RefCountPtr(T *t) :
  m_d(NULL),
  m_rc(NULL)
{
  if (t) {
    m_d = t;
    m_rc = new unsigned int;
    *m_rc = 1;
  }
}

template<typename T>
RefCountPtr<T>::RefCountPtr(const RefCountPtr<T> &other) :
  m_d(other.m_d),
  m_rc(other.m_rc)
{  
  // increase the count if the count storage exists
  if (m_rc) ++(*m_rc);
}

template<typename T>
RefCountPtr<T>::~RefCountPtr()
{
  if (m_rc && (--(*m_rc) == 0)) {
    // no more references to the data ... delete
    delete m_d;
    delete m_rc;
  }
}

template<typename T>
RefCountPtr<T>& RefCountPtr<T>::operator=(const RefCountPtr<T> &rhs)
{
  if (m_rc && (--(*m_rc) == 0)) {
    // no more references
    delete m_d;
    delete m_rc;
  }
  m_d = rhs.m_d;   // point to same data
  m_rc = rhs.m_rc; // same ref count storage 
  
  if (m_rc) ++(*m_rc); // increase the reference count

  return *this;
}

template<typename T>
void RefCountPtr<T>::detach(void)
{
  if (m_rc && *m_rc > 1) {
    // shared reference to the data ... cut loose and copy
    T *tmp = new T(*m_d); // copy constructor ...
    --(*m_rc);
    m_rc = new unsigned int;
    *m_rc = 1;
    m_d = tmp;
  }
}

template<typename T>
T& RefCountPtr<T>::operator*(void)
{
  return *m_d;
}

template<typename T>
const T& RefCountPtr<T>::operator*(void) const
{
  return *m_d;
}

template<typename T>
T* RefCountPtr<T>::operator->(void)
{
  return m_d;
}

template<typename T>
const T* RefCountPtr<T>::operator->(void) const
{
  return m_d;
}

template<typename T>
bool RefCountPtr<T>::operator==(const RefCountPtr<T> &rhs) const
{
  return (m_d == rhs.m_d);
}
 
template<typename T>
bool RefCountPtr<T>::operator!=(const RefCountPtr<T> &rhs) const
{
  return (m_d != rhs.m_d);
}

template<typename T>
bool RefCountPtr<T>::operator==(unsigned long rhs) const
{
  return (m_d == (T*)rhs);
}

template<typename T>
bool RefCountPtr<T>::operator!=(unsigned long rhs) const
{
  return (m_d != (T*)rhs);
}

template<typename T>
bool RefCountPtr<T>::operator==(const RefCountConstPtr<T> &rhs) const
{
  return (m_d == rhs.m_d);
}
 
template<typename T>
bool RefCountPtr<T>::operator!=(const RefCountConstPtr<T> &rhs) const
{
  return (m_d != rhs.m_d);
}

//------------------------------------------------------------


template<typename T>
RefCountConstPtr<T>::RefCountConstPtr() :
  m_d(NULL),
  m_rc(NULL)
{
}

template<typename T>
RefCountConstPtr<T>::RefCountConstPtr(const T *t) :
  m_d(NULL),
  m_rc(NULL)
{
  if (t) {
    m_d = t;
    m_rc = new unsigned int;
    *m_rc = 1;
  }
}

template<typename T>
RefCountConstPtr<T>::RefCountConstPtr(const RefCountConstPtr<T> &other) :
  m_d(other.m_d),
  m_rc(other.m_rc)
{  
  // increase the count if the count storage exists
  if (m_rc) ++(*m_rc);
}

template<typename T>
RefCountConstPtr<T>::RefCountConstPtr(const RefCountPtr<T> &other) :
  m_d(other.m_d),
  m_rc(other.m_rc)
{  
  // increase the count if the count storage exists
  if (m_rc) ++(*m_rc);
}

template<typename T>
RefCountConstPtr<T>::~RefCountConstPtr()
{
  if (m_rc && (--(*m_rc) == 0)) {
    // no more references to the data ... delete
    delete m_d;
    delete m_rc;
  }
}

template<typename T>
RefCountConstPtr<T>& RefCountConstPtr<T>::operator=(const RefCountConstPtr<T> &rhs)
{
  if (m_rc && (--(*m_rc) == 0)) {
    // no more references
    delete m_d;
    delete m_rc;
  }
  m_d = rhs.m_d;   // point to same data
  m_rc = rhs.m_rc; // same ref count storage 
  
  if (m_rc) ++(*m_rc); // increase the reference count

  return *this;
}

template<typename T>
RefCountConstPtr<T>& RefCountConstPtr<T>::operator=(const RefCountPtr<T> &rhs)
{
  if (m_rc && (--(*m_rc) == 0)) {
    // no more references
    delete m_d;
    delete m_rc;
  }
  m_d = rhs.m_d;   // point to same data
  m_rc = rhs.m_rc; // same ref count storage 
  
  if (m_rc) ++(*m_rc); // increase the reference count

  return *this;
}

template<typename T>
const T& RefCountConstPtr<T>::operator*(void) const
{
  return *m_d;
}

template<typename T>
const T* RefCountConstPtr<T>::operator->(void) const
{
  return m_d;
}

template<typename T>
bool RefCountConstPtr<T>::operator==(const RefCountConstPtr<T> &rhs) const
{
  return (m_d == rhs.m_d);
}
 
template<typename T>
bool RefCountConstPtr<T>::operator!=(const RefCountConstPtr<T> &rhs) const
{
  return (m_d != rhs.m_d);
}

template<typename T>
bool RefCountConstPtr<T>::operator==(unsigned long rhs) const
{
  return (m_d == (const T*)rhs);
}

template<typename T>
bool RefCountConstPtr<T>::operator!=(unsigned long rhs) const
{
  return (m_d != (const T*)rhs);
}

template<typename T>
bool RefCountConstPtr<T>::operator==(const RefCountPtr<T> &rhs) const
{
  return (m_d == rhs.m_d);
}
 
template<typename T>
bool RefCountConstPtr<T>::operator!=(const RefCountPtr<T> &rhs) const
{
  return (m_d != rhs.m_d);
}

//------------------------------------------------------------

#endif
