/**
 * Helper Class to convert Args to String and back
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class StreamSerializer {
	
	private String stream;

	public StreamSerializer(){
		Clear();
	}
	
	public void Clear(){
		stream = "#";
	}
	
	public void Add(String in){
		stream = stream + in + "#";
	}

	public void Add(int in){
		stream = stream + Integer.toString(in) + "#";
	}
	public void Add(boolean in){
		if(in == true) stream = stream + "1#";
		else stream = stream + "0#";
	}
	public void Add(long in){
		stream = stream + Long.toString(in) + "#";
	}
	
	public void addStream(String stream){
		this.stream = stream;
	}
	
	public String getStream(){
		return stream;
	}
	
	class StreamToken{
		public int length;
		public String value;
	}

	public StreamToken getToken(){
		
		StreamToken token = new StreamToken();
		
		int first = stream.indexOf("#");
		int second = stream.indexOf("#", first+1);
		
		token.value = stream.substring(first+1, second);
		token.length = token.value.length();
		
		stream = stream.substring(second);
		return token;
	}
	
}
