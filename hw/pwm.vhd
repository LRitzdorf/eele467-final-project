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

    -- Derive control values
    per_limit <= "*"(period, CLKS_PER_MS)(38 downto 7);
    duty_limit <= "*"(per_limit, duty_cycle)(43 downto 12);

    -- Use a basic counter to track progress through the PWM cycle
    PWM_counter : process (clk, reset) is
    begin

        if reset then
            count <= (others => '0');
            pwm_out <= '0';
        elsif rising_edge(clk) then
            -- Update the PWM output state
            if count < duty_limit - 1 then
                pwm_out <= '1';
            else
                pwm_out <= '0';
            end if;
            -- Count up or reset, as appropriate
            if count >= per_limit - 1 then
                count <= (others => '0');
            else
                count <= count + 1;
            end if;
        end if;

    end process;

end architecture;
