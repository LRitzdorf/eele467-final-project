onerror {resume}
radix define fixed#7#decimal -fixed -fraction 7 -base signed -precision 6
radix define fixed#12#decimal -fixed -fraction 12 -base signed -precision 6
quietly WaveActivateNextPane {} 0
add wave -noupdate /pwm_tb/clk
add wave -noupdate /pwm_tb/reset
add wave -noupdate -radix fixed#7#decimal /pwm_tb/period
add wave -noupdate -radix fixed#12#decimal /pwm_tb/duty_cycle
add wave -noupdate /pwm_tb/output
add wave -noupdate -divider DUT
add wave -noupdate -radix decimal /pwm_tb/dut/per_limit
add wave -noupdate -radix decimal /pwm_tb/dut/duty_limit
add wave -noupdate -radix decimal /pwm_tb/dut/count
add wave -noupdate -radix decimal /pwm_tb/dut/next_count
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {1045000 ns} 0} {{Cursor 2} {2045000 ns} 0}
quietly wave cursor active 0
configure wave -namecolwidth 182
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 0
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits us
update
WaveRestoreZoom {0 ns} {9287250 ns}
