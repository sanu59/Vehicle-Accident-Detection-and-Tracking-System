#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

// UART module base addresses and configurations
#define UART0_MODULE UART0_BASE
#define UART1_MODULE UART1_BASE
#define UART5_MODULE UART5_BASE
#define UART0_GPIO_PORT GPIO_PORTA_BASE
#define UART1_GPIO_PORT GPIO_PORTB_BASE
#define UART5_GPIO_PORT GPIO_PORTE_BASE
#define UART0_GPIO_TX GPIO_PIN_1
#define UART0_GPIO_RX GPIO_PIN_0
#define UART1_GPIO_TX GPIO_PIN_1
#define UART1_GPIO_RX GPIO_PIN_0
#define UART5_GPIO_TX GPIO_PIN_5
#define UART5_GPIO_RX GPIO_PIN_4
#define BAUD_RATE_0 115200
#define BAUD_RATE_1 9600
#define BAUD_RATE_5 9600

#define PHONE_NUMBER "+91xxxxxxxxxx"  // Target phone number

// Function prototypes
void UART0Init(void);
void UART1Init(void);
void UART5Init(void);
void UARTPrint(const char *message, uint32_t uart_module);
void GSM_SendSMS(const char *phoneNumber, const char *message);
void delay(uint32_t seconds);
int Accident_detection(uint32_t port, uint8_t pin);
void GPS_ReceiveData(double *latitude, double *longitude);
void ConvertDoubleToString(double value, char *buffer);

volatile double latitude_decimal, longitude_decimal;

int main(void)
{
    // Enable system clock for PF4
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Configure the pin of port F as output for red LED
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);

    // Configure the pin of port F as input for vibration sensor
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    // Configure the pin to which vibration sensor is connected as pull-up
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Set system clock to 40 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    UART0Init(); // Initialization function for UART0 (communication with PC)
    UART1Init(); // Initialization function for UART1 (communication with GSM)
    UART5Init(); // Initialization function for UART5 (communication with GPS)

    double latitude, longitude;
    char lat_str[50], lon_str[50];

    while (1)
    {
        if (Accident_detection(GPIO_PORTF_BASE, GPIO_PIN_4) == 1)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x02); // Set the red LED HIGH if accident is detected

            UARTPrint("Sending SMS: Accident detected\r\n", UART0_MODULE); // Print in UART0 that accident is detected

            // Receive GPS data
            GPS_ReceiveData(&latitude, &longitude);

            // Convert latitude and longitude to strings
            ConvertDoubleToString(latitude, lat_str);
            ConvertDoubleToString(longitude, lon_str);

            // Create Google Maps link
            char google_maps_link[150];
            snprintf(google_maps_link, sizeof(google_maps_link), "Accident detected at: https://www.google.com/maps/@%s,%s,16z", lat_str, lon_str);

            // Send SMS with the location
            GSM_SendSMS(PHONE_NUMBER, google_maps_link);

            delay(5); // Delay approximately 5 seconds
        }
    }
    return 0;
}

void UART0Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(UART0_GPIO_PORT, UART0_GPIO_RX | UART0_GPIO_TX);

    UARTConfigSetExpClk(UART0_MODULE, SysCtlClockGet(), BAUD_RATE_0,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void UART1Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(UART1_GPIO_PORT, UART1_GPIO_RX | UART1_GPIO_TX);

    UARTConfigSetExpClk(UART1_MODULE, SysCtlClockGet(), BAUD_RATE_1,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void UART5Init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);

    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);
    GPIOPinTypeUART(UART5_GPIO_PORT, UART5_GPIO_RX | UART5_GPIO_TX);

    UARTConfigSetExpClk(UART5_MODULE, SysCtlClockGet(), BAUD_RATE_5,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void UARTPrint(const char *message, uint32_t uart_module)
{
    while (*message != '\0')
    {
        UARTCharPut(uart_module, *message);
        message++;
    }
}

void GSM_SendSMS(const char *phoneNumber, const char *message)
{
    // Send AT command to set SMS to text mode
    UARTPrint("AT+CMGF=1\r", UART1_MODULE);
    SysCtlDelay(SysCtlClockGet());  // Delay to allow command processing

    // Send AT command to set the recipient phone number
    UARTPrint("AT+CMGS=\"", UART1_MODULE);
    UARTPrint(phoneNumber, UART1_MODULE);
    UARTPrint("\"\r", UART1_MODULE);
    SysCtlDelay(SysCtlClockGet() / 10);  // Delay to allow command processing

    // Send the actual SMS message
    UARTPrint(message, UART1_MODULE);

    // End the message with Ctrl+Z (ASCII 26)
    UARTCharPut(UART1_MODULE, 26);
    SysCtlDelay(SysCtlClockGet() / 10);  // Delay to allow command processing

    // Debug message via UART
    UARTPrint("SMS sent successfully\r\n", UART0_MODULE);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0x00); // Turn off the LED
}

