import types.Addr_t;
import types.Len_t;
import types.Off_t;
import types.Paddr_t;
import types.Plen_t;
import args.Args;

/**
 * This class converts an args object to a string and back,
 * to be able to send it throw a socket. 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class ConvertArgs {
	
	static public String ArgsToString(Args args){
		StreamSerializer stream = new StreamSerializer();
		stream.Add(args.log_addr.getAddr_t());
		stream.Add(args.log_len.getLen_t());
		stream.Add(args.try_addr.getAddr_t());
		stream.Add(args.try_len.getLen_t());
		stream.Add(args.offset.getOff_t());
		stream.Add(args.offset_max.getOff_t());
		stream.Add(args.phys_addr.getData());
		stream.Add(args.phys_len.getPlen_t());
		stream.Add(args.forwrite);
		stream.Add(args.clear);
		stream.Add(args.melt);
		stream.Add(args.direction);
		stream.Add(args.prio);
		stream.Add(args.reader);
		stream.Add(args.exclu);
		stream.Add(args.data_lock);
		stream.Add(args.addr_lock);
		stream.Add(args.action);
		stream.Add(args.destr);
		stream.Add(args.constr);
		stream.Add(args.success);
		stream.Add(args.version);
		stream.Add(args.op_code);
		//gadr
		//stream.Add(args.where);*/
		return stream.getStream() + "\0";
	}
	
	public static Args StringtoArgs(String stream){
		Args args = new Args();
		StreamSerializer str = new StreamSerializer();
		str.addStream(stream);
		StreamSerializer.StreamToken tok = str.getToken();
		args.log_addr.setAddr_t(Long.parseLong(tok.value));
		tok = str.getToken();
		args.log_len.setLen_t(Long.parseLong(tok.value));
		tok = str.getToken();
		args.try_addr.setAddr_t(Long.parseLong(tok.value));
		tok = str.getToken();
		args.try_len.setLen_t(Long.parseLong(tok.value));
		tok = str.getToken();
		args.offset.setOff_t(Long.parseLong(tok.value));
		tok = str.getToken();
		args.offset_max.setOff_t(Long.parseLong(tok.value));
		tok = str.getToken();
		args.phys_addr.setData(tok.value);
		tok = str.getToken();
		args.phys_len.setPlen_t(Long.parseLong(tok.value));
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.forwrite = true;
		}
		else {
			args.forwrite = false;
		}
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.clear = true;
		}
		else {
			args.clear = false;
		}
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.melt = true;
		}
		else {
			args.melt = false;
		}
		tok = str.getToken();
		args.direction = Integer.parseInt(tok.value);
		tok = str.getToken();
		args.prio = Integer.parseInt(tok.value);
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.reader = true;
		}
		else {
			args.reader = false;
		}
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.exclu = true;
		}
		else {
			args.exclu = false;
		}
		tok = str.getToken();
		args.data_lock = Integer.parseInt(tok.value);
		tok = str.getToken();
		args.addr_lock = Integer.parseInt(tok.value);
		tok = str.getToken();
		args.action = Integer.parseInt(tok.value);
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.destr = true;
		}
		else {
			args.destr = false;
		}
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.constr = true;
		}
		else {
			args.constr = false;
		}
		tok = str.getToken();
		args.success = Integer.parseInt(tok.value);
		tok = str.getToken();
		args.version = Integer.parseInt(tok.value);
		tok = str.getToken();
		args.op_code = Integer.parseInt(tok.value);
		tok = str.getToken();
		if(Integer.parseInt(tok.value) == 1){
			args.where = true;
		}
		else {
			args.where = false;
		}
		
		return args;
	}
	
	
	
}
