/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

// splits the block (start, len) at position p and returns the left part as (s1, l1).
// p is not part of (s1, l1).
@.define splitleft(addr_t _splitleftprefix_start, len_t _splitleftprefix_len, addr_t _splitleftprefix_p) => (addr_t _splitleftprefix_s1, len_t _splitleftprefix_l1)
{
  if (_splitleftprefix_p <= _splitleftprefix_start) {
    _splitleftprefix_s1 = 0;
    _splitleftprefix_l1 = 0;
  }
  else if (_splitleftprefix_p >= _splitleftprefix_start + _splitleftprefix_len) {
    _splitleftprefix_s1 = _splitleftprefix_start;
    _splitleftprefix_l1 = _splitleftprefix_len;
  }
  else {
    _splitleftprefix_s1 = _splitleftprefix_start;
    _splitleftprefix_l1 = _splitleftprefix_p - _splitleftprefix_start;
  }
}




// splits the block (start, len) at position p and returns the right part as (s2, l2).
// p is part of (s2, l2) if p was in (start, len).
@.define splitright(addr_t _splitrightprefix_start, len_t _splitrightprefix_len, addr_t _splitrightprefix_p) => (addr_t _splitrightprefix_s2, len_t _splitrightprefix_l2)
{
  if (_splitrightprefix_p <= _splitrightprefix_start) {
    _splitrightprefix_s2 = _splitrightprefix_start;
    _splitrightprefix_l2 = _splitrightprefix_len;
  }
  else if (_splitrightprefix_p >= _splitrightprefix_start + _splitrightprefix_len) {
    _splitrightprefix_s2 = 0;
    _splitrightprefix_l2 = 0;
  }
  else {
    _splitrightprefix_s2 = _splitrightprefix_p;
    _splitrightprefix_l2 = _splitrightprefix_start + _splitrightprefix_len - _splitrightprefix_p;
  }
}




// returns the intersection of (s1, l1) and (s2, l2).
// if they do not intersect, len is set to 0.
@.define intersect(addr_t _intersectprefix_s1, len_t _intersectprefix_l1, addr_t _intersectprefix_s2, len_t _intersectprefix_l2) => (addr_t _intersectprefix_start, len_t _intersectprefix_len)
{
  if ((_intersectprefix_s1 + _intersectprefix_l1 <= _intersectprefix_s2) || (_intersectprefix_s2 + _intersectprefix_l2 <= _intersectprefix_s1)) {
    // cases where the regions do not intersect
    _intersectprefix_start = 0;
    _intersectprefix_len = 0;
  }
  else if (_intersectprefix_s1 < _intersectprefix_s2) {
    // calculate start address and length of intersection
    _intersectprefix_start = _intersectprefix_s2;
    _intersectprefix_len = _intersectprefix_s1 + _intersectprefix_l1 - _intersectprefix_s2;
    if (_intersectprefix_len > _intersectprefix_l2) _intersectprefix_len = _intersectprefix_l2;
  }
  else {
    // calculate start address and length of intersection
    _intersectprefix_start = _intersectprefix_s1;
    _intersectprefix_len = _intersectprefix_s2 + _intersectprefix_l2 - _intersectprefix_s1;
    if (_intersectprefix_len > _intersectprefix_l1) _intersectprefix_len = _intersectprefix_l1;
  }
}




// removes (s2, l) from (s1, l) in case they overlap.
// note: both blocks have the same length l.
// returns the shortened block, or (s1, l) in case there is no overlap
@.define substract(addr_t _substractprefix_s1, addr_t _substractprefix_s2, len_t _substractprefix_l) => (addr_t _substractprefix_start, len_t _substractprefix_len)
{
  if ((_substractprefix_s2 <= _substractprefix_s1) && (_substractprefix_s2 + _substractprefix_l > _substractprefix_s1)) {
    _substractprefix_start = _substractprefix_s2 + _substractprefix_l;
    _substractprefix_len = _substractprefix_s1 - _substractprefix_s2;
  }
  else if ((_substractprefix_s2 > _substractprefix_s1) && (_substractprefix_s1 + _substractprefix_l > _substractprefix_s2)) {
    _substractprefix_start = _substractprefix_s1;
    _substractprefix_len = _substractprefix_s2 - _substractprefix_s1;
  }
  else {
    _substractprefix_start = _substractprefix_s1;
    _substractprefix_len = _substractprefix_l;
  }
}




// try to merge (s2, l2) with (s1, l1).
// input blocks may not overlap.
// returns merged block if merge is possible.
// returns (s1, l1) if no merge is possible.
@.define merge(addr_t _mergeprefix_s1, len_t _mergeprefix_l1, addr_t _mergeprefix_s2, len_t _mergeprefix_l2) => (addr_t _mergeprefix_start, len_t _mergeprefix_len)
{
  if (_mergeprefix_s1 + _mergeprefix_l1 == _mergeprefix_s2) {
    _mergeprefix_start = _mergeprefix_s1;
    _mergeprefix_len = _mergeprefix_l1 + _mergeprefix_l2;
  }
  else if (_mergeprefix_s2 + _mergeprefix_l2 == _mergeprefix_s1) {
    _mergeprefix_start = _mergeprefix_s2;
    _mergeprefix_len = _mergeprefix_l1 + _mergeprefix_l2;
  }
  else {
    // no merge possible
    _mergeprefix_start = _mergeprefix_s1;
    _mergeprefix_len = _mergeprefix_l1;
  }
}




// try to merge (s2, l2) with (s1, l1).
// input blocks may overlap.
// returns merged block if merge is possible.
// returns (s1, l1) if no merge is possible.
@.define merge_overlap(addr_t _mergeprefix_s1, len_t _mergeprefix_l1, addr_t _mergeprefix_s2, len_t _mergeprefix_l2) => (addr_t _mergeprefix_start, len_t _mergeprefix_len)
{
  if (_mergeprefix_s1 + _mergeprefix_l1 >= _mergeprefix_s2) {
    _mergeprefix_start = _mergeprefix_s1;
    _mergeprefix_len = MAX(_mergeprefix_s1 + _mergeprefix_l1, _mergeprefix_s2 + _mergeprefix_l2) - _mergeprefix_s2;
  }
  else if (_mergeprefix_s2 + _mergeprefix_l2 >= _mergeprefix_s1) {
    _mergeprefix_start = _mergeprefix_s2;
    _mergeprefix_len = MAX(_mergeprefix_s1 + _mergeprefix_l1, _mergeprefix_s2 + _mergeprefix_l2) - _mergeprefix_s2;
  }
  else {
    // no merge possible
    _mergeprefix_start = _mergeprefix_s1;
    _mergeprefix_len = _mergeprefix_l1;
  }
}
