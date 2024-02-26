#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
  int quantum;
} SchedSJFArgs;

void schedSJF(FakeOS* os, void* args_){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;
    
  ListItem* aux=os->ready.first;
  FakePCB* pcb=(FakePCB*) aux;
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  int min = e->duration;
  aux = aux->next;
  while (aux) {
	  FakePCB* pcb=(FakePCB*) aux;
	  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
	  if (e->duration < min) {
		  min = e->duration;
	  }
	  aux = aux->next;
  }
  
  aux=os->ready.first;
  while (aux) {
	  FakePCB* pcb=(FakePCB*) aux;
	  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
	  if (e->duration == min) {
		  break;
	  }
	  aux = aux->next;
  }
  os->running=pcb;
  
  assert(pcb->events.first);
  e = (ProcessEvent*)pcb->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>args->quantum) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }
};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJFArgs srr_args;
  srr_args.quantum=5;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedSJF;
  
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
  while(os.running
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}