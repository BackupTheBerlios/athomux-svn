package types;

/**
 * The type Off_t <br> 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Off_t {
	private long off_t;
	
	public Off_t(long off_t){
		this.off_t = off_t;
	}

	public long getOff_t() {
		return off_t;
	}

	public void setOff_t(long off_t) {
		this.off_t = off_t;
	}
}
