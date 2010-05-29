----------------------------------------------------------------------
-  STM32MCUtest:                                                     -
-  (C) 2010 Edward Cheeseman <cheesemanedward@gmail.com>                  -
-  A commissioning and fault finding tool for circuits connected to  -
-  a STM32.                                                          -
----------------------------------------------------------------------

Specifically designed for the tumanako STM32MCU (http://tumanako.net)
vehicle motor control board, this tool is generic enough to be useful
of anyone wishing to fault find the hardware layer surrounding the
microcontroller. 

The tool is controlled via USART1, which is set at 38400 8N1.

To use the tool, simply type in a Port (Uppercase A-E), a bit (0-f)
and a command (r,s,o,i,g). Only change what you need to. Unchanged
addresses are remembered. If you want to toggle a pin on and off, once you have set the port and bit you can just press r and s as needed.

Hold down <space> to get an updating analog value.

Commands:
0-f - working pin
A-E - working port
r   - reset output
s   - set output
o   - set pin as output
i   - set pin as input
g   - set pin as analog in


Display:

Axxxxxxxx Bxxxxxxxx Cxxxxxxxx Dxxxxxxxx Exxxxxxxx abc dyyyyy

where x:{0,1}, a:{A-E}, b:{0-f}, c:{i,o,s,r,g}, d:{0-g}, yyyyy: {0-32000}

