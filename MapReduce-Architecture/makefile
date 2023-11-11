CC := g++ -std=c++11

Obj_files = main_process.o functions.o

all: main.out map.out reduce.out

main.out: ${Obj_files}
	$(CC) ${Obj_files} -o main.out

map.out: map_process.o functions.o
	$(CC) map_process.o functions.o -o map.out

reduce.out: reduce_process.o functions.o
	$(CC) reduce_process.o functions.o -o reduce.out

main_procces.o: headers.hpp main_process.cpp
	$(CC) -c main_process.cpp -o main_process.o

functions.o: functions.cpp
	$(CC) -c functions.cpp -o functions.o

map_procces.o: headers.hpp map_process.cpp
	$(CC) -c map_process.cpp -o map_process.o

reduce_procces.o: headers.hpp reduce_process.cpp
	$(CC) -c reduce_process.cpp -o reduce_process.o

