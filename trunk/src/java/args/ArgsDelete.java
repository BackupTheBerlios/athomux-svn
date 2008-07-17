package args;

import types.*;

/**
 * Args for the Operation Delete<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
$delete (addr_t log_addr, len_t log_len, bool melt := TRUE) => (success_t success)
*/

public class ArgsDelete extends Args {
	
	public ArgsDelete(Addr_t log_addr, Len_t log_len, boolean melt){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.melt = melt;
	}
	
}
