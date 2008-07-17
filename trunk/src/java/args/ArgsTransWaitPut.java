package args;

import types.Addr_t;
import types.Len_t;
import types.Paddr_t;

/**
 * Args for the operation transwaitput<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $transwaitput (addr_t log_addr, len_t log_len, paddr_t
  phys_addr, prio_t prio := prio_normal) => (success_t
  success)
 */

public class ArgsTransWaitPut extends Args {
	public ArgsTransWaitPut(Addr_t log_addr, Len_t log_len, Paddr_t phys_addr,int prio){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.phys_addr = phys_addr;
		this.prio = prio;
	}
}
