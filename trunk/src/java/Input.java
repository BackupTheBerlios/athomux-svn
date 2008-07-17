import args.*;
import types.*;

/**
 * All Input Operations are (not yet) implemented here 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */


public class Input implements Operations{

	public String name;
	
	public void _op(Args args,String param){
		switch(args.op_code){
			case Opcode_t.opcode_input_init:
				_input_init((ArgsInput_Init)args,param);
			case Opcode_t.opcode_retract:
				_retract((ArgsRetract)args,param);
			break;	
		}
		
	}

	public void _input_init(ArgsInput_Init args, String param){
		
	}
	public void _retract(ArgsRetract args, String param){
		
	}
	
	public ArgsInput_Init input_init(boolean destr, boolean constr, boolean clear, String param)
	{
		ArgsInput_Init args = new ArgsInput_Init(destr,constr,clear);
		_op(args, param);
		return args;
	}
	
	public ArgsRetract retract(int prio, Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len, String param)
	{
		ArgsRetract args = new ArgsRetract(prio,log_addr,log_len,try_addr,try_len);
		_op(args, param);
		return args;
	}

	public void _create(ArgsCreate args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _delete(ArgsDelete args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _gadr(ArgsGadr args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _get(ArgsGet args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _lock(ArgsLock args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _move(ArgsMove args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _output_init(ArgsOutput_init args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _padr(ArgsPadr args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _put(ArgsPut args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _trans(ArgsTrans args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _unlock(ArgsUnlock args, String param) {
		// TODO Auto-generated method stub
		
	}

	public void _wait(ArgsWait args, String param) {
		// TODO Auto-generated method stub
		
	}

	public ArgsCreate create(Addr_t log_addr, Len_t log_len, boolean clear, boolean melt, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsDelete delete(Addr_t log_addr, Len_t log_len, boolean melt, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsGadr gadr(Len_t log_len, boolean where, boolean exclu, int action, Len_t try_len, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsGet get(Addr_t log_addr, Len_t log_len, boolean forwrite, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsLock lock(Addr_t log_addr, Len_t log_len, int data_lock, int addr_lock, Addr_t try_addr, Len_t try_len, int action, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsMove move(Addr_t log_addr, Len_t log_len, Off_t offset, Off_t offset_max, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsOutput_init output_init(boolean destr, boolean constr, boolean clear, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsPadr padr(Addr_t log_addr, Len_t log_len, boolean where, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsPut put(Addr_t log_addr, Len_t log_len, int prio, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsTrans trans(Addr_t log_addr, Len_t log_len, Paddr_t phys_addr, int direction, int prio, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsUnlock unlock(Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len, String param) {
		// TODO Auto-generated method stub
		return null;
	}

	public ArgsWait wait(Addr_t log_addr, Len_t log_len, int prio, int action, String param) {
		// TODO Auto-generated method stub
		return null;
	}
}
