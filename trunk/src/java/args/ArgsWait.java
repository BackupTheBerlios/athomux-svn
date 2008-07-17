package args;

import types.Addr_t;
import types.Len_t;

/**
 * Args for the operation wait<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $wait (addr_t log_addr, len_t log_len, prio_t prio :=
  prio_normal, action_t action := action_wait) => (success_t
  success)
 */

public class ArgsWait extends Args {

	public ArgsWait(Addr_t log_addr, Len_t log_len, int prio, int action){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.prio = prio;
		this.action = action;
	}
}
