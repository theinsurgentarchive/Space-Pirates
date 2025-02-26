CFLAGS = -I ./include
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm
SRC = asteroids.cpp log.cpp timers.cpp jlo.cpp dchu.cpp
OBJ = ${SRC:.cpp=.o}

# Build everything
all: asteroids

# Compile object files for all .cpp files
%.o: %.cpp
	g++ ${CFLAGS} -c $< -Wall -Wextra ${LFLAGS} -o $@

asteroids: ${OBJ}
	g++ $^ libggfonts.a -o $@ ${LFLAGS}

clean:
	rm -f asteroids
	rm -f *.o
