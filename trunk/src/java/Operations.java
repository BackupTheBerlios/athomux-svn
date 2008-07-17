/************
Author: Matthäus Szturc, Teresa Gamez Zerban	
Copyright: Matthäus Szturc, Teresa Gamez Zerban	
License: see files SOFTWARE-LICENSE, PATENT-LICENSE
************/


import types.Addr_t;
import types.Len_t;
import types.Off_t;
import types.Opcode_t;
import types.Paddr_t;
import args.Args;
import args.ArgsCreate;
import args.ArgsDelete;
import args.ArgsGadr;
import args.ArgsGet;
import args.ArgsInput_Init;
import args.ArgsLock;
import args.ArgsMove;
import args.ArgsOutput_init;
import args.ArgsPadr;
import args.ArgsPut;
import args.ArgsRetract;
import args.ArgsTrans;
import args.ArgsUnlock;
import args.ArgsWait;

/**
 * Interface that includes all input and output operations
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */



public interface Operations {
	public void _op(Args args,String param);

	// Input Operations
	
	public void _input_init(ArgsInput_Init args, String param);
	public void _retract(ArgsRetract args, String param);
	
	public ArgsInput_Init input_init(boolean destr, boolean constr, boolean clear, String param);	
	public ArgsRetract retract(int prio, Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len, String param);

	//Output Operations
	public void _output_init(ArgsOutput_init args, String param);
	public void _trans(ArgsTrans args, String param);
	public void _wait(ArgsWait args, String param);
	public void _get(ArgsGet args, String param);
	public void _put(ArgsPut args, String param);
	public void _lock(ArgsLock args, String param);
	public void _move(ArgsMove args, String param);
	public void _create(ArgsCreate args, String param);
	public void _delete(ArgsDelete args, String param);
	public void _unlock(ArgsUnlock args, String param);
	public void _gadr(ArgsGadr args, String param);
	public void _padr(ArgsPadr args, String param);
	
	public ArgsOutput_init output_init(boolean destr, boolean constr, boolean clear, String param);
	public ArgsTrans trans(Addr_t log_addr, Len_t log_len, Paddr_t phys_addr,
			int direction, int prio, String param);
	public ArgsWait wait(Addr_t log_addr, Len_t log_len, int prio, int action, String param);
	public ArgsGet get(Addr_t log_addr, Len_t log_len, boolean forwrite, String param);
	public ArgsPut put(Addr_t log_addr, Len_t log_len, int prio, String param);
	public ArgsLock lock(Addr_t log_addr, Len_t log_len, int data_lock, int addr_lock, Addr_t try_addr, Len_t try_len, int action, String param);
	public ArgsUnlock unlock(Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len, String param);
	public ArgsCreate create (Addr_t log_addr,Len_t log_len, boolean clear, boolean melt, String param);
	public ArgsDelete delete(Addr_t log_addr, Len_t log_len, boolean melt, String param);
	public ArgsMove move(Addr_t log_addr, Len_t log_len,Off_t offset,Off_t offset_max, String param);
	public ArgsGadr gadr(Len_t log_len, boolean where, boolean exclu, int action, Len_t try_len, String param);
	public ArgsPadr padr (Addr_t log_addr, Len_t log_len, boolean where, String param);


	
}
