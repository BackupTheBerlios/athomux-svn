package args;

import types.*;

/**
 * The class Args shall represent the struct args in Athomux defined in the "common.h" 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Args {

	public Addr_t log_addr;
	public Len_t log_len;
	public Addr_t try_addr;
	public Len_t try_len;
	public Off_t offset;
	public Off_t offset_max; 
	public Paddr_t phys_addr;
	public Plen_t phys_len;
	//privateMand_t mandate; ????
	public boolean forwrite;
	public boolean clear;
	public boolean melt;
	public int direction; 
	public int prio; 
	public boolean  reader;
	public boolean exclu;
	public int data_lock; 
	public int addr_lock; 
	public int action; 
	public boolean destr;
	public boolean constr;
	public int success;
	public int version;
	public int op_code;
	
	public boolean where;
	
	int getSuccess(){
		return success;
	}
	
}
