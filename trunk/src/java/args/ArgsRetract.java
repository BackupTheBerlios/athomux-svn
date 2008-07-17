package args;

import types.*;

/**
 * Args for the operation retract<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $retract (prio_t prio, addr_t log_addr := 0, len_t log_len
  := (len_t)-1, addr_t try_addr := log_addr, len_t try_len :=
  log_len) => (success_t success)
 */

public class ArgsRetract extends Args {

	public ArgsRetract(int prio, Addr_t log_addr, Len_t log_len, Addr_t try_addr, Len_t try_len){
		this.prio = prio;
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.try_addr = try_addr;
		this.try_len = try_len;
	}
}
