package args;

import types.Addr_t;
import types.Len_t;
import types.Paddr_t;
import types.Plen_t;

/**
 * Args for the operation transwait<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
 $transwait (addr_t log_addr, len_t log_len, paddr_t
 phys_addr, direction_t direction, prio_t prio :=
 prio_normal) => (success_t success, plen_t phys_len)
 */

public class ArgsTransWait extends Args {
	
	public ArgsTransWait(Addr_t log_addr,Len_t log_len,Paddr_t phys_addr,int direction,int prio){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.phys_addr = phys_addr;
		this.direction = direction;
		this.prio = prio;
	}
	
	Plen_t getPhys_len(){
		return phys_len;
	}
	
	void setPhys_len(Plen_t phys_len){
		this.phys_len = phys_len;
	}

}
