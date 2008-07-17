
package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for the Operation ArgsGadrCreatet <br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
 $gadrcreatet (len_t log_len, bool clear := FALSE, bool exclu
 := TRUE, action_t action := action_wait, bool melt := TRUE,
 len_t try_len := log_len) => (success_t success, addr_t
 log_addr, len_t log_len)
 */

public class ArgsGadrCreatet extends Args {
	
	public ArgsGadrCreatet(Len_t log_len, boolean clear,boolean exclu, int action, boolean melt){
		this.log_len = log_len;
		this.clear = clear;
		this.exclu = exclu;
		this.action = action;
		this.melt = melt;
	}
	
	Addr_t getLog_addr(){
		return log_addr;
	}
	Len_t getLog_len(){
		return log_len;
	}
}
