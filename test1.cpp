#include <vector>

// try custom allocator for stl vector

#include <iostream>
#include <cstdio>
#include <memory>

constexpr std::size_t POOL_SIZE = 1024 * 1024;

class MemoryPool
{
public:
    MemoryPool()
        : _buffer(static_cast<std::byte*>(::operator new(POOL_SIZE)))
        , _offset(0)
    {
    }

    ~MemoryPool()
    {
        ::operator delete(_buffer);
    }

    void* allocate(size_t size, size_t alignment)
    {
        size_t space = POOL_SIZE - _offset;
        void* ptr = _buffer + _offset;
        void* aligned_ptr = std::align(alignment, size, ptr, space);

        if (!aligned_ptr || static_cast<std::byte*>(aligned_ptr) + size > _buffer + POOL_SIZE)
            throw std::bad_alloc();

        _offset = static_cast<std::byte*>(aligned_ptr) - _buffer + size;
        return aligned_ptr;
    }

    

    void reset()
    {
        printf("MemoryPool reset from offset %zu to 0\n", _offset);
        _offset = 0;
    }

private:
    std::byte* _buffer;
    size_t _offset = 0;
};

MemoryPool& GetSharedPool()
{
    static MemoryPool pool;
    return pool;
}

template <typename T>
struct LoggingAllocator
{
    using value_type = T;

    LoggingAllocator() = default;

    template <typename U>
    constexpr LoggingAllocator(const LoggingAllocator<U>& alloc) noexcept {}

    T* allocate(std::size_t n)
    {
        printf("Allocating %zu elements of size %zu\n", n, sizeof(T));
        // return static_cast<T*>(::operator new(n * sizeof(T)));
        return static_cast<T*>(GetSharedPool().allocate(n * sizeof(T), alignof(T)));
    }

    void deallocate(T* p, std::size_t n) noexcept
    {
        printf("Deallocating %zu elements at address %p\n", n, (void*) p);
        // ::operator delete(p);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        new (p) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p) noexcept
    {
        p->~U();
    }

    template <typename U>
    struct rebind
    {
        using other = LoggingAllocator<U>;
    };
};

// assignments require that allocators can be compared
template <typename T, typename U>
bool operator==(const LoggingAllocator<T>&, const LoggingAllocator<U>&) { return true; }

template <typename T, typename U>
bool operator!=(const LoggingAllocator<T>&, const LoggingAllocator<U>&) { return false; }

int main()
{
    for (int ii = 0; ii < 5; ++ii)
    {
        printf("Iteration #%d:\n", ii);

        std::vector<int, LoggingAllocator<int>> pool_vec;
        for (int jj = 0; jj < 100; ++jj)
            pool_vec.push_back(jj * ii);

        for (int val : pool_vec)
            printf("%d ", val);

        printf("\n");

        GetSharedPool().reset();
    }
    return 0;
}
