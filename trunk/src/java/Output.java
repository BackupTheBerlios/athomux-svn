import types.*;
import args.*;


/**
 * All Output Operations are (not yet) implemented here 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */



public class Output implements Operations{
	
	public String name;
	
	
/*******************************
 * _op() or  _*() have to be implemented! 
 * 
 */	
	public void _op(Args args,String param){
		switch(args.op_code){
			case Opcode_t.opcode_output_init:
				_output_init((ArgsOutput_init)args, param);
			case Opcode_t.opcode_trans:
				_trans((ArgsTrans)args, param);
			case Opcode_t.opcode_wait:
				_wait((ArgsWait)args, param);
			case Opcode_t.opcode_get:
				_get((ArgsGet)args, param);
			case Opcode_t.opcode_put:
				_put((ArgsPut)args, param);
			case Opcode_t.opcode_lock:
				_lock((ArgsLock)args, param);
			case Opcode_t.opcode_move:
				_move((ArgsMove)args, param);
			case Opcode_t.opcode_create:
				_create((ArgsCreate)args, param);
			case Opcode_t.opcode_delete:
				_delete((ArgsDelete)args, param);
			case Opcode_t.opcode_unlock:
				_unlock((ArgsUnlock)args, param);
			case Opcode_t.opcode_gadr:
				_gadr((ArgsGadr)args, param);
			case Opcode_t.opcode_padr:
				_padr((ArgsPadr)args, param);
			break;	
		}
		
	}
	public void _output_init(ArgsOutput_init args, String param){
		
	}
	public void _trans(ArgsTrans args, String param){
		
	}
	public void _wait(ArgsWait args, String param){
		
	}
	public void _get(ArgsGet args, String param){
		
	}
	public void _put(ArgsPut args, String param){
		
	}
	public void _lock(ArgsLock args, String param){
		
	}
	public void _move(ArgsMove args, String param){
		
	}
	public void _create(ArgsCreate args, String param){
		
	}
	public void _delete(ArgsDelete args, String param){
		
	}
	public void _unlock(ArgsUnlock args, String param){
		
	}
	public void _gadr(ArgsGadr args, String param){
		
	}
	public void _padr(ArgsPadr args, String param){
		
	}
/*************************************** */	
	
	public ArgsOutput_init output_init(boolean destr, boolean constr, boolean clear, String param)
	{
	    ArgsOutput_init args = new ArgsOutput_init(destr,constr,clear);
		_op(args, param);
		return args;
	}
	public ArgsTrans trans(Addr_t log_addr, Len_t log_len, Paddr_t phys_addr,
			int direction, int prio, String param){
		
		ArgsTrans args = new ArgsTrans(log_addr,log_len,phys_addr,direction,prio);
		_op(args, param);
		return args;
	}
	public ArgsWait wait(Addr_t log_addr, Len_t log_len, int prio, int action, String param)
	{
		ArgsWait args = new ArgsWait(log_addr,log_len,prio,action);
		_op(args, param);
		return args;
	}
	public ArgsGet get(Addr_t log_addr, Len_t log_len, boolean forwrite, String param)
	{
		ArgsGet args = new ArgsGet(log_addr,log_len,forwrite);
		_op(args, param);
		return args;	
	}
	public ArgsPut put(Addr_t log_addr, Len_t log_len, int prio, String param)
	{	
		ArgsPut args = new ArgsPut(log_addr,log_len,prio);
		_op(args, param);
		return args;
	}
	public ArgsLock lock(Addr_t log_addr, Len_t log_len, int data_lock, int addr_lock, Addr_t try_addr, Len_t try_len, int action, String param)
	{
		ArgsLock args = new ArgsLock(log_addr,log_len,data_lock,addr_lock,try_addr,try_len,action);
		_op(args, param);
		return args;
	}
	public ArgsUnlock unlock(Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len, String param)
	{
		ArgsUnlock args = new ArgsUnlock(log_addr,log_len,try_addr,try_len);
		_op(args, param);
		return args;
	}
	public ArgsCreate create (Addr_t log_addr,Len_t log_len, boolean clear, boolean melt, String param)
	{
		ArgsCreate args = new ArgsCreate(log_addr,log_len,clear,melt);
		_op(args, param);
		return args;
	}
	public ArgsDelete delete(Addr_t log_addr, Len_t log_len, boolean melt, String param)
	{
		ArgsDelete args = new ArgsDelete(log_addr,log_len,melt);
		_op(args, param);
		return args;	
	}
	public ArgsMove move(Addr_t log_addr, Len_t log_len,Off_t offset,Off_t offset_max, String param)
	{
		ArgsMove args = new ArgsMove(log_addr,log_len,offset,offset_max);
		_op(args, param);
		return args;	
	}
	public ArgsGadr gadr(Len_t log_len, boolean where, boolean exclu, int action, Len_t try_len, String param)
	{
		ArgsGadr args = new ArgsGadr(log_len,where,exclu,action,try_len);
		_op(args, param);
		return args;
	}
	public ArgsPadr padr (Addr_t log_addr, Len_t log_len, boolean where, String param)
	{
		ArgsPadr args = new ArgsPadr(log_addr,log_len,where);
		_op(args, param);
		return args;
	}
	public void _input_init(ArgsInput_Init args, String param) {
		
	}
	public void _retract(ArgsRetract args, String param) {
		
		
	}
	public ArgsInput_Init input_init(boolean destr, boolean constr, boolean clear, String param) {
		
		return null;
	}
	public ArgsRetract retract(int prio, Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len, String param) {
		
		return null;
	}
}

