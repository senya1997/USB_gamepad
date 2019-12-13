onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /psone_tb/DUT/iCLK
add wave -noupdate /psone_tb/DUT/en
add wave -noupdate -height 35 -expand -group SPI -color Plum -height 32 /psone_tb/DUT/EN_SPI
add wave -noupdate -height 35 -expand -group SPI -color {Slate Blue} -height 35 /psone_tb/DUT/oCS
add wave -noupdate -height 35 -expand -group SPI -color Gold -height 35 /psone_tb/DUT/oCLK
add wave -noupdate -height 35 -expand -group SPI -color {Medium Orchid} -height 35 /psone_tb/DUT/oMOSI
add wave -noupdate -height 35 -expand -group SPI -color {Sky Blue} -height 35 /psone_tb/miso
add wave -noupdate -radix unsigned /psone_tb/DUT/cnt_half_per
add wave -noupdate -radix unsigned /psone_tb/DUT/cnt_edge
add wave -noupdate -radix unsigned /psone_tb/DUT/cnt_byte
add wave -noupdate -radix binary /psone_tb/DUT/req_data
add wave -noupdate /psone_tb/DUT/FRONT_CLK
add wave -noupdate -height 35 -expand -group transmit /psone_tb/DUT/tx_st
add wave -noupdate -height 35 -expand -group transmit -radix unsigned -childformat {{{/psone_tb/DUT/cnt_tran_byte[3]} -radix unsigned} {{/psone_tb/DUT/cnt_tran_byte[2]} -radix unsigned} {{/psone_tb/DUT/cnt_tran_byte[1]} -radix unsigned} {{/psone_tb/DUT/cnt_tran_byte[0]} -radix unsigned}} -expand -subitemconfig {{/psone_tb/DUT/cnt_tran_byte[3]} {-height 15 -radix unsigned} {/psone_tb/DUT/cnt_tran_byte[2]} {-height 15 -radix unsigned} {/psone_tb/DUT/cnt_tran_byte[1]} {-height 15 -radix unsigned} {/psone_tb/DUT/cnt_tran_byte[0]} {-height 15 -radix unsigned}} /psone_tb/DUT/cnt_tran_byte
add wave -noupdate -height 35 -expand -group transmit /psone_tb/DUT/TRAN_BUSY
add wave -noupdate -height 35 -expand -group transmit /psone_tb/DUT/TRAN_BUSY_FALL
add wave -noupdate -height 35 -expand -group transmit -expand /psone_tb/DUT/tx_byte
add wave -noupdate /psone_tb/DUT/HALF_PER
add wave -noupdate /psone_tb/DUT/ONE_BYTE
add wave -noupdate /psone_tb/DUT/FIRST_BYTE
add wave -noupdate /psone_tb/DUT/SEC_BYTE
add wave -noupdate /psone_tb/DUT/END_PACKET
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {12544000 ns} 0}
quietly wave cursor active 1
configure wave -namecolwidth 181
configure wave -valuecolwidth 76
configure wave -justifyvalue left
configure wave -signalnamewidth 2
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits ms
update
WaveRestoreZoom {0 ns} {29400 us}
