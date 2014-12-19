DIR=./out
all:
	mkdir -p $(DIR)
	gcc src/program.c -lXi -lX11  -o $(DIR)/program 
	chmod +x $(DIR)/program


