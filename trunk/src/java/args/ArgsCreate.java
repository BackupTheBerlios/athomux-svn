package args;

import types.*;

/**
 * Args for the Operation Create<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
 $create (addr_t log_addr, len_t log_len, bool clear :=
 FALSE, bool melt := TRUE) => (success_t success)
 */

public class ArgsCreate extends Args {

	
	public ArgsCreate(Addr_t log_addr,Len_t log_len, boolean clear, boolean melt){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.clear = clear;
		this.melt = melt;
	}
	
}
