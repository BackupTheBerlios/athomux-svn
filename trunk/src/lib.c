/* Author: Thomas Schoebel-Theuer
 * Copyright: University of Stuttgart
 */

#include "common.h"

#define BASEFILE "common.c"

#define MISSING(name)                                                         \
void missing_##name(const union connector * on, struct args * args, const char * param)\
{                                                                             \
  printf("operation " #name " not implemented\n");                            \
  DEBUG_EXIT;                                                                 \
}

#define OPTIONAL(name)                                                        \
void missing_##name(const union connector * on, struct args * args, const char * param)\
{                                                                             \
  args->success = FALSE;                                                      \
}

/* This is only used for unimplemented init operations, which are
 * allowed since they only produce side effects and return nothing
 * besides the success.
 */
#define UNUSED(name)                                                          \
void missing_##name(const union connector * on, struct args * args, const char * param)\
{ /* Do nothing */                                                            \
  args->success = TRUE;                                                       \
}

/* Define all the default elementary operations
 */
UNUSED(output_init)
// static ops
MISSING(transfer)
MISSING(wait)
MISSING(get)
MISSING(put)
MISSING(lock)
MISSING(unlock)
MISSING(getaddr)
MISSING(putaddr)
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

void missing_tr(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_transfer;
  on->output.ops[args->sect_code][opcode_transfer](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->action = action_wait;
  args->op_code = opcode_wait;
  on->output.ops[args->sect_code][opcode_wait](on, args, param);
}

void missing_gettr(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_get;
  on->output.ops[args->sect_code][opcode_get](on, args, param);
  if(!args->success) {
    return;
  }
  if(args->version < vers_newest) {
    args->success = FALSE;
    args->direction = direct_read;
    args->op_code = opcode_tr;
    on->output.ops[args->sect_code][opcode_tr](on, args, param);
    if(!args->success) { /* undo reservation once again */
      args->prio = prio_none;
      on->output.ops[args->sect_code][opcode_put](on, args, param);
      args->success = FALSE;
    }
  }
}

void missing_trput(const union connector * on, struct args * args, const char * param)
{
  success_t res;
  args->direction = direct_write;
  args->op_code = opcode_tr;
  on->output.ops[args->sect_code][opcode_tr](on, args, param);
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

void missing_getaddrcreateget(const union connector * on, struct args * args, const char * param)
{
  args->where = FALSE;
  args->op_code = opcode_getaddr;
  on->output.ops[args->sect_code][opcode_getaddr](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_createget;
  on->output.ops[args->sect_code][opcode_createget](on, args, param);
}

void missing_getaddrgettr(const union connector * on, struct args * args, const char * param)
{
  args->where = TRUE;
  args->op_code = opcode_getaddr;
  on->output.ops[args->sect_code][opcode_getaddr](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->op_code = opcode_gettr;
  on->output.ops[args->sect_code][opcode_gettr](on, args, param);
}

void missing_putputaddr(const union connector * on, struct args * args, const char * param)
{
  args->prio = prio_background;
  args->op_code = opcode_put;
  on->output.ops[args->sect_code][opcode_put](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->where = TRUE;
  args->op_code = opcode_putaddr;
  on->output.ops[args->sect_code][opcode_putaddr](on, args, param);
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

void missing_putdeleteputaddr(const union connector * on, struct args * args, const char * param)
{
  args->op_code = opcode_putdelete;
  on->output.ops[args->sect_code][opcode_putdelete](on, args, param);
  if(!args->success) {
    return;
  }
  args->success = FALSE;
  args->where = FALSE;
  args->op_code = opcode_putaddr;
  on->output.ops[args->sect_code][opcode_putaddr](on, args, param);
}

#define ADD_MISSING(name,sect) [opcode_##name] = &missing_##sect##_##name

#define ADD_ALL_MISSING(sect)                                                 \
{                                                                             \
  ADD_MISSING(output_init,sect),                                              \
  ADD_MISSING(transfer,sect),                                                 \
  ADD_MISSING(wait,sect),                                                     \
  ADD_MISSING(get,sect),                                                      \
  ADD_MISSING(put,sect),                                                      \
  ADD_MISSING(lock,sect),                                                     \
  ADD_MISSING(unlock,sect),                                                   \
  ADD_MISSING(getaddr,sect),                                                  \
  ADD_MISSING(putaddr,sect),                                                  \
                                                                              \
  ADD_MISSING(create,sect),                                                   \
  ADD_MISSING(delete,sect),                                                   \
  ADD_MISSING(move,sect),                                                     \
                                                                              \
  ADD_MISSING(instbrick,sect),                                                \
  ADD_MISSING(deinstbrick,sect),                                              \
  ADD_MISSING(instconn,sect),                                                 \
  ADD_MISSING(deinstconn,sect),                                               \
  ADD_MISSING(connect,sect),                                                  \
  ADD_MISSING(disconnect,sect),                                               \
  ADD_MISSING(getconn,sect),                                                  \
  ADD_MISSING(findconn,sect),                                                 \
                                                                              \
  ADD_MISSING(tr,sect),                                                       \
  ADD_MISSING(gettr,sect),                                                    \
  ADD_MISSING(trput,sect),                                                    \
  ADD_MISSING(putwait,sect),                                                  \
  ADD_MISSING(createget,sect),                                                \
  ADD_MISSING(getaddrcreateget,sect),                                         \
  ADD_MISSING(getaddrgettr,sect),                                             \
  ADD_MISSING(putputaddr,sect),                                               \
  ADD_MISSING(putdelete,sect),                                                \
  ADD_MISSING(putdeleteputaddr,sect),                                         \
                                                                              \
  ADD_MISSING(input_init,sect),                                               \
  ADD_MISSING(retract,sect),                                                  \
                                                                              \
  ADD_MISSING(brick_init,sect),                                               \
}

name_t op_names[opcode_brick_max+1] = {
  // OUTPUT OPCODES
  "output_init",
  // static
  "transfer",
  "wait",
  "get",
  "put",
  "lock",
  "unlock",
  "getaddr",
  "putaddr",
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
  "tr",
  "gettr",
  "trput",
  "putwait",
  "createget",
  "getaddrcreateget",
  "getaddrgettr",
  "putputaddr",
  "putdelete",
  "putdeleteputaddr",
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
    .op_code = opcode_gettr,
    .sect_code = pc->pc_sect,
  };
  const union connector * other = (void*)pc->pc_input->connect;
  other->output.ops[pc->pc_sect][opcode_gettr](other, &args, "");
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
    .op_code = opcode_getaddrcreateget,
    .sect_code = pc->pc_sect,
  };
  const union connector * other = (void*)pc->pc_input->connect;
  other->output.ops[pc->pc_sect][opcode_getaddrcreateget](other, &args, "");
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
    .op_code = opcode_putdeleteputaddr,
    .sect_code = pc->pc_sect,
  };
  const union connector * other = (void*)pc->pc_input->connect;
  other->output.ops[pc->pc_sect][opcode_putdeleteputaddr](other, &args, "");
}
