#ifndef __FAKE_REGFILE_HPP__
#define __FAKE_REGFILE_HPP__
template <class T>
class FakeRegFile
{
public:
  void write(size_t i, T value)
  {
    if (!_zero || i != 0)
      _data[i] = value;
  }
  T get(size_t i){
    return _data[i];
  }
  const T& operator [] (size_t i) const
  {
    return _data[i];
  }
  FakeRegFile(const int& n, const bool& zero):
    _size(n), _zero(zero)
  {
    _data = new T[_size];
    reset();
  }
  ~FakeRegFile(){
    delete _data;
  }
  void reset()
  {
    memset(_data, 0, sizeof(_data));
  }
private:
  T* _data;
  bool _zero;
  int _size;
};
#endif // __FAKE_REGFILE_HPP__