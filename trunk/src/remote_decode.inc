/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

// decodes the contents of given packet and translates to an output_call.
// the results are sent back (including data block in case of $trans).
@.func decode(struct packet_t *_decodeprefix_packet) => (success_t _decodeprefix_success)
{
  _decodeprefix_success = FALSE;

  success_t _decodeprefix_succ;

  char *_decodeprefix_param;
  struct args *_decodeprefix_args;

  paddr_t _decodeprefix_s_phys_addr;
  paddr_t _decodeprefix_s_log_len;

  check_header(_decodeprefix_packet, RPC_REQ) => (_decodeprefix_succ);
  @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "wrong packet header");

  extract_args(_decodeprefix_packet) => (_decodeprefix_succ, _decodeprefix_args);
  @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "cannot extract _args");

  extract_param(_decodeprefix_packet) => (_decodeprefix_succ, _decodeprefix_param);
  @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "cannot extract _param");

  // FIXME: all ops and combiops, that should be decoded, have to be handled in this switch statement!!!
  switch (_decodeprefix_args->op_code) {
  case opcode_trans:
  case opcode_transwait:
    @.checkjump((_decodeprefix_args->direction!=direct_read) && (_decodeprefix_args->direction!=direct_write), error_rsp, "invalid direction");

    // save log_len and phys_addr
    _decodeprefix_s_phys_addr = _decodeprefix_args->phys_addr;
    _decodeprefix_s_log_len = _decodeprefix_args->log_len;
      
    if (_decodeprefix_args->direction==direct_read) {
      // get temporary buffer
      addr_t _decodeprefix_buf_log_addr;
      len_t _decodeprefix_buf_log_len;
      paddr_t _decodeprefix_buf_phys_addr;
      plen_t _decodeprefix_buf_phys_len;
      @=call :<mem$gadrcreateget [@mandate] (_decodeprefix_args->log_len) => (_decodeprefix_succ, _decodeprefix_buf_log_addr, _decodeprefix_buf_log_len, _decodeprefix_buf_phys_addr, _decodeprefix_buf_phys_len);
      @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "cannot allocate buffer for reading");
    
      // cannot transfer all the data if buffer is too small
      if (_decodeprefix_buf_phys_len < _decodeprefix_args->log_len) _decodeprefix_args->log_len = _decodeprefix_buf_phys_len;
    
      // set physical address to buffer
      _decodeprefix_args->phys_addr = _decodeprefix_buf_phys_addr;
    
      // call $trans in input
      @=call :<in$trans [@mandate] (_decodeprefix_args) : _decodeprefix_param;
      
      // restore log_len and phys_addr
      _decodeprefix_args->phys_addr = _decodeprefix_s_phys_addr;
      _decodeprefix_args->log_len = _decodeprefix_s_log_len;
      
      // transfer _args + data back to client
      if (_decodeprefix_args->success==TRUE) {
        // transfer data that was read back
        send_rsp(cpw, _decodeprefix_args, _decodeprefix_buf_phys_addr, _decodeprefix_args->phys_len) => (_decodeprefix_succ);
      }
      else {
        // transfer only _args back
        send_rsp(cpw, _decodeprefix_args, 0, 0) => (_decodeprefix_succ);
      }

      // destroy buffer
      success_t _decodeprefix_succ_free;
      @=call :<mem$putdelete [@mandate] (_decodeprefix_buf_log_addr, _decodeprefix_buf_log_len) => (_decodeprefix_succ_free);
      @.checkjump(_decodeprefix_succ_free!=TRUE, error_state, "cannot free buffer");

      @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "cannot send response");
    }
    else {
      extract_data(_decodeprefix_packet) => (_decodeprefix_succ, _decodeprefix_args->phys_addr, _decodeprefix_args->log_len);
      @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "cannot extract data");
      
      // call $trans in input
      @=call :<in$trans [@mandate] (_decodeprefix_args) : _decodeprefix_param;
      
      // restore log_len and phys_addr
      _decodeprefix_args->phys_addr = _decodeprefix_s_phys_addr;
      _decodeprefix_args->log_len = _decodeprefix_s_log_len;
      
      // transfer _args back to client
      send_rsp(cpw, _decodeprefix_args, 0, 0) => (_decodeprefix_succ);
      @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "cannot send response");
    }
    break;
  case opcode_get:
    @.errmsg("opcode_get should be implemented on client side");
    goto error_rsp;
    break;
  case opcode_put:
    @.errmsg("opcode_put should be implemented on client side");
    goto error_rsp;
    break;
  case opcode_wait:
  case opcode_gadr:
  case opcode_padr:
  case opcode_create:
  case opcode_delete:
  case opcode_move:
  case opcode_lock:
  case opcode_unlock:

  case opcode_instbrick:
  case opcode_deinstbrick:
  case opcode_instconn:
  case opcode_deinstconn:
  case opcode_connect:
  case opcode_disconnect:
  case opcode_getconn:
  case opcode_findconn:

  case opcode_gadrcreate:
  case opcode_deletepadr:
    // call operation in input
    @=call :<in$op [@mandate] (_decodeprefix_args) : _decodeprefix_param;
    
    // transfer _args back to client
    send_rsp(cpw, _decodeprefix_args, 0, 0) => (_decodeprefix_succ);
    @.checkjump(_decodeprefix_succ!=TRUE, error_rsp, "cannot send response");
    break;
  default:
    @.errmsg("unimplemented opcode %d", @op_code);
    goto error_rsp;
  }

  _decodeprefix_success = TRUE;
  return;

  error_rsp:
  send_error_packet(cpw) => (_decodeprefix_succ);
  @.checkjump(_decodeprefix_succ!=TRUE, error_state, "cannot send error packet");
  _decodeprefix_success = TRUE;
  return;

  error_state:
  @.err("FIXME: switch to error state");
}




