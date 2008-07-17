
package types; 

/**
 * The type Addr_t <br> 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Addr_t {
	private long addr;
	
	public Addr_t(long addr){
		this.addr = addr;
	}

	public long getAddr_t() {
		return addr;
	}

	public void setAddr_t(long addr) {
		this.addr = addr;
	}
	
}
