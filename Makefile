CFLAGS = -I ./include
LFLAGS = -lrt -lX11 -lGLU -lGL -pthread -lm
SRC = asteroids.cpp log.cpp timers.cpp jlo.cpp jsandoval.cpp dchu.cpp balrowhany.cpp mchitorog.cpp
OBJ = ${SRC:.cpp=.o}
D_OBJ = ${SRC:.cpp=-debug.o}

all: asteroids

%.o: %.cpp
	g++ ${CFLAGS} -c $< -Wall -Wextra ${LFLAGS} -o $@	

%-debug.o: %.cpp
	g++ ${CFLAGS} -DDEBUG -c $< -Wall -Wextra ${LFLAGS} -o $@

asteroids: ${OBJ}
	g++ $^ libggfonts.a -o $@ ${LFLAGS}

debug: ${D_OBJ}
	g++ $^ libggfonts.a -g -o $@ ${LFLAGS}

clean:
	rm -f asteroids
	rm -f *.o
