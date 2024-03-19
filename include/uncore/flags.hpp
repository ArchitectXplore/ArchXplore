#ifndef __FLAGS_HPP__
#define __FLAGS_HPP__


namespace archXplore{

template <typename T>
class Flags{
private:
    T _flag;
public:
    Flags():_flag(0){}
    // copy constructor
    Flags(const T& val): _flag(val){};
    const Flags<T>& operator = (T val){
        _flag = val;
        return *this;
    } 
    // move constructor
    Flags(T&& rhs): _flag(rhs){};
    const Flags<T>& operator = (T&& rhs){
        _flag = rhs;
        return *this;
    } 
    // 
    constexpr T get() const{
        return _flag;
    }
    void set(const T& val) noexcept{
        _flag |= val;
    }
    void reset(const T& val) noexcept{
        _flag = val;
    }
    void clear(){
        _flag = 0;
    }
    void clear(const T& val){
        _flag &= ~val;
    }

    constexpr bool isSet(const T& val) const{
        return (val & _flag);
    }
    constexpr bool allSet(const T& val) {
        return ((val & _flag) == val);
    }
    constexpr bool noneSet(const T& val) {
        return ((val & _flag) == 0);
    }


};


};

#endif // __FLAGS_HPP__