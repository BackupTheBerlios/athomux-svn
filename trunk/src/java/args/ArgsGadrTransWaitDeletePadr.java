package args;

import types.Len_t;
import types.Paddr_t;
import types.Plen_t;

/**
 * Args for the Operation ArgsGadrTransWaitDeletePadr<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $gadrtranswaitdeletepadr (paddr_t phys_addr,
  plen_t phys_len, action_t action := action_wait) =>
  (success_t success, len_t log_len)
 */

public class ArgsGadrTransWaitDeletePadr extends Args{
	
	public ArgsGadrTransWaitDeletePadr(Paddr_t phys_addr, Plen_t phys_len, int action){
		this.phys_addr = phys_addr;
		this.phys_len = phys_len;
		this.action = action;
	}
	
	Len_t getLog_len(){
		return log_len;
	}
	
}
