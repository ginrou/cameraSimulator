# Makefile of camera simulator

##ディレクトリ
INCLUDE_DIR = . #current directory


MAINFILE = main.c
TARGET = ${MAINFILE:.c=.out}
SRCS = ${MAINFILE} viewer.c glm.c camera.c batch.c parameter.c
OBJS = ${SRCS:.c=.o}

##OpenCVを追加する時
#オブジェクトファイルの生成はCVFLAGS
#リンクするときはCVLIBS
CVFLAGS = `pkg-config --cflags opencv`
CVLIBS = `pkg-config --libs opencv`

##OpenGLを追加する時
GLFLAGS = -framework OpenGL \
	-framework GLUT \
	-framework Foundation\
	-lglui

##マクロ定義
CC = g++
CFLAGS =  -m64 -I${INCLUDE_DIR} 
DEBUG = -g -O0
CLIBFLAGS = -lm -lstdc++ #リンクするもの


##生成規則

#TARGET
${TARGET}:${OBJS}
	${CC} ${CFLAGS} -o $@ ${DEBUG} ${CLIBS} ${OBJS} ${CVLIBS} ${GLFLAGS}

#mainファイルの生成
${MAINFILE:.c=.o}:${MAINFILE}
	${CC} $< ${CFLAGS} -c -o $@ ${DEBUG} ${CVFLAGS} #${GLFLAGS}

#サフィックスルール
%.o:%.c %.h settings.h
	${CC} $< ${CFLAGS} -c -o $@ ${DEBUG} ${CVFLAGS} #${GLFLAGS}

#ヘッダファイルの更新



clean:
	rm -f ${TARGET} ${OBJS}
