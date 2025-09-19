enum extstate { UNUSED, LOADED, ATTACHED };

struct extension {
  enum extstate state;         // extension state
  void* text;                  // Pointer to the extension code
  char name[16]                // extension name (debugging)
};
