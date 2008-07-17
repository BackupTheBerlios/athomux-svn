package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for the operation putwait<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
 $putwait (addr_t log_addr, len_t log_len, prio_t prio :=
 prio_normal) => (success_t success)
 */

public class ArgsPutWait extends Args {
	public ArgsPutWait(Addr_t log_addr,Len_t log_len,int prio){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.prio = prio;
	}
}
