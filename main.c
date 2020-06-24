#include <stdio.h>
#include <stdlib.h>

#define STACK_MAX 256

typedef enum { 
  OBJ_INT, OBJ_PAIR
} ObjectType;

typedef struct sObject {
  unsigned char marked;
  ObjectType type;
  struct sObject* next;

  union {
    int value;

    struct {
      struct sObject* head;
      struct sObject* tail;
    };
  };
} Object;

typedef struct { 
  Object *objectLLHead;
  Object *stack[STACK_MAX];
  int numObj; 
  int maxObj;
  int stackSize;
} VM;

void assert(int condition, const char* message) {
  if (!condition) {
    printf("%s\n", message);
    exit(1);
  }
}

VM* newVM() { 
  VM* vm = malloc(sizeof(VM));
  vm->stackSize = 0; 
  vm->numObj = 0; 
  vm->maxObj = 5;
  return vm; 
}

void push(VM* vm, Object* value) { 
  assert(vm->stackSize < STACK_MAX, "Overflow!");
  vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm) {
  assert(vm->stackSize > 0, "Underflow!");
  return vm->stack[--vm->stackSize];
}

Object* newObject(VM* vm, ObjectType type) {
  if (vm->numObj == vm->maxObj) gc(vm);

  Object* object = malloc(sizeof(Object));
  object->type = type;
  object->marked = 0; 


  object->next = vm->objectLLHead;
  vm->objectLLHead = object;
  vm->numObjects++;
  return object;
}

void pushInt(VM* vm, int intValue) {
  Object* object = newObject(vm, OBJ_INT);
  object->value = intValue;
  push(vm, object);
}

Object* pushPair(VM* vm) {
  Object* object = newObject(vm, OBJ_PAIR);
  object->tail = pop(vm);
  object->head = pop(vm);

  push(vm, object);
  return object;
}

void mark(Object* object) {
  if (object->marked) return;

  object->marked = 1;

  if (object->type == OBJ_PAIR) {
    mark(object->head);
    mark(object->tail);
  }
}

void markAll(VM* vm)
{
  for (int i = 0; i < vm->stackSize; i++) {
    mark(vm->stack[i]);
  }
}

void sweep(VM* vm)
{
  Object** object = &vm->firstObject;
  while (*object) {
    if (!(*object)->marked) { //cant reach object, i.e. no reference in program
      Object* unreached = *object;

      *object = unreached->next;
      free(unreached);
      vm->numObj--;
    } else { 
      (*object)->marked = 0;
      object = &(*object)->next;
    }
  }
}

void gc(VM* vm) {
  markAll(vm);
  sweep(vm);
  
  vm->maxObjects = vm->numObjects * 2;
}