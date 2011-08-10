#ifndef CIRCULARBUFFER_h
#define CIRCULARBUFFER_h

#ifdef CIRCULARBUFFER_EMPTY_CHECK
#define CIRCULARBUFFER_NOT_EMPTY if(!empty())
#else
#define CIRCULARBUFFER_NOT_EMPTY 
#endif

template <class T, class SizeType, uint16_t Size>
class CircularBuffer {
public:
  CircularBuffer() :
    _back(0), _front(0) {}
  ~CircularBuffer() {}

  bool push(const T& value) {

    SizeType back = (_back+1)%Size;
    if ( back != _front   )
    {
        _buf[_back] = value;
        _back = back;
        return true;
    }
    else
    {
        return false;
    }
  }

  void pop() {
    CIRCULARBUFFER_NOT_EMPTY 
    {
        _front = (_front+1)%Size;
    }
  }

  inline const T& front() {
    CIRCULARBUFFER_NOT_EMPTY 
    {
        return _buf[_front];
    }
  }

  inline const T& back() {
    CIRCULARBUFFER_NOT_EMPTY 
    {
        return _buf[_back];
    }
  }


  inline volatile bool empty() const {
    return _front == _back;
  }


private:
  T _buf[Size];
  SizeType _back;
  SizeType _front;

};

#endif
