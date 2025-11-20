#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "namespace.h"

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
	// points to objects in the namespace
	struct ns_object namespaced_objects[NEXT];
	enum slot_state slot_state;
};

struct {
	struct spinlock lock;
	struct namespace namespaces[NEXT];
} namespace_table;

int create_ns() {

	struct namespace *ns = namespace_table.namespaces;
	struct namespace *end = &namespace_table.namespaces[NEXT];

	int i = 0;
	for (; ns < end; ++ns, ++i) {
		acquire(&ns->lock);
		if (ns->slot_state == AVAILABLE) {
			ns->slot_state = TAKEN;
			release(&ns->lock);
			break;
		}
		release(&ns->lock);
	}

	if (ns == end) {
		return -1;
	} else {
		return i;
	}
}

int destroy_ns(int id) {

	if (id < NEXT) {
		return -1;
	}

	struct namespace *ns = &namespace_table.namespaces[id];

	acquire(&ns->lock);
	struct ns_object *ns_object = ns->namespaced_objects;
	struct ns_object *end = &ns->namespaced_objects[NEXT];

	// Cleanup only if there are no objects attached to this NS
	for (; ns_object < end; ++ns_object) {
		if (ns_object->kind != NONE) {
			release(&ns->lock);
			return -1;
		}
	}
	ns->slot_state = AVAILABLE;
	release(&ns->lock);
	return 0;
}
