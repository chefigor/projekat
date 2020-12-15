
output: main.o server.o klijent.o
	g++ -pthread -g  main.o server.o klijent.o -o output -std=c++17

main.o: main.cpp
	g++ -pthread -g -c main.cpp -std=c++17

server.o: server.cpp
	g++ -pthread -g -c server.cpp -std=c++17

klijent.o: klijent.cpp
	g++ -pthread -g -c klijent.cpp -std=c++17
	
clean:
	rm *.o output klijent

