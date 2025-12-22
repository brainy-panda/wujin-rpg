#include "darray.h"

#include <cstdint>
#include <cstdio>
#include <string>

struct Cub
{
    std::string name;
    int32_t cuteness;
    uint32_t size;

    Cub(const std::string& name, int32_t cuteness, uint32_t size)
        : name(name)
        , cuteness(cuteness)
        , size(size)
    {
        printf("CONSTRUCTOR name=%s cuteness=%d size=%u\n", name.c_str(), cuteness, size);
    }

    ~Cub() noexcept { printf("DTOR\n"); }

    Cub(const Cub& cub)
        : name(cub.name)
        , cuteness(cub.cuteness)
        , size(cub.size)
    {
        printf("COPY-CTOR name=%s cuteness=%d size=%u\n", name.c_str(), cuteness, size);
    }

    Cub(Cub&& cub)
        : name(std::move(cub.name))
        , cuteness(cub.cuteness)
        , size(cub.size)
    {
        printf("MOVE-CTOR name=%s cuteness=%d size=%u\n", name.c_str(), cuteness, size);
    }

    Cub& operator=(const Cub& cub)
    {
        name = cub.name;
        cuteness = cub.cuteness;
        size = cub.size;
        printf("COPY-ASSIGN name=%s cuteness=%d size=%u\n", name.c_str(), cuteness, size);
        return *this;
    }

    Cub& operator=(Cub&& cub)
    {
        name = std::move(cub.name);
        cuteness = cub.cuteness;
        size = cub.size;
        printf("MOVE-ASSIGN name=%s cuteness=%d size=%u\n", name.c_str(), cuteness, size);
        return *this;
    }
};

int main()
{
    printf("make cubs vector, push_back henry and heidi\n");
    bear::vector<Cub> cubs;
    cubs.push_back(Cub("henry", 1000, 18));
    cubs.push_back(Cub("heidi", 1000, 8));

    printf("make other vector, push_back random\n");
    bear::vector<Cub> otherCubs;
    cubs.push_back(Cub("random", 42, 12));

    printf("assign other = cubs\n");
    otherCubs = cubs;

    printf("move assign other = cubs\n");
    otherCubs = std::move(cubs);

    return otherCubs.size();
}
