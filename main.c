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

void objectPrint(Object* object) {
  switch (object->type) {
    case OBJ_INT:
      printf("%d", object->value);
      break;

    case OBJ_PAIR:
      printf("(");
      objectPrint(object->head);
      printf(", ");
      objectPrint(object->tail);
      printf(")");
      break;
  }
}

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
  vm->maxObj = 8;
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
  Object** object = &vm->objectLLHead;
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
  int numObj = vm->numObj;

  markAll(vm);
  sweep(vm);

  vm->maxObj = vm->numObj * 2;

  printf("Collected %d objects, %d remaining.\n", numObj - vm->numObj,
         vm->numObj);
}

Object* newObject(VM* vm, ObjectType type) {
  if (vm->numObj == vm->maxObj) gc(vm);

  Object* object = malloc(sizeof(Object));
  object->type = type;
  object->marked = 0; 


  object->next = vm->objectLLHead;
  vm->objectLLHead = object;
  vm->numObj++;
  return object;
}

void pushInt(VM* vm, int intValue) {
  Object* object = newObject(vm, OBJ_INT);
  object->value = intValue;
  objectPrint(object);
  push(vm, object);
}

Object* pushPair(VM* vm) {
  Object* object = newObject(vm, OBJ_PAIR);
  object->tail = pop(vm);
  object->head = pop(vm);
  objectPrint(object);
  push(vm, object);
  return object;
}



void freeVM(VM *vm) {
  vm->stackSize = 0;
  gc(vm);
  free(vm);
}


void test1() {
  printf("Test 1: Objects on stack are preserved.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);

  gc(vm);
  gc(vm);
  assert(vm->numObj == 2, "Should have preserved objects.");
  freeVM(vm);
}

void test2() {
  printf("Test 2: Unreached objects are collected.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pop(vm);
  pop(vm);

  gc(vm);
  assert(vm->numObj == 0, "Should have collected objects.");
  freeVM(vm);
}

void test3() {
  printf("Test 3: Reach nested objects.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  pushPair(vm);
  pushInt(vm, 3);
  pushInt(vm, 4);
  pushPair(vm);
  pushPair(vm);

  gc(vm);
  assert(vm->numObj == 7, "Should have reached objects.");
  freeVM(vm);
}

void test4() {
  printf("Test 4: Handle cycles.\n");
  VM* vm = newVM();
  pushInt(vm, 1);
  pushInt(vm, 2);
  Object* a = pushPair(vm);
  pushInt(vm, 3);
  pushInt(vm, 4);
  Object* b = pushPair(vm);

  /* Set up a cycle, and also make 2 and 4 unreachable and collectible. */
  a->tail = b;
  b->tail = a;

  gc(vm);
  assert(vm->numObj == 4, "Should have collected objects.");
  freeVM(vm);
}

void perfTest() {
  printf("Performance Test.\n");
  VM* vm = newVM();

  for (int i = 0; i < 1000; i++) {
    for (int j = 0; j < 20; j++) {
      pushInt(vm, i);
    }

    for (int k = 0; k < 20; k++) {
      pop(vm);
    }
  }
  freeVM(vm);
}

int main(int argc, const char * argv[]) {
  test3();
  // test2();
  // test3();
  // test4();
  // perfTest();

  return 0;
}