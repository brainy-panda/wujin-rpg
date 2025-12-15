COMPILER_FLAGS = -Wall -ggdb3 -O0 -Wextra -Wpedantic -Werror -std=c++20

trade:
	g++ $(COMPILER_FLAGS) main.cpp -o trade

test1:
	g++ $(COMPILER_FLAGS) test1.cpp -o test1

clean:
	rm -f trade test1

.PHONY: clean
