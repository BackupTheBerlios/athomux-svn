/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#ifndef _COMMON_H
#define _COMMON_H

// general constansts

#define DEFAULT_TRANSFER 4096

#define SECT_MAX 4

// compiling mode dependent things

#ifdef DEBUG
void open_debug();
extern int call_level;
extern char blanks[32];
#endif

#ifdef TRACE
#define trace(str,myargs...) printf("%smand(%d) %s(%s): " str, blanks+31-call_level, _brick->_mand, OPERATION, op_names[_args->op_code], ##myargs)
#define rawtrace(str,args...) printf("%s%s: " str, blanks+31-call_level, OPERATION, ##args)
#define DEBUG_EXIT exit(-1)
#else
#define trace(args...) /*empty*/
#define rawtrace(args...) /*empty*/
#define DEBUG_EXIT /**/
#endif

#ifdef SILENT
#define printf(args...) /*empty*/
#endif

#if 1 // testing
#include <string.h> // memset()
#include <stdio.h> // only for printf()
#include <stdlib.h> // to be removed!!!!!
#ifndef __KERNEL__
void exit(int status);
#endif
#else
#define NULL (void*)0
#endif

#define MAKE_PTR(x)              (void*)(x)
#define MAKE_PADDR(x)            (paddr_t)(x)
#define STATIC_OFFSET(type,field)       ((unsigned long)&(((type*)0)->field))
#define DEREF(ptr,type,field)    (((type*)(ptr))->field)
#define BASE(ptr,basetype,field) ((basetype*)(((char*)(ptr))-STATIC_OFFSET(basetype,field)))
#define GEN_DEREF(expr,gentype,offset,type) (*(type*)(((char*)(expr))+(offset)))
#define STUPID_MAX(a,b) ((a) > (b) ? (a) : (b))


#define PACKED __attribute__((packed))
#define PURE   __attribute__((pure))
#define CONST   __attribute__((const))

// hardware-specific basic types, use only in device drivers!

typedef char                       int1;
typedef unsigned char              uns1;
typedef short                      int2;
typedef unsigned short             uns2;
typedef int                        int4;
typedef unsigned                   uns4;
typedef long long                  int8;
typedef unsigned long long         uns8;

// model-dependent basic types
// currently only 64bit

typedef enum {
  FALSE = 0,
  TRUE = 1,
} PACKED bool;

typedef enum {
  direct_read,
  direct_write,
  direct_stop
} PACKED direction_t;

typedef enum {
  prio_none,
  prio_background,
  prio_normal,
  prio_urgent
} PACKED prio_t;

typedef enum {
  vers_undef,
  vers_old,
  vers_newest
} PACKED version_t;

typedef enum {
  lock_none,
  lock_read,
  lock_write,
} PACKED lock_t;

typedef enum {
  action_ask,
  action_try,
  action_wait,
} PACKED action_t;

typedef enum {
  // OUTPUT OPERATIONS
  opcode_output_init,
  // static
  opcode_trans,
  opcode_wait,
  opcode_get,
  opcode_put,
  opcode_lock,
  opcode_unlock,
  opcode_gadr,
  opcode_padr,
  // dynamic
  opcode_create,
  opcode_delete,
  opcode_move,
  // strategy ops
  opcode_instbrick,
  opcode_deinstbrick,
  opcode_instconn,
  opcode_deinstconn,
  opcode_connect,
  opcode_disconnect,
  opcode_getconn,
  opcode_findconn,
  // combinations
  opcode_transwait,
  opcode_gettranswait,
  opcode_transwaitput,
  opcode_putwait,
  opcode_createget,
  opcode_gadrcreateget,
  opcode_gadrgettranswait,
  opcode_putpadr,
  opcode_putdelete,
  opcode_putdeletepadr,
  opcode_gadrtranswaitdeletepadr,
  opcode_gadrcreatetranswaitpadr,
  // last dummy index
  opcode_output_max,
  // INPUT OPERATIONS
  opcode_input_init,
  opcode_retract,
  opcode_input_max,
  // BRICK OPERATIONS
  opcode_brick_init,
  opcode_brick_max,
} PACKED op_t;

typedef uns8      addr_t;
typedef uns8      len_t;
typedef int8      offs_t;
typedef unsigned long paddr_t;
typedef unsigned long plen_t;
typedef long          poffs_t;
typedef uns4      mand_t;
typedef int       index_t;
typedef bool      success_t;
typedef char      sname_t[8];
typedef char      name_t[24];
typedef char      lname_t[32];

/* Helper struct for strategy ops:
 * Representation of lists of connected inputs
 */
struct conn_info {
  addr_t conn_addr;
  index_t conn_index;
  sname_t conn_name;
};

/* The main args structure.
 * Used at all operations for parameter passing.
 */
