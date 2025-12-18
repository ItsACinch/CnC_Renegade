/*
 * gnu_regex.h - Stub/compatibility header for GNU regex library
 *
 * The original Renegade build used GNU regex. This file provides
 * minimal stub definitions to allow compilation. A full implementation
 * would require either bundling GNU regex or adapting to use C++11 <regex>.
 *
 * Note: This stub allows compilation but regex functionality will not work.
 * For full functionality, GNU regex source should be added or the code
 * should be migrated to use std::regex.
 */

#ifndef GNU_REGEX_H
#define GNU_REGEX_H

#ifdef __cplusplus
extern "C" {
#endif

/* Syntax bits */
#define RE_CHAR_CLASSES             (1 << 0)
#define RE_CONTEXT_INDEP_ANCHORS    (1 << 1)
#define RE_CONTEXT_INDEP_OPS        (1 << 2)
#define RE_CONTEXT_INVALID_OPS      (1 << 3)
#define RE_DOT_NEWLINE              (1 << 4)
#define RE_DOT_NOT_NULL             (1 << 5)
#define RE_HAT_LISTS_NOT_NEWLINE    (1 << 6)
#define RE_INTERVALS                (1 << 7)
#define RE_LIMITED_OPS              (1 << 8)
#define RE_NEWLINE_ALT              (1 << 9)
#define RE_NO_BK_BRACES             (1 << 10)
#define RE_NO_BK_PARENS             (1 << 11)
#define RE_NO_BK_REFS               (1 << 12)
#define RE_NO_BK_VBAR               (1 << 13)
#define RE_NO_EMPTY_RANGES          (1 << 14)
#define RE_UNMATCHED_RIGHT_PAREN_ORD (1 << 15)
#define RE_NO_POSIX_BACKTRACKING    (1 << 16)
#define RE_NO_GNU_OPS               (1 << 17)
#define RE_DEBUG                    (1 << 18)
#define RE_INVALID_INTERVAL_ORD     (1 << 19)
#define RE_ICASE                    (1 << 20)
#define RE_CARET_ANCHORS_HERE       (1 << 21)
#define RE_CONTEXT_INVALID_DUP      (1 << 22)
#define RE_NO_SUB                   (1 << 23)

/* Type definitions */
typedef unsigned long int reg_syntax_t;

/* Pattern buffer structure */
struct re_pattern_buffer {
    unsigned char *buffer;
    unsigned long int allocated;
    unsigned long int used;
    reg_syntax_t syntax;
    char *fastmap;
    unsigned char *translate;
    unsigned long int re_nsub;
    unsigned int can_be_null : 1;
    unsigned int regs_allocated : 2;
    unsigned int fastmap_accurate : 1;
    unsigned int no_sub : 1;
    unsigned int not_bol : 1;
    unsigned int not_eol : 1;
    unsigned int newline_anchor : 1;
};

typedef struct re_pattern_buffer regex_t;

/* Registers structure for subexpression matches */
struct re_registers {
    unsigned int num_regs;
    int *start;
    int *end;
};

typedef struct re_registers regmatch_t;

/* Error codes */
#define REG_NOERROR     0
#define REG_NOMATCH     1
#define REG_BADPAT      2
#define REG_ECOLLATE    3
#define REG_ECTYPE      4
#define REG_EESCAPE     5
#define REG_ESUBREG     6
#define REG_EBRACK      7
#define REG_EPAREN      8
#define REG_EBRACE      9
#define REG_BADBR       10
#define REG_ERANGE      11
#define REG_ESPACE      12
#define REG_BADRPT      13
#define REG_EEND        14
#define REG_ESIZE       15
#define REG_ERPAREN     16

/* Global syntax variable */
static reg_syntax_t re_syntax_options = 0;

/* Stub function declarations - these return failure/no-match */
static inline const char *re_compile_pattern(const char *pattern, int length,
                                              struct re_pattern_buffer *buffer) {
    (void)pattern;
    (void)length;
    (void)buffer;
    return "regex stub: compilation not supported";
}

static inline int re_match(struct re_pattern_buffer *buffer, const char *string,
                           int length, int start, struct re_registers *regs) {
    (void)buffer;
    (void)string;
    (void)length;
    (void)start;
    (void)regs;
    return -1; /* No match */
}

static inline int re_search(struct re_pattern_buffer *buffer, const char *string,
                            int length, int start, int range,
                            struct re_registers *regs) {
    (void)buffer;
    (void)string;
    (void)length;
    (void)start;
    (void)range;
    (void)regs;
    return -1; /* Not found */
}

static inline reg_syntax_t re_set_syntax(reg_syntax_t syntax) {
    reg_syntax_t old = re_syntax_options;
    re_syntax_options = syntax;
    return old;
}

static inline void regfree(regex_t *preg) {
    if (preg && preg->buffer) {
        /* Would free allocated memory in real implementation */
        preg->buffer = 0;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* GNU_REGEX_H */
