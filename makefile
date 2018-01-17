sfml-app: main.o
	g++ main.o -o sfml-app.out -lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system
main.o: main.cpp
	g++ -c main.cpp -std=c++11
clean:
	rm -f main.o sfml-app.out