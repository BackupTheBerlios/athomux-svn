package args;

import types.*;

/**
 * Args for the operation padr<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $padr (addr_t log_addr, len_t log_len, bool where := FALSE)
  => (success_t success)
 */

public class ArgsPadr extends Args {
	
	public ArgsPadr(Addr_t log_addr, Len_t log_len, boolean where){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.where = where;
	}
}
