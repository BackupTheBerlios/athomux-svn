import args.*;
import types.*;

/**
 * Just for testing...
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*TODO: 
 * Strategy Operations?
 * 
 */


public class Main {

	public static void main(String[] args) {
		InputRemote i= new InputRemote("localhost");
		
		Args arg = new Args();
		arg.log_addr = new Addr_t(0);
		arg.log_len = new Len_t(0);
		arg.try_addr = new Addr_t(0);
		arg.try_len = new Len_t(0);
		arg.offset = new Off_t(0);
		arg.offset_max = new Off_t(0); //in common.h heißt off_f gleich offs_t 
		arg.phys_addr = new Paddr_t();
		arg.phys_len = new Plen_t(0);
		arg.op_code = 20;
		i._op(arg,"localhost");
	}

}
