-- altera vhdl_input_version vhdl_2008

-- Lucas Ritzdorf
-- 03/11/2023
-- EELE 467

library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;


-- Convenience library with common utilities
package common is

    subtype uint is unsigned(31 downto 0);
    function clog2 (x : positive) return natural;

end package;


package body common is

    -- Reused from https://stackoverflow.com/a/12751341
    function clog2 (x : positive) return natural is
        variable i : natural;
    begin
        i := 0;
        while (2**i < x) and i < 31 loop
            i := i + 1;
        end loop;
        return i;
    end function;

end package body;
