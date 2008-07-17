package types;

/**
 * The type Opcode_t <br> 
 * @author Matthäus Szturc, Teresa Gamez Zerban <br>
 *  Copyright: Matthäus Szturc, Teresa Gamez Zerban	<br>
 *  License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 */

public class Opcode_t {
	
	//Output Operations
	public static final int opcode_output_init=0;
	
	//static
	public static final int opcode_trans=1;	
	public static final int opcode_wait=2;
	public static final int opcode_get=3;
	public static final int opcode_put=4;
	public static final int opcode_lock=5;
	public static final int opcode_unlock=6;
	public static final int opcode_gadr=7;
	public static final int opcode_padr=8;
	//dynamic
	public static final int opcode_move=9;
	public static final int opcode_create=10;
	public static final int opcode_delete=11;
	
	//strategy ops
	public static final int opcode_instbrick = 12;
	public static final int opcode_deinstbrick = 13;
	public static final int opcode_instconn = 14;
	public static final int opcode_deinstconn = 15;
	public static final int opcode_connect = 16;
	public static final int opcode_disconnect = 17;
	public static final int opcode_getconn = 18;
	public static final int opcode_findconn = 19;
	
	
	//combined opcodes
	public static final int opcode_transwait= 20;
	public static final int opcode_gettranswait=21;
	public static final int opcode_transwaitput=22;
	public static final int opcode_putwait = 23;
	public static final int opcode_createget=24;
	public static final int opcode_gadrcreate = 25;
	public static final int opcode_gadrcreateget=26;
	public static final int opcode_gadrgettranswait=27;
	public static final int opcode_putpadr = 28;
	public static final int opcode_putdelete=29;
	public static final int opcode_deletepadr=30;
	public static final int opcode_putdeletepadr=31;
	public static final int opcode_gadrtranswaitdeletepadr=32;
	public static final int opcode_gadrcreatetranswaitpadr=33;
	
	//last dummy index
	public static final int opcode_output_max= 34;
	
	//Input
	public static final int opcode_input_init = 35;
	public static final int opcode_retract = 36;
	public static final int opcode_input_max = 37;
	// STATIC OPERATIONS
	public static final int opcode_brick_init = 38;
	public static final int opcode_brick_max = 39;
}
