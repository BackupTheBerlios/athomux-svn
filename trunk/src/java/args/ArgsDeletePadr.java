package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for Operation ArgsDeletePadr <br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $deletepadr (addr_t log_addr, len_t log_len) => (success_t success)
 */

public class ArgsDeletePadr extends Args {
	
	public ArgsDeletePadr(Addr_t log_addr, Len_t log_len){
		this.log_addr = log_addr;
		this.log_len = log_len;
	}
}
