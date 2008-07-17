

package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for the Operation lock<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $lock [mandate] (addr_t log_addr, len_t log_len, lock_t
  data_lock := lock_write, lock_t addr_lock := lock_read,
  addr_t try_addr := log_addr, len_t try_len := log_len,
  action_t action := action_wait) => (success_t success,
  addr_t try_addr, len_t try_len)
 */


public class ArgsLock extends Args {
	
	public ArgsLock(Addr_t log_addr, Len_t log_len, int data_lock, int addr_lock, Addr_t try_addr, Len_t try_len, int action){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.data_lock = data_lock;
		this.addr_lock = addr_lock;
		this.try_addr = try_addr;
		this.try_len = try_len;
		this.action = action;
	}

	
	Addr_t getTry_addr(){
		return try_addr;
	}
	
	Len_t getTry_len(){
		return try_len;
	}
	
}
