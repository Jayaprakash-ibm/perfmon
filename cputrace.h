#ifndef CPUTRACE_H
#define CPUTRACE_H

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/perf_event.h>

#define CPUTRACE_MAX_ANCHORS 128

struct HW_conf {
    bool capture_swi;
    bool capture_cyc;
    bool capture_cmiss;
    bool capture_bmiss;
    bool capture_ins;
};

struct HW_ctx {
    int fd_swi;
    int fd_cyc;
    int fd_cmiss;
    int fd_bmiss;
    int fd_ins;
    struct HW_conf conf;
};

struct HW_measure {
    long long swi;
    long long cyc;
    long long cmiss;
    long long bmiss;
    long long ins;
};

struct ArenaRegion {
    void* start;
    void* end;
    void* current;
    struct ArenaRegion* next;
};

struct Arena {
    struct ArenaRegion* regions;
    struct ArenaRegion* current_region;
    bool growable;
};

struct cputrace_anchor {
    const char* name;
    pthread_mutex_t mutex;
    struct Arena* results_arena;
    uint64_t call_count;
    uint64_t sum_swi;
    uint64_t sum_cyc;
    uint64_t sum_cmiss;
    uint64_t sum_bmiss;
    uint64_t sum_ins;
};

enum cputrace_result_type {
    CPUTRACE_RESULT_SWI = 0,
    CPUTRACE_RESULT_CYC = 1,
    CPUTRACE_RESULT_CMISS = 2,
    CPUTRACE_RESULT_BMISS = 3,
    CPUTRACE_RESULT_INS = 4,
    CPUTRACE_RESULT_LAST = 5
};

struct cputrace_result {
    enum cputrace_result_type type;
    uint64_t value;
};

struct cputrace_profiler {
    struct cputrace_anchor* anchors;
    bool profiling;
    pthread_mutex_t file_mutex;
};

void HW_init(struct HW_ctx* ctx, struct HW_conf* conf);
void HW_start(struct HW_ctx* ctx);
void HW_stop(struct HW_ctx* ctx, struct HW_measure* measure);
void HW_clean(struct HW_ctx* ctx);

struct Arena* arena_create(size_t size, bool growable);
void* arena_alloc(struct Arena* arena, size_t size);
void arena_destroy(struct Arena* arena);

void cputrace_start(void);
void cputrace_stop(void);
void cputrace_reset(void);
void cputrace_dump(void);
void cputrace_close(void);

struct HW_profile {
    struct HW_ctx ctx;
    const char* function;
    uint64_t index;
    uint64_t flags;

    HW_profile(const char* function, uint64_t index, uint64_t flags);
    ~HW_profile();
};

enum HW_profile_flags {
    HW_PROFILE_SWI = 1,
    HW_PROFILE_CYC = 2,
    HW_PROFILE_CMISS = 4,
    HW_PROFILE_BMISS = 8,
    HW_PROFILE_INS = 16
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define HWProfileFunction(variable, label) \
    struct HW_profile variable(label, (uint64_t)(__COUNTER__ + 1), HW_PROFILE_CYC)
#define HWProfileFunctionF(variable, label, flags) \
    struct HW_profile variable(label, (uint64_t)(__COUNTER__ + 1), flags)

#endif // CPUTRACE_H