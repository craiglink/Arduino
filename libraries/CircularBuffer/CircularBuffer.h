#ifndef CIRCULARBUFFER_h
#define CIRCULARBUFFER_h

#include <util/atomic.h>

#ifdef CIRCULARBUFFER_EMPTY_CHECK
#define CIRCULARBUFFER_NOT_EMPTY !empty()
#else
#define CIRCULARBUFFER_NOT_EMPTY true
#endif

template <class T, class SizeType, uint16_t Size>
class CircularBuffer {
public:
  CircularBuffer() :
    _back(0), _front(0),  _count(0) {}
  ~CircularBuffer() {}

  bool push(const T& value) {
    if ( _count < Size )
    {
        _buf[_back] = value;
        _back = (_back+1)%Size;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            _count++;
        }
        return true;
    }
    else
    {
        return false;
    }
  }

  inline void pop() {
    if ( CIRCULARBUFFER_NOT_EMPTY )
    {
        _front = (_front+1)%Size;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            _count--;
        }
    }
  }

  inline const T& front() {
    if ( CIRCULARBUFFER_NOT_EMPTY )
    {
        return _buf[_front];
    }
  }

  inline const T& back() {
    if ( CIRCULARBUFFER_NOT_EMPTY )
    {
        return _buf[_back];
    }
  }


  inline volatile bool empty() const {
    return _count == 0;
  }

  inline volatile SizeType size() const {
    return _count;
  }

private:
  T _buf[Size];
  SizeType _back;
  SizeType _front;

  volatile SizeType _count;
};

#endif
