#include "motor.h"

#include "pico/printf.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"

motor_t motor_init(int FAULT_pin, 
					int SLEEP_pin, 
					int DRVOFF_pin,
					int SCS_pin,
					int BRAKE_pin,
					int PWM_pin,
					int FGOUT_pin){

	gpio_set_function(PWM_pin, GPIO_FUNC_PWM);
	int slice_num = pwm_gpio_to_slice_num(PWM_pin);

	motor_t motor = (motor_t) {
		.FAULT_pin_ = FAULT_pin,
		.SLEEP_pin_ = SLEEP_pin,
		.DRVOFF_pin_ = DRVOFF_pin,
		.SCS_pin_ = SCS_pin,
		.BRAKE_pin_ = BRAKE_pin,
		.PWM_pin_ = PWM_pin,
		.FGOUT_pin_ = FGOUT_pin,
		.pwm_slice_ = slice_num, 
		.set_speed_ = 0,
	};

	gpio_init(motor.FAULT_pin_); // Sets function SIO
	gpio_set_dir(motor.FAULT_pin_, GPIO_IN);
	gpio_disable_pulls(motor.FAULT_pin_);

	// Set freq to 20hz
	pwm_set_clkdiv(slice_num, 1.83f);
	pwm_set_wrap(slice_num, 1<<12);
	// SPI Driver
	gpio_init(motor.SCS_pin_);
    gpio_set_dir(motor.SCS_pin_, GPIO_OUT);
	gpio_pull_down(motor.SCS_pin_);
    gpio_put(motor.SCS_pin_, 1);

	return motor;
}

void motor_enable(motor_t* motor){
	// First take it out of sleep
	gpio_init(motor->SLEEP_pin_); // Sets function SIO
	gpio_set_dir(motor->SLEEP_pin_, GPIO_OUT);
	gpio_pull_up(motor->SLEEP_pin_);
	gpio_put(motor->SLEEP_pin_, 1);
	sleep_ms(10);
	// Unlock registers for writing
	//motor_write_register(motor, 0x3, 0b011);

	//motor_write_register(motor, 0x4, 0x2);

	pwm_set_enabled(motor->pwm_slice_, true);
}

void motor_disable(motor_t* motor) {
	pwm_set_enabled(motor->pwm_slice_, false);
	gpio_put(motor->SLEEP_pin_, 0);
}

void motor_set_speed(motor_t* motor, uint16_t speed) {
	pwm_set_gpio_level(motor->PWM_pin_, speed);
	pwm_set_enabled(motor->pwm_slice_, true);
}

uint8_t motor_read_register(motor_t* motor, uint8_t reg_addr) {
	uint8_t tx_buf[2] = {0,0};
	uint8_t rx_buf[2] = {0,0};

	// Construct the read xcommand (MSB = 1 for read)
	tx_buf[0] = 0;
	tx_buf[0] |= (1 << 7);  // Read from register
	tx_buf[0] |= (reg_addr << 1);

	uint8_t parity_count = 0;
	for (int i=0; i < 8; i++) {
		if (tx_buf[0] & (1 << i)){
			parity_count++;
		}
	}
	if (parity_count % 2 == 1) {
		tx_buf[0] |= 1;
	}

	tx_buf[1] = 0x00;  // Dummy byte

	gpio_put(motor->SCS_pin_, 0);
	spi_write_read_blocking(spi0, tx_buf, rx_buf, 2);
	gpio_put(motor->SCS_pin_, 1);

	sleep_us(400);

	return rx_buf[1];  // Second byte contains the register value
}

uint8_t motor_write_register(motor_t* motor, uint8_t reg_addr, uint8_t data) {
	uint8_t tx_buf[2] = {0,0};
	uint8_t rx_buf[2] = {0,0};

	tx_buf[0] = 0;
	// Construct the write command (MSB = 0 for write - so do nothing)
	tx_buf[0] |= (reg_addr << 1);

	tx_buf[1] = data; 

	uint8_t parity_count = 0;
	for (int i=0; i < 8; i++) {
		if (tx_buf[0] & (1 << i)){
			parity_count++;
		}
        if (tx_buf[1] & (1 << i)) {
            parity_count++;
        }
	}
	if (parity_count % 2 == 1) {
		tx_buf[0] |= 1;
	}
	

	gpio_put(motor->SCS_pin_, 0);
	spi_write_read_blocking(spi0, tx_buf, rx_buf, 2);
	gpio_put(motor->SCS_pin_, 1);

	sleep_us(400);

    return rx_buf[0];  // Second byte contains the previous value
}

void motor_reset_fault(motor_t* motor) {
    gpio_put(motor->SLEEP_pin_, 0);
	sleep_us(30);
    gpio_put(motor->SLEEP_pin_, 1);
}
