package args;

import types.Addr_t;
import types.Len_t;
import types.Paddr_t;
import types.Plen_t;

/**
 * Args for the Operation CreateGet<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $createget (addr_t log_addr, len_t log_len, bool clear :=
  FALSE, bool melt := TRUE) => (success_t success, paddr_t
  phys_addr, plen_t phys_len)
 */

public class ArgsCreateGet extends Args{
	
	public ArgsCreateGet(Addr_t log_addr, Len_t log_len, boolean clear, boolean melt){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.clear = clear;
		this.melt = melt;
	}
	
	Plen_t getPhys_len(){
		return phys_len;
	}
	
	Paddr_t getPhys_addr(){
		return phys_addr;
	}
}
