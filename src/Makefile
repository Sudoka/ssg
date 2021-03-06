OPTS = -g
#OPTS = -O3

INC_DIR = ../include
GLEW_SRC = ../win32/ssg-lib/glew-1.9.0/src/glew.c

HOST_PLATFORM := $(shell $(CPP) -dumpmachine)
$(info $(HOST_PLATFORM))

# settings for turing
ifeq   "$(HOST_PLATFORM)" "x86_64-redhat-linux"
INCLUDES = -I/opt/viz/include
LIBS = -lGL -lGLU -lglut -L/opt/viz/lib -lGLEW
endif

ifeq   "$(HOST_PLATFORM)" "x86_64-linux-gnu"
#for sn336,sn148, ubuntu 
INCLUDES = -I../include -I.
LIBS = -lGL -lGLU -lglut -lGLEW 
endif

# for macosx
# NOTE for mac you have to use the GLSL 1.2 shaders....
ifeq   "$(HOST_PLATFORM)" "i686-apple-darwin11"
INCLUDES = -I../include -framework OpenGL -framework GLUT -DUSE_APPLE_VERTEXARRAYS
LIBS =
endif

# for MINGW32 on Windows
ifeq   "$(HOST_PLATFORM)" "mingw32"
INCLUDES = -I../include -I../win32/ssg-lib/glew-1.9.0/include -I../win32/ssg-lib/glut-3.7.6-bin -DGLEW_STATIC
OBJS = glew.o
LIBS =
endif

OBJS += InitShader.o \
	ObjLoader.o \
	Texture.o \
	TextureLoader.o \
	FrameBufferObject.o \
	ParticleSystem.o \
	ssg.o

SSG_INCS = $(INC_DIR)/ssg.h \
	$(INC_DIR)/CheckError.h \
	$(INC_DIR)/InitShader.h \
	$(INC_DIR)/Texture.h \
	$(INC_DIR)/TextureLoader.h \
	$(INC_DIR)/ObjLoader.h \
	$(INC_DIR)/FrameBufferObject.h \
	$(INC_DIR)/ParticleSystem.h \
	$(INC_DIR)/Camera.h \
	$(INC_DIR)/Trackball.h 


all : 	ssg.a

ssg.o : ssg.cpp $(SSG_INCS)
	g++ $(OPTS) -c $(INCLUDES) ssg.cpp

InitShader.o : InitShader.cpp $(SSG_INCS)
	g++ $(OPTS) -c $(INCLUDES) InitShader.cpp

ObjLoader.o : ObjLoader.cpp $(SSG_INCS)
	g++ $(OPTS) -c $(INCLUDES) ObjLoader.cpp

Texture.o : Texture.cpp $(SSG_INCS)
	g++ $(OPTS) -c $(INCLUDES) Texture.cpp

TextureLoader.o : TextureLoader.cpp $(SSG_INCS)
	g++ $(OPTS) -c $(INCLUDES) TextureLoader.cpp

FrameBufferObject.o : FrameBufferObject.cpp $(SSG_INCS)
	g++ $(OPTS) -c $(INCLUDES) FrameBufferObject.cpp

ParticleSystem.o : ParticleSystem.cpp $(SSG_INCS)
	g++ $(OPTS) -c $(INCLUDES) ParticleSystem.cpp

ssg.a : $(OBJS)
	ar rvs ssg.a $(OBJS)

glew.o : $(GLEW_SRC)
	g++ $(OPTS) -DGLEW_STATIC -c $(GLEW_SRC) $(INCLUDES)

clean :
	rm -f *.o
	rm -f *~
	rm -rf *.dSYM
	rm -f ssg.a
