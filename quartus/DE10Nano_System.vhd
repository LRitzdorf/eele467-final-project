--! @file
--!
--! @author Ross Snider
--!


library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;

library altera;
use altera.altera_primitives_components.all;


-----------------------------------------------------------
-- Signal Names are defined in the DE10-Nano User Manual
-- http://de10-nano.terasic.com
-----------------------------------------------------------
entity DE10Nano_System is
    port (
        ----------------------------------------
        --  CLOCK Inputs
        --  See DE10 Nano User Manual page 23
        ----------------------------------------
        FPGA_CLK1_50 : in    std_logic; --! 50 MHz clock input #1
        FPGA_CLK2_50 : in    std_logic; --! 50 MHz clock input #2
        FPGA_CLK3_50 : in    std_logic; --! 50 MHz clock input #3

        ----------------------------------------
        --  Push Button Inputs (KEY)
        --  See DE10 Nano User Manual page 24
        --  The KEY push button inputs produce a '0'
        --  when pressed (asserted)
        --  and produce a '1' in the rest (non-pushed) state
        --  a better label for KEY would be Push_Button_n
        ----------------------------------------
        KEY : in    std_logic_vector(1 downto 0); --! Two Pushbuttons (active low)

        ----------------------------------------
        --  Slide Switch Inputs (SW)
        --  See DE10 Nano User Manual page 25
        --  The slide switches produce a '0' when
        --  in the down position
        --  (towards the edge of the board)
        ----------------------------------------
        SW : in    std_logic_vector(3 downto 0); --! Four Slide Switches

        ----------------------------------------
        --  LED Outputs
        --  See DE10 Nano User Manual page 26
        --  Setting LED to 1 will turn it on
        ----------------------------------------
        LED : out   std_logic_vector(7 downto 0); --! Eight LEDs

        ----------------------------------------
        --  GPIO Expansion Headers (40-pin)
        --  See DE10 Nano User Manual page 27
        --  Pin 11 = 5V supply (1A max)
        --  Pin 29 - 3.3 supply (1.5A max)
        --  Pins 12, 30 GND
        ----------------------------------------
        GPIO_0 : inout std_logic_vector(35 downto 0);            --! The 40 pin header on the top of the board
        GPIO_1 : inout std_logic_vector(35 downto 0);            --! The 40 pin header on the bottom of the board

        ----------------------------------------
        --  Arduino Uno R3 Expansion Header
        --  See DE10 Nano User Manual page 30
        --  500 ksps, 8-channel, 12-bit ADC
        ----------------------------------------
        ARDUINO_IO      : inout std_logic_vector(15 downto 0); --! 16 Arduino I/O
        ARDUINO_RESET_N : inout std_logic;                     --! Reset signal, active low

        ----------------------------------------
        --  ADC
        --  See DE10 Nano User Manual page 33
        --  500 ksps, 8-channel, 12-bit ADC
        ----------------------------------------
        ADC_CONVST : out   std_logic; --! ADC Conversion Start
        ADC_SCK    : out   std_logic; --! ADC Serial Data Clock
        ADC_SDI    : out   std_logic; --! ADC Serial Data Input  (FPGA to ADC)
        ADC_SDO    : in    std_logic; --! ADC Serial Data Output (ADC to FPGA)

        ----------------------------------------
        --  Hard Processor System (HPS)
        --  See DE10 Nano User Manual page 36
        ----------------------------------------
        HPS_CONV_USB_N   : inout std_logic;
        HPS_DDR3_ADDR    : out   std_logic_vector(14 downto 0);
        HPS_DDR3_BA      : out   std_logic_vector(2 downto 0);
        HPS_DDR3_CAS_N   : out   std_logic;
        HPS_DDR3_CKE     : out   std_logic;
        HPS_DDR3_CK_N    : out   std_logic;
        HPS_DDR3_CK_P    : out   std_logic;
        HPS_DDR3_CS_N    : out   std_logic;
        HPS_DDR3_DM      : out   std_logic_vector(3 downto 0);
        HPS_DDR3_DQ      : inout std_logic_vector(31 downto 0);
        HPS_DDR3_DQS_N   : inout std_logic_vector(3 downto 0);
        HPS_DDR3_DQS_P   : inout std_logic_vector(3 downto 0);
        HPS_DDR3_ODT     : out   std_logic;
        HPS_DDR3_RAS_N   : out   std_logic;
        HPS_DDR3_RESET_N : out   std_logic;
        HPS_DDR3_RZQ     : in    std_logic;
        HPS_DDR3_WE_N    : out   std_logic;
        HPS_ENET_GTX_CLK : out   std_logic;
        HPS_ENET_INT_N   : inout std_logic;
        HPS_ENET_MDC     : out   std_logic;
        HPS_ENET_MDIO    : inout std_logic;
        HPS_ENET_RX_CLK  : in    std_logic;
        HPS_ENET_RX_DATA : in    std_logic_vector(3 downto 0);
        HPS_ENET_RX_DV   : in    std_logic;
        HPS_ENET_TX_DATA : out   std_logic_vector(3 downto 0);
        HPS_ENET_TX_EN   : out   std_logic;
        HPS_GSENSOR_INT  : inout std_logic;
        HPS_I2C0_SCLK    : inout std_logic;
        HPS_I2C0_SDAT    : inout std_logic;
        HPS_KEY          : inout std_logic;
        HPS_LED          : inout std_logic;
        HPS_LTC_GPIO     : inout std_logic;
        HPS_SD_CLK       : out   std_logic;
        HPS_SD_CMD       : inout std_logic;
        HPS_SD_DATA      : inout std_logic_vector(3 downto 0);
        HPS_UART_RX      : in    std_logic;
        HPS_UART_TX      : out   std_logic;
        HPS_USB_DATA     : inout std_logic_vector(7 downto 0);
        HPS_USB_CLKOUT   : in    std_logic;
        HPS_USB_STP      : out   std_logic;
        HPS_USB_DIR      : in    std_logic;
        HPs_USB_NXT      : in    std_logic
    );

