#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
  int quantum;
  float a;
} SchedSJFArgs;

void schedSJF(FakeOS* os, void* args_, int i){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;
    
  ListItem* aux=os->ready.first;
  FakePCB* pcb=(FakePCB*) aux;
  FakePCB* pcb1=(FakePCB*) aux;
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  int min = e->duration;
  
  while (aux) {
	  pcb=(FakePCB*) aux;
	  e = (ProcessEvent*)pcb->events.first;
	  if (e->duration < min && e->type == CPU) {
		  min = e->duration;
		  pcb1 = (FakePCB*) List_find(&os->ready, (ListItem*) pcb);
	  }
	  aux = aux->next;
  }
  pcb = (FakePCB*) List_detach(&os->ready, (ListItem*) pcb1);
  os->running[i]=pcb;
  
  assert(pcb->events.first);
  e = (ProcessEvent*)pcb->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (pcb->quantum_prediction == 0) {
	  pcb->quantum_prediction = args->quantum;
  }
  
  if (e->duration>pcb->quantum_prediction) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=pcb->quantum_prediction;
    e->duration-=pcb->quantum_prediction;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }
  printf("%f\n", pcb->quantum_prediction);
  pcb->quantum_prediction = args->a * e->duration + (1-args->a) * pcb->quantum_prediction;
  printf("%f\n", pcb->quantum_prediction);
};

int main(int argc, char** argv) {
  int n;
  printf("Inserisci numero CPU: ");
  scanf("%d", &n);
  os.NUM_CPU = n;
  FakeOS_init(&os);
  SchedSJFArgs srr_args;
  //int n;
  printf("Inserisci quantum: ");
  scanf("%d", &srr_args.quantum);
  printf("Inserisci a: ");
  scanf("%f", &srr_args.a);
  //printf("Inserisci numero CPU: ");
  //scanf("%d", &n);
  os.schedule_args=&srr_args;
  os.schedule_fn=schedSJF;
  //os.NUM_CPU = n;

  
  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  while(os.running[0]
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
  printf("FINISHED\n");
}
