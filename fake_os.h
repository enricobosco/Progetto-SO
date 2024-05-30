#include "fake_process.h"
#include "linked_list.h"
#pragma once

typedef struct {
  ListItem list;
  int pid;
  ListHead events;
  float quantum_prediction;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args, int i);

typedef struct FakeOS{
  FakePCB** running;
  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
  int NUM_CPU;
  ListHead processes;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
