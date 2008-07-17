package args;

/**
 * Args for the Operation ArgsInput_Init<br> 
 * A custom constructor was added and also getter needed for the Operation
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

/*
  $input_init (bool destr, bool constr, bool clear := FALSE)
  => (success_t success)
 */

public class ArgsInput_Init extends Args {

	public ArgsInput_Init(boolean destr, boolean constr, boolean clear){
		this.destr = destr;
		this.constr = constr;
		this.clear = clear;
	}

}