end entity;


architecture DE10Nano_arch of DE10Nano_System is

    --------------------------------------------------------------
    -- SoC Component from Intel Platform Designer
    --------------------------------------------------------------
    component soc_system is
        port (
            clk_clk                             : in    std_logic;
            hps_f2h_cold_reset_req_reset_n      : in    std_logic;
            hps_f2h_debug_reset_req_reset_n     : in    std_logic;
            hps_f2h_warm_reset_req_reset_n      : in    std_logic;
            hps_h2f_reset_reset_n               : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_TX_CLK : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_TXD0   : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_TXD1   : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_TXD2   : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_TXD3   : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_RXD0   : in    std_logic;
            hps_hps_io_hps_io_emac1_inst_MDIO   : inout std_logic;
            hps_hps_io_hps_io_emac1_inst_MDC    : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_RX_CTL : in    std_logic;
            hps_hps_io_hps_io_emac1_inst_TX_CTL : out   std_logic;
            hps_hps_io_hps_io_emac1_inst_RX_CLK : in    std_logic;
            hps_hps_io_hps_io_emac1_inst_RXD1   : in    std_logic;
            hps_hps_io_hps_io_emac1_inst_RXD2   : in    std_logic;
            hps_hps_io_hps_io_emac1_inst_RXD3   : in    std_logic;
            hps_hps_io_hps_io_sdio_inst_CMD     : inout std_logic;
            hps_hps_io_hps_io_sdio_inst_D0      : inout std_logic;
            hps_hps_io_hps_io_sdio_inst_D1      : inout std_logic;
            hps_hps_io_hps_io_sdio_inst_CLK     : out   std_logic;
            hps_hps_io_hps_io_sdio_inst_D2      : inout std_logic;
            hps_hps_io_hps_io_sdio_inst_D3      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D0      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D1      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D2      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D3      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D4      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D5      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D6      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_D7      : inout std_logic;
            hps_hps_io_hps_io_usb1_inst_CLK     : in    std_logic;
            hps_hps_io_hps_io_usb1_inst_STP     : out   std_logic;
            hps_hps_io_hps_io_usb1_inst_DIR     : in    std_logic;
            hps_hps_io_hps_io_usb1_inst_NXT     : in    std_logic;
            hps_hps_io_hps_io_uart0_inst_RX     : in    std_logic;
            hps_hps_io_hps_io_uart0_inst_TX     : out   std_logic;
            hps_hps_io_hps_io_i2c0_inst_SDA     : inout std_logic;
            hps_hps_io_hps_io_i2c0_inst_SCL     : inout std_logic;
            hps_hps_io_hps_io_gpio_inst_GPIO09  : inout std_logic;
            hps_hps_io_hps_io_gpio_inst_GPIO35  : inout std_logic;
            hps_hps_io_hps_io_gpio_inst_GPIO40  : inout std_logic;
            hps_hps_io_hps_io_gpio_inst_GPIO53  : inout std_logic;
            hps_hps_io_hps_io_gpio_inst_GPIO54  : inout std_logic;
            hps_hps_io_hps_io_gpio_inst_GPIO61  : inout std_logic;
            memory_mem_a                        : out   std_logic_vector(14 downto 0);
            memory_mem_ba                       : out   std_logic_vector(2 downto 0);
            memory_mem_ck                       : out   std_logic;
            memory_mem_ck_n                     : out   std_logic;
            memory_mem_cke                      : out   std_logic;
            memory_mem_cs_n                     : out   std_logic;
            memory_mem_ras_n                    : out   std_logic;
            memory_mem_cas_n                    : out   std_logic;
            memory_mem_we_n                     : out   std_logic;
            memory_mem_reset_n                  : out   std_logic;
            memory_mem_dq                       : inout std_logic_vector(31 downto 0);
            memory_mem_dqs                      : inout std_logic_vector(3 downto 0);
            memory_mem_dqs_n                    : inout std_logic_vector(3 downto 0);
            memory_mem_odt                      : out   std_logic;
            memory_mem_dm                       : out   std_logic_vector(3 downto 0);
            memory_oct_rzqin                    : in    std_logic;
            reset_reset_n                       : in    std_logic;
            adc_iface_sclk                      : out   std_logic;
            adc_iface_cs_n                      : out   std_logic;
            adc_iface_dout                      : in    std_logic                     := 'X';
            adc_iface_din                       : out   std_logic;
            pwms_out_channels                   : out   std_logic_vector(2 downto 0)
        );
    end component soc_system;

    ---------------------------------------------------------
    -- Signal declarations
    ---------------------------------------------------------
    signal hps_fpga_reset_n : std_logic;
    signal hps_reset_req    : std_logic_vector(2 downto 0);

    signal hps_cold_reset  : std_logic;
    signal hps_warm_reset  : std_logic;
    signal hps_debug_reset : std_logic;
    signal hps_h2f_reset   : std_logic;

    signal system_rst  : std_logic;                    --! Global reset pin
    signal Push_Button : std_logic_vector(1 downto 0); --! a better description of KEY input, which should really be labelled as KEY_n

    signal LED_GPIOs : std_logic_vector(0 to 2);

