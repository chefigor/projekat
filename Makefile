
output: main.o server.o
	g++ -pthread -g  main.o server.o -o output -std=c++17

main.o: main.cpp
	g++ -pthread -g -c main.cpp -std=c++17

server.o: server.cpp
	g++ -pthread -g -c server.cpp -std=c++17

klijent: klijent.cpp
	g++ klijent.cpp -o klijent
	
clean:
	rm *.o output klijent