struct args {
  addr_t log_addr;
  len_t  log_len;
  addr_t try_addr;
  len_t  try_len;
  offs_t offset;
  offs_t offset_max;
  paddr_t phys_addr;
  plen_t  phys_len;
  mand_t mandate;
  bool   forwrite;
  bool   clear;
  bool   melt;
  direction_t direction;
  prio_t prio;
  bool   where;
  bool   exclu;
  lock_t data_lock;
  lock_t addr_lock;
  action_t action;
  bool   destr;
  bool   constr;
  // results
  success_t success;
  version_t version;
  // polymorphism
  op_t    op_code;
  index_t sect_code;
  // strategy args
  name_t name;
  struct conn_info * conn1;
  struct conn_info * conn2;
  struct conn_info * res_conn;
  index_t conn_len;
};

struct input;
struct output;
union connector;

typedef void static_operation(const union connector * on, struct args * args, const char * param);
typedef static_operation (*operation);

typedef const operation output_operation_set[opcode_output_max];
typedef const operation input_operation_set[opcode_input_max-opcode_output_max-1];
typedef const operation brick_operation_set[opcode_brick_max-opcode_input_max-1];

typedef void (*init_conn)(void * _ini, void * _brick);

struct output {
  output_operation_set * ops;
  struct input * rev_chain;
};

struct input {
  input_operation_set * ops;
  struct output * connect;
  struct input * rev_next;
};

union connector {
  struct input input;
  struct output output;
};

/////////////////////////////////////////////////////////////////////////

/* This is automatically inserted for unimplemented operations
 * in order to avoid crashes due to null-pointer dereferences.
 * It also becomes relevant if e.g. static nests are erreanously
 * used as dynamic ones or similar.
 * Rule of thumb: better implement your own default operation
 * returning your own error instead of indirectly using this.
 */

#define DEF_OPERATION(sect)                                                   \
static_operation missing_##sect##_output_init;                                \
static_operation missing_##sect##_trans;                                   \
static_operation missing_##sect##_wait;                                       \
static_operation missing_##sect##_get;                                        \
static_operation missing_##sect##_put;                                        \
static_operation missing_##sect##_lock;                                       \
static_operation missing_##sect##_unlock;                                     \
static_operation missing_##sect##_gadr;                                    \
static_operation missing_##sect##_padr;                                    \
                                                                              \
static_operation missing_##sect##_create;                                     \
static_operation missing_##sect##_delete;                                     \
static_operation missing_##sect##_move;                                       \
                                                                              \
static_operation missing_##sect##_instbrick;                                  \
static_operation missing_##sect##_deinstbrick;                                \
static_operation missing_##sect##_instconn;                                   \
static_operation missing_##sect##_deinstconn;                                 \
static_operation missing_##sect##_connect;                                    \
static_operation missing_##sect##_disconnect;                                 \
static_operation missing_##sect##_getconn;                                    \
static_operation missing_##sect##_findconn;                                   \
                                                                              \
static_operation missing_##sect##_transwait;                                         \
static_operation missing_##sect##_gettranswait;                                      \
static_operation missing_##sect##_transwaitput;                                      \
static_operation missing_##sect##_putwait;                                    \
static_operation missing_##sect##_createget;                                  \
static_operation missing_##sect##_gadrcreateget;                           \
static_operation missing_##sect##_gadrgettranswait;                               \
static_operation missing_##sect##_putpadr;                                 \
static_operation missing_##sect##_putdelete;                                  \
static_operation missing_##sect##_putdeletepadr;                           \
static_operation missing_##sect##_gadrtranswaitdeletepadr;                     \
static_operation missing_##sect##_gadrcreatetranswaitpadr;                     \
                                                                              \
static_operation missing_##sect##_input_init;                                 \
static_operation missing_##sect##_retract;                                    \
                                                                              \
static_operation missing_##sect##_brick_init;                                 \

DEF_OPERATION(0)
DEF_OPERATION(1)
DEF_OPERATION(2)
DEF_OPERATION(3)

#define MAKE_ALIAS(prefix,name,sect)                                          \
  static_operation prefix##_##sect##_##name __attribute__((alias(#prefix"_"#name)));

#define MAKE_ALL_ALIAS(name)                                                  \
  MAKE_ALIAS(missing,name,0)                                                  \
  MAKE_ALIAS(missing,name,1)                                                  \
  MAKE_ALIAS(missing,name,2)                                                  \
  MAKE_ALIAS(missing,name,3)                                                  \

