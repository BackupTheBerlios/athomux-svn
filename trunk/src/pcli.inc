/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

// wrapper for pointer cache and list functions.
// usage like original functions. There is a prefix "my" in function names.
// compile with "-D_USE_PC_" to use original pointer cache and list functions.
// otherwise direct malloc and free calls will be used.




@.define myPC_ALLOC(name, len_t _mypcallocprefix_len) => (_mypcallocprefix_log_addr)
({
#ifdef _USE_PC_
  PC_ALLOC(name, _mypcallocprefix_len) => (_mypcallocprefix_log_addr);
#else
  _mypcallocprefix_log_addr = (addr_t)(long)malloc(_mypcallocprefix_len);
  if (_mypcallocprefix_log_addr==0) @.abort("myPC_ALLOC: malloc failed");
  (void *)(long)_mypcallocprefix_log_addr;
#endif
})




@.define myPC_FREE(name, addr_t _mypcfreeprefix_addr, len_t _mypcfreeprefix_len)
{
#ifdef _USE_PC_
  PC_FREE(name, _mypcfreeprefix_addr, _mypcfreeprefix_len);
#else
  free((void *)(long)_mypcfreeprefix_addr);
#endif
}




@.define myPC_FLUSH(name)
{
#ifdef _USE_PC_
  PC_FLUSH(name);
#endif
}




@.define _myLI_CONNECT(addr_t _mylicconnectprefix_left, struct link * _mylicconnectprefix_ptr_left, addr_t _mylicconnectprefix_right, struct link * _mylicconnectprefix_ptr_right)
{
#ifdef _USE_PC_
  _LI_CONNECT(_mylicconnectprefix_left, _mylicconnectprefix_ptr_left, _mylicconnectprefix_right, _mylicconnectprefix_ptr_right);
#else
  _mylicconnectprefix_ptr_left->l_next = _mylicconnectprefix_right;
  _mylicconnectprefix_ptr_right->l_prev = _mylicconnectprefix_left;
#endif
}




@.define myLI_CONNECT(name, addr_t _myliconnectprefix_left, addr_t _myliconnectprefix_right)
{
#ifdef _USE_PC_
  LI_CONNECT(name, _myliconnectprefix_left, _myliconnectprefix_right);
#else
  struct link * _myliconnectprefix_ptr_left = (struct link *)(long)_myliconnectprefix_left;
  struct link * _myliconnectprefix_ptr_right = (struct link *)(long)_myliconnectprefix_right;
  _myLI_CONNECT(_myliconnectprefix_left, _myliconnectprefix_ptr_left, _myliconnectprefix_right, _myliconnectprefix_ptr_right);
#endif
}




@.define myLI_INIT(name, addr_t _myliinitprefix_start)
{
#ifdef _USE_PC_
  LI_INIT(name, _myliinitprefix_start);
#else
  struct link * _myliinitprefix_ptr_start = (struct link *)(long)_myliinitprefix_start;
  _myLI_CONNECT(_myliinitprefix_start, _myliinitprefix_ptr_start, _myliinitprefix_start, _myliinitprefix_ptr_start);
#endif
}




@.define myLI_APPEND(name, addr_t _myliappendprefix_start, addr_t _myliappendprefix_elem)
{
#ifdef _USE_PC_
  LI_APPEND(name, _myliappendprefix_start, _myliappendprefix_elem);
#else
  struct link * _myliappendprefix_ptr_start = (struct link *)(long)_myliappendprefix_start;
  struct link * _myliappendprefix_ptr_elem = (struct link *)(long)_myliappendprefix_elem;
  addr_t _myliappendprefix_next = _myliappendprefix_ptr_start->l_next;
  struct link * _myliappendprefix_ptr_next = (struct link *)(long)_myliappendprefix_next;
  _myLI_CONNECT(_myliappendprefix_start, _myliappendprefix_ptr_start, _myliappendprefix_elem, _myliappendprefix_ptr_elem);
  _myLI_CONNECT(_myliappendprefix_elem, _myliappendprefix_ptr_elem, _myliappendprefix_next, _myliappendprefix_ptr_next);
#endif
}




@.define myLI_PREPEND(name, addr_t _myliprependprefix_start, addr_t _myliprependprefix_elem)
{
#ifdef _USE_PC_
  LI_PREPEND(name, _myliprependprefix_start, _myliprependprefix_elem);
#else
  struct link * _myliprependprefix_ptr_start = (struct link *)(long)_myliprependprefix_start;
  struct link * _myliprependprefix_ptr_elem = (struct link *)(long)_myliprependprefix_elem;
  addr_t _myliprependprefix_prev = _myliprependprefix_ptr_start->l_prev;
  struct link * _myliprependprefix_ptr_prev = (struct link *)(long)_myliprependprefix_prev;
  _myLI_CONNECT(_myliprependprefix_prev, _myliprependprefix_ptr_prev, _myliprependprefix_elem, _myliprependprefix_ptr_elem);
  _myLI_CONNECT(_myliprependprefix_elem, _myliprependprefix_ptr_elem, _myliprependprefix_start, _myliprependprefix_ptr_start);
#endif
}





@.define myLI_REMOVE(name, addr_t _myliremoveprefix_elem)
{
#ifdef _USE_PC_
  LI_REMOVE(name, _myliremoveprefix_elem);
#else
  struct link * _myliremoveprefix_ptr_elem = (struct link *)(long)_myliremoveprefix_elem;
  myLI_CONNECT(name, _myliremoveprefix_ptr_elem->l_prev, _myliremoveprefix_ptr_elem->l_next);
#endif
}





@.define myLI_GET(name, addr_t _myligetprefix_elem, bool _myligetprefix_dirty)
({
#ifdef _USE_PC_
  LI_GET(name, _myligetprefix_elem, _myligetprefix_dirty);
#else
  (void *)(long)_myligetprefix_elem;
#endif
})




@.define myLI_NEXT(NAME, addr_t _mylinextprefix_elem) => (addr_t _mylinextprefix_next)
{
#ifdef _USE_PC_
  LI_NEXT(NAME, _mylinextprefix_elem) => (_mylinextprefix_next);
#else
  struct link * _mylinextprefix_ptr_elem = (struct link *)(long)_mylinextprefix_elem;
  _mylinextprefix_next = _mylinextprefix_ptr_elem->l_next;
#endif
}




@.define myLI_PREV(NAME, addr_t _myliprevprefix_elem) => (addr_t _myliprevprefix_prev)
{
#ifdef _USE_PC_
  LI_PREV(NAME, _myliprevprefix_elem) => (_myliprevprefix_prev);
#else
  struct link * _myliprevprefix_ptr_elem = (struct link *)(long)_myliprevprefix_elem;
  _myliprevprefix_prev = _myliprevprefix_ptr_elem->l_prev;
#endif
}
