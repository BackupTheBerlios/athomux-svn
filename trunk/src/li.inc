/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

// returns the address of the next element in the list
@.define LI_NEXT(NAME, addr_t _linextprefix_elem) => (addr_t _linextprefix_next)
{
  struct link *_linextprefix_link = LI_GET(NAME, _linextprefix_elem, FALSE);
  _linextprefix_next = _linextprefix_link->l_next;
}




// returns the address of the previous element in the list
@.define LI_PREV(NAME, addr_t _liprevprefix_elem) => (addr_t _liprevprefix_prev)
{
  struct link *_liprevprefix_link = LI_GET(NAME, _liprevprefix_elem, FALSE);
  _liprevprefix_prev = _liprevprefix_link->l_prev;
}




// finds first block in 'hook' of larger or equal size than 'len'
// 'success' is false if no block can be found
@.define LI_FIND_GE(NAME, addr_t _lifindgeprefix_hook, len_t _lifindgeprefix_len) => (success_t _lifindgeprefix_success, addr_t _lifindgeprefix_first)
{
  _lifindgeprefix_success = FALSE;

  addr_t _lifindgeprefix_elem_ptr;
  LI_NEXT(NAME, _lifindgeprefix_hook) => (_lifindgeprefix_elem_ptr);
  while (_lifindgeprefix_elem_ptr != _lifindgeprefix_hook) {
    addr_t _lifindgeprefix_next_ptr;
    LI_NEXT(NAME, _lifindgeprefix_elem_ptr) => (_lifindgeprefix_next_ptr);
    struct block_t *_lifindgeprefix_block = LI_GET(NAME, _lifindgeprefix_elem_ptr, FALSE);

    if (_lifindgeprefix_block->len >= _lifindgeprefix_len) {
      _lifindgeprefix_success = TRUE;
      _lifindgeprefix_first = _lifindgeprefix_elem_ptr;
      break;
    }

    _lifindgeprefix_elem_ptr = _lifindgeprefix_next_ptr;
  }
}




// finds the first intersection with 'start' and 'len' in 'hook'
// 'success' is false if no intersecting block can be found
@.define LI_FIND_ISECT(NAME, addr_t _lifindisectprefix_hook, addr_t _lifindisectprefix_start, len_t _lifindisectprefix_len) => (success_t _lifindisectprefix_success, addr_t _lifindisectprefix_first)
{
  //printf("li_find_isect(%lld, %lld)\n", _lifindisectprefix_start, _lifindisectprefix_len);
  _lifindisectprefix_success = FALSE;

  addr_t _lifindisectprefix_elem_ptr;
  LI_NEXT(NAME, _lifindisectprefix_hook) => (_lifindisectprefix_elem_ptr);
  //printf("li: %lld, %lld\n", _lifindisectprefix_hook, _lifindisectprefix_elem_ptr);
  while (_lifindisectprefix_elem_ptr != _lifindisectprefix_hook) {
    //printf("li_find_isect2:\n");
    addr_t _lifindisectprefix_next_ptr;
    LI_NEXT(NAME, _lifindisectprefix_elem_ptr) => (_lifindisectprefix_next_ptr);
    struct block_t *_lifindisectprefix_block = LI_GET(NAME, _lifindisectprefix_elem_ptr, FALSE);

    addr_t _lifindisectprefix_istart;
    addr_t _lifindisectprefix_ilen;

    //printf("fi_isect(%lld,%lld,%lld,%lld)\n", _lifindisectprefix_start, _lifindisectprefix_len, _lifindisectprefix_block->start, _lifindisectprefix_block->len);
    intersect(_lifindisectprefix_start, _lifindisectprefix_len, _lifindisectprefix_block->start, _lifindisectprefix_block->len) => (_lifindisectprefix_istart, _lifindisectprefix_ilen);

    if (_lifindisectprefix_ilen>0) {
      _lifindisectprefix_success = TRUE;
      _lifindisectprefix_first = _lifindisectprefix_elem_ptr;
      break;
    }

    _lifindisectprefix_elem_ptr = _lifindisectprefix_next_ptr;
  }
}




