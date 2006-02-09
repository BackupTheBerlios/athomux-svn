/* Author: Thomas Schoebel-Theuer
 * Author: Roland Niese (added "init_one_instance()")
 * Copyright: University of Stuttgart
 */

#include "common.h"

#define BASEFILE "common.c"

//////////////////////////////////////////////////////////////////////////

// debugging

#ifdef DEBUG
#include <stdio.h>
int call_level = 0;
char blanks[32] = "                               ";
#endif

#include "../debug.names"

#define DEBUG_OPEN(name)                                                      \
  _debug_##name = fopen("debug."#name, "w");

#define DEBUG_CLOSE(name)                                                     \
  fclose(_debug_##name);

void open_debug()
{
#ifdef DEBUG
  setvbuf(stdout, NULL, _IONBF, 0);
#endif
#include "../debug.init"
}

void close_debug()
{
#include "../debug.exit"
}

#include <stdio.h>

FILE * _debug_strategy;
FILE * _debug_fuse;
FILE * _debug_syscall;

char * ATHOMUX_ULINUX_BASE;
char ATHOMUX_ULINUX_ROOT[256];
char ATHOMUX_ULINUX_USERFS[256];
char ATHOMUX_ULINUX_SYSCALLS[256];
char ATHOMUX_ULINUX_LOG[256];


//////////////////////////////////////////////////////////////////////////

// general brick init

void init_all_conns(const struct loader * loader, int type, void * brick, struct args * args, const char * param)
{
  const struct load_conn * load_conn;
  int i = loader->conn_count;
  args->success = TRUE;
  void do_it() {
    void * conn = (char*)brick + load_conn->offset;
    int j;
    for(j = load_conn->count; j > 0; conn += load_conn->size, j--) {
      args->success = FALSE;
      load_conn->init_conn(conn, args, param);
      if(!args->success) {
	return;
      }
    }
  }
  if(args->constr) {
    for(load_conn = loader->conn; i > 0; load_conn++, i--) {
      if(!load_conn->autoinit || (type >= 0 && load_conn->type != type)) {
	continue;
      }
      do_it();
      if(!args->success) {
	return;
      }
    }
  } else {
    for(load_conn = loader->conn + i - 1; i > 0; load_conn--, i--) {
      if(!load_conn->autoinit || (type >= 0 && load_conn->type != type)) {
	continue;
      }
      do_it();
      if(!args->success) {
	return;
      }
    }
  }
}

void init_all_instances(const struct loader * loader, void * brick, struct args * args, const char * param)
{
  const struct load_instance * load_inst;
  int i = loader->inst_count;
  for(load_inst = loader->instances; i > 0; load_inst++, i--) {
    void * subbrick = brick + load_inst->offset;
    const struct loader * subloader = load_inst->loader;
    args->success = TRUE;
    subloader->init_brick(subbrick, args, param);
    if(!args->success) {
      return;
    }
  }
}

// Roland Niese: workaround for manual initialization of subinstances in the right order (from left to right).
void init_one_instance (const struct loader * loader, void * brick, void *subbrick, struct args * args, const char * param) {
	int i;
	for (i = 0; i < loader->inst_count; i++) {
		if (loader->instances[i].offset == (char *)subbrick - (char *)brick) {
			loader->instances[i].loader->init_brick ((void *)brick + loader->instances[i].offset, args, param);
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////

#define UNINITIALIZED(name)                                                   \
void uninitialized_##name(const union connector * on, struct args * args, const char * param)\
{                                                                             \
  printf("operation " #name " called at stateless phase\n");                  \
  DEBUG_EXIT;                                                                 \
}

#define MISSING(name)                                                         \
void missing_##name(const union connector * on, struct args * args, const char * param)\
{                                                                             \
  printf("operation " #name " not implemented\n");                            \
  DEBUG_EXIT;                                                                 \
}                                                                             \
UNINITIALIZED(name)


#define OPTIONAL(name)                                                        \
void missing_##name(const union connector * on, struct args * args, const char * param)\
{                                                                             \
  args->success = FALSE;                                                      \
}                                                                             \
UNINITIALIZED(name)

/* This is only used for unimplemented init operations, which are
 * allowed since they only produce side effects and return nothing
 * besides the success.
 */
#define UNUSED(name)                                                          \
void missing_##name(const union connector * on, struct args * args, const char * param)\
{ /* Do nothing */                                                            \
  args->success = TRUE;                                                       \
}                                                                             \
UNINITIALIZED(name)

/* Define all the default elementary operations
 */
UNUSED(output_init)
// static ops
MISSING(trans)
MISSING(wait)
MISSING(get)
MISSING(put)
MISSING(lock)
MISSING(unlock)
MISSING(gadr)
MISSING(padr)
// combined ops
UNINITIALIZED(transwait)
UNINITIALIZED(gettranswait)
UNINITIALIZED(transwaitput)
UNINITIALIZED(putwait)
UNINITIALIZED(createget)
UNINITIALIZED(gadrcreate)
UNINITIALIZED(gadrcreateget)
UNINITIALIZED(gadrgettranswait)
UNINITIALIZED(putpadr)
UNINITIALIZED(putdelete)
UNINITIALIZED(deletepadr)
UNINITIALIZED(putdeletepadr)
UNINITIALIZED(gadrtranswaitdeletepadr)
UNINITIALIZED(gadrcreatetranswaitpadr)
// strategy ops
MISSING(instbrick)
MISSING(deinstbrick)
MISSING(instconn)
MISSING(deinstconn)
MISSING(connect)
MISSING(disconnect)
MISSING(getconn)
MISSING(findconn)
// dynamic ops
MISSING(create)
MISSING(delete)
MISSING(move)

UNUSED(input_init)
OPTIONAL(retract)

UNUSED(brick_init)

/* These are the default implementations for combined elementary
 * operations.
 *
 * If you don't implement a combined operation, this version is
 * automatically used.
 *
 * However, not implementing a combined operation may produce
 * bad performance due to unnecessary calling overhead along a wire
 * path.
 * In a distributed system, implementation of combined operations is
 * almost mandatory. Otherwise the network latencies will add up to
 * multiples of what is really necessary to solve the problem.
 *
 * For quick implementation of prototypes, you can safely skip
 * implementing your own combined operation. As soon as performance
 * becomes an issue, check this first.
 *
 * NOTE: this implementation also defines the _default semantics_
 * of combined operations.
 *
 * Your own implementation _must_ ensure the same semantics as
 * defined here (just yielding better performance), otherwise
 * we get no drop-in replacement and/or bad semantics!
 */

void missing_transwait(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_trans;
  on->output.ops[args->sect_code][opcode_trans](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->action = action_wait;
  args->op_code = opcode_wait;
  on->output.ops[args->sect_code][opcode_wait](on, args, param);
}

void missing_gettranswait(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_get;
  on->output.ops[args->sect_code][opcode_get](on, args, param);
  if(!args->success) {
    return;
  }
  if(args->version < vers_newest) {
    args->success = FALSE;
    args->direction = direct_read;
    args->op_code = opcode_transwait;
    on->output.ops[args->sect_code][opcode_transwait](on, args, param);
    if(!args->success) { /* undo reservation once again */
      args->prio = prio_none;
      on->output.ops[args->sect_code][opcode_put](on, args, param);
      args->success = FALSE;
    }
  }
}

void missing_transwaitput(const union connector * on, struct args * args, const char * param)
{
  success_t res;
  args->direction = direct_write;
  args->op_code = opcode_transwait;
  on->output.ops[args->sect_code][opcode_transwait](on, args, param);
  res = args->success;
  args->success = FALSE;
  /*try to put always --> try to avoid memory leaks */
  args->op_code = opcode_put;
  on->output.ops[args->sect_code][opcode_put](on, args, param);
  args->success &= res; /* both must have succeeded */
}

void missing_putwait(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_put;
  on->output.ops[args->sect_code][opcode_put](on, args, param);
  if(!args->success) {
    return;
  }
  args->op_code = opcode_wait;
  on->output.ops[args->sect_code][opcode_wait](on, args, param);
}

void missing_createget(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_create;
  on->output.ops[args->sect_code][opcode_create](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->forwrite = TRUE;
  args->op_code = opcode_get;
  on->output.ops[args->sect_code][opcode_get](on, args, param);
}

void missing_gadrcreate(const union connector * on, struct args * args, const char * param)
{
  args->reader = FALSE;
  args->op_code = opcode_gadr;
  on->output.ops[args->sect_code][opcode_gadr](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_create;
  on->output.ops[args->sect_code][opcode_create](on, args, param);
}

void missing_gadrcreateget(const union connector * on, struct args * args, const char * param)
{
  args->reader = FALSE;
  args->op_code = opcode_gadr;
  on->output.ops[args->sect_code][opcode_gadr](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_createget;
  on->output.ops[args->sect_code][opcode_createget](on, args, param);
}

void missing_gadrgettranswait(const union connector * on, struct args * args, const char * param)
{
  args->reader = TRUE;
  args->op_code = opcode_gadr;
  on->output.ops[args->sect_code][opcode_gadr](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_gettranswait;
  on->output.ops[args->sect_code][opcode_gettranswait](on, args, param);
}

void missing_putpadr(const union connector * on, struct args * args, const char * param)
{
  args->prio = prio_background;
  args->op_code = opcode_put;
  on->output.ops[args->sect_code][opcode_put](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->reader = FALSE;
  args->op_code = opcode_padr;
  on->output.ops[args->sect_code][opcode_padr](on, args, param);
}

void missing_putdelete(const union connector * on, struct args * args, const char * param)
{
  args->prio = prio_none;
  args->op_code = opcode_put;
  on->output.ops[args->sect_code][opcode_put](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_delete;
  on->output.ops[args->sect_code][opcode_delete](on, args, param);
}

void missing_deletepadr(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_delete;
  on->output.ops[args->sect_code][opcode_delete](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_padr;
  on->output.ops[args->sect_code][opcode_padr](on, args, param);
}

void missing_putdeletepadr(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_putdelete;
  on->output.ops[args->sect_code][opcode_putdelete](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_padr;
  on->output.ops[args->sect_code][opcode_padr](on, args, param);
}

void missing_gadrtranswaitdeletepadr(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_gadr;
  args->log_len = args->try_len = args->phys_len;
  args->reader = TRUE;
  args->exclu = TRUE;
  on->output.ops[args->sect_code][opcode_gadr](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_transwait;
  args->direction = direct_read;
  on->output.ops[args->sect_code][opcode_transwait](on, args, param);
  if(!args->success || args->phys_len != args->log_len) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_delete;
  args->log_len = args->phys_len;
  on->output.ops[args->sect_code][opcode_delete](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_padr;
  on->output.ops[args->sect_code][opcode_padr](on, args, param);
}

void missing_gadrcreatetranswaitpadr(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_gadr;
  args->log_len = args->try_len = args->phys_len;
  args->reader = TRUE;
  args->exclu = TRUE;
  on->output.ops[args->sect_code][opcode_gadr](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_create;
  args->clear = FALSE;
  args->melt = TRUE;
  on->output.ops[args->sect_code][opcode_create](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_transwait;
  args->direction = direct_read;
  on->output.ops[args->sect_code][opcode_transwait](on, args, param);
  if(!args->success || args->phys_len != args->log_len) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_padr;
  args->log_len = args->phys_len;
  on->output.ops[args->sect_code][opcode_padr](on, args, param);
}

#define ADD_UNINITIALIZED(name,sect,sub) [opcode_##name sub] = &uninitialized_##name

#define ADD_ALL_UNINITIALIZED_OUTPUT(sect)                                    \
{                                                                             \
  ADD_UNINITIALIZED(output_init,sect,),                                       \
  ADD_UNINITIALIZED(trans,sect,),                                             \
  ADD_UNINITIALIZED(wait,sect,),                                              \
  ADD_UNINITIALIZED(get,sect,),                                               \
  ADD_UNINITIALIZED(put,sect,),                                               \
  ADD_UNINITIALIZED(lock,sect,),                                              \
  ADD_UNINITIALIZED(unlock,sect,),                                            \
  ADD_UNINITIALIZED(gadr,sect,),                                              \
  ADD_UNINITIALIZED(padr,sect,),                                              \
                                                                              \
  ADD_UNINITIALIZED(create,sect,),                                            \
  ADD_UNINITIALIZED(delete,sect,),                                            \
  ADD_UNINITIALIZED(move,sect,),                                              \
                                                                              \
  ADD_UNINITIALIZED(instbrick,sect,),                                         \
  ADD_UNINITIALIZED(deinstbrick,sect,),                                       \
  ADD_UNINITIALIZED(instconn,sect,),                                          \
  ADD_UNINITIALIZED(deinstconn,sect,),                                        \
  ADD_UNINITIALIZED(connect,sect,),                                           \
  ADD_UNINITIALIZED(disconnect,sect,),                                        \
  ADD_UNINITIALIZED(getconn,sect,),                                           \
  ADD_UNINITIALIZED(findconn,sect,),                                          \
                                                                              \
  ADD_UNINITIALIZED(transwait,sect,),                                         \
  ADD_UNINITIALIZED(gettranswait,sect,),                                      \
  ADD_UNINITIALIZED(transwaitput,sect,),                                      \
  ADD_UNINITIALIZED(putwait,sect,),                                           \
  ADD_UNINITIALIZED(createget,sect,),                                         \
  ADD_UNINITIALIZED(gadrcreate,sect,),                                        \
  ADD_UNINITIALIZED(gadrcreateget,sect,),                                     \
  ADD_UNINITIALIZED(gadrgettranswait,sect,),                                  \
  ADD_UNINITIALIZED(putpadr,sect,),                                           \
  ADD_UNINITIALIZED(putdelete,sect,),                                         \
  ADD_UNINITIALIZED(deletepadr,sect,),                                        \
  ADD_UNINITIALIZED(putdeletepadr,sect,),                                     \
  ADD_UNINITIALIZED(gadrtranswaitdeletepadr,sect,),                           \
  ADD_UNINITIALIZED(gadrcreatetranswaitpadr,sect,),                           \
},

                                                                              \
#define ADD_ALL_UNINITIALIZED_INPUT(sect)                                     \
{                                                                             \
  ADD_UNINITIALIZED(input_init,sect,-opcode_output_max-1),                    \
  ADD_UNINITIALIZED(retract,sect,-opcode_output_max-1),                       \
}
                                                                              \
#define ADD_ALL_UNINITIALIZED_BRICK(sect)                                     \
  ADD_UNINITIALIZED(brick_init,sect,-opcode_input_max-1),                     \
}

output_operation_set uninitialized_output[4] = {
  ADD_ALL_UNINITIALIZED_OUTPUT(0)
  ADD_ALL_UNINITIALIZED_OUTPUT(1)
  ADD_ALL_UNINITIALIZED_OUTPUT(2)
  ADD_ALL_UNINITIALIZED_OUTPUT(3)
};
#if 0
input_operation_set uninitialized_input[4] = {
  ADD_ALL_UNINITIALIZED_INPUT(0)
  ADD_ALL_UNINITIALIZED_INPUT(1)
  ADD_ALL_UNINITIALIZED_INPUT(2)
  ADD_ALL_UNINITIALIZED_INPUT(3)
};

brick_operation_set uninitialized_brick[4] = {
  ADD_ALL_UNINITIALIZED_BRICK(0)
  ADD_ALL_UNINITIALIZED_BRICK(1)
  ADD_ALL_UNINITIALIZED_BRICK(2)
  ADD_ALL_UNINITIALIZED_BRICK(3)
};
#endif
name_t op_names[opcode_brick_max+1] = {
  // OUTPUT OPCODES
  "output_init",
  // static
  "trans",
  "wait",
  "get",
  "put",
  "lock",
  "unlock",
  "gadr",
  "padr",
  // dynamic
  "create",
  "delete",
  "move",
  // strategy ops
  "instbrick",
  "deinstbrick",
  "instconn",
  "deinstconn",
  "connect",
  "disconnect",
  "getconn",
  "findconn",
  // combinations
  "transwait",
  "gettranswait",
  "transwaitput",
  "putwait",
  "createget",
  "gadrcreate",
  "gadrcreateget",
  "gadrgettranswait",
  "putpadr",
  "putdelete",
  "deletepadr",
  "putdeletepadr",
  "gadrtranswaitdeletepadr",
  "gadrcreatetranswaitpadr",
  "bad_output_max",
  // INPUT OPCODES
  "input_init",
  "retract",
  "bad_input_max",
  // BRICK OPCODES
  "brick_init",
  "bad_brick_max",
};

const struct gen_type type_empty[1] = {{}};

////////////////////////////////////////////////////////////////////////

/* Pointer Cache routines
 */

void _PC_PUT(struct pc * pc, struct pc_elem * elem)
{
  struct args args = {
    .log_addr = elem->pc_addr & ~(addr_t)(PC__DIRTY | PC__PRESENT),
    .log_len =  elem->pc_len,
    .phys_addr = MAKE_PADDR(elem->pc_ptr),
    .prio = (elem->pc_addr & PC__DIRTY) ? prio_background : prio_none,
    .op_code = opcode_put,
    .sect_code = pc->pc_sect,
  };
  const union connector * other = (void*)pc->pc_input->connect;
  other->output.ops[pc->pc_sect][opcode_put](other, &args, "");
  memset(elem, 0, sizeof(struct pc_elem));
}

void _PC_GET(struct pc * pc, struct pc_elem * elem, addr_t __addr, plen_t __len)
{
  if(elem->pc_addr & PC__PRESENT) {
    _PC_PUT(pc, elem);
  }
  struct args args = {
    .log_addr = __addr & ~(addr_t)(PC__DIRTY | PC__PRESENT),
    .log_len =  __len,
    .forwrite = TRUE,
    .prio = prio_normal,
    .op_code = opcode_gettranswait,
    .sect_code = pc->pc_sect,
  };
  const union connector * other = (void*)pc->pc_input->connect;
  other->output.ops[pc->pc_sect][opcode_gettranswait](other, &args, "");
  if(!args.success || args.phys_len < __len) {
    return;
  }
  elem->pc_addr = __addr | PC__PRESENT;
  elem->pc_len = __len;
  elem->pc_ptr = MAKE_PTR(args.phys_addr);
}

void _PC_ALL_PUT(struct pc * pc, index_t max)
{
  index_t i;
  for(i = 0; i < max; i++) {
    struct pc_elem * elem = pc->pc_elem + i;
    if(elem->pc_addr & PC__PRESENT) {
      _PC_PUT(pc, elem);
    }
  }
}

struct pc_elem * _PC_ALLOC(struct pc * pc, plen_t __len, plen_t align, plen_t whole, plen_t max)
{
  plen_t base_len = PC__BASE_LEN(0, __len, whole);
  struct args args = {
    .log_len =  base_len,
    .try_len =  base_len,
    .exclu = TRUE,
    .melt = TRUE,
    .action = action_wait,
    .op_code = opcode_gadrcreateget,
    .sect_code = pc->pc_sect,
  };
  const union connector * other = (void*)pc->pc_input->connect;
  other->output.ops[pc->pc_sect][opcode_gadrcreateget](other, &args, "");
  if(!args.success || args.log_len < base_len) {
    return NULL;
  }
  index_t index = PC__HASH(args.log_addr, align, max);
  struct pc_elem * elem = &pc->pc_elem[index];
  if(elem->pc_addr & PC__PRESENT) {
    _PC_PUT(pc, elem);
  }
  elem->pc_addr = PC__BASE_ADDR(args.log_addr, whole) | PC__PRESENT | PC__DIRTY;
  elem->pc_len = args.log_len;
  elem->pc_ptr = MAKE_PTR(args.phys_addr);
  pc->pc_write = TRUE;
  return elem;
}

void  _PC_FREE(struct pc * pc, struct pc_elem * elem, addr_t __addr, plen_t __len)
{
  if((elem->pc_addr & PC__PRESENT) && (elem->pc_addr & ~(addr_t)(PC__DIRTY | PC__PRESENT)) == __addr) {
    _PC_PUT(pc, elem);
  }
  struct args args = {
    .log_addr = __addr,
    .log_len =  __len,
    .try_len =  __len,
    .exclu = TRUE,
    .melt = TRUE,
    .action = action_wait,
    .op_code = opcode_putdeletepadr,
    .sect_code = pc->pc_sect,
  };
  const union connector * other = (void*)pc->pc_input->connect;
  other->output.ops[pc->pc_sect][opcode_putdeletepadr](other, &args, "");
}
