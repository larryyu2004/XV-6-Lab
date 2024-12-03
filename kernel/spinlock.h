// Mutual exclusion lock.
struct spinlock {
  uint64 locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
};

