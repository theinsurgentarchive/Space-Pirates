CFLAGS = -I ./include -DUSE_OPENAL_SOUND -g
LFLAGS = -lrt -lX11 -lGL -lGLU -pthread -lm -lopenal -lalut
# Target source files
SRC = asteroids.cpp log.cpp timers.cpp jlo.cpp jsandoval.cpp dchu.cpp balrowhany.cpp mchitorog.cpp image.cpp
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

asteroids: ${OBJ}
	
	@g++ $^ libggfonts.a -o $@ ${LFLAGS}
	@echo "Generated $@\n"

debug: ${D_OBJ}
	@g++ $^ libggfonts.a -g -o $@ ${LFLAGS}
	@echo "Generated $@\n"

else
%.o: %.cpp
	g++ ${CFLAGS} -c $< -Wall -Wextra ${LFLAGS} -o $@	

%-debug.o: %.cpp
	g++ ${CFLAGS} -DDEBUG -c $< -Wall -Wextra ${LFLAGS} -o $@

asteroids: ${OBJ}
	g++ $^ -g libggfonts.a -o $@ ${LFLAGS}

debug: ${D_OBJ}
	g++ $^ libggfonts.a -g -o $@ ${LFLAGS}

endif

clean:
	rm -f asteroids
	rm -f *.o
	rm -f debug
