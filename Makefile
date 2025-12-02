COMPILER_FLAGS = -Wall -ggdb3 -O0 -Wextra -Wpedantic -Werror

trade:
	g++ $(COMPILER_FLAGS) main.cpp -o trade

clean:
	rm -f trade

.PHONY: clean
