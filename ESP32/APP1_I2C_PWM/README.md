# I2C and PWM Example

This example reads the temperature sensor TC74 and then converts it to a duty cycle in order to control the ilumination of a led.

The I2C implementation is based in the solution of [https://github.com/ThomasGeor/esp32_tc74_temperature_sensor/tree/master/main]. However the writting and reading was followed by the documentation.
Timer configuration is based on espressif-idf documentation.

The main consists in a simple loop that reads from the temperature sensor by the I2C peripheral using a master. Then we convert the temperature to the duty cycle with static values. Finally we set and update the duty cyle of the PWM module.

Note: whenever the temperature sensor is not reading values, it is set to STANDBY_MODE.
 
# STATUS
TESTED SUCCESSFULLY
