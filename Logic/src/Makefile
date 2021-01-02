OBJ_DIR = ../Obj
# 获取所以源文件     
SRC = ${wildcard  *.c}
# 把.c后缀替换为.o，并加上目录        
OBJ = ${patsubst %.c, ${OBJ_DIR}/%.o, ${notdir ${SRC}}}

INC = ${wildcard  *.h}

TARGET=${OBJ_DIR}/prop
CC=cc
CCFLAGS=-g -Wall 

all:${TARGET}

${TARGET}: ${OBJ} ${OBJ_DIR}/prop.lemon.o
	${CC}  $^ -lsqlite3 -L. -o $@
	@echo "Compile done."
    

${OBJ_DIR}/%.o:%.c lex.yy.h token.h ${INC}
	@echo "Compiling $< ==> $@"
	${CC} ${CCFLAGS} -c $< -o $@
	
${OBJ_DIR}/prop.lemon.o:prop.lemon.c

lex.yy.c lex.yy.h:prop.lex.l 
	flex --header-file=lex.yy.h prop.lex.l 

token.h ast.h:prop.lemon.h

prop.lemon.c prop.lemon.h:prop.lemon.y
	${CC} ${CCFLAGS} ../lemon.c -o ../lemon
	../lemon prop.lemon.y
    
clean:
# -r删除目录，-f删除文件
	rm -f *.o
	rm -f ${OBJ_DIR}/*.o
	rm -f ${TARGET}
	#rm -f lex.yy.c lex.yy.h
	rm -f prop.lemon.c prop.lemon.h