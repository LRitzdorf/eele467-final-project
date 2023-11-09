-- Lucas Ritzdorf
-- 03/11/2023
-- EELE 467, Lab 5

use work.common.all;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


-- PWM controller interface
entity PWM is
    generic (
        SYS_CLKs_sec : positive -- Number of system clocks periods in one second
    );
    port (
        clk        : in  std_logic;             -- system clock
        reset      : in  std_logic;             -- system reset, active high
        period     : in  unsigned(16 downto 0); -- PWM period in milliseconds, UQ10.7
        duty_cycle : in  unsigned(13 downto 0); -- PWM duty cycle, UQ2.12, range [0 1] (out-of-range values saturate)
        pwm_out    : out std_logic              -- PWM output signal
    );
end entity;


-- PWM controller functionality
architecture PWM_Arch of PWM is
    -- Number of system clocks per millisecond (synthesis-time derived constant)
    constant CLKS_PER_MS : natural := SYS_CLKs_sec / 1000;
    -- Derived control values
    signal per_limit, duty_limit : natural;
    -- Counter
    signal count, next_count : natural;
begin

    -- Count up or reset, as appropriate
    count_gen : process (all) is
    begin
        if reset then
            next_count <= 0;
        elsif (count >= per_limit - 1) or (per_limit = 0) then
            next_count <= 0;
        else
            next_count <= count + 1;
        end if;
    end process;

    -- Use a basic counter to track progress through the PWM cycle
    pwm_gen : process (clk, reset) is
        -- Internal intermediate variable for control value updates
        variable clks_per_period : natural;
    begin
        clks_per_period := to_integer(shift_right("*"(period, CLKS_PER_MS), 7));

        if reset then
            -- Reset counter
            count <= 0;
            -- Update control values
            per_limit <= clks_per_period;
            duty_limit <= to_integer(shift_right("*"(clks_per_period, duty_cycle), 12));
        elsif rising_edge(clk) then

            -- Update control values
            if next_count = 0 then
                per_limit <= clks_per_period;
                duty_limit <= to_integer(shift_right("*"(clks_per_period, duty_cycle), 12));
            end if;
            -- Update counter
            count <= next_count;

        end if;
    end process;

    -- PWM output is a simple combinational comparison
    pwm_out <= '1' when (reset = '0') and (count < duty_limit) else '0';

end architecture;
