module psone_uart(
    input iCLK, // The master clock for this module
    input iRESET, // Synchronous reset.
	 
    input iRX, // Incoming serial line
    output oTX, // Outgoing serial line
	 
    input iTRAN_ST, // Signal to transmit
    input [7:0] iTX_BYTE, // Byte to transmit
	 
    output oREC_END, // Indicated that a byte has been received.
    output [7:0] oRX_BYTE, // Byte received
	 
    output oREC_BUSY, // Low when receive line is idle.
    output oTRAN_BUSY, // Low when transmit line is idle.
    output oREC_ER // Indicates error in receiving packet.
 );
 
parameter CLOCK_DIVIDE = 11'd1302; // clock rate (50Mhz) / (baud rate (9600) * 4)
 
// States for the receiving state machine.
// These are just constants, not parameters to override.
parameter RX_IDLE = 0;
parameter RX_CHECK_START = 1;
parameter RX_READ_BITS = 2;
parameter RX_CHECK_STOP = 3;
parameter RX_DELAY_RESTART = 4;
parameter RX_ERROR = 5;
parameter RX_RECEIVED = 6;
 
// States for the transmitting state machine.
// Constants - do not override.
parameter TX_IDLE = 0;
parameter TX_SENDING = 1;
parameter TX_DELAY_RESTART = 2;

reg [10:0] rx_clk_divider = CLOCK_DIVIDE;
reg [10:0] tx_clk_divider = CLOCK_DIVIDE;

reg [2:0] recv_state = RX_IDLE;
reg [5:0] rx_countdown;
reg [3:0] rx_bits_remaining;
reg [7:0] rx_data;
 
reg tx_out = 1'b1;
reg [1:0] tx_state = TX_IDLE;
reg [5:0] tx_countdown;
reg [3:0] tx_bits_remaining;
reg [7:0] tx_data;
 
assign oREC_END = (recv_state == RX_RECEIVED);
assign oREC_ER = (recv_state == RX_ERROR);
assign oREC_BUSY = (recv_state != RX_IDLE);
assign oRX_BYTE = rx_data;
 
assign oTX = tx_out;
assign oTRAN_BUSY = tx_state != TX_IDLE;
 
always @(posedge iCLK) begin
    if (!iRESET) begin
        recv_state = RX_IDLE;
        tx_state = TX_IDLE;
    end
 
    // The clk_divider counter counts down from
    // the CLOCK_DIVIDE constant. Whenever it
    // reaches 0, 1/16 of the bit period has elapsed.
   // Countdown timers for the receiving and transmitting
    // state machines are decremented.
    rx_clk_divider = rx_clk_divider - 1'b1;
    if (!rx_clk_divider) begin
        rx_clk_divider = CLOCK_DIVIDE;
        rx_countdown = rx_countdown - 1'b1;
    end
    tx_clk_divider = tx_clk_divider - 1'b1;
    if (!tx_clk_divider) begin
        tx_clk_divider = CLOCK_DIVIDE;
        tx_countdown = tx_countdown - 1'b1;
    end
 
    // Receive state machine
    case (recv_state)
        RX_IDLE: begin
            // A low pulse on the receive line indicates the
            // start of data.
            if (!iRX) begin
                // Wait half the period - should resume in the
                // middle of this first pulse.
                rx_clk_divider = CLOCK_DIVIDE;
                rx_countdown = 2;
                recv_state = RX_CHECK_START;
            end
        end
        RX_CHECK_START: begin
            if (!rx_countdown) begin
                // Check the pulse is still there
                if (!iRX) begin
                    // Pulse still there - good
                    // Wait the bit period to resume half-way
                    // through the first bit.
                    rx_countdown = 4;
                    rx_bits_remaining = 8;
                    recv_state = RX_READ_BITS;
                end else begin
                    // Pulse lasted less than half the period -
                    // not a valid transmission.
                    recv_state = RX_ERROR;
                end
            end
        end
        RX_READ_BITS: begin
            if (!rx_countdown) begin
                // Should be half-way through a bit pulse here.
                // Read this bit in, wait for the next if we
                // have more to get.
                rx_data = {iRX, rx_data[7:1]};
                rx_countdown = 4;
                rx_bits_remaining = rx_bits_remaining - 1'b1;
                recv_state = rx_bits_remaining ? RX_READ_BITS : RX_CHECK_STOP;
            end
        end
        RX_CHECK_STOP: begin
            if (!rx_countdown) begin
                // Should resume half-way through the stop bit
                // This should be high - if not, reject the
                // transmission and signal an error.
                recv_state = iRX ? RX_RECEIVED : RX_ERROR;
            end
        end
        RX_DELAY_RESTART: begin
            // Waits a set number of cycles before accepting
            // another transmission.
            recv_state = rx_countdown ? RX_DELAY_RESTART : RX_IDLE;
        end
        RX_ERROR: begin
            // There was an error receiving.
            // Raises the recv_error flag for one clock
            // cycle while in this state and then waits
            // 2 bit periods before accepting another
            // transmission.
            rx_countdown = 8;
            recv_state = RX_DELAY_RESTART;
        end
        RX_RECEIVED: begin
            // Successfully received a byte.
            // Raises the received flag for one clock
            // cycle while in this state.
            recv_state = RX_IDLE;
        end
    endcase

    // Transmit state machine
    case (tx_state)
        TX_IDLE: begin
            if (iTRAN_ST) begin
                // If the transmit flag is raised in the idle
                // state, start transmitting the current content
                // of the tx_byte input.
                tx_data = iTX_BYTE;
                // Send the initial, low pulse of 1 bit period
                // to signal the start, followed by the data
                tx_clk_divider = CLOCK_DIVIDE;
                tx_countdown = 4;
                tx_out = 0;
                tx_bits_remaining = 8;
                tx_state = TX_SENDING;
            end
        end
        TX_SENDING: begin
            if (!tx_countdown) begin
                if (tx_bits_remaining) begin
                    tx_bits_remaining = tx_bits_remaining - 1'b1;
                    tx_out = tx_data[0];
                    tx_data = {1'b0, tx_data[7:1]};
                    tx_countdown = 4;
                    tx_state = TX_SENDING;
                end else begin
                    // Set delay to send out 2 stop bits.
                    tx_out = 1;
                    tx_countdown = 8;
                    tx_state = TX_DELAY_RESTART;
                end
            end
        end
        TX_DELAY_RESTART: begin
            // Wait until tx_countdown reaches the end before
            // we send another transmission. This covers the
            // "stop bit" delay.
            tx_state = tx_countdown ? TX_DELAY_RESTART : TX_IDLE;
        end
    endcase
end
 
endmodule 