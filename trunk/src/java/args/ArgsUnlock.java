package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for the operation unlock<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $unlock [mandate] (addr_t log_addr, len_t log_len,
  addr_t try_addr := log_addr, len_t try_len := log_len) =>
  (success_t success, addr_t try_addr, len_t try_len)
 */

public class ArgsUnlock extends Args {
	
	public ArgsUnlock(Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.try_addr = try_addr;
		this.try_len = try_len;
	}
			
	void setTry_addr(Addr_t try_addr){
		this.try_addr = try_addr;
	}
	
	Addr_t getTry_addr(){
		return try_addr;
	}
	
	Len_t getTry_len(){
		return try_len;
	}
	
	void setTry_len(Len_t try_len){
		this.try_len = try_len;
	}
}
