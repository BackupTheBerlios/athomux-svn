import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;
import java.net.UnknownHostException;

import types.Opcode_t;

import args.Args;

/**
 * In this class will all output operations send throw 
 * socket to a athomux enviroment and executed there.
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */


public class InputRemote extends Input {
	private Socket socket;
	private PrintWriter printWriter;
	private BufferedReader bufferedReader;
	private boolean conn;
	
	public InputRemote(String host){
		
		try {
			socket = new Socket(host,3490);
			printWriter = new PrintWriter(new OutputStreamWriter(socket.getOutputStream()));
			bufferedReader = new BufferedReader(new InputStreamReader(socket.getInputStream()));
			conn = true;
		} catch (UnknownHostException e) {
			System.out.println("Host can not be resolved");
			e.printStackTrace();
			conn = false;
		} catch (IOException e) {
			System.out.println("IOExcepion");
			e.printStackTrace();
			conn = false;
		}
	}
	
	public void closeConn(){
		try {
			printWriter.write("$$$KILL");
			printWriter.flush();
			printWriter.close();
			bufferedReader.close();
		} catch (IOException e) {
			System.out.println("IOExcepion");
			e.printStackTrace();
			conn = false;
		}
	}
	
	public void _op(Args args,String param){	
		if(args.op_code>Opcode_t.opcode_output_max && args.op_code <Opcode_t.opcode_brick_init){
			super._op(args, param);
		}
		if(args.op_code<Opcode_t.opcode_input_init && args.op_code>=Opcode_t.opcode_output_init){
			try {
				if(!conn){
					System.out.println("Connection allready closed. Cannot procceed with output operations!");
					args.success = 0;
				}
				else{
					
					System.out.println(ConvertArgs.ArgsToString(args));
					printWriter.write(ConvertArgs.ArgsToString(args));
					printWriter.flush();
				
					//ESCAPE zeichen...
					String tmp = bufferedReader.readLine();
					System.out.println(tmp);
					//args = convArgs.StringtoArgs(tmp);
				}
				
				
			} catch (UnknownHostException e) {
				System.out.println("Can not find Server.");
				e.printStackTrace();
				conn = false;
				args.success = 0;
			} catch (IOException e) {
				System.out.println("IO Exception");
				e.printStackTrace();
				conn = false;
				args.success = 0;
			}
		}	
	}
}
