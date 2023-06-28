EXE = file_transfer

SRC = $(wildcard ./SourceCode/*.c)
OBJ = $(SRC:.c=.o)

#--


all : $(EXE) clean


#--


CFLAGS = -Wall -g
#LDLIBS = -lm


#--


purge :	clean
	@rm -f $(EXE)
	@echo "terminado purge"

clean :
	@rm -f $(OBJ)
	@echo "terminado clean"


#--


$(EXE) : $(OBJ)


#--