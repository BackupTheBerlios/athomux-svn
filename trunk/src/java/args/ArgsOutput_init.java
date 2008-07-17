package args;

/**
 * Args for the operation output_init<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $output_init (bool destr, bool constr, bool clear := FALSE)
  => (success_t success)
 */

public class ArgsOutput_init extends Args {

	public ArgsOutput_init(boolean destr, boolean constr,boolean clear){
		this.destr = destr;
		this.constr = constr;
		this.clear = clear;
	}
	public ArgsOutput_init(boolean destr, boolean constr){
		this.destr = destr;
		this.constr = constr;
		this.clear = false;
	}
}