// encode input_call with given _args and _param, send the packet, wait for response and set _args to result.
@.func encode_rev(struct args *_encoderevprefix_args, const char *_encoderevprefix_param) => (success_t _encoderevprefix_success)
{
  _encoderevprefix_success = FALSE;

  success_t _encoderevprefix_succ;
  struct packet_t _encoderevprefix_packet;
  struct args *_encoderevprefix_rsp_args;
  
  send_req(icpw, _encoderevprefix_args, _encoderevprefix_param, 0, 0) => (_encoderevprefix_succ);

  @.check(_encoderevprefix_succ!=TRUE, "WARNING: cannot send packet");
  
  // --------------------
  // waiting for response
  // --------------------

  get_packet(icpr) => (_encoderevprefix_succ, _encoderevprefix_packet);
  @.checkjump(_encoderevprefix_succ!=TRUE, error_state, "cannot get packet");

  check_header(&_encoderevprefix_packet, RPC_RSP) => (_encoderevprefix_succ);
  @.checkjump(_encoderevprefix_succ!=TRUE, error_state_free, "invalid header");

  extract_args(&_encoderevprefix_packet) => (_encoderevprefix_succ, _encoderevprefix_rsp_args);
  @.checkjump(_encoderevprefix_succ!=TRUE, error_state_free, "cannot extract _args");

  success_t _encoderevprefix_succ_s = _encoderevprefix_rsp_args->success;
  if (_encoderevprefix_succ_s==TRUE) {
    memcpy(_encoderevprefix_args, _encoderevprefix_rsp_args, sizeof(struct args));
    if (_encoderevprefix_rsp_args->conn1!=NULL) memcpy(_encoderevprefix_args->conn1, _encoderevprefix_rsp_args->conn1, sizeof(struct conn_info));
    else _encoderevprefix_args->conn1=NULL;
    if (_encoderevprefix_rsp_args->conn2!=NULL) memcpy(_encoderevprefix_args->conn2, _encoderevprefix_rsp_args->conn2, sizeof(struct conn_info));
    else _encoderevprefix_args->conn2=NULL;
    if (_encoderevprefix_rsp_args->res_conn!=NULL) memcpy(_encoderevprefix_args->res_conn, _encoderevprefix_rsp_args->res_conn, sizeof(struct conn_info));
    else _encoderevprefix_args->res_conn=NULL;
  }

  delete_packetr(icpr, &_encoderevprefix_packet) => (_encoderevprefix_succ);
  @.checkjump(_encoderevprefix_succ!=TRUE, error_state, "cannot delete packet");

  _encoderevprefix_success = _encoderevprefix_succ_s;
  return;

  error_state_free:
  delete_packetr(icpr, &_encoderevprefix_packet) => (_encoderevprefix_succ);

  error_state:
  @.err("FIXME: switch to error state");
}
