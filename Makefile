CFLAGS = -I ./include
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm
SRC = asteroids.cpp log.cpp timers.cpp jlo.cpp 
OBJ = ${SRC:.cpp=.o}

all: asteroids debug

%.o: %.cpp
	g++ ${CFLAGS} -c $< -Wall -Wextra ${LFLAGS} -o $@

asteroids: ${OBJ}
	g++ $^ libggfonts.a -o $@ ${LFLAGS}

debug: ${OBJ}
	g++ $^ libggfonts.a -g -D DEBUG -o $@ ${LFLAGS}
clean:
	rm -f asteroids
	rm -f *.o
