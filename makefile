sfml-app: main.o
	g++ main.o -o sfml-app -lsfml-audio -lsfml-system
main.o: main.cpp
	g++ -c main.cpp -std=c++11
clean:
	rm -f main.o sfml-app