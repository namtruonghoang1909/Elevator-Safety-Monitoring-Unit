### MPU6050 REGISTER CONFIGURATING ###

1. Register Configuration
    a. Registers 13 to 16 – Self Test Registers (READ) (not needed)
        - before testing, full-scale gyro and acce should be set to ~250 deg/sec and ~8g
        - before testing, register 27 test bits should be set to 1 first
        - These register contain results collected from tests, which will be compared with the provided bar from the FACTORY TRIM

        - gyroscope and acceleration test XYZ, from register 13 - 15:
            + 0x0D - 13 : XA_TEST[4-2] XG_TEST[4-0]
            + 0x0E - 14 : YA_TEST[4-2] YG_TEST[4-0]
            + 0x0F - 15 : ZA_TEST[4-2] ZG_TEST[4-0] 
            + 0x10 - 16 : ZA_TEST[1-0] _ bit 0-1, YA_TEST[1-0] _ bit 2-3, XA_TEST[1-0] _ bit 4-5, bit 6-7 _ rr
            + the factory trim values for the accel should be in decimal format, and they are determined by concatenating 
              the upper accelerometer self test bits (bits 4-2) with the lower accelerometer self test bits (bits 1-0).  

    b. Register 0x19 _ 25 – Sample Rate Divider(SMPRT_DIV) (0000 1001)
        - The sensor register output, FIFO output, and DMP sampling are all based on the Sample Rate. 
        - Sample rate is the rate that new datas appear in the mpu registers.
        - Gyroscope output rate is the rate that the mpu reads from the MEMS gyro.
            <<< Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV) >>>

            + Gyroscope Output Rate = 1kHz
            + Sample Rate = 100 hz 
            -> SMPLRT_DIV = 9 

    c. Register 0x1A _ 26 – Configuration (00000011)
        - This register configurates FSYNC and Digital Low Pass Filter(DLPF)
        - EST_SYCN_SET: bit 3 - 5, this should be 0
        - DLPF_CFG: bit 0-2, this should be 3( 44 and 42 bandwidth is okay for this model)
        - bit 6-7: reserved

    d. Register 0x1B _ 27 - Gyro Config (00010rrr)
        - This register starts xyz gyro self-test (not needed), set each bit to 1:
            - xg test bit 7 
            - yg test bit 6
            - zg test bit 5
        - Configures the gyros full scope: bit 3-4, this should be 2 (1000 deg/sec).

    e. Register 0x1C _ 28 - Acceleration configuration (00010rrr)
        - This register starts xyz gyro self-test(not needed), set each bit to 1:
            - xa test bit 7 
            - ya test bit 6
            - za test bit 5
        - Configures the acceleroometers full range: bit 3-4, this should be 2 (8g).
    
    f. Register 35 - FIFO Enable (not needed)
        - This register determines which sensor measurements are loaded into the FIFO buffer.  

    g. Register 35 - Master i2c control (not needed)
    h. Register 37 to 53 - i2c slaves control (not needed)
    i. Register 54 - i2c master status

    j. Register 59 - 64: Accelerometers measurements (READ)
        - 0x3B : ACCEL_XOUT[15:8]
        - 0x3C : ACCEL_XOUT[7:0] 
        - 0x3D : ACCEL_YOUT[7:0] 
        - 0x3E : ACCEL_YOUT[7:0] 
        - 0x3F : ACCEL_ZOUT[15:8]
        - 0x40 : ACCEL_ZOUT[7:0] 
        - Accelerometer measurements are written to these registers at the Sample Rate(reg 25)

    k. Register 65 - 66: Temperature measurement
        - 0x41 : TEMP_OUT[15:8] 
        - 0x42 : TEMP_OUT[7:0] 
        - Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity)/340 + 36.53 
    
    l. Registers 73 to 96 – External Sensor Data (not needed)
    m. Register 0x68 _ 104 – Signal Path Reset 
        - Use this when we want to  reset the analog and digital signal paths of the gyroscope, accelerometer, 
        and temperature sensors.
        - Set bit 0 to 1: temperature reset.
        - Set bit 1 to 1: accelemeters reset.
        - Set bit 2 to 1: gyrometers reset.

    o. Register 0x6A _ 106 - User Control (0x00)
        - This register allows the user to enable and disable the FIFO buffer, I2C Master Mode, and primary 
        I2C interface. The FIFO buffer, I2C Master, sensor signal paths and sensor registers can also be 
        reset using this register. 
        - bit 6 = 0 : fifo is disabled
        - bit 5 = 0 : i2c master mode is disabled
        - bit 4 = 0 : always
        - bit 3 = r
        - bit 2 = 1 : resets the fifo buffer
        - bit 1 = 1 : reests the i2c master 
        - bit 0 = 1 : resets sensor's signal pass and registers

    p. Register 0x6B _ 107 – Power Management 1(0x01)
        - This register allows the user to configure the power mode and clock source. It also provides a bit for 
        resetting the entire device, and a bit for disabling the temperature sensor. 
        - After power-up, the default state is:
            + SLEEP = 1
            + CLKSEL = 000 (internal oscillator)
        - We need to set sleep bit to 0, clksel to 001 after powering the mpu
        - bit 7 : set to 1 to reset the device
        - bit 6 : set to 1 to enter power sleep mode, this should be 0
        - bit 5 : when bit 6 = 0, set this bit to 1 to enter CYCLE mode
        - bit 4 : r
        - bit 3 : set to 1 to disable the temp sensor
        - bit 2-0 : chooses the clock source 

    q. Register 0x6C _ 108 - Power Management 2 (0x00)
        - This register allows the user to configure the frequency of wake-ups in Accelerometer Only Low 
        Power Mode. This register also allows the user to put individual axes of the accelerometer and 
        gyroscope into standby mode.  
        - Cause we dont use cycle mode we dont want to set axes to standbys

    r. Register 0x75 _ 117 - mpu's address
        - This register is used to verify the identity of the device.
        - The MCU can connect to max 2 mpus, decided by the AD0 pin
        
