
output: main.o server.o klijent.o
	g++ -pthread -g  main.o server.o klijent.o -o output -std=c++17 -lssl -lcrypto

main.o: main.cpp
	g++ -pthread -g -c main.cpp -std=c++17 -lssl -lcrypto

server.o: server.cpp
	g++ -pthread -g -c server.cpp -std=c++17 -lssl -lcrypto

klijent.o: klijent.cpp
	g++ -pthread -g -c klijent.cpp -std=c++17 -lssl -lcrypto
	
clean:
	rm *.o output klijent

