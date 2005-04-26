/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

// encode output_call with given _args and _param, send the packet, wait for response and set _args to result.
// in case of $trans, the phys buffer will be sent or filled with the response data.
@.func encode(struct args *_encodeprefix_args, const char *_encodeprefix_param) => (success_t _encodeprefix_success)
{
  _encodeprefix_success = FALSE;

  success_t _encodeprefix_succ;
  paddr_t _encodeprefix_bufstart = _encodeprefix_args->phys_addr;
  len_t _encodeprefix_buflen = _encodeprefix_args->log_len;
  struct packet_t _encodeprefix_packet;
  struct args *_encodeprefix_rsp_args;

  if (((_encodeprefix_args->op_code==opcode_trans) || (_encodeprefix_args->op_code==opcode_transwait)) && (_encodeprefix_args->direction == direct_write)) send_req(cpw, _encodeprefix_args, _encodeprefix_param, _encodeprefix_bufstart, _encodeprefix_buflen) => (_encodeprefix_succ);
  else send_req(cpw, _encodeprefix_args, _encodeprefix_param, 0, 0) => (_encodeprefix_succ);

  @.check(_encodeprefix_succ!=TRUE, "WARNING: cannot send packet");
  
  // --------------------
  // waiting for response
  // --------------------

  get_packet(cpr) => (_encodeprefix_succ, _encodeprefix_packet);
  @.checkjump(_encodeprefix_succ!=TRUE, error_state, "cannot get packet");

  check_header(&_encodeprefix_packet, RPC_RSP) => (_encodeprefix_succ);
  @.checkjump(_encodeprefix_succ!=TRUE, error_state_free, "invalid header");

  extract_args(&_encodeprefix_packet) => (_encodeprefix_succ, _encodeprefix_rsp_args);
  @.checkjump(_encodeprefix_succ!=TRUE, error_state_free, "cannot extract _args");

  success_t _encodeprefix_succ_s = _encodeprefix_rsp_args->success;
  if (_encodeprefix_succ_s==TRUE) {
    memcpy(_encodeprefix_args, _encodeprefix_rsp_args, sizeof(struct args));
    if (_encodeprefix_rsp_args->conn1!=NULL) memcpy(_encodeprefix_args->conn1, _encodeprefix_rsp_args->conn1, sizeof(struct conn_info));
    else _encodeprefix_args->conn1=NULL;
    if (_encodeprefix_rsp_args->conn2!=NULL) memcpy(_encodeprefix_args->conn2, _encodeprefix_rsp_args->conn2, sizeof(struct conn_info));
    else _encodeprefix_args->conn2=NULL;
    if (_encodeprefix_rsp_args->res_conn!=NULL) memcpy(_encodeprefix_args->res_conn, _encodeprefix_rsp_args->res_conn, sizeof(struct conn_info));
    else _encodeprefix_args->res_conn=NULL;

    if (((_encodeprefix_args->op_code==opcode_trans) || (_encodeprefix_args->op_code==opcode_transwait)) && (_encodeprefix_args->direction == direct_read)) {
      paddr_t _encodeprefix_paddr;
      len_t _encodeprefix_plen;
      extract_data(&_encodeprefix_packet) => (_encodeprefix_succ, _encodeprefix_paddr, _encodeprefix_plen);
      @.checkjump(_encodeprefix_buflen<_encodeprefix_plen, error_state_free, "too much data");

      // copy data from packet to destination
      memcpy(MAKE_PTR(_encodeprefix_bufstart), MAKE_PTR(_encodeprefix_paddr), _encodeprefix_plen);
      @.checkjump(_encodeprefix_succ!=TRUE, error_state_free, "cannot extract data");
    }
  }

  delete_packetr(cpr, &_encodeprefix_packet) => (_encodeprefix_succ);
  @.checkjump(_encodeprefix_succ!=TRUE, error_state, "cannot delete packet");

  _encodeprefix_success =  _encodeprefix_succ_s;
  return;

  error_state_free:
  delete_packetr(cpr, &_encodeprefix_packet) => (_encodeprefix_succ);

  error_state:
  @.err("FIXME: switch to error state");
}




// decodes the contents of given packet and translates to an input_call.
// the resulting _args are sent back.
@.func decode_rev(struct packet_t *_decoderevprefix_packet) => (success_t _decoderevprefix_success)
{
  _decoderevprefix_success = FALSE;

  success_t _decoderevprefix_succ;

  char *_decoderevprefix_param;
  struct args *_decoderevprefix_args;

  check_header(_decoderevprefix_packet, RPC_REQ) => (_decoderevprefix_succ);
  @.checkjump(_decoderevprefix_succ!=TRUE, error_rsp, "wrong packet header");

  extract_args(_decoderevprefix_packet) => (_decoderevprefix_succ, _decoderevprefix_args);
  @.checkjump(_decoderevprefix_succ!=TRUE, error_rsp, "cannot extract _args");

  extract_param(_decoderevprefix_packet) => (_decoderevprefix_succ, _decoderevprefix_param);
  @.checkjump(_decoderevprefix_succ!=TRUE, error_rsp, "cannot extract _param");

  switch (_decoderevprefix_args->op_code) {
  case opcode_retract:
    // call operation in input
    @=inputcall :>out$op [@mandate] (_decoderevprefix_args) : _decoderevprefix_param;
    
    // transfer _args back to client
    send_rsp(icpw, _decoderevprefix_args, 0, 0) => (_decoderevprefix_succ);
    @.checkjump(_decoderevprefix_succ!=TRUE, error_rsp, "cannot send response");
    break;
  default:
    @.errmsg("unimplemented opcode %d", @op_code);
    goto error_rsp;
  }

  _decoderevprefix_success = TRUE;
  return;

  error_rsp:
  send_error_packet(icpw) => (_decoderevprefix_succ);
  @.checkjump(_decoderevprefix_succ!=TRUE, error_state, "cannot send error packet");
  _decoderevprefix_success = TRUE;
  return;

  error_state:
  @.err("FIXME: switch to error state");
}
