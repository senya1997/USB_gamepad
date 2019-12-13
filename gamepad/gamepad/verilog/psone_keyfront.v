module psone_keyfront(
	input iCLK,
	input iRESET,
	input iKEY,
	output oPRESS	
);
	
reg press_1;
reg press_2;

assign oPRESS = !press_1 & press_2;
	
always@ (posedge iCLK or negedge iRESET) begin
	if (!iRESET) begin
		press_1 <= 0;
		press_2 <= 0;
	end
	else begin
		press_1 <= iKEY;
		press_2 <= press_1;
	end
end
	
endmodule
