#include "main.h"
#include "dht20.h"
#include "global.h"

#define DHT20_ADDRESS       (0x38 << 1)
#define DHT20_MEASURE_TIME  80
#define DHT20_TIMEOUT_MS  200

typedef enum {
    DHT20_IDLE,
    DHT20_WAITING,
    DHT20_READING
} DHT20_State_t;

static DHT20_State_t state = DHT20_IDLE;

static uint8_t measurement_valid = 0;
static uint8_t data_buffer[6];

static uint32_t measurement_start_time = 0;

void dht20_init(void)
{
	dht20_temp = 0.0f;
	dht20_humi = 0.0f;
    measurement_valid = 0;

    measurement_start_time = 0;
    state = DHT20_IDLE;
}

static void dht20_start_measurement(void)
{
    uint8_t command[3] = {0xAC, 0x33, 0x00};

    if (HAL_I2C_Master_Transmit(&hi2c1,
                                DHT20_ADDRESS,
                                command,
                                3,
                                10) == HAL_OK)
    {
        measurement_start_time = HAL_GetTick();
        state = DHT20_WAITING;
    }
}

void dht20_read_task(void)
{
    switch (state)
    {
        case DHT20_IDLE:
            dht20_start_measurement();
            break;

        case DHT20_WAITING:
            if (HAL_GetTick() - measurement_start_time >= DHT20_MEASURE_TIME)
            {
                state = DHT20_READING;
            }
            break;

        case DHT20_READING:{
            if (HAL_I2C_Master_Receive(&hi2c1,
                                       DHT20_ADDRESS,
                                       data_buffer,
                                       6,
                                       10) == HAL_OK)
            {	uint8_t sensor_is_ready = (data_buffer[0] & 0x80) == 0;

            	uint8_t timeout_reached =
                (HAL_GetTick() - measurement_start_time) >= DHT20_TIMEOUT_MS;

            	if (sensor_is_ready) {
                	uint32_t raw_humidity =
                	    ((uint32_t)data_buffer[1] << 12) |
                	    ((uint32_t)data_buffer[2] << 4) |
                	    (data_buffer[3] >> 4);

                	uint32_t raw_temperature =
                	    (((uint32_t)data_buffer[3] & 0x0F) << 16) |
                	    ((uint32_t)data_buffer[4] << 8) |
                	    data_buffer[5];
                	dht20_humi = ((float)raw_humidity / 1048576.0f) * 100.0f;

                	dht20_temp = ((float)raw_temperature / 1048576.0f)
                	             * 200.0f - 50.0f;

                	state = DHT20_IDLE;
                }
            	else if (timeout_reached) {
            	    state = DHT20_IDLE;
            	}
            }
            else
            {
                state = DHT20_IDLE;
            }
            break;
        }
    }
}
