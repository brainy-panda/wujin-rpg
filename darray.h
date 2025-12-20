#pragma once

namespace bear
{

template <typename T, typename A = std::allocator<T>>
class vector
{
public:
    vector(const A& alloc);
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

    vector& operator=(const vector& rhs);
    vector& operator=(vector&& rhs);

    size_t capacity() const { return _numCapacity; }
    size_t size() const { return _numElems; }
    size_t empty() const { return _numElems == 0; }

    void push_back(const T& value);
    void push_back(T&& value);
    void reserve(size_t newCap);
    void resize(size_t newCap);
    void resize(size_t newCap, const T& value);

    void clear();
    void pop_back();
    void shrink_to_fit();

    T& operator[](size_t index);
    const T& operator[](size_t index) const;

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
	_alloc.construct(_start + ii);
}

template <typename T, typename A>
vector<T, A>::vector(size_t count, const T& value, const A& alloc)
{
    _alloc = alloc;
    _numElems = count;
    _numCapacity = count;
    _start = _alloc.allocate(count);
    for (size_t ii = 0; ii < _numElems; ++ii)
	_alloc.construct(_start + ii, value);
}

} // namespace bear

// void reserve(size_t newCap)
// {
//     size_t curCap = this->capacity();
//     if (curCap >= newCap)
// 	return;

//     T* newArray = nullptr;
//     try
//     {
// 	newArray = new T[newCap];
//     }
//     catch (...)
//     {
// 	delete[] newArray; // clean up if partially allocated
// 	throw;
//     }

//     if (_numElems)
//     {
// 	T* _end = _start + _numElems;
// 	for (T* ptr = _end - 1; ptr >= _start; --ptr)
// 	    ptr->~T();
//     }
// }
