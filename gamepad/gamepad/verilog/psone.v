//`include "psone_defines.v"
`define DEBUG

module psone(
	input		iCLK,
	input		iRESET,
	
	output	oCS,
	output	oCLK,
	output	oMOSI,
	input		iMISO,
	input		iACK,
	
	input iRX,
	output oTX
	
`ifndef MODEL_TECH
	,
	output [2 : 0] oLED
`endif	
);

`ifndef MODEL_TECH
	reg [2 : 0] led;
	reg [23 : 0] cnt_led;
	
	wire SPARK_LED = (cnt_led == 24'd15_000_000);
`endif

reg cs;
reg clk;
reg mosi;

reg [1:0] miso;
reg [1:0] ack;

reg tx_st;
reg [7 : 0] tx_byte;

wire TRAN_BUSY;

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) tx_st <= 1'b0;
	
`ifdef DEBUG
	else if(!TRAN_BUSY & SPARK_LED) tx_st <= 1'b1;
	else tx_st <= 1'b0;
`endif

end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) tx_byte <= 8'd0;
	
`ifdef DEBUG
	else if(SPARK_LED) tx_byte <= tx_byte + 1'b1;
`endif

end

/*
localparam  =	;

wire STATE_ =	(state == );

wire  = ();
wire ;

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) state <= 3'd0;
	else
		case(state)
			:
		endcase
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET)  <= ;
	else if() 
		begin 
		
		end
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET)  <= ;
	else 
end

always@(iRESET)begin
	 =	;
end
*/

`ifndef MODEL_TECH
	always@(posedge iCLK or negedge iRESET)begin
		if(!iRESET) cnt_led <= 24'd0;
		else if(SPARK_LED) cnt_led <= 24'd0;
		else cnt_led <= cnt_led + 24'd1;
	end

	always@(posedge iCLK or negedge iRESET)begin
		if(!iRESET) led[0] <= 1'b0;
		else if(SPARK_LED) led[0] <= ~led[0];
	end
`endif

/*
filter #(.BIT(12)) FIL(
	.iCLK(iCLK),
	.iRESET(iRESET),
	
	.iEN(),
	.iREL(3'd3),
	
	.iDATA(),
	.iOLD_DATA(),
	
	.oDATA(),
	.oRDY()
);
*/

psone_uart UART(
    .iCLK(iCLK), // The master clock for this module
    .iRESET(iRESET), // Synchronous reset.
	 
    .iRX(iRX), // Incoming serial line
    .oTX(oTX), // Outgoing serial line
	 
    .iTRAN_ST(tx_st), // Signal to transmit
    .iTX_BYTE(tx_byte), // Byte to transmit
	 
    .oREC_END(), // Indicated that a byte has been received.
    .oRX_BYTE(), // Byte received
	 
    .oREC_BUSY(), // Low when receive line is idle.
    .oTRAN_BUSY(TRAN_BUSY), // Low when transmit line is idle.
	 
    .oREC_ER() // Indicates error in receiving packet.
 );
 
assign oCS = cs;
assign oCLK = clk;
assign oMOSI = mosi;

assign oLED = led;

endmodule 