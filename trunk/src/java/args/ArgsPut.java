package args;

import types.*;

/**
 * Args for the operation put<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $put (addr_t log_addr, len_t log_len, prio_t prio :=
  prio_none) => (success_t success)
*/
public class ArgsPut extends Args {
	
	public ArgsPut(Addr_t log_addr, Len_t log_len, int prio){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.prio = prio;
	}
}
