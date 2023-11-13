# Homework 6: HPS Multiple PWM Module


## RGB LED Resistor Calculations

We perform our analysis using the lowest expected forward voltages $V_f$.
This means that our calculated minimum resistances will be appropriate for the full range of expected voltages.
If we were to work from the highest expected forward voltages, the resulting resistor values would allow too much current through diodes with lower forward voltages.

$$
\begin{align*}
    2.0V &\le V_{f,red}   \le 2.2V  &  R_{s,red}   &= (3.3V - 2.0V) / 20mA = 65\Omega \\
    3.0V &\le V_{f,green} \le 3.2V  &  R_{s,green} &= (3.3V - 3.0V) / 20mA = 15\Omega \\
    3.0V &\le V_{f,blue}  \le 3.2V  &  R_{s,blue}  &= (3.3V - 3.0V) / 20mA = 15\Omega
\end{align*}
$$


## Basic RGB LED Operation

![RGB LED displaying red](/figures/hw-6-led-red.jpg)
![RGB LED displaying green](/figures/hw-6-led-green.jpg)
![RGB LED displaying blue](/figures/hw-6-led-blue.jpg)
