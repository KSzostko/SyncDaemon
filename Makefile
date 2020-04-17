OBJ = main.o functions.o
all: syncFolders
syncFolders: $(OBJ)
	gcc $(OBJ) -o output
$(OBJ): functions.h
.PHONY: clean
clean:
	rm -f *.o syncFolders