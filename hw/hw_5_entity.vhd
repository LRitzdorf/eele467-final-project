
entity myPWM is
    port(
        clk          : in  std_logic;                      -- system clock
        reset        : in  std_logic;                      -- system reset (assume active high, change at top level if needed)
        SYS_CLKs_sec : in  std_logic_vector( 31 downto 0); -- Number of system clocks periods in one second
        period       : in  std_logic_vector(W-1 downto 0); -- PWM repetition period in milliseconds; Data type (W.F) individually assigned
        duty_cycle   : in  std_logic_vector(W-1 downto 0); -- PWM control word: [0 100]; Out-of-range values ignored and hard limited; Data type (W.F) individually assigned
        signal       : out std_logic                       -- PWM output signal
    );
end entity myPWM;

