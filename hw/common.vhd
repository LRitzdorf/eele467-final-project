-- altera vhdl_input_version vhdl_2008

-- Lucas Ritzdorf
-- 03/11/2023
-- EELE 467

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use ieee.std_logic_unsigned.all;


-- Convenience library with common utilities
package common is

    subtype uint is unsigned(31 downto 0);

end package;
