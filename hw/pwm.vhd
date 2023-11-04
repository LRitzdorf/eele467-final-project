-- Lucas Ritzdorf
-- 03/11/2023
-- EELE 467, Lab 5

use work.common.all;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;


-- PWM controller interface
entity PWM is
    generic (
        SYS_CLKs_sec : integer range 0 to 2**30 -- Number of system clocks periods in one second
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
    constant CLKS_PER_MS : uint := to_unsigned(SYS_CLKs_sec / 1000, 32);
    -- Derived control values
    signal per_limit, duty_limit : uint;
    -- Counter
    signal count : uint;
begin

    -- Use a basic counter to track progress through the PWM cycle
    PWM_counter : process (clk, reset) is
        -- Internal intermediate variable for control value updates
        variable clks_per_period : uint;
    begin
        clks_per_period := "*"(period, CLKS_PER_MS)(38 downto 7);

        if reset then
            -- Reset counter
            count <= (others => '0');
            -- Update control values
            per_limit <= clks_per_period;
            duty_limit <= "*"(clks_per_period, duty_cycle)(43 downto 12);
        elsif rising_edge(clk) then

            -- Count up or reset, as appropriate
            if count >= per_limit - 1 then
                count <= (others => '0');
            else
                count <= count + 1;
            end if;
            -- Update control values
            if count = 0 then
                per_limit <= clks_per_period;
                duty_limit <= "*"(clks_per_period, duty_cycle)(43 downto 12);
            end if;

        end if;
    end process;

    -- PWM output is a simple combinational comparison
    pwm_out <= '1' when (reset = '0') and (count < duty_limit) else '0';

end architecture;