MAKE_ALL_ALIAS(output_init)
// static ops
MAKE_ALL_ALIAS(trans)
MAKE_ALL_ALIAS(wait)
MAKE_ALL_ALIAS(get)
MAKE_ALL_ALIAS(put)
MAKE_ALL_ALIAS(lock)
MAKE_ALL_ALIAS(unlock)
MAKE_ALL_ALIAS(gadr)
MAKE_ALL_ALIAS(padr)
// strategy ops
MAKE_ALL_ALIAS(instbrick)
MAKE_ALL_ALIAS(deinstbrick)
MAKE_ALL_ALIAS(instconn)
MAKE_ALL_ALIAS(deinstconn)
MAKE_ALL_ALIAS(connect)
MAKE_ALL_ALIAS(disconnect)
MAKE_ALL_ALIAS(getconn)
MAKE_ALL_ALIAS(findconn)
// dynamic ops
MAKE_ALL_ALIAS(create)
MAKE_ALL_ALIAS(delete)
MAKE_ALL_ALIAS(move)
// combinations
MAKE_ALL_ALIAS(transwait)
MAKE_ALL_ALIAS(gettranswait)
MAKE_ALL_ALIAS(transwaitput)
MAKE_ALL_ALIAS(putwait)
MAKE_ALL_ALIAS(createget)
MAKE_ALL_ALIAS(gadrcreateget)
MAKE_ALL_ALIAS(gadrgettranswait)
MAKE_ALL_ALIAS(putpadr)
MAKE_ALL_ALIAS(putdelete)
MAKE_ALL_ALIAS(putdeletepadr)
MAKE_ALL_ALIAS(gadrtranswaitdeletepadr)
MAKE_ALL_ALIAS(gadrcreatetranswaitpadr)

MAKE_ALL_ALIAS(input_init)
MAKE_ALL_ALIAS(retract)

MAKE_ALL_ALIAS(brick_init)

/////////////////////////////////////////////////////////////////////////

extern name_t op_names[opcode_brick_max+1];

/////////////////////////////////////////////////////////////////////////

/* The Pointer Cache (PC) infrastructure
 */

#define PC__DIRTY TRUE
#define PC__PRESENT 2
#ifdef __KERNEL__
#ifdef __i386__
#include <asm/div64.h>
#define PC__HASH(addr,align,max) ({addr_t addr_tmp = addr; do_div(addr_tmp, align); addr_tmp & (max-1);})
#else
#define PC__HASH(addr,align,max) (((addr) / (align)) & ((max)-1))
#endif
#define PC__BASE_ADDR2(addr,whole) (addr & ~((len_t)(whole)-1))
#define PC__BASE_ADDR(addr,whole) (PC__BASE_ADDR2(addr,whole) | PC__PRESENT)
#define PC__BASE_OFFSET(addr,whole) (addr & ((whole)-1))
#define PC__BASE_LEN(offset,len,whole) (((offset + len - 1) & ~((whole)-1)) + (whole))
#else
#define PC__HASH(addr,align,max) (((addr) / (align)) % (max))
#define PC__BASE_ADDR2(addr,whole) ((addr) / (whole) * (whole))
#define PC__BASE_ADDR(addr,whole) (PC__BASE_ADDR2(addr,whole) | PC__PRESENT)
#define PC__BASE_OFFSET(addr,whole) (addr % (whole))
#define PC__BASE_LEN(offset,len,whole) (((offset + len - 1) / (whole) * (whole)) + (whole))
#endif

struct pc_elem {
  addr_t pc_addr;
  plen_t pc_len;
  void * pc_ptr;
};

struct pc {
  struct input * pc_input;
  plen_t pc_version;
  index_t pc_sect;
  bool pc_write;
  struct pc_elem pc_elem[0]; // dynamic array
};

void _PC_PUT(struct pc * pc, struct pc_elem * elem);
void  _PC_GET(struct pc * pc, struct pc_elem * elem, addr_t __addr, plen_t __len);
void _PC_ALL_PUT(struct pc * pc, index_t max);
struct pc_elem * _PC_ALLOC(struct pc * pc, plen_t __len, plen_t align, plen_t whole, plen_t max);
void  _PC_FREE(struct pc * pc, struct pc_elem * elem, addr_t __addr, plen_t __len);

/////////////////////////////////////////////////////////////////////////

/* Cyclic ring list infrastructure
 */

struct link {
  addr_t l_next;
  addr_t l_prev;
};

struct empty {};

/////////////////////////////////////////////////////////////////////////

/* Generic Types
 */

struct gen_tabentry {
  const char * gen_name;
  const char * gen_type;
  index_t gen_offset;
  index_t gen_len;
};

/////////////////////////////////////////////////////////////////////////

/* Simple provisionary brick type descriptors for static code linking
 * with control_dummy_linux
 * This is a hack! Not intended for full-fledged dynamic linking.
 * Probably we need different concepts for the latter.
 */

struct gen_type {
  const char * gen_typename;
  const char * gen_string;
  const struct gen_tabentry * gen_table;
  index_t gen_offset;
};

extern const struct gen_type type_empty[1];

struct load_conn {
  sname_t name;
  const char * static_info;
  const struct gen_type * gen_type;
  const init_conn init_code;
  const init_conn exit_code;
  bool type;
  index_t start_index;
  index_t count;
  poffs_t offset;
  index_t sect_count;
  plen_t size;
};

struct loader {
  name_t name;
  addr_t magic;
  const char * static_info;
  const char * full_info;
  plen_t instance_size;
  index_t conn_count;
  index_t conn_totalcount;
  const struct load_conn * conn;
  void (*static_init)(void); // called upon code loading
  void (*static_exit)(void); // called upon code unloading, NYI!!
  mand_t (*dynamic_init)(void*, const char*, mand_t); // called upon instantiation
  void (*dynamic_exit)(void*, const char*); // called upon de-instantiation
};

#endif
