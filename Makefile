CFLAGS = -I ./include
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm #-lXrandr
SRC = asteroids/asteroids.cpp asteroids/log.cpp asteroids/timers.cpp jlo.cpp ecs/system.cpp ecs/components.cpp ecs/textures.cpp ecs/math.cpp ecs/anim.cpp
OBJ = $(SRC:.cpp=.o)

# Build everything
all: test

# Compile object files for all .cpp files
%.o: %.cpp
	g++ $(CFLAGS) -c $< -Wall -Wextra $(LFLAGS) -o $@

# Link everything together
test: $(OBJ)
	g++ $^ asteroids/libggfonts.a -o $@ $(LFLAGS)

# Clean generated files
clean:
	rm -f test
	rm -f *.o
