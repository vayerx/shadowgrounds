default: compiler testshaders

.PHONY: default compile test


compile: compiler
	./compiler shaders/*.txt


compiler: compiler.hs Parser.hs Shader.hs Lex.hs
	ghc -O -fasm --make -Wall -Werror compiler.hs -o compiler

Parser.hs: Parser.g
	frown --lexer --expected -O Parser.g

Lex.hs: Lex.x
	alex -g -o Lex.hs Lex.x


testshaders: testshaders.c
	gcc -o testshaders -O -Wall -Werror testshaders.c $(shell sdl-config --cflags) $(shell sdl-config --libs) -lGLEW

test: testshaders
	./testshaders glshaders/*.txt
