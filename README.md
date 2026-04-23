*This project has been created as part of the 42 curriculum by dgaillet.*

## Description

In this project:

- each **coder** is a thread,
- each coder alternates between:
  - **compiling**
  - **debugging**
  - **refactoring**
- compiling requires **two shared USB dongles**,
- each dongle is protected against concurrent access,
- the simulation ends when:
  - a coder **burns out** because they failed to start compiling before their deadline, or
  - every coder has compiled at least `number_of_compiles_required` times.

The goal of the project is to design a correct multithreaded simulation in C using `pthread`, while avoiding the usual concurrency pitfalls such as race conditions, deadlocks, starvation, and inconsistent timing behavior.

---

## Features

- One thread per coder
- One monitor thread for burnout detection
- Mutex-protected shared state
- Condition variables for blocking/wakeup around dongle availability
- Support for:
  - `fifo` scheduling
  - `edf` scheduling
- Dongle cooldown enforcement
- Deterministic state logging
- Clean shutdown on burnout or completion target
- No global variables
- Custom scheduling/arbitration logic suitable for priority queue integration

---

## Project Rules Summary

The program is launched with the following arguments:

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

### Arguments

- `number_of_coders`  
  Number of coder threads, and also number of dongles.

- `time_to_burnout`  
  Maximum time in milliseconds a coder can go without starting a new compile.

- `time_to_compile`  
  Duration of the compile phase in milliseconds.

- `time_to_debug`  
  Duration of the debug phase in milliseconds.

- `time_to_refactor`  
  Duration of the refactor phase in milliseconds.

- `number_of_compiles_required`  
  If every coder reaches this count, the simulation stops successfully.

- `dongle_cooldown`  
  Time in milliseconds during which a dongle remains unavailable after being released.

- `scheduler`  
  Arbitration policy:
  - `fifo`
  - `edf`

---

## Instructions

### Compilation

Build the project with:

```bash
make
```

Available Makefile rules:

```bash
make        # build codexion
make clean  # remove object files and dependency files
make fclean # remove object files, dependency files, and binary
make re     # rebuild everything
make debug  # build with debug symbols
make lint   # run norminette and clang checks
make format # format sources with c_formatter_42
```

### Execution

Example:

```bash
./codexion 5 800 200 200 200 3 50 fifo
```

Or with EDF scheduling:

```bash
./codexion 5 800 200 200 200 3 50 edf
```

### Output format

The program prints state changes using the required format:

```text
timestamp_in_ms X has taken a dongle
timestamp_in_ms X is compiling
timestamp_in_ms X is debugging
timestamp_in_ms X is refactoring
timestamp_in_ms X burned out
```

Example:

```text
0 1 has taken a dongle
1 1 has taken a dongle
1 1 is compiling
201 1 is debugging
401 1 is refactoring
1204 3 burned out
```

---

## Usage Notes

### Valid input

The program rejects invalid inputs such as:

- missing arguments,
- negative numbers,
- non-integer values,
- unsupported scheduler names,
- invalid zero/overflow-like values depending on the parser rules used in the implementation.

### Scheduler behavior

#### FIFO
Dongles grant access according to request arrival order.

#### EDF
Dongles favor the coder whose burnout deadline is the earliest:

```text
deadline = last_compile_start + time_to_burnout
```

This is intended to reduce the risk of burnout and improve liveness under feasible timing configurations.

---

## File Structure

From the provided Makefile, the project is organized around these source files:

```text
coders/main.c
coders/init.c
coders/parsing.c
coders/compile_process.c
coders/destroy.c
coders/thread_work.c
coders/utils.c
coders/burnout_checker.c
coders/config_utils.c
coders/time_utils.c
coders/codexion.h
Makefile
README.md
```

---

## Technical Overview

### Main components

- **Configuration structure**
  Holds simulation parameters and shared global-like state without using actual global variables.

- **Coder structure**
  Represents one worker thread with its own:
  - id
  - compile counters
  - timing data
  - synchronization lock

- **Dongle structure**
  Represents one shared resource with:
  - mutex
  - condition variable
  - cooldown timestamp
  - possible ownership/request metadata

- **Monitor thread**
  Continuously checks whether:
  - a coder has exceeded `time_to_burnout`,
  - or the compile quota has been reached by all coders.

---

## Blocking cases handled

This section describes the main concurrency issues addressed by the implementation.

### 1. Mutual exclusion on dongles

A dongle cannot be held by more than one coder at the same time.

**Handled by:**
- one `pthread_mutex_t` per dongle,
- protected state updates on acquisition/release.

This prevents resource duplication and inconsistent ownership.

---

### 2. Deadlock prevention

The classical dining philosophers deadlock happens when every thread grabs one resource and waits forever for the second.

This project must prevent that situation.

Typical prevention strategies include:
- ordering resource acquisition,
- releasing partial acquisitions when the second resource is not obtainable,
- using arbitration rules before granting access,
- letting dongles independently control who may proceed.

In this implementation, deadlock prevention is tied to the dongle arbitration model and controlled acquisition flow rather than naive “take left then take right and wait forever”.

---

### 3. Starvation prevention

A correct solution must not let one coder be postponed forever while others continue making progress.

**Handled by:**
- FIFO fairness when `scheduler == fifo`,
- deadline-aware priority when `scheduler == edf`,
- explicit priority checks during arbitration.

EDF is especially important when one coder is closer to burnout than another.

---

### 4. Dongle cooldown handling

A dongle cannot be immediately reused after release.

**Handled by:**
- storing the dongle’s `last_release` timestamp,
- refusing grants until the cooldown interval has elapsed,
- waiting via condition variables rather than busy spinning.

This adds a timing constraint that affects liveness and scheduling behavior.

---

