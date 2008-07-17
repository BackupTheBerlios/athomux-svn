package types;

/**
 * The type Success_t <br> 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Success_t {
	private int success_t;
	public static final int TRUE = 1;
	public static final int FALSE = 0;
	
	public Success_t(int success_t){
		this.success_t = success_t;
	}

	public int getSuccess_t() {
		return success_t;
	}

	public void setSuccess_t(int success_t) {
		this.success_t = success_t;
	}
	
}
