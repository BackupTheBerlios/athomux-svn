package args;

import types.*;

/**
 * Args for the Operation ArgsGet<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $get (addr_t log_addr, len_t log_len, bool forwrite :=
  FALSE) => (success_t success, paddr_t phys_addr, plen_t
  phys_len, version_t version)
 */

public class ArgsGet extends Args {
	
	public ArgsGet(Addr_t log_addr,Len_t log_len, boolean forwrite){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.forwrite = forwrite;
	}
		
	Paddr_t getPhys_addr(){
		return phys_addr;
	}
	
	Plen_t getPhys_len(){
		return phys_len;
	}
	
	
	int getVersion(){
		return version;
	}

}
