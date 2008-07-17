package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for the operation putpadr<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $putpadr (addr_t log_addr, len_t log_len, prio :=
  prio_background) => (success_t success)
 */

public class ArgsPutPadr extends Args {
	public ArgsPutPadr(Addr_t log_addr, Len_t log_len){
		this.log_addr = log_addr;
		this.log_len = log_len;
	}
}
