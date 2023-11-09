-- altera vhdl_input_version vhdl_2008

-- Lucas Ritzdorf
-- 11/08/2023
-- EELE 467, Homework 6

use work.common.all;
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


-- HPS interface for color LED module
entity HPS_Multi_PWM is
    generic (
        ADDR_WIDTH   : positive := 1;       -- address bus width for Platform Designer
        NUM_CHANNELS : positive := 1;       -- number of PWM channels to produce, limited by address width
        SYS_CLKs_sec : positive := 50000000 -- number of system clock periods in one second
    );
    port (
        clk   : in std_logic; -- system clock
        reset : in std_logic; -- system reset, active high

        -- Memory-mapped Avalon agent interface
        avs_s1_read      : in  std_logic;
        avs_s1_write     : in  std_logic;
        avs_s1_address   : in  std_logic_vector(ADDR_WIDTH-1 downto 0);
        avs_s1_readdata  : out std_logic_vector(31 downto 0);
        avs_s1_writedata : in  std_logic_vector(31 downto 0);

        -- PWM output channels
        out_channels : out std_logic_vector(0 to NUM_CHANNELS-1)
    );
end entity;


architecture HPS_Multi_PWM_Arch of HPS_Multi_PWM is

    -- Avalon-mapped control registers
    type duty_cycle_t is array (natural range <>) of unsigned(13 downto 0);
    signal Period      : unsigned(16 downto 0);
    signal Duty_Cycles : duty_cycle_t(out_channels'range);

    -- PWM driver component
    component PWM is
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
    end component;

begin
    assert 2**(ADDR_WIDTH) >= NUM_CHANNELS + 1
        report "Address space must be able to hold [1 + NUM_CHANNELS] elements"
        severity error;

    -- Manage reading from mapped registers
    avalon_register_read : process (clk) is
    begin
        if rising_edge(clk) and avs_s1_read = '1' then
            if unsigned(avs_s1_address) = to_unsigned(0, avs_s1_address'length) then
                -- Register zero: period
                avs_s1_readdata <= std_logic_vector(resize(Period, avs_s1_readdata'length));
            elsif
                    (to_unsigned(1, avs_s1_address'length) <= unsigned(avs_s1_address))
                    and
                    (unsigned(avs_s1_address) <= to_unsigned(NUM_CHANNELS, avs_s1_address'length))
                then
                    -- Next NUM_CHANNELS registers: duty cycle array
                    avs_s1_readdata <= std_logic_vector(resize(
                                       Duty_Cycles(to_integer(unsigned(avs_s1_address)) - 1),
                                       avs_s1_readdata'length));
            elsif unsigned(avs_s1_address) > NUM_CHANNELS then
                -- Unused registers: zeros
                avs_s1_readdata <= (others => '0');
            end if;
        end if;
    end process;

    -- Manage writing to mapped registers
    avalon_register_write : process (clk, reset) is
    begin
        if reset then
            -- Reset all registers to their default values
            Period <= (others => '0');
            Duty_Cycles <= (others => (others => '0'));
        elsif rising_edge(clk) and avs_s1_write = '1' then
            if unsigned(avs_s1_address) = to_unsigned(0, avs_s1_address'length) then
                -- Register zero: period
                Period <= unsigned(avs_s1_writedata(Period'length-1 downto 0));
            elsif
                    (to_unsigned(1, avs_s1_address'length) <= unsigned(avs_s1_address))
                    and
                    (unsigned(avs_s1_address) <= to_unsigned(NUM_CHANNELS, avs_s1_address'length))
                then
                    -- Next NUM_CHANNELS registers: duty cycle array
                    Duty_Cycles(to_integer(unsigned(avs_s1_address)) - 1)
                        <= unsigned(avs_s1_writedata(Duty_Cycles(0)'length-1 downto 0));
            elsif unsigned(avs_s1_address) > NUM_CHANNELS then
                -- Unused registers: ignored
                null;
            end if;
        end if;
    end process;

    -- Instantiate three PWM drivers
    PWM_Drivers: for N in out_channels'range generate
        driver : PWM
            generic map (
                SYS_CLKs_sec => SYS_CLKs_sec
            )
            port map (
                clk        => clk,
                reset      => reset,
                period     => Period,
                duty_cycle => Duty_Cycles(N),
                pwm_out    => out_channels(N)
            );
    end generate;

end architecture;
