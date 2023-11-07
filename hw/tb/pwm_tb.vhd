-- Lucas Ritzdorf
-- 11/04/2023
-- EELE 467, HW 5

use std.env.all;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;


-- PWM test bench
entity PWM_TB is
end entity;

architecture PWM_TB_Arch of PWM_TB is
    constant CLK_PER : time := 10 us;

    signal clk, reset : std_logic;
    signal period     : unsigned(16 downto 0);
    signal duty_cycle : unsigned(13 downto 0);
    signal output     : std_logic;
begin

    -- PWM DUT instance
    dut : entity work.PWM
        generic map (
            SYS_CLKs_sec => 100000
        )
        port map (
            clk        => clk,        -- system clock
            reset      => reset,      -- system reset, active high
            period     => period,     -- PWM period in milliseconds, UQ10.7
            duty_cycle => duty_cycle, -- PWM duty cycle, UQ2.12, range [0 1] (out-of-range values saturate)
            pwm_out    => output      -- PWM output signal
        );

    -- Clock driver
    clock : process is
    begin
        clk <= '1';
        while true loop
            wait for CLK_PER / 2;
            clk <= not clk;
        end loop;
    end process;

    -- Test driver
    tester : process is
    begin
        wait until falling_edge(clk);

        -- Initialization: reset system
        reset <= '1';
        period <= b"00000000010000000"; -- 1
        duty_cycle <= b"01000000000000"; -- 1
        for i in 1 to 5 loop
            wait until falling_edge(clk);
        end loop;
        reset <= '0';
        -- Basic test: full duty cycle
        for i in 1 to 99 loop
            wait until falling_edge(clk);
        end loop;

        -- Basic test: 10% duty cycle
        duty_cycle <= b"00000110011010";
        for i in 1 to 100 loop
            wait until falling_edge(clk);
        end loop;

        -- Basic test: 50% duty cycle
        duty_cycle <= b"00100000000000";
        for i in 1 to 100 loop
            wait until falling_edge(clk);
        end loop;

        -- Basic test: 90% duty cycle
        duty_cycle <= b"00111001100111";
        for i in 1 to 100 loop
            wait until falling_edge(clk);
        end loop;

        -- Fancy test: duty cycle is zero
        duty_cycle <= b"00000000000000";
        for i in 1 to 100 loop
            wait until falling_edge(clk);
        end loop;

        -- Fancy test: duty cycle just barely nonzero
        duty_cycle <= b"00000000101001";
        for i in 1 to 100 loop
            wait until falling_edge(clk);
        end loop;

        duty_cycle <= b"00100000000000";  -- Back to 50%
        -- Basic test: longer period
        period <= b"00000000100000000";
        for i in 1 to 200 loop
            wait until falling_edge(clk);
        end loop;

        -- Basic test: shorter period
        period <= b"00000000001000000";
        for i in 1 to 50 loop
            wait until falling_edge(clk);
        end loop;

        -- Fancy test: period is zero
        period <= b"00000000000000000";
        for i in 1 to 10 loop
            wait until falling_edge(clk);
        end loop;

        -- Fancy test: period just barely nonzero
        period <= b"00000000000000010";
        for i in 1 to 10 loop
            wait until falling_edge(clk);
        end loop;

        -- Fancy test: period slightly more nonzero
        period <= b"00000000000000011";
        for i in 1 to 10 loop
            wait until falling_edge(clk);
        end loop;

        finish;
    end process;

end architecture;
