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
        SYS_CLKs_sec : uint -- Number of system clocks periods in one second
    );
    port (
        clk          : in  std_logic;             -- system clock
        reset        : in  std_logic;             -- system reset (assume active high, change at top level if needed)
        period       : in  unsigned(17 downto 0); -- PWM repetition period in milliseconds; Data type UQ10.7
        duty_cycle   : in  unsigned(14 downto 0); -- PWM control word: [0 1]; Out-of-range values ignored and hard limited; Data type UQ2.12
        pwm_out      : out std_logic              -- PWM output signal
    );
end entity;


-- PWM controller functionality
-- TODO
architecture PWM_Arch of PWM is
begin
end architecture;
