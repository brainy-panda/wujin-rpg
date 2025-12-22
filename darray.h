#pragma once

#include <memory>
#include <cstdint>

// refer to: https://github.com/nbird11/vector-cpp-stl

namespace bear
{

template <typename T, typename A = std::allocator<T>>
class vector
{
public:
    vector(const A& alloc = A());
    explicit vector(size_t count, const A& alloc = A());
    explicit vector(size_t count, const T& value = T(), const A& alloc = A());

    vector(const vector& rhs);
    vector(vector&& rhs);

    ~vector() noexcept;

    void swap(vector<T, A>& rhs)
    {
	std::swap(_start, rhs._start);
	std::swap(_numElems, rhs._numElems);
	std::swap(_numCapacity, rhs._numCapacity);
	std::swap(_alloc, rhs._alloc);
    }

    size_t capacity() const { return _numCapacity; }
    size_t size() const { return _numElems; }
    size_t empty() const { return _numElems == 0; }

    void reserve(size_t newCap);
    void resize(size_t newCap);
    void resize(size_t newCap, const T& value);
    void push_back(const T& value);
    void push_back(T&& value);

    void clear() // does not deallocate, just destructs elements and reduces size to 0
    {
        for (int64_t ii = int64_t(size()) - 1; ii >= 0; --ii)
            std::allocator_traits<A>::destroy(_alloc, _start + ii);
        _numElems = 0;
    }
    void pop_back();
    void shrink_to_fit();

    T& operator[](size_t index);
    const T& operator[](size_t index) const;

    vector& operator=(vector rhs); // copy-and-swap idiom for strong exception safety

    class iterator;
    iterator begin() { return iterator(_start); }
    iterator end() { return iterator(_start + _numElems); }

private:
    A _alloc;
    T* _start;
    size_t _numElems;
    size_t _numCapacity;
};

template <typename T, typename A>
class vector<T, A>::iterator
{
public:
    iterator() : _ptr(nullptr) {}
    iterator(T* ptr) : _ptr(ptr) {}
    iterator(const iterator& rhs) : _ptr(rhs._ptr) {}

    iterator& operator=(const iterator& rhs)
    {
	_ptr = rhs._ptr;
	return *this;
    }

    bool operator==(const iterator& rhs) const { return _ptr == rhs._ptr; }
    bool operator!=(const iterator& rhs) const { return _ptr != rhs._ptr; }

    T& operator*() { return *_ptr; }

    iterator& operator++()
    {
	_ptr++;
	return *this;
    }

    iterator operator++(int postfix)
    {
	iterator tmp(_ptr);
	_ptr++;
	return tmp;
    }

    iterator& operator--()
    {
	_ptr--;
	return *this;
    }

