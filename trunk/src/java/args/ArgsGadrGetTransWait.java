package args;

import types.Addr_t;
import types.Len_t;
import types.Paddr_t;
import types.Plen_t;

/**
 * Args for the Operation ArgsGadrGetTransWait<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $gadrgettranswait (len_t log_len, bool exclu := TRUE,
  action_t action := action_wait, forwrite := FALSE, prio
  := prio_normal, len_t try_len := log_len) => (success_t
  success, addr_t log_addr, len_t log_len, paddr_t phys_addr,
  plen_t phys_len)
 */

public class ArgsGadrGetTransWait extends Args {
	
	public ArgsGadrGetTransWait(Len_t log_len, boolean exclu, int action, boolean forwrite, int prio, Len_t try_len){
		this.log_len = log_len;
		this.exclu = exclu;
		this.action = action;
		this.forwrite = forwrite;
		this.prio = prio;
		this.try_len = try_len;
		
	}
	
	Addr_t getLog_addr(){
		return log_addr;
	}
	
	Plen_t getPhys_len(){
		return phys_len;
	}
	
	Paddr_t getPhys_addr(){
		return phys_addr;
	}
}
