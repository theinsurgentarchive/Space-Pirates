CFLAGS = -I ./include
LFLAGS = -lX11 -lGL -lm
# Target source files
SRC = asteroids.cpp log.cpp timers.cpp jlo.cpp jsandoval.cpp dchu.cpp balrowhany.cpp mchitorog.cpp
# Compile each source file in 'SRC' to O, when 'OBJ' is used, the files are 
# spread, so in asteroids the targets appear to be like 
# asteroids.o log.o timers.o etc.
OBJ = ${SRC:.cpp=.o}
# Similar for the debug objects, preprocessor directive occurs during compile 
# time so we must compile for each .cpp file with the DEBUG flag for 
# DPRINTF/DPRINT
D_OBJ = ${SRC:.cpp=-debug.o}
# NOTE: IF YOU WANT DEBUG MODE: make debug
all: asteroids

silent:
	@$(MAKE) all SILENT=1 --no-print-directory

ifdef SILENT
%.o: %.cpp
	@g++ ${CFLAGS} -c $< -Wall -Wextra ${LFLAGS} -o $@	

%-debug.o: %.cpp
	@g++ ${CFLAGS} -DDEBUG -c $< -Wall -Wextra ${LFLAGS} -o $@

else
%.o: %.cpp
	g++ ${CFLAGS} -c $< -Wall -Wextra ${LFLAGS} -o $@	

%-debug.o: %.cpp
	g++ ${CFLAGS} -DDEBUG -c $< -Wall -Wextra ${LFLAGS} -o $@
endif

asteroids: ${OBJ}
	g++ $^ libggfonts.a -o $@ ${LFLAGS}

debug: ${D_OBJ}
	g++ $^ libggfonts.a -g -o $@ ${LFLAGS}

clean:
	rm -f asteroids
	rm -f *.o
