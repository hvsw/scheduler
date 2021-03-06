#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
# 

# HOW TO MAKEFILE:
# target: prerequisites
# <TAB> recipe

# Manually build test_cidentify
# gcc -g -w -o ./testes/test_cidentify ./testes/test_cidentify.c -L./lib -lcthread -Wall

CC=gcc -g -w
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src
TST_DIR=./testes
MAIN_NAME=mainProg

all: objetos mvObj libcthread.a mvLib main testCCREATE testCJOIN testCYIELD testCWAIT

objetos: $(SRC_DIR)/cthread.c $(INC_DIR)/cdata.h $(INC_DIR)/cthread.h $(INC_DIR)/support.h
	$(CC) -c $(SRC_DIR)/cthread.c -Wall

mvObj:
	mv *.o $(BIN_DIR)

libcthread.a: $(BIN_DIR)/cthread.o
	ar crs libcthread.a $(BIN_DIR)/*.o

mvLib:
	mv *.a $(LIB_DIR)

main:
	$(CC) -o $(MAIN_NAME) main.c -L$(LIB_DIR) -lcthread -Wall

testCCREATE:
	$(CC) -o $(TST_DIR)/test_ccreate $(TST_DIR)/test_ccreate.c -L$(LIB_DIR) -lcthread -Wall

testCJOIN:
	$(CC) -o $(TST_DIR)/test_cjoin $(TST_DIR)/test_cjoin.c -L$(LIB_DIR) -lcthread -Wall

testCYIELD:
	$(CC) -o $(TST_DIR)/test_cyield $(TST_DIR)/test_cyield.c -L$(LIB_DIR) -lcthread -Wall

testCWAIT:
	$(CC) -o $(TST_DIR)/test_cwait $(TST_DIR)/test_cwait.c -L$(LIB_DIR) -lcthread -Wall

testFILA:
	$(CC) -o $(TST_DIR)/test_fila $(TST_DIR)/test_fila.c -Wall

clean:
	rm -rf $(MAIN_NAME) $(LIB_DIR)/*.a $(BIN_DIR)/cthread.o $(TST_DIR)/*.o $(TST_DIR)/test_cjoin $(TST_DIR)/test_ccreate $(TST_DIR)/test_cyield $(SRC_DIR)/*~ $(INC_DIR)/*~ *~
