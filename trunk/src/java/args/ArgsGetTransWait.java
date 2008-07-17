package args;

import types.Addr_t;
import types.Len_t;
import types.Paddr_t;
import types.Plen_t;

/**
 * Args for the Operation ArgsGetTransWait<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $gettranswait (addr_t log_addr, len_t log_len, prio_t prio
  := prio_normal) => (success_t success, paddr_t phys_addr,
  plen_t phys_len)
 */

public class ArgsGetTransWait extends Args {	
	public ArgsGetTransWait(Addr_t log_addr, Len_t log_len, int prio){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.prio = prio;
	}
	
	Paddr_t getPhys_addr(){
		return this.phys_addr;
	}
	
	Plen_t getPhys_len(){
		return this.phys_len;
	}
	
}
