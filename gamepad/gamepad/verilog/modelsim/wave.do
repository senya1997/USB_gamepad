onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /psone_tb/DUT/iCLK
add wave -noupdate /psone_tb/DUT/iKEY_ST
add wave -noupdate -color {Slate Blue} -height 35 /psone_tb/DUT/oCS
add wave -noupdate -color Gold -height 35 /psone_tb/DUT/oCLK
add wave -noupdate -color {Medium Orchid} -height 35 /psone_tb/DUT/oMOSI
add wave -noupdate /psone_tb/DUT/en
add wave -noupdate -radix unsigned /psone_tb/DUT/cnt_half_per
add wave -noupdate -radix unsigned /psone_tb/DUT/cnt_data
add wave -noupdate -radix unsigned /psone_tb/DUT/cnt_edge
add wave -noupdate /psone_tb/DUT/tx_st
add wave -noupdate /psone_tb/DUT/tx_byte
add wave -noupdate /psone_tb/DUT/HALF_PER
add wave -noupdate /psone_tb/DUT/ONE_BYTE
add wave -noupdate /psone_tb/DUT/FIRST_BYTE
add wave -noupdate /psone_tb/DUT/SEC_BYTE
add wave -noupdate /psone_tb/DUT/END_PACKET
add wave -noupdate /psone_tb/DUT/TRAN_BUSY
add wave -noupdate /psone_tb/DUT/EN_SPI
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {19785 ns} 0}
quietly wave cursor active 1
configure wave -namecolwidth 181
configure wave -valuecolwidth 100
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
configure wave -timelineunits ns
update
WaveRestoreZoom {0 ns} {42 us}
