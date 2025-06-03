# make
CC = g++
CFLAGS = -std=c++17 -Wall

program_demo = demo
program_data = data

objects_demo = main_demo.o
objects_data = main_data.o

all: demo data

demo: $(objects_demo)
	$(CC) -o $(program_demo) $(objects_demo)

data: $(objects_data)
	$(CC) -o $(program_data) $(objects_data)

$(objects_demo): src/main_demo.cpp
	$(CC) $(CFLAGS) -c src/main_demo.cpp

$(objects_data): src/main_data.cpp
	$(CC) $(CFLAGS) -c src/main_data.cpp

$(objects_demo): src/NaiveBayes.hpp
$(objects_data): src/NaiveBayes.hpp

clean:
	rm -f $(objects_demo) $(objects_data) $(program_demo) $(program_data)
