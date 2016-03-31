F = 
CC = g++
CFLAGS = -Wall $(F)
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = $(SRCS:.cpp=)
TESTCASE = input.txt

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $< -o $@

%.o: %.cpp
	$(CC) $< $(CFLAGS) -c

clean:
	@rm -rf $(EXEC) $(OBJS) *~

run:
	@./$(EXEC) < $(TESTCASE)