    iterator operator--(int postfix)
    {
	iterator tmp(_ptr);
	_ptr--;
	return tmp;
    }
private:
    T* _ptr;
};

template <typename T, typename A>
vector<T, A>::vector(const A& alloc)
{
    _alloc = alloc;
    _start = nullptr;
    _numElems = 0;
    _numCapacity = 0;
}

template <typename T, typename A>
vector<T, A>::vector(size_t count, const A& alloc)
{
    _alloc = alloc;
    _numElems = count;
    _numCapacity = count;
    _start = _alloc.allocate(count);
    for (size_t ii = 0; ii < _numElems; ++ii)
	std::allocator_traits<A>::construct(_alloc, _start + ii);
}

template <typename T, typename A>
vector<T, A>::vector(size_t count, const T& value, const A& alloc)
{
    _alloc = alloc;
    _numElems = count;
    _numCapacity = count;
    _start = _alloc.allocate(count);
    for (size_t ii = 0; ii < _numElems; ++ii)
	std::allocator_traits<A>::construct(_alloc, _start + ii, value);
}

template <typename T, typename A>
vector<T, A>::vector(const vector& rhs)
{
    if (!rhs.empty())
    {
        _start = _alloc.allocate(rhs._numElems);
        _numCapacity = rhs._numCapacity;
        _numElems = rhs._numElems;
        for (size_t ii = 0; ii < _numElems; ++ii)
            std::allocator_traits<A>::construct(_alloc, _start + ii, rhs._start[ii]);
    }
    else
    {
        _start = nullptr;
        _numElems = 0;
        _numCapacity = 0;
    }
}

template <typename T, typename A>
vector<T, A>::vector(vector&& rhs)
{
    // should be nothrow even if type T can throw on move
    _start = rhs._start;
    rhs._start = nullptr;
    _numElems = rhs._numElems;
    rhs._numElems = 0;
    _numCapacity = rhs._numCapacity;
    rhs._numCapacity = 0;
}

template <typename T, typename A>
vector<T, A>::~vector() noexcept
{
    for (int64_t ii = size() - 1; ii >= 0; --ii)
        std::allocator_traits<A>::destroy(_alloc, _start + ii); // destructors nothrow as required by our and STL's API
    _alloc.deallocate(_start, _numCapacity); // the std::allocator doesn't need the capacity, but others might
}

template <typename T, typename A>
void vector<T, A>::reserve(size_t newCapacity)
{
    if (newCapacity <= _numCapacity)
        return;

    T* newBuf = _alloc.allocate(newCapacity);
    int64_t ii = size() - 1;
    try
    {
        for (ii = size() - 1; ii >= 0; --ii)
        {
            std::allocator_traits<A>::construct(_alloc, newBuf + ii, std::move(_start[ii])); // non-moveable T will copy
            std::allocator_traits<A>::destroy(_alloc, _start + ii);
        }
        _alloc.deallocate(_start, _numCapacity);
        _start = newBuf;
        _numCapacity = newCapacity;
    }
    catch (...)
    {
        // if exception is thrown, it must be thrown by the construction of T, so we need to rewind
        // and destruct any already-constructed Ts
        ++ii;
        for (; ii < int64_t(size()); ++ii)
            std::allocator_traits<A>::destroy(_alloc, newBuf + ii);
    }
}

template <typename T, typename A>
void vector<T, A>::resize(size_t newElems)
{
    if (newElems < _numElems)
    {
        for (int64_t ii = int64_t(_numElems) - 1; ii >= newElems; --ii)
            std::allocator_traits<A>::destroy(_alloc, _start + ii);
    }
    else if (newElems > _numElems)
    {
        if (newElems > _numCapacity)
            reserve(newElems);
        for (size_t ii = _numElems; ii < newElems; ++ii)
            std::allocator_traits<A>::construct(_alloc, _start + ii);
    }
    _numElems = newElems;
}

template <typename T, typename A>
void vector<T, A>::resize(size_t newElems, const T& value) // doesn't require a default constructor
{
    if (newElems < _numElems)
    {
        for (int64_t ii = _numElems - 1; ii >= newElems; --ii)
            std::allocator_traits<A>::destroy(_alloc, _start + ii);
    }
    else if (newElems > _numElems)
    {
        if (newElems > _numCapacity)
            reserve(newElems);
        for (size_t ii = _numElems; ii < newElems; ++ii)
            std::allocator_traits<A>::construct(_alloc, _start + ii, value); // no move. value is an exemplar for copying into the extra slots
    }
    _numElems = newElems;
}

template <typename T, typename A>
void vector<T, A>::pop_back()
{
    std::allocator_traits<A>::destroy(_alloc, _start + _numElems - 1); // UB if size() is already 0, intentional. C++ yay!
    --_numElems;
}

template <typename T, typename A>
void vector<T, A>::shrink_to_fit()
{
    if (_numCapacity > _numElems)
    {
        if (_numElems)
        {
            T* newBuf = _alloc.allocate(_numElems);
            int64_t ii = size() - 1;
            try
            {
                for (; ii >= 0; --ii)
                {
                    std::allocator_traits<A>::construct(_alloc, newBuf + ii, std::move(_start[ii])); // non-moveable T will copy
                    std::allocator_traits<A>::destroy(_alloc, _start + ii);
                }
                _alloc.deallocate(_start, _numCapacity);
                _start = newBuf;
                _numCapacity = _numElems;
            }
            catch (...)
            {
                // if exception is thrown, it must be thrown by the construction of T, so we need to rewind
                // and destruct any already-constructed Ts
                ++ii;
                for (; ii < int64_t(size()); ++ii)
                    std::allocator_traits<A>::destroy(_alloc, newBuf + ii);
            }
        }
        else
        {
            _alloc.deallocate(_start, _numCapacity);
            _start = nullptr;
            _numCapacity = 0;
        }
    }
}

template <typename T, typename A>
void vector<T, A>::push_back(const T& value)
{
    if (_numCapacity == _numElems)
        reserve(_numCapacity == 0 ? 1 : _numCapacity * 2);
    std::allocator_traits<A>::construct(_alloc, _start + _numElems, value);
    ++_numElems;
}

template <typename T, typename A>
void vector<T, A>::push_back(T&& value)
{
    if (_numCapacity == _numElems)
        reserve(_numCapacity == 0 ? 1 : _numCapacity * 2);
    std::allocator_traits<A>::construct(_alloc, _start + _numElems, std::move(value));
    ++_numElems;
}

template <typename T, typename A>
T& vector<T, A>::operator[](size_t index)
{
    return _start[index];
}

template <typename T, typename A>
const T& vector<T, A>::operator[](size_t index) const
{
    return _start[index];
}

template <typename T, typename A>
vector<T, A>& vector<T, A>::operator=(vector rhs)
{
    swap(rhs);
    return *this;
}

} // namespace bear
