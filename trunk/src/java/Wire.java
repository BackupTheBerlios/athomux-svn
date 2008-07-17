/**
 * Represents the Wire in Athomux
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Wire {
	private Wire next;
	private Input _input;
	private Output _output;
	
	public Wire(){
		this(null,null,null);
	}
	
	public Wire(Input _input){
		this(_input,null,null);
	}
	
	public Wire(Input _input,Output _output,Wire next){
		this._input = _input;
		this._output = _output;
		this.next = next;
	}
	
	
	
	
	
}