// removes all intersections with 'start' and 'len' in 'hook'
// intersecting blocks are split and the non-intersecting areas are reinserted
// 'success' should always be TRUE, unless the pointer cache or the list is doing something wrong
@.define LI_REMOVE_ISECT(NAME, addr_t _liremoveisectprefix_hook, addr_t _liremoveisectprefix_start, len_t _liremoveisectprefix_len) => (success_t _liremoveisectprefix_success)
{
  _liremoveisectprefix_success = FALSE;

  addr_t _liremoveisectprefix_elem_ptr;
  LI_NEXT(NAME, _liremoveisectprefix_hook) => (_liremoveisectprefix_elem_ptr);

  while (_liremoveisectprefix_elem_ptr != _liremoveisectprefix_hook) {
    addr_t _liremoveisectprefix_next_ptr;
    LI_NEXT(NAME, _liremoveisectprefix_elem_ptr) => (_liremoveisectprefix_next_ptr);
    struct block_t *_liremoveisectprefix_block = LI_GET(NAME, _liremoveisectprefix_elem_ptr, FALSE);

    addr_t _liremoveisectprefix_istart;
    addr_t _liremoveisectprefix_ilen;

    intersect(_liremoveisectprefix_start, _liremoveisectprefix_len, _liremoveisectprefix_block->start, _liremoveisectprefix_block->len) => (_liremoveisectprefix_istart, _liremoveisectprefix_ilen);

    if (_liremoveisectprefix_ilen>0) {
      addr_t _liremoveisectprefix_lstart;
      len_t _liremoveisectprefix_llen;
      addr_t _liremoveisectprefix_rstart;
      len_t _liremoveisectprefix_rlen;

      splitleft(_liremoveisectprefix_block->start, _liremoveisectprefix_block->len, _liremoveisectprefix_istart) => (_liremoveisectprefix_lstart, _liremoveisectprefix_llen);
      splitright(_liremoveisectprefix_block->start, _liremoveisectprefix_block->len, _liremoveisectprefix_istart+_liremoveisectprefix_ilen) => (_liremoveisectprefix_rstart, _liremoveisectprefix_rlen);

      if (_liremoveisectprefix_llen>0) {
        addr_t _liremoveisectprefix_ptr;
        struct block_t *_liremoveisectprefix_b;
        _liremoveisectprefix_b = PC_ALLOC(NAME, sizeof(struct block_t)) => (_liremoveisectprefix_ptr);
        _liremoveisectprefix_b->start = _liremoveisectprefix_lstart;
        _liremoveisectprefix_b->len = _liremoveisectprefix_llen;
        _liremoveisectprefix_b->offset = _liremoveisectprefix_block->offset;
        LI_PREPEND(NAME, _liremoveisectprefix_elem_ptr, _liremoveisectprefix_ptr);
      }

      if (_liremoveisectprefix_rlen>0) {
        addr_t _liremoveisectprefix_ptr;
        struct block_t *_liremoveisectprefix_b;
        _liremoveisectprefix_b = PC_ALLOC(NAME, sizeof(struct block_t)) => (_liremoveisectprefix_ptr);
        _liremoveisectprefix_b->start = _liremoveisectprefix_rstart;
        _liremoveisectprefix_b->len = _liremoveisectprefix_rlen;
        _liremoveisectprefix_b->offset = _liremoveisectprefix_block->offset;
        LI_PREPEND(NAME, _liremoveisectprefix_elem_ptr, _liremoveisectprefix_ptr);
      }

      // delete buffer that contains intersection
      LI_REMOVE(NAME, _liremoveisectprefix_elem_ptr);
      PC_FREE(NAME, _liremoveisectprefix_elem_ptr, sizeof(struct block_t));
    }

    _liremoveisectprefix_elem_ptr = _liremoveisectprefix_next_ptr;
  }

  _liremoveisectprefix_success = TRUE;
}




// try to merge all blocks in 'hook' with 'start' and 'len' and insert new block
// 'success' should always be TRUE, unless the pointer cache or the list is doing something wrong
@.define LI_MERGE_BLOCK(NAME, addr_t _limergeblockprefix_hook, addr_t _limergeblockprefix_start, len_t _limergeblockprefix_len) => (success_t _limergeblockprefix_success)
{
  _limergeblockprefix_success = FALSE;

  struct block_t *_limergeblockprefix_block;
  addr_t _limergeblockprefix_elem_ptr;
  LI_NEXT(NAME, _limergeblockprefix_hook) => (_limergeblockprefix_elem_ptr);

  while (_limergeblockprefix_elem_ptr != _limergeblockprefix_hook) {
    addr_t next__limergeblockprefix_ptr;
    LI_NEXT(NAME, _limergeblockprefix_elem_ptr) => (_limergeblockprefix_next_ptr);
    _limergeblockprefix_block = LI_GET(NAME, _limergeblockprefix_elem_ptr, FALSE);

    if (((_limergeblockprefix_start <= _limergeblockprefix_block->start) && (_limergeblockprefix_start+_limergeblockprefix_len >= _limergeblockprefix_block->start)) ||
        ((_limergeblockprefix_block->start <= _limergeblockprefix_start) && (_limergeblockprefix_block->start+_limergeblockprefix_block->len >= _limergeblockprefix_start))) {

      // merge is possible
      _limergeblockprefix_start = MIN(_limergeblockprefix_start, _limergeblockprefix_block->start);
      _limergeblockprefix_len = MAX(_limergeblockprefix_start+_limergeblockprefix_len, _limergeblockprefix_block->start+_limergeblockprefix_block->len) - _limergeblockprefix_start;

      // remove block from list
      LI_REMOVE(NAME, _limergeblockprefix_elem_ptr);
      PC_FREE(NAME, _limergeblockprefix_elem_ptr, sizeof(struct block_t));
    }

    _limergeblockprefix_elem_ptr = _limergeblockprefix_next_ptr;
  }

  // insert new block in list
  _limergeblockprefix_block = PC_ALLOC(NAME, sizeof(struct block_t)) => (_limergeblockprefix_elem_ptr);
  _limergeblockprefix_block->start = _limergeblockprefix_start;
  _limergeblockprefix_block->len = _limergeblockprefix_len;
  LI_APPEND(NAME, _limergeblockprefix_hook, _limergeblockprefix_elem_ptr);

  _limergeblockprefix_success = TRUE;
}
