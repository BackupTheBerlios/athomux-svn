package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for the Operation ArgsGadr <br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  	$gadr (len_t log_len, bool where := FALSE, bool exclu :=
	TRUE, action_t action := action_wait, len_t try_len :=
	log_len) => (success_t success, addr_t log_addr, len_t
	log_len)
*/

public class ArgsGadr extends Args {
	
	public ArgsGadr(Len_t log_len, boolean where, boolean exclu, int action, Len_t try_len){
		this.log_len = log_len;
		this.where = where;
		this.exclu = exclu;
		this.action = action;
		this.try_len = try_len;
	}
	
	Addr_t getLog_addr(){
		return log_addr;
	}
	
	Len_t getLog_len(){
		return log_len;
	}
	
}
