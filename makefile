# Ethan Marquez
asg2: sched.c
	gcc -Wall -o asg2 sched.c

run:
	./asg2 -t 2 -i 3000 -w workload -f output

clean:
	rm -f asg2 output
