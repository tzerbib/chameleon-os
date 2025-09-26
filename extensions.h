enum extstate { EXT_UNUSED, EXT_LOADED, EXT_ATTACHED };

struct extension {
  enum extstate state;         // extension state
  void* (*text)(void);         // Pointer to the extension code
  char name[16];               // extension name (debugging)
};

struct extension* ext_load(void* (*fn)(void), int n);
void ext_attach(struct extension* ext);
