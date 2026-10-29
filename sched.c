// Ethan Maarquez

// Compile on SOC
// Submit as asg2.tar.gz containing sched.c & makefile
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RR 0
#define SJF 1

struct task{
  int
    task_id,
    arrival_time,
    service_time,
    remaining_time,
    completion_time,
    response_time,
    wait_time;
  struct task *next;
  struct task *qnext;
};

// Adding task to back of queue
void enqueue(struct task **queue, struct task *t){
  struct task *p;

  t->qnext = NULL;
  if (*queue == NULL){
    *queue = t;
    return;
  }

  for (p = *queue; p->qnext != NULL; p = p->qnext);

  p->qnext = t;
}

int main(int argc, char *argv[]){
  int time_slice = 1;
  int start_id = 1000;
  char *outputFile = NULL;

  // -t 2: -(any int) time slice, specified in seconds
  // -i 3000: -(any large int) starting task ids
  // -f filename: -output file that contains all output data
  int opt;
  while ((opt = getopt(argc, argv, "t:i:f:")) != -1){
    switch (opt){
      case 't': time_slice = atoi(optarg); break;
      case 'i': start_id = atoi(optarg); break;
      case 'f': outputFile = optarg; break;
      default:
        printf("Error in usage: ./asg2 -t [small-int] -i [1000 * sinlge-digit-int] -f [output-file]\n");
    }
  }

  // Input Reading w/ input redirection
  // (arrival time of task, service time)
  FILE *output = fopen(outputFile, "w");

  // Read workload into linked list
  struct task *head = NULL, *tail = NULL, *t;
  int arrival, service;
  int n = 0;

  while (fscanf(stdin, "%d %d", &arrival, &service) == 2){
    t = malloc(sizeof(struct task));
    t->task_id = start_id + n;
    t->arrival_time = arrival;
    t->service_time = service;
    t->next = NULL;

    if (head == NULL) head = t;
    else tail->next = t;
    tail = t;
    n++;
  }

  // Round Robbin
  double avg_response[2];
  int switches[2];
  int alg;

  for (alg = RR; alg <= SJF; alg++){
    struct task *queue = NULL;
    struct task *cpu = NULL;
    struct task *prev, *p;
    int time = 0, done = 0, used = 0;
    double sum = 0;

    switches[alg] = 0;
    for (t = head; t != NULL; t = t->next)
      t->remaining_time = t->service_time;

    if (alg == RR) fprintf(output, "Round Robin Scheduling Results\n");
    else fprintf(output, "\nPreemptive Shortest Job First Scheudling Results\n");
    fprintf(output, "%-6s %-8s %-10s %-13s %s\n", "time", "cpu", "serv time", "remain time", "ready queue");
    fprintf(output, "-------------------------------------------------------------\n");

    while (done < n){
      for (t = head; t != NULL; t = t->next)
        if (t->arrival_time == time) enqueue(&queue, t);

      // SJF preempt
      if (alg == SJF && cpu != NULL){
        for(t = queue; t != NULL; t = t->qnext){
          if (t->remaining_time < cpu->remaining_time){
            enqueue(&queue, cpu);
            cpu = NULL;
            break;
          }
        }
      }

      if (cpu == NULL && queue != NULL){
        cpu = queue;
        prev = NULL;
        if (alg == SJF){
          for (p = NULL, t = queue; t != NULL; p = t, t = t->qnext){
            if (t->remaining_time < cpu->remaining_time){
              cpu = t;
              prev = p;
            }
          }
        }

        // unlink from queue
        if (prev == NULL) queue = cpu->qnext;
        else prev->qnext = cpu->qnext;
        cpu->qnext = NULL;

        used = 0;
        switches[alg]++;
      }

      if (cpu != NULL) fprintf(output, "%-6d %-8d %-10d %-13d ", time, cpu->task_id, cpu->service_time, cpu->remaining_time - 1);
      else fprintf(output, "%-6d %-8s %-10s %-13s ", time, "-", "-", "-");

      if (queue == NULL) fprintf(output, "-");
      for (t = queue; t != NULL; t = t->qnext)
        fprintf(output, "%d-%d%s", t->task_id, t->remaining_time, (t->qnext != NULL) ? ", " : "");
      fprintf(output, "\n");

      // run task for one tick
      if (cpu != NULL){
        cpu->remaining_time--;
        used++;

        if (cpu->remaining_time == 0){
          cpu->completion_time = time + 1;
          cpu->response_time = cpu->completion_time - cpu->arrival_time;
          cpu->wait_time = cpu->response_time - cpu->service_time;
          done++;
          cpu = NULL;
        }
        else if (alg == RR && used == time_slice){
          enqueue(&queue, cpu);
          cpu = NULL;
        }
      }
      time++;
    }

    // statistics table
    fprintf(output, "\n%-8s %-10s %-14s %-12s %-10s\n", "tid", "serv time", "complete time", "resp time", "wait time");
    fprintf(output, "--------------------------------------------------\n");
    for (t = head; t != NULL; t = t->next){
      fprintf(output, "%-8d %-10d %-14d %-12d %-10d\n", t->task_id, t->service_time, t->completion_time, t->response_time, t->wait_time);
      sum += t->response_time;
    }
    avg_response[alg] = sum / n;
  }


  // Compare for best alg
  char *better = (avg_response[RR] <= avg_response[SJF]) ? "RR" : "SJF";
  fprintf(output, "\nBest algorithm for this mix of tasks is: %s\n", better);
  fprintf(output, "\nAverage response time for RR:  %.2f\n", avg_response[RR]);
  fprintf(output, "Average response time for SJF: %.2f\n", avg_response[SJF]);
  fprintf(output, "\nContext switches for RR:  %d\n", switches[RR]);
  fprintf(output, "Context switches for SJF: %d\n", switches[SJF]);
  fprintf(output, "\n%s algorithm is better for this mix of jobs.\n", better);
  fprintf(output, "\n(Optimal algorithm depends on the set of tasks in the input file)\n");

  // Cleaning
  while (head != NULL){
    t = head;
    head = head->next;
    free(t);
  }

  fclose(output);
  return 0;
}
