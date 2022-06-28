build:
	clang ./*.c -o game  -L/opt/homebrew/lib -lSDL2 -L/opt/homebrew/lib -lSDL2_ttf

run:
	./game

clean:
	rm game
