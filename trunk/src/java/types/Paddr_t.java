package types;

/**
 * The type Paddr_t <br> 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Paddr_t {
	/* TODO:
	 * - setter und getter für die wichtigsten Primitiven datentypen &
	 * typedef char                       int1;
		typedef unsigned char              uns1;
		typedef short                      int2;
		typedef unsigned short             uns2;
		typedef int                        int4;
		typedef unsigned                   uns4;
		typedef long long                  int8;
		typedef unsigned long long         uns8;
	 * 
	 */
	String data;
	
	public Paddr_t(){
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
