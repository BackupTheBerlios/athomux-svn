package types;


//TODO: siehe Paddr_t

/**
 * The type Poff_t <br> 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Poff_t {
String data;
	
	public Poff_t(){
		String data = new String();
	}
	
	public void setData(char data){
		this.data = String.valueOf(data);
	}
	
	public void setData(int data){
		this.data = String.valueOf(data);
	}
	
	public void setData(long data){
		this.data = String.valueOf(data);
	}
	
	public void setData(String data){
		this.data = String.valueOf(data);
	}
	
	public String getData(){
		return data;
	}
	
}
