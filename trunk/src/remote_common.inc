/* Author: Hardy Kahl
 * Copyright: Hardy Kahl
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

@.define @.checkjump (_checkjumpprefix_condition,_checkjumpprefix_target,_checkjumpprefix_text...)
{ 
  if(_checkjumpprefix_condition) { 
    @.msg("error: ", "("@.string{@.protect{_checkjumpprefix_condition}}"): "_checkjumpprefix_text ...);
    goto _checkjumpprefix_target;
  }
}




@.define set_header(struct rpc_header_t *_setheaderprefix_header, uns1 _setheaderprefix_type) {
  _setheaderprefix_header->identification = RPC_IDENTIFICATION;
  _setheaderprefix_header->version = RPC_VERSION;
  _setheaderprefix_header->type = _setheaderprefix_type;
  _setheaderprefix_header->param_len = 0;
  _setheaderprefix_header->data_len = 0;
}




// creates a new packet.
// packet can be filled using the insert_* functions.
// make sure you call send_packet or delete_packet at the end.
@.define new_packet(_newpacketprefix_input, const uns1 _newpacketprefix_type, const len_t _newpacketprefix_userlen) => (success_t _newpacketprefix_success, struct packet_t _newpacketprefix_packet)
{
  success_t _newpacketprefix_succ;
  plen_t _newpacketprefix_plen;

  @=call :<_newpacketprefix_input$gadrcreateget [@mandate] (sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)+_newpacketprefix_userlen) => (_newpacketprefix_succ, _newpacketprefix_packet.start, _newpacketprefix_packet.len, _newpacketprefix_packet.paddr, _newpacketprefix_plen);
  if ((_newpacketprefix_succ!=TRUE) || (_newpacketprefix_packet.len!=_newpacketprefix_plen)) {
    @.errmsg("cannot create new packet");
    _newpacketprefix_success = FALSE;
  }
  else {
    struct rpc_header_t *_newpacketprefix_header = MAKE_PTR(_newpacketprefix_packet.paddr);
    set_header(_newpacketprefix_header, _newpacketprefix_type);
    _newpacketprefix_success = TRUE;
  }
}




@.define insert_args(struct packet_t *_insertargsprefix_packet, const struct args *_insertargsprefix_args) => (success_t _insertargsprefix_success)
{
  struct rpc_args_t *_insertargsprefix_prc_args = MAKE_PTR(_insertargsprefix_packet->paddr+sizeof(struct rpc_header_t));
  memcpy(&(_insertargsprefix_prc_args->plain_args), _insertargsprefix_args, sizeof(struct args));

  if (((_insertargsprefix_args->op_code == opcode_instconn) || (_insertargsprefix_args->op_code == opcode_deinstconn) || (_insertargsprefix_args->op_code == opcode_connect) || (_insertargsprefix_args->op_code == opcode_disconnect) || (_insertargsprefix_args->op_code == opcode_getconn) || (_insertargsprefix_args->op_code == opcode_findconn)) && (_insertargsprefix_args->conn1 != NULL)) {
    memcpy(&(_insertargsprefix_prc_args->conn1), _insertargsprefix_args->conn1, sizeof(struct conn_info));
    _insertargsprefix_prc_args->conn1_valid=TRUE;
  }
  else _insertargsprefix_prc_args->conn1_valid=FALSE;

  if ((_insertargsprefix_args->op_code == opcode_connect) && (_insertargsprefix_args->conn2 != NULL)) {
    memcpy(&(_insertargsprefix_prc_args->conn2), _insertargsprefix_args->conn2, sizeof(struct conn_info));
    _insertargsprefix_prc_args->conn2_valid=TRUE;
  }
  else _insertargsprefix_prc_args->conn2_valid=FALSE;

  if (((_insertargsprefix_args->op_code == opcode_getconn) || (_insertargsprefix_args->op_code == opcode_findconn)) && (_insertargsprefix_args->res_conn != NULL)) {
    memcpy(&(_insertargsprefix_prc_args->res_conn), _insertargsprefix_args->res_conn, sizeof(struct conn_info));
    _insertargsprefix_prc_args->res_conn_valid=TRUE;
  }
  else _insertargsprefix_prc_args->res_conn_valid=FALSE;

  _insertargsprefix_success = TRUE;
}




// FIXME: insert_param must alway be called before insert_data !!!!
@.define insert_param(struct packet_t *_insertparamprefix_packet, const char *_insertparamprefix_param) => (success_t _insertparamprefix_success)
{
  plen_t _insertparamprefix_param_len = strlen(_param)+1;
  if (_insertparamprefix_packet->len<sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)+_insertparamprefix_param_len) {
    @.errmsg("packet too small");
    _insertparamprefix_success = FALSE;
  }
  else {
    struct rpc_header_t *_insertparamprefix_header = MAKE_PTR(_insertparamprefix_packet->paddr);
    _insertparamprefix_header->param_len = _insertparamprefix_param_len;
    memcpy(MAKE_PTR(_insertparamprefix_packet->paddr+sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)), _insertparamprefix_param, _insertparamprefix_param_len);
    _insertparamprefix_success = TRUE;
  }
}




@.define insert_data(struct packet_t *_insertdataprefix_packet, const paddr_t _insertdataprefix_data_start, const len_t _insertdataprefix_data_len) => (success_t _insertdataprefix_success)
{
  struct rpc_header_t *_insertdataprefix_header = MAKE_PTR(_insertdataprefix_packet->paddr);
  if (_insertdataprefix_packet->len<sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)+_insertdataprefix_header->param_len+_insertdataprefix_data_len) {
    @.errmsg("packet too small");
    _insertdataprefix_success = FALSE;
  }
  else {
    _insertdataprefix_header->data_len = _insertdataprefix_data_len;
    memcpy(MAKE_PTR(_insertdataprefix_packet->paddr+sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)+_insertdataprefix_header->param_len), MAKE_PTR(_insertdataprefix_data_start), _insertdataprefix_data_len);
    _insertdataprefix_success = TRUE;
  }
}




@.define send_packet(_sendpacketprefix_input, const struct packet_t *_sendpacketprefix_packet) => (success_t _sendpacketprefix_success)
{
  @=call :<_sendpacketprefix_input$putpadr [@mandate] (_sendpacketprefix_packet->start, _sendpacketprefix_packet->len, FALSE) => (_sendpacketprefix_success);
}




// fetches the next packet from input stream.
// packet contents can be accessed via the extract_* functions.
// make sure you call delete_packet at the end.
@.define get_packet(_getpacketprefix_input) => (success_t _getpacketprefix_success, struct packet_t _getpacketprefix_packet)
{
  success_t _getpacketprefix_succ;
  plen_t _getpacketprefix_plen;

  @=call :<_getpacketprefix_input$gadrgettranswait [@mandate] (0, TRUE, action_wait, FALSE, prio_normal, 0) => (_getpacketprefix_succ, _getpacketprefix_packet.start, _getpacketprefix_packet.len, _getpacketprefix_packet.paddr, _getpacketprefix_plen);
  if (_getpacketprefix_succ!=TRUE) {
    @.errmsg("cannot get next packet");
    _getpacketprefix_success = FALSE;
  }
  else if (_getpacketprefix_packet.len!=_getpacketprefix_plen) {
    @.errmsg("cannot get whole packet");
    _getpacketprefix_success = FALSE;
  }
  else if (_getpacketprefix_packet.len<sizeof(struct rpc_header_t)) {
    @.errmsg("packet too small");
    _getpacketprefix_success = FALSE;
  }
  else _getpacketprefix_success = TRUE;
}




// checks if the given packet is a valid RPC packet with correct version number.
@.define check_header(const struct packet_t *_checkheaderprefix_packet, const uns1 _checkheaderprefix_type) => (success_t _checkheaderprefix_success)
{
  _checkheaderprefix_success = TRUE;

  struct rpc_header_t *_checkheaderprefix_header = MAKE_PTR(_checkheaderprefix_packet->paddr);

  if (_checkheaderprefix_header->identification!=RPC_IDENTIFICATION) {
    @.errmsg("wrong packet identification");
    _checkheaderprefix_success = FALSE;
  }
  if (_checkheaderprefix_header->version!=RPC_VERSION) {
    @.errmsg("wrong packet version");
    _checkheaderprefix_success = FALSE;
  }
  if (_checkheaderprefix_header->type!=_checkheaderprefix_type) {
    @.errmsg("wrong packet type");
    _checkheaderprefix_success = FALSE;
  }
}




@.define extract_args(const struct packet_t *_extractargsprefix_packet) => (success_t _extractargsprefix_success, struct args *_extractargsprefix_args)
{
  struct rpc_args_t *_extractargsprefix_rpc_args = MAKE_PTR(_extractargsprefix_packet->paddr + sizeof(struct rpc_header_t));
  _extractargsprefix_args = &(_extractargsprefix_rpc_args->plain_args);

  if (_extractargsprefix_rpc_args->conn1_valid == TRUE) _extractargsprefix_args->conn1 = &(_extractargsprefix_rpc_args->conn1);
  else _extractargsprefix_args->conn1 = NULL;
  if (_extractargsprefix_rpc_args->conn2_valid == TRUE) _extractargsprefix_args->conn2 = &(_extractargsprefix_rpc_args->conn2);
  else _extractargsprefix_args->conn2 = NULL;
  if (_extractargsprefix_rpc_args->res_conn_valid == TRUE) _extractargsprefix_args->res_conn = &(_extractargsprefix_rpc_args->res_conn);
  else _extractargsprefix_args->res_conn = NULL;

  if (_extractargsprefix_packet->len<sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)) {
    @.errmsg("packet too small");
    _extractargsprefix_success = FALSE;
  }
  else _extractargsprefix_success = TRUE;
}




@.define extract_param(const struct packet_t *_extractparamprefix_packet) => (success_t _extractparamprefix_success, char *_extractparamprefix_param)
{
  struct rpc_header_t *_extractparamprefix_header = MAKE_PTR(_extractparamprefix_packet->paddr);
  _extractparamprefix_param = MAKE_PTR(_extractparamprefix_packet->paddr+sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t));

  if (_extractparamprefix_packet->len<sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)+_extractparamprefix_header->param_len) {
    @.errmsg("packet too small");
    _extractparamprefix_success = FALSE;
  }
  else _extractparamprefix_success = TRUE;
}




@.define extract_data(const struct packet_t *_extractdataprefix_packet) => (success_t _extractdataprefix_success, paddr_t _extractdataprefix_bufstart, len_t _extractdataprefix_buflen)
{
  struct rpc_header_t *_extractdataprefix_header = MAKE_PTR(_extractdataprefix_packet->paddr);
  _extractdataprefix_bufstart = _extractdataprefix_packet->paddr+sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)+_extractdataprefix_header->param_len;
  _extractdataprefix_buflen = _extractdataprefix_header->data_len;

  if (_extractdataprefix_packet->len<sizeof(struct rpc_header_t)+sizeof(struct rpc_args_t)+_extractdataprefix_header->param_len+_extractdataprefix_header->data_len) {
    @.errmsg("packet too small");
    _extractdataprefix_success = FALSE;
  }
  else _extractdataprefix_success = TRUE;
}




@.define delete_packetr(_deletepacketrprefix_input, struct packet_t *_deletepacketrprefix_packet) => (success_t _deletepacketrprefix_success)
{
  @=call :<_deletepacketrprefix_input$putdeletepadr [@mandate] (_deletepacketrprefix_packet->start, _deletepacketrprefix_packet->len, TRUE) => (_deletepacketrprefix_success);
}




// creates a new request packet with args and param and (if data_len > 0) data.
@.define send_req(_sendreqprefix_input, struct args *_sendreqprefix_req_args, const char *_sendreqprefix_param, paddr_t _sendreqprefix_data_start, plen_t _sendreqprefix_data_len) => (success_t _sendreqprefix_success)
{
  struct packet_t _sendreqprefix_packet;
  plen_t _sendreqprefix_param_len = strlen(_sendreqprefix_param)+1;

  new_packet(_sendreqprefix_input, RPC_REQ, _sendreqprefix_param_len+_sendreqprefix_data_len) => (_sendreqprefix_success, _sendreqprefix_packet);
  @.check(_sendreqprefix_success!=TRUE, "cannot create packet - FIXME: switch to error state");
  insert_args(&_sendreqprefix_packet, _sendreqprefix_req_args) => (_sendreqprefix_success);
  @.check(_sendreqprefix_success!=TRUE, "cannot insert _args - FIXME: switch to error state");
  insert_param(&_sendreqprefix_packet, _sendreqprefix_param) => (_sendreqprefix_success);
  @.check(_sendreqprefix_success!=TRUE, "cannot insert _param - FIXME: switch to error state");

  if (_sendreqprefix_data_len>0) {
    insert_data(&_sendreqprefix_packet, _sendreqprefix_data_start, _sendreqprefix_data_len) => (_sendreqprefix_success);
    @.check(_sendreqprefix_success!=TRUE, "cannot insert data - FIXME: switch to error state");
  }

  send_packet(_sendreqprefix_input, &_sendreqprefix_packet) => (_sendreqprefix_success);
  @.check(_sendreqprefix_success!=TRUE, "cannot send packet - FIXME: switch to error state");
}




// creates a new response packet with args and (if data_len > 0) data.
@.define send_rsp(_sendrspprefix_input, struct args *_sendrspprefix_rsp_args, paddr_t _sendrspprefix_data_start, plen_t _sendrspprefix_data_len) => (success_t _sendrspprefix_success)
{
  struct packet_t _sendrspprefix_rpacket;

  new_packet(_sendrspprefix_input, RPC_RSP, _sendrspprefix_data_len) => (_sendrspprefix_success, _sendrspprefix_rpacket);
  @.check(_sendrspprefix_success!=TRUE, "cannot create packet - FIXME: switch to error state");
  insert_args(&_sendrspprefix_rpacket, _sendrspprefix_rsp_args) => (_sendrspprefix_success);
  @.check(_sendrspprefix_success!=TRUE, "cannot insert _args - FIXME: switch to error state");

  if (_sendrspprefix_data_len>0) {
    insert_data(&_sendrspprefix_rpacket, _sendrspprefix_data_start, _sendrspprefix_data_len) => (_sendrspprefix_success);
    @.check(_sendrspprefix_success!=TRUE, "cannot insert data - FIXME: switch to error state");
  }

  send_packet(_sendrspprefix_input, &_sendrspprefix_rpacket) => (_sendrspprefix_success);
  @.check(_sendrspprefix_success!=TRUE, "cannot send packet - FIXME: switch to error state");
}




// creates a new response packet with args.success=FALSE.
@.define send_error_packet(_senderrorpacketprefix_input) => (success_t _senderrorpacketprefix_success)
{
  struct args _senderrorpacketprefix_rsp_args;
  _senderrorpacketprefix_rsp_args.success = FALSE;
  
  send_rsp(_senderrorpacketprefix_input, &_senderrorpacketprefix_rsp_args, 0, 0) => (_senderrorpacketprefix_success);
}
