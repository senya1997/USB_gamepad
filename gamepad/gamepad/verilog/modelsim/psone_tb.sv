`timescale 1ns/1ns

`define TACT 20 
`define HALF_TACT (`TACT/2)

module psone_tb;

bit clk;
bit reset;

bit key = 1'b0;
bit miso;
bit ack;

initial begin
	$timeformat(-3, 3, " ms", 6);
	clk = 0;
	forever	#(`HALF_TACT) clk = ~ clk;
end

initial begin
	reset = 1'b0; #(`TACT);
	reset = 1'b1; #(2*`TACT);
end

initial begin
	$display("	*                          START                          *");
	
	#(10*`TACT);
	key = 1'b1;
	#(`TACT);
	key = 1'b0;
	
	$display("	*                         COMPLETE                        *");
	// mti_fli::mti_Cmd("stop -sync");
end

psone DUT(
	.iCLK(clk),
	.iRESET(reset),
	
	.iKEY_ST(key), // 1st press - start; 2nd - end
	
	.oCS(),
	.oCLK(),
	.oMOSI(),
	
	.iMISO(miso),
	.iACK(ack),
	
	.iRX(),
	.oTX()
);

endmodule