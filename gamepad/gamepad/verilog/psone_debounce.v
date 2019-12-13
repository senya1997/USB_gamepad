module psone_debounce(
    input	iCLK, 
	 input	iRESET, 
	 input	iKEY,      
    output	oKEY_FRONT
);
     
/*
	parameter N defines the debounce time, assuming 50 KHz clock,
	the debounce time is 2^(11-1)/ 50 KHz = 20 ms
  
	for 50 MHz clock increase value of N accordingly to 21
*/
 
parameter N = 11; //27 MHz
	
reg key_deb; 

reg [1 : 0] dff;
reg [1 : 0] pressed;

reg  [N-1 : 0]  delaycount_reg;                     
reg  [N-1 : 0]  delaycount_next;
  
wire Q_ADD = ~(delaycount_reg[N-1]); // check count using MSB of counter                                   
wire Q_RES = (dff[0] ^ dff[1]); // Ex OR iKEY on conecutive clocks to detect level change

wire FRONT = (!pressed[0] & pressed[1]);

// shift reg:
always@(posedge iCLK or negedge iRESET)begin
	if (!iRESET) pressed <= 2'd0;
	else pressed <= {pressed[0], key_deb};
end

always@(posedge iCLK or negedge iRESET)begin
	if (!iRESET) dff <= 2'd0;
	else dff <= {dff[0], iKEY};
end

// comb:
always@(Q_RES, Q_ADD, delaycount_reg)begin
	case({Q_RES, Q_ADD})
		2'b00: delaycount_next <= delaycount_reg;
		2'b01: delaycount_next <= delaycount_reg + 10'b1;
		default: delaycount_next <= {N{1'b0}}; // in this case "Q_RES = 1" => change in level; reset the counter			
	endcase    
end

// reg:
always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) delaycount_reg <= {N{1'b0}};
	else delaycount_reg <= delaycount_next;
end  

always@(posedge iCLK or negedge iRESET)begin
	if(!iRESET) key_deb <= 1'b0;
	else if(delaycount_reg[N-1] == 1'b1) key_deb <= dff[1];
	else key_deb <= key_deb;
end

assign oKEY_FRONT = FRONT;
 
endmodule 