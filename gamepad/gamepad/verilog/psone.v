`ifdef MODEL_TECH
	`define HALF_PER_TIME 10 
`else
	`define HALF_PER_TIME 2000 
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
	output oIND
`endif	
);

`ifndef MODEL_TECH
	reg led;
	reg [23 : 0] cnt_led;
	
	wire SPARK_LED = (cnt_led == 24'd15_000_000);
`endif

reg en;

reg cs = 1'b1;
reg clk = 1'b1;
reg mosi = 1'b0;

reg [11 : 0] cnt_half_per; // 500 kHz <=> 100; 7 kHz ~ 7142
reg [3 : 0] cnt_byte;
reg [4 : 0] cnt_edge;

reg [7 : 0] req_data;
reg [1 : 0] miso;
reg [1 : 0] ack;

reg tx_st;
reg [3 : 0] cnt_tran_byte;
reg [7 : 0] tx_byte [0 : 8];

reg [1 : 0] tran_busy;

`ifdef MODEL_TECH
	wire KEY_PR = iKEY_ST;
`else
	wire KEY_PR; // key "start" was pressed
`endif
	
wire HALF_PER = (cnt_half_per == `HALF_PER_TIME);

wire ONE_BYTE = (cnt_edge == 5'd25);
wire ACT_TRANS = (cnt_edge < 5'd16); // active stage of SPI

wire FIRST_BYTE = (cnt_byte == 4'd0);
wire SEC_BYTE = (cnt_byte == 4'd1);
wire END_PACKET = (cnt_byte == 4'd9);

wire TRAN_BUSY;
wire TRAN_BUSY_FALL = (tran_busy[0] & !tran_busy[1]);
wire TRAN_LAST_BYTE = (cnt_tran_byte == 4'd8);

wire EN_SPI = (en & !TRAN_BUSY & !END_PACKET & !tx_st & (cnt_tran_byte == 4'd0));

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) en <= 1'b0;
	else if(KEY_PR) en <= ~en;
end

// uart:
always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) tx_st <= 1'b0;
	else if(END_PACKET | ((cnt_tran_byte > 4'd0) & TRAN_BUSY_FALL)) tx_st <= 1'b1;
	else tx_st <= 1'b0;
end

genvar i;
generate
	for(i = 0; i < 9; i = i + 1)begin: gen
		always@(posedge iCLK or negedge iRESET)begin
			if(!iRESET) tx_byte[i] <= 0;
			else if(ONE_BYTE & (cnt_byte == i)) tx_byte[i] <= req_data;
		end		
	end
endgenerate

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) cnt_tran_byte <= 4'b0;
	else if(TRAN_LAST_BYTE) cnt_tran_byte <= 4'b0;
	else if(tx_st) cnt_tran_byte = cnt_tran_byte + 1'b1;
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) tran_busy <= 2'b0;
	else tran_busy <= {tran_busy[0], !TRAN_BUSY};
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
	if(!iRESET) cnt_byte <= 4'd0;
	else if(!cs)
		begin
			if(ONE_BYTE) cnt_byte <= cnt_byte + 1'b1;
			else if(END_PACKET) cnt_byte <= 4'd0;
		end
	else cnt_byte <= 4'd0;
end

// spi:
always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) cs <= 1'b1;
	else if(EN_SPI) 
		begin
			if(FIRST_BYTE) cs <= 1'b0;
			else if(END_PACKET) cs <= 1'b1;
		end
	else cs <= 1'b1;
end

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) clk <= 1'b1;
	else if(!cs)
		begin
			if(HALF_PER & ACT_TRANS) clk <= ~clk;
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

// shift reg:
always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) miso <= 2'd0;
	else miso <= {miso[0], iMISO};
end

wire FRONT_CLK = (HALF_PER & cnt_edge[0]);

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) req_data <= 8'd0;  // all data issue and recieve LSB
	else if(!cs)
		begin
			if(FRONT_CLK & ACT_TRANS) req_data <= {req_data[6:0], miso[1]}; // latch data on front CLK
		end
	else req_data <= 8'd0;
end

// led:
`ifndef MODEL_TECH
	always@(posedge iCLK or negedge iRESET)begin
		if(!iRESET) cnt_led <= 24'd0;
		else if(SPARK_LED) cnt_led <= 24'd0;
		else cnt_led <= cnt_led + 24'd1;
	end
/*
	always@(posedge iCLK or negedge iRESET)begin
		if(!iRESET) led <= 1'b0;
		else if(SPARK_LED) led <= ~led;
	end
*/	
	always@(posedge iCLK or negedge iRESET)begin
		if(!iRESET) led <= 1'b0;
		else if(KEY_PR) led <= ~led;
	end

	psone_debounce DEB(
		 .iCLK(iCLK), 
		 .iRESET(iRESET), 
		 .iKEY(iKEY_ST),      
		 .oKEY_FRONT(KEY_PR)
	);
`endif

psone_uart UART(
    .iCLK(iCLK),
    .iRESET(iRESET),
	 
    .iRX(iRX),
    .oTX(oTX),
	 
    .iTRAN_ST(tx_st), // Signal to transmit
    .iTX_BYTE(tx_byte[cnt_tran_byte]), // Byte to transmit
	 
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
	assign oIND = led;
`endif

endmodule 