void delay(uint32_t seconds)
{
    // Delays for the specified number of seconds.
    SysCtlDelay(seconds * (SysCtlClockGet() / 3));
}

void ConvertDoubleToString(double value, char *buffer)
{
    // Decompose the floating point value
    char *sign = (value < 0) ? "-" : "";
    double abs_value = (value < 0) ? -value : value;

    int integer_part = (int)abs_value;
    double fractional_part = abs_value - integer_part;
    int fractional_as_int = trunc(fractional_part * 1000000);

    sprintf(buffer, "%s%d.%06d", sign, integer_part, fractional_as_int);
}

void GPS_ReceiveData(double *latitude, double *longitude)
{
    char buffer[128];
    int index = 0;
    int i;
    double latitude_decimal, longitude_decimal;

    // Read characters from the GPS UART until a newline or buffer limit is reached
    while (index < sizeof(buffer) - 1)
    {
        if (UARTCharsAvail(UART5_MODULE))
        {
            char c = UARTCharGet(UART5_MODULE);
            buffer[index++] = c;
            if (c == '\n')
            {
                // Null-terminate the string
                buffer[index] = '\0';

                // Check if the NMEA statement starts with "$GPRMC"
                if (strncmp(buffer, "$GPRMC", 6) == 0)
                {
                    char *token;
                    char latitude_str[50] = {0}, longitude_str[50] = {0};

                    // Tokenize the string using strtok
                    token = strtok(buffer, ",");
                    for (i = 0; i < 6; i++)
                    {
                        if (token == NULL)
                        {
                            // Error handling for unexpected format
                            return;
                        }
                        if (i == 3)
                        {
                            // Latitude
                            strncpy(latitude_str, token, sizeof(latitude_str) - 1);
                            latitude_str[sizeof(latitude_str) - 1] = '\0'; // Null-terminate the string
                        }
                        else if (i == 5)
                        {
                            // Longitude
                            strncpy(longitude_str, token, sizeof(longitude_str) - 1);
                            longitude_str[sizeof(longitude_str) - 1] = '\0'; // Null-terminate the string
                        }
                        token = strtok(NULL, ",");
                    }

                    // Convert latitude and longitude strings to double
                    double latitude_val = atof(latitude_str);
                    double longitude_val = atof(longitude_str);

                    // Convert latitude and longitude to decimal degrees
                    latitude_decimal = trunc(latitude_val / 100) + fmod(latitude_val, 100) / 60;
                    longitude_decimal = trunc(longitude_val / 100) + fmod(longitude_val, 100) / 60;

                    // Set output parameters
                    *latitude = latitude_decimal;
                    *longitude = longitude_decimal;
                    return;
                }
                index = 0; // Reset buffer index for the next sentence
            }
        }
    }
}

int Accident_detection(uint32_t port, uint8_t pin)
{
    int count_low = 0;
    int count_high = 0;
    int count_max = 15;

    do
    {
        delay(1);
        if (GPIOPinRead(port, pin) == 1)
        {
            count_low++;
            count_high = 0;
        }
        else
        {
            count_high++;
            count_low = 0;
        }
    } while (count_low < count_max && count_high < count_max);

    if (count_low >= count_max)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
