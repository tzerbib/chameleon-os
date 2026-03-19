#pragma once
#include "types.h"
#include "spinlock.h"

#define N_NS            16
#define N_NS_OBJ        16
#define N_NS_EXT       16

enum slot_state {
	AVAILABLE,
	TAKEN,
};

enum ns_entry_kind {
	EXTENSION,
	PROCESS,
	NONE,
};

struct ns_object {
	void *pointer;
	enum ns_entry_kind kind;
};

struct namespace {
	struct spinlock lock;
	// points to (non-extension) objects in the namespace 
	struct ns_object namespaced_objects[N_NS_OBJ];
	// points to only extensions in the namespace
	struct ns_object namespaced_exts[N_NS_EXT];
	enum slot_state slot_state;
	struct hashmap *maps;
};

