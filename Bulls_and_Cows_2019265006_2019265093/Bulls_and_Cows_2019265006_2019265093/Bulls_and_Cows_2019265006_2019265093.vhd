library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity Bulls_and_Cows_2019265006_2019265093 is
    port (
        clk : in std_logic;
        rst : in std_logic;
        uart_rx : in std_logic;
        uart_tx : out std_logic;
        led : out std_logic_vector(7 downto 0); -- 8-bit LED output
        seg : out std_logic_vector(6 downto 0); -- 7-segment display output
        User_Number1 : out std_logic_vector(3 downto 0); -- User number 1
        User_Number2 : out std_logic_vector(3 downto 0); -- User number 2
        User_Number3 : out std_logic_vector(3 downto 0); -- User number 3
        User_Number4 : out std_logic_vector(3 downto 0)  -- User number 4
    );
end Bulls_and_Cows_2019265006_2019265093;

architecture Behavioral of Bulls_and_Cows_2019265006_2019265093 is
    type StateType is (WAIT_FOR_INPUT, PROCESS_INPUT, CHECK_RESULT, DISPLAY_RESULT);
    signal state : StateType := WAIT_FOR_INPUT;
    signal secret1, secret2, secret3, secret4 : std_logic_vector(3 downto 0);
    signal attempts : integer := 9; -- Remaining attempts
    signal bulls, cows : integer := 0; -- Bulls and Cows counts
    signal lfsr1, lfsr2, lfsr3, lfsr4 : std_logic_vector(3 downto 0) := "0000"; -- LFSR initial values
    signal guess : std_logic_vector(15 downto 0) := (others => '0'); -- User guess
    signal uart_buffer : std_logic_vector(15 downto 0) := (others => '0');
    signal bit_count : integer := 0;
    signal receiving : boolean := false;
    signal data_valid : std_logic := '0';

    signal user_num1, user_num2, user_num3, user_num4 : std_logic_vector(3 downto 0); -- Internal signals for User numbers

    -- 7-segment display encoding function
    function seg7(input : integer) return std_logic_vector is
    begin
        case input is
            when 0 => return "0000001"; -- 0
            when 1 => return "1001111"; -- 1
            when 2 => return "0010010"; -- 2
            when 3 => return "0000110"; -- 3
            when 4 => return "1001100"; -- 4
            when 5 => return "0100100"; -- 5
            when 6 => return "0100000"; -- 6
            when 7 => return "0001111"; -- 7
            when 8 => return "0000000"; -- 8
            when 9 => return "0000100"; -- 9
            when others => return "1111111"; -- All off
        end case;
    end function;

begin
    -- Assign internal signals to output ports
    User_Number1 <= user_num1;
    User_Number2 <= user_num2;
    User_Number3 <= user_num3;
    User_Number4 <= user_num4;

    process(clk, rst)
    begin
        if rst = '1' then
            -- Reset initialization
            state <= WAIT_FOR_INPUT;
            lfsr1 <= "0000";
            lfsr2 <= "0000";
            lfsr3 <= "0000";
            lfsr4 <= "0000";
            secret1 <= (others => '0');
            secret2 <= (others => '0');
            secret3 <= (others => '0');
            secret4 <= (others => '0');
            guess <= (others => '0');
            attempts <= 9;
            bulls <= 0;
            cows <= 0;
            uart_buffer <= (others => '0');
            bit_count <= 0;
            receiving <= false;
            data_valid <= '0';
        elsif rising_edge(clk) then
            -- LFSRs for secret number generation
            lfsr1 <= lfsr1(2 downto 0) & (lfsr1(3) xor lfsr1(2));
            lfsr2 <= lfsr2(2 downto 0) & (lfsr2(3) xor lfsr2(2));
            lfsr3 <= lfsr3(2 downto 0) & (lfsr3(3) xor lfsr3(2));
            lfsr4 <= lfsr4(2 downto 0) & (lfsr4(3) xor lfsr4(2));
            if state = WAIT_FOR_INPUT then
                secret1 <= std_logic_vector(to_unsigned(to_integer(unsigned(lfsr1)) mod 10, secret1'length));
                secret2 <= std_logic_vector(to_unsigned(to_integer(unsigned(lfsr2)) mod 10, secret2'length));
                secret3 <= std_logic_vector(to_unsigned(to_integer(unsigned(lfsr3)) mod 10, secret3'length));
                secret4 <= std_logic_vector(to_unsigned(to_integer(unsigned(lfsr4)) mod 10, secret4'length));
            end if;

            -- UART reception process
            if uart_rx = '1' then
                if not receiving then
                    receiving <= true;
                    bit_count <= 0;
                else
                    if bit_count < 16 then
                        uart_buffer(bit_count) <= uart_rx;
                        bit_count <= bit_count + 1;
                    end if;
                end if;
            else
                if receiving then
                    data_valid <= '1';
                    guess <= uart_buffer;
                    receiving <= false;
                end if;
            end if;

            case state is
                when WAIT_FOR_INPUT =>
                    -- Wait for input from UART
                    if data_valid = '1' then
                        state <= PROCESS_INPUT;
                        data_valid <= '0';
                    end if;
                when PROCESS_INPUT =>
                    -- Process input from UART
                    user_num1 <= guess(15 downto 12);
                    user_num2 <= guess(11 downto 8);
                    user_num3 <= guess(7 downto 4);
                    user_num4 <= guess(3 downto 0);
                    state <= CHECK_RESULT;
                when CHECK_RESULT =>
                    -- Compare User_Number with secret number
                    bulls <= 0;
                    cows <= 0;
                    if user_num1 = secret1 then
                        bulls <= bulls + 1;
                    else
                        if user_num1 = secret2 or user_num1 = secret3 or user_num1 = secret4 then
                            cows <= cows + 1;
                        end if;
                    end if;
                    
                    if user_num2 = secret2 then
                        bulls <= bulls + 1;
                    else
                        if user_num2 = secret1 or user_num2 = secret3 or user_num2 = secret4 then
                            cows <= cows + 1;
                        end if;
                    end if;
                    
                    if user_num3 = secret3 then
                        bulls <= bulls + 1;
                    else
                        if user_num3 = secret1 or user_num3 = secret2 or user_num3 = secret4 then
                            cows <= cows + 1;
                        end if;
                    end if;
                    
                    if user_num4 = secret4 then
                        bulls <= bulls + 1;
                    else
                        if user_num4 = secret1 or user_num4 = secret2 or user_num4 = secret3 then
                            cows <= cows + 1;
                        end if;
                    end if;
                    
                    attempts <= attempts - 1;
                    state <= DISPLAY_RESULT;
                when DISPLAY_RESULT =>
                    -- Display the result
                    if bulls = 4 then
                        -- If correct guess
                        seg <= "0111000"; -- Display "CLEAR"
                        led <= (others => '0'); -- Turn off LEDs
                    elsif attempts = 0 then
                        -- If out of attempts
                        seg <= "0111010"; -- Display "FAIL"
                        led <= (others => '0'); -- Turn off LEDs
                    else
                        -- If incorrect guess and attempts left
                        seg <= seg7(bulls); -- Display bulls on 7-segment
                        led <= std_logic_vector(to_unsigned(attempts, 8)); -- Display remaining attempts on LEDs
                    end if;
                    state <= WAIT_FOR_INPUT; -- Wait for next input after displaying result
            end case;
        end if;
    end process;

    -- UART communication (not implemented in detail)
    uart_tx <= '0'; -- Not used

end Behavioral;
