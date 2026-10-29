CC     = gcc
CFLAGS = -Wall -Wextra
TARGET = asg2

all: $(TARGET)

asg2: sched.c
	$(CC) $(CFLAGS) sched.c -o asg2

clean:
	rm -f $(TARGETS)
