CPPC=g++
FLAGS=-g -o3 -Wall -Werror

all: compile run display

display:
	@feh test.bmp

compile:
	@echo "Compiling..."
	${CPPC} ${FLAGS} main.cpp libbmp.cpp -o mosaic

run:
	@./mosaic images/bmp/sand_fox.bmp
