package args;

import types.*;

/**
 * Args for the Operation move<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $move (addr_t log_addr, len_t log_len, offs_t offset, offs_t
  offset_max := offset) => (success_t success, offs_t offset)
 */

public class ArgsMove extends Args {
	
	public ArgsMove(Addr_t log_addr, Len_t log_len,Off_t offset,Off_t offset_max){
		this.log_addr = log_addr;
		this.log_len = log_len;
		this.offset = offset;
		this.offset_max = offset_max;
	}

			
	void setOffset(Off_t offset){
		this.offset = offset;
	}
	
	Off_t getOffset(){
		return offset;
	}
}
