`timescale 1ns/1ns

`define TACT 20 
`define HALF_TACT (`TACT/2)

module psone_tb;

bit clk;
bit reset;

bit key = 1'b0;
bit miso;
bit ack;

bit [7 : 0] data [0 : 8];
bit [7 : 0] cur_data;

wire TX;
wire REC_END;
wire REC_ER;
wire [7 : 0] REC_BYTE;

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
	$display("\n\n\t\t\tSTART\n\n");
	
	data[0] = 8'hFF;
	data[1] = 8'h73;
	data[2] = 8'h5A;
	
	data[3] = 8'h12;
	data[4] = 8'h34;
	
	data[5] = 8'h56;
	data[6] = 8'h78;
	data[7] = 8'h9A;
	data[8] = 8'hBC;
	
	#(10*`TACT);
	key = 1'b1;
	#(`TACT);
	key = 1'b0;
	
	// mti_fli::mti_Cmd("stop -sync");
end

always@(negedge DUT.clk)begin
	cur_data = data[DUT.cnt_byte];
	miso = cur_data[((DUT.cnt_edge - 1) >> 1)]; // LSB, issue on fall, latch on front
	
	$display("\tcur_data: 0x%2h, bit: 0b%1b, time: %t", cur_data, miso, $time);
end

always@(posedge REC_END) $display("\t\tRECIEVED DATA: 0x%h, time: %t", REC_BYTE, $time);
always@(posedge REC_ER) $display("\t\t ***** ERROR RECIEVED DATA, time: %t *****", $time);

psone_uart PC_UART(
    .iCLK(clk),
    .iRESET(reset), 
	 
    .iRX(TX),
    .oTX(),
	 
    .iTRAN_ST(),
    .iTX_BYTE(),
	 
    .oREC_END(REC_END),
    .oRX_BYTE(REC_BYTE),
	 
    .oREC_BUSY(),
    .oTRAN_BUSY(),
    .oREC_ER(REC_ER)
 );

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
	.oTX(TX)
);

endmodule