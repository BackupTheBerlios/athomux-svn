/**
 * The class Args shall represent a brick 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Brick {
	protected Output[] outputs = null;
	protected Input[] inputs = null;
	
	public Brick(int nr_inputs, int nr_outputs){
		outputs = new Output[nr_outputs];
		inputs = new Input[nr_inputs];
	}
	
	
	public Output getOutput(String name){
		for(int i=0;i<outputs.length;i++){
			if(outputs[i] !=null && outputs[i].name == name){
				return outputs[i];
			}
		}
		return null;
	}
	
	public Input getInput(String name){
		for(int i=0;i<inputs.length;i++){
			if(inputs[i] !=null && inputs[i].name == name){
				return inputs[i];
			}
		}
		return null;
	}
	
}
