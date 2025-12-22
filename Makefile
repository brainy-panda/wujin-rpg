COMPILER_FLAGS = -Wall -ggdb3 -O0 -Wextra -Wpedantic -Werror -std=c++20

trade:
	g++ $(COMPILER_FLAGS) main.cpp -o trade

test1:
	g++ $(COMPILER_FLAGS) test1.cpp -o test1

darray:
	g++ $(COMPILER_FLAGS) darray.cpp -o darray

clean:
	rm -f trade test1 darray

.PHONY: clean