### 5. Precise burnout detection

The simulation must print burnout within 10 ms of the actual burnout time.

**Handled by:**
- a dedicated monitor thread,
- periodic deadline checks,
- careful timestamp comparison using real time (`gettimeofday`-based utilities),
- immediate stop propagation once burnout is detected.

This is more reliable than expecting coder threads to self-detect burnout at the correct instant.

---

### 6. Log serialization

Thread outputs must never interleave.

**Handled by:**
- a mutex protecting all prints.

Without this, logs such as:

```text
123 2 is comp456 3 is debugging
```

could happen due to concurrent writes.

Serialized logging guarantees readable and valid output.

---

### 7. Shared state race conditions

Common shared variables include:
- simulation stop flag,
- compile counters,
- last compile timestamps,
- scheduler configuration,
- dongle ownership metadata.

**Handled by:**
- mutex-protected reads/writes,
- clearly scoped lock ownership,
- avoiding unsynchronized access to shared structures.

---

### 8. Single-coder edge case

When there is only one coder, only one dongle exists.

That coder can never acquire two dongles, so burnout must still be handled cleanly and logged correctly.

This edge case must not deadlock or crash.

---

## Thread synchronization mechanisms

This section explains the synchronization primitives used and how they cooperate.

### 1. `pthread_mutex_t`

Mutexes are the foundation of the synchronization model.

They are used to protect:

- each dongle’s internal state,
- each coder’s timing/counter data,
- shared configuration state,
- log output.

#### Example uses

- reading/updating `total_compile`
- checking or setting ownership of a dongle
- reading the simulation stop flag safely
- ensuring exactly one full log line is written at a time

### 2. `pthread_cond_t`

Condition variables are used so threads can sleep while waiting for a resource-related condition to become true.

Typical conditions include:

- a dongle becoming free,
- cooldown expiration,
- another coder releasing the resource,
- arbitration state changing.

This is preferable to active polling because it reduces useless CPU consumption and improves responsiveness.

### 3. Monitor-thread coordination

The monitor thread observes coders’ deadlines and shared completion state.

Safe communication between the monitor and coder threads is achieved through shared state protected by mutexes. The monitor can:

- detect burnout,
- mark the simulation as stopped,
- trigger the final state transition.

Coder threads regularly check this shared stop condition and exit cleanly when required.

### 4. Thread-safe communication patterns

The implementation relies on classic thread-safe patterns:

- **lock → read/update → unlock**
- **wait in a loop on condition variable**
- **re-check state after wakeup**
- **broadcast/signal when a resource state changes**

This avoids race conditions caused by:
- spurious wakeups,
- stale reads,
- partial state changes visible to other threads.

### 5. Race condition prevention example

Suppose two coders try to take the same dongle simultaneously.

Without a mutex:
- both could observe it as free,
- both could mark it as taken,
- resource ownership becomes invalid.

With a mutex:
- only one thread enters the critical section first,
- the second sees the updated state afterward,
- ownership remains consistent.

### 6. Logging example

Without synchronization:
- two threads could print at the same time,
- lines could overlap.

With a print mutex:
- one log line is emitted completely,
- then the next one starts.

This guarantees valid output formatting.

---

## Scheduling / Arbitration Notes

Your implementation includes scheduler-related helper logic in `utils.c`, including functions such as:

- `remain_compile(...)`
- `has_priority(...)`
- `create_dongle(...)`

### `remain_compile`

Computes how many compiles a coder still needs before reaching the required target.

### `has_priority`

Determines whether a coder should have priority for a dongle depending on the configured scheduler.

Behavior shown by the code:
- returns priority immediately under `FIFO`,
- under `EDF`, compares remaining time before burnout against the other requester/holder,
- includes a tie-break rule using remaining compile count.

This aligns with the project requirement that equal deadlines must still produce deterministic behavior.

### `create_dongle`

Allocates and initializes a dongle:
- id
- release timestamp
- mutex
- condition variable

It also safely cleans up if initialization fails.

---

## Timing Considerations

This project uses real-time measurements, typically based on `gettimeofday()`, which is allowed and suitable for the assignment.

Important timing-sensitive points include:

- compile start timestamps,
- cooldown expiration,
- burnout deadline calculation,
- monitor reactivity.

Because thread scheduling depends on the OS, exact micro-behavior may vary slightly, but the implementation should still satisfy the required burnout precision window.

---

## Limitations / Feasibility Notes

Even with correct synchronization, some parameter sets may be inherently infeasible.

For example, if:
- `time_to_burnout` is too small,
- compile/debug/refactor durations are too long,
- cooldown is too restrictive,

then no scheduler can prevent burnout for all coders.

Under feasible parameters, EDF should improve the chance of survival and preserve liveness better than a naive policy.

---

## Example Runs

### FIFO

```bash
./codexion 5 800 200 200 200 3 50 fifo
```

### EDF

```bash
./codexion 5 800 200 200 200 3 50 edf
```

### Single coder

```bash
./codexion 1 800 200 200 200 3 50 fifo
```

Expected behavior:
- the coder may take one dongle,
- cannot acquire a second,
- eventually burns out,
- burnout must be logged correctly.

---

## Resources

### Documentation and references

- POSIX Threads Programming
- `man pthread_create`
- `man pthread_join`
- `man pthread_mutex_init`
- `man pthread_cond_init`
- `man pthread_cond_wait`
- `man pthread_cond_timedwait`
- `man gettimeofday`
- EDF scheduling theory
- Coffman deadlock conditions

### AI usage

AI was used as an assistant for:
- clarifying the project requirements,
- reviewing synchronization design ideas,
- helping structure documentation,
- improving English phrasing in the README,
- discussing deadlock/starvation/cooldown edge cases.