begin

    ---------------------------------------------------------------------------------------------
    -- Signal renaming to make code more readable
    ---------------------------------------------------------------------------------------------
    Push_Button <= not KEY; -- Rename signal to push button, which is a better description of KEY input (which really should be labelled as KEY_n since it is active low).

    -------------------------------------------------------
    -- HPS
    -------------------------------------------------------
    hps_cold_reset  <= '0';
    hps_debug_reset <= '0';
    hps_warm_reset  <= '0';

    ---------------------------------------------------------------------------------------------
    -- SoC System
    ---------------------------------------------------------------------------------------------
    u0 : component soc_system
        port map (
            -- HPS Clock and Reset
            clk_clk                         => FPGA_CLK1_50,
            reset_reset_n                   => not hps_cold_reset,
            hps_f2h_cold_reset_req_reset_n  => not hps_cold_reset,
            hps_f2h_debug_reset_req_reset_n => not hps_debug_reset,
            hps_f2h_warm_reset_req_reset_n  => not hps_warm_reset,
            hps_h2f_reset_reset_n           => hps_h2f_reset,

            -- HPS Ethernet
            hps_hps_io_hps_io_emac1_inst_TX_CLK => HPS_ENET_GTX_CLK,
            hps_hps_io_hps_io_emac1_inst_TXD0   => HPS_ENET_TX_DATA(0),
            hps_hps_io_hps_io_emac1_inst_TXD1   => HPS_ENET_TX_DATA(1),
            hps_hps_io_hps_io_emac1_inst_TXD2   => HPS_ENET_TX_DATA(2),
            hps_hps_io_hps_io_emac1_inst_TXD3   => HPS_ENET_TX_DATA(3),
            hps_hps_io_hps_io_emac1_inst_RXD0   => HPS_ENET_RX_DATA(0),
            hps_hps_io_hps_io_emac1_inst_MDIO   => HPS_ENET_MDIO,
            hps_hps_io_hps_io_emac1_inst_MDC    => HPS_ENET_MDC,
            hps_hps_io_hps_io_emac1_inst_RX_CTL => HPS_ENET_RX_DV,
            hps_hps_io_hps_io_emac1_inst_TX_CTL => HPS_ENET_TX_EN,
            hps_hps_io_hps_io_emac1_inst_RX_CLK => HPS_ENET_RX_CLK,
            hps_hps_io_hps_io_emac1_inst_RXD1   => HPS_ENET_RX_DATA(1),
            hps_hps_io_hps_io_emac1_inst_RXD2   => HPS_ENET_RX_DATA(2),
            hps_hps_io_hps_io_emac1_inst_RXD3   => HPS_ENET_RX_DATA(3),

            -- HPS USB OTG
            hps_hps_io_hps_io_usb1_inst_D0  => HPS_USB_DATA(0),
            hps_hps_io_hps_io_usb1_inst_D1  => HPS_USB_DATA(1),
            hps_hps_io_hps_io_usb1_inst_D2  => HPS_USB_DATA(2),
            hps_hps_io_hps_io_usb1_inst_D3  => HPS_USB_DATA(3),
            hps_hps_io_hps_io_usb1_inst_D4  => HPS_USB_DATA(4),
            hps_hps_io_hps_io_usb1_inst_D5  => HPS_USB_DATA(5),
            hps_hps_io_hps_io_usb1_inst_D6  => HPS_USB_DATA(6),
            hps_hps_io_hps_io_usb1_inst_D7  => HPS_USB_DATA(7),
            hps_hps_io_hps_io_usb1_inst_CLK => HPS_USB_CLKOUT,
            hps_hps_io_hps_io_usb1_inst_STP => HPS_USB_STP,
            hps_hps_io_hps_io_usb1_inst_DIR => HPS_USB_DIR,
            hps_hps_io_hps_io_usb1_inst_NXT => HPs_USB_NXT,

            -- HPS SD Card
            hps_hps_io_hps_io_sdio_inst_CMD => HPS_SD_CMD,
            hps_hps_io_hps_io_sdio_inst_D0  => HPS_SD_DATA(0),
            hps_hps_io_hps_io_sdio_inst_D1  => HPS_SD_DATA(1),
            hps_hps_io_hps_io_sdio_inst_CLK => HPS_SD_CLK,
            hps_hps_io_hps_io_sdio_inst_D2  => HPS_SD_DATA(2),
            hps_hps_io_hps_io_sdio_inst_D3  => HPS_SD_DATA(3),

            -- HPS UART
            hps_hps_io_hps_io_uart0_inst_RX => HPS_UART_RX,
            hps_hps_io_hps_io_uart0_inst_TX => HPS_UART_TX,

            -- HPS I2C 0 to G-Sensor
            hps_hps_io_hps_io_i2c0_inst_SDA => HPS_I2C0_SDAT,
            hps_hps_io_hps_io_i2c0_inst_SCL => HPS_I2C0_SCLK,

            -- HPS GPIO
            hps_hps_io_hps_io_gpio_inst_GPIO09 => HPS_CONV_USB_N,
            hps_hps_io_hps_io_gpio_inst_GPIO35 => HPS_ENET_INT_N,
            hps_hps_io_hps_io_gpio_inst_GPIO40 => HPS_LTC_GPIO,
            hps_hps_io_hps_io_gpio_inst_GPIO53 => HPS_LED,
            hps_hps_io_hps_io_gpio_inst_GPIO54 => HPS_KEY,
            hps_hps_io_hps_io_gpio_inst_GPIO61 => HPS_GSENSOR_INT,

            -- HPS DDR3 DRAM
            memory_mem_a       => HPS_DDR3_ADDR,
            memory_mem_ba      => HPS_DDR3_BA,
            memory_mem_ck      => HPS_DDR3_CK_P,
            memory_mem_ck_n    => HPS_DDR3_CK_N,
            memory_mem_cke     => HPS_DDR3_CKE,
            memory_mem_cs_n    => HPS_DDR3_CS_N,
            memory_mem_ras_n   => HPS_DDR3_RAS_N,
            memory_mem_cas_n   => HPS_DDR3_CAS_N,
            memory_mem_we_n    => HPS_DDR3_WE_N,
            memory_mem_reset_n => HPS_DDR3_RESET_N,
            memory_mem_dq      => HPS_DDR3_DQ,
            memory_mem_dqs     => HPS_DDR3_DQS_P,
            memory_mem_dqs_n   => HPS_DDR3_DQS_N,
            memory_mem_odt     => HPS_DDR3_ODT,
            memory_mem_dm      => HPS_DDR3_DM,
            memory_oct_rzqin   => HPS_DDR3_RZQ,

            -- On-board ADC interface
            adc_iface_sclk => ADC_SCK,
            adc_iface_cs_n => ADC_CONVST,
            adc_iface_dout => ADC_SDO,
            adc_iface_din  => ADC_SDI,

            -- PWM outputs, in ascending order
            pwms_out_channels => LED_GPIOs
        );

    -- Connect PWM outputs to GPIOs
    GPIO_1(11) <= LED_GPIOs(0);
    GPIO_1(14) <= LED_GPIOs(1);
    GPIO_1(16) <= LED_GPIOs(2);

    -------------------------------------------------------
    -- DE10-Nano Board (unused output signals)
    -------------------------------------------------------
    GPIO_0          <= (others => 'Z');
    GPIO_1          <= (others => 'Z');
    ARDUINO_IO      <= (others => 'Z');
    ARDUINO_RESET_N <= 'Z';
    LED             <= (others => '0');

end architecture;
