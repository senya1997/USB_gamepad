//`define DEBUG

`ifdef MODEL_TECH
	`define HALF_PER_TIME 10 
`else
	`define HALF_PER_TIME 500 
`endif
module psone(
	input		iCLK,
	input		iRESET,
	
	input		iKEY_ST, // 1st press - start; 2nd - end
	
	output	oCS,
	output	oCLK,
	output	oMOSI,
	input		iMISO,
	input		iACK,
	
	input		iRX,
	output	oTX
	
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

reg en;

reg cs = 1'b1;
reg clk = 1'b1;
reg mosi = 1'b0;

reg [11 : 0] cnt_half_per; // 500 kHz <=> 100; 7 kHz ~ 7142
reg [3 : 0] cnt_data;
reg [4 : 0] cnt_edge;

reg [1:0] miso;
reg [1:0] ack;

reg tx_st;
reg [7 : 0] tx_byte;

`ifdef MODEL_TECH
	wire KEY_PR = iKEY_ST;
`else
	wire KEY_PR; // key "start" was pressed
`endif
	
wire HALF_PER = (cnt_half_per == `HALF_PER_TIME);
wire ONE_BYTE = (cnt_edge == 5'd20);

wire FIRST_BYTE = (cnt_data == 4'd0);
wire SEC_BYTE = (cnt_data == 4'd1);
wire END_PACKET = (cnt_data == 4'd8);

wire TRAN_BUSY;

wire EN_SPI = (en & !TRAN_BUSY);

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) en <= 1'b0;
	else if(KEY_PR) en <= ~en;
end

// uart:
always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) tx_st <= 1'b0;
	
`ifdef DEBUG
	else if(!TRAN_BUSY & SPARK_LED) tx_st <= 1'b1;
`endif
	else if(END_PACKET) tx_st <= 1'b1;

	else tx_st <= 1'b0;
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) tx_byte <= 8'd0;
	
`ifdef DEBUG
	else if(SPARK_LED) tx_byte <= tx_byte + 1'b1;
`endif

end

// spi counters:
always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) cnt_half_per <= 12'd0;
	else if(EN_SPI) 
		begin
			if(HALF_PER) cnt_half_per <= 12'd0;
			else cnt_half_per <= cnt_half_per + 1'b1;
		end
	else cnt_half_per <= 12'd0;
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) cnt_edge <= 5'd0;
	else if(!cs)
		begin
			if(ONE_BYTE) cnt_edge <= 5'd0;
			else if(HALF_PER) cnt_edge <= cnt_edge + 1'b1;
		end
	else cnt_edge <= 5'd0;
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) cnt_data <= 4'd0;
	else if(!cs)
		begin
			if(ONE_BYTE) cnt_data <= cnt_data + 1'b1;
			else if(END_PACKET) cnt_data <= 4'd0;
		end
	else cnt_data <= 4'd0;
end

// spi:
always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) cs <= 1'b1;
	else if(EN_SPI) 
		begin
			if(FIRST_BYTE) cs <= 1'b0;
			else if(END_PACKET) cs <= 1'b1;
		end
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) clk <= 1'b1;
	else if(!cs)
		begin
			if(HALF_PER & (cnt_edge <= 5'd15)) clk <= ~clk;
		end
	else clk <= 1'b1;
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) mosi <= 1'b0;
	else if(!cs)
		begin
			if((FIRST_BYTE & (cnt_edge < 5'd3)) | 
				(SEC_BYTE & ((cnt_edge == 5'd3) | (cnt_edge == 5'd4) |
								 (cnt_edge == 5'd13) | (cnt_edge == 5'd14)))) mosi <= 1'b1;
			else mosi <= 1'b0;
		end
	else mosi <= 1'b0;
end

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
	
wire KEY_BUF;
psone_debounce DEB(
    .iCLK(iCLK), 
	 .iRESET(iRESET), 
	 .iKEY(iKEY_ST),      
    .oKEY(KEY_BUF)
);

psone_keyfront KEY_FRONT(
	.iCLK(iCLK),
	.iRESET(iRESET),
	.iKEY(KEY_BUF),
	.oPRESS(KEY_PR)	
);
`endif

psone_uart UART(
    .iCLK(iCLK),
    .iRESET(iRESET),
	 
    .iRX(iRX),
    .oTX(oTX),
	 
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

`ifndef MODEL_TECH
	assign oLED = led;
`endif

endmodule 