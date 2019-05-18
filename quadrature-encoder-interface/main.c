
#ifndef TICK
#define TICK            1000
#endif

#ifndef rvmdk
#define rvmdk
#endif

#ifndef PART_TM4C123GH6PM
#define PART_TM4C123GH6PM
#endif

#ifndef TARGET_IS_TM4C123_RB1
#define TARGET_IS_TM4C123_RB1
#endif

#include "TM4C123GH6PM.h"
//TIVAWARE
#include <stdint.h> 
#include <stdbool.h>
#include <stdio.h>
#include "driverlib/sysctl.h"
#include "driverlib/fpu.h" 
#include "driverlib/sysctl.h" //Prototypes for the system control driver
#include "driverlib/rom.h" //Macros to facilitate calling functions in the ROM
#include "driverlib/pin_map.h" //Mapping of peripherals to pins for all parts
#include "driverlib/uart.h" //Defines and Macros for the UART
#include "utils/uartstdio.h" //Prototypes for the UART console fun
#include "driverlib/gpio.h" //Defines and Macros for GPIO API
#include "driverlib/qei.h"

static uint32_t systick_ms_count = 1;

uint16_t UART_Write_String(uint32_t uart, uint8_t* txt)
{
    uint16_t length = 0;

    while('\0' != *txt)
    {
        UARTCharPut(uart, *(txt++));
        length++;
    }

    return length;
}

void Delay_ms(uint32_t time)
{
    uint32_t start_time = systick_ms_count;

    while (time > (systick_ms_count - start_time))
    {
        __NOP();
    }
}

uint8_t Hardware_Init(void)
{
    /*  Habilita PLL e configura clock para 80MHz */
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | \
        SYSCTL_XTAL_16MHZ);
    
    /*  Configura UART0 para debug. UART0 esta nos pinos PA0(RX) e PA1(TX) */
    /*  Habilita clock do GPIOA */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);    
    /*  Habilita clock da UART0 */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);    
    /*  Configura as funcoes alternativas para os pinos */
    GPIOPinConfigure(GPIO_PA0_U0RX);    
    GPIOPinConfigure(GPIO_PA1_U0TX);    
    /*  Configura os pinos para usarem a UART0 */
    GPIOPinTypeUART(GPIOA_BASE, (GPIO_PIN_0 | GPIO_PIN_1)); 
    /*  Desabilita a UART0 antes de realizar a configuracao */
    UARTDisable(UART0_BASE);
    /*  Configuracao da UART0 com baudrate de 115200 */
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, \
        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
    /*  Envia mensagem de teste */
    UART_Write_String(UART0_BASE, "Quadrature Encoder Interface \n");

    /*  Configura Quadrature Encoder Interface */
    /*  SERAO UTILIZADOS OS PINOS PD7 E PD6 */
    /*  Possiveis pinagens:
        IDX0 - PF4 | PD3 - Nivel TTL - QEI module 0 index
        PhA0 - PF0 | PD6 - Nivel TTL - QEI module 0 phase A
        PhB0 - PD7 | PF1 - Nivel TTL - QEI module 0 phase B
        IDX1 -    PC4    - Nivel TTL - QEI module 1 index
        PhA1 -    PC5    - Nivel TTL - QEI module 1 phase A
        PhB1 -    PC6    - Nivel TTL - QEI module 1 phase B
    */
    /*  Habilita o clock da GPIOD */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    /*  Habilita clock do QEI0 */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);
    /*  Configura as funcoes alternativas nos pinos */
    GPIOPinConfigure(GPIO_PD6_PHA0);
    GPIOPinConfigure(GPIO_PD7_PHB0);
    /*  Habilita o QEI0 nos pinos */
    GPIOPinTypeQEI(GPIOD_BASE, (GPIO_PIN_6 | GPIO_PIN_7));
    /*  Desabilita QEI0 antes de configurar */
    QEIDisable(QEI0_BASE);
    /* Configura o QEI0 */
    QEIFilterConfigure(QEI0_BASE, QEI_FILTCNT_5);
    //QEIFilterEnable(QEI_FILTCNT_17);
    QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A  | QEI_CONFIG_NO_RESET 	| QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 100);
    QEIEnable(QEI0_BASE);
    QEIPositionSet(QEI0_BASE, 50);
    return 0;
}

uint32_t last_read = 0;
uint32_t new_read = 0;
int main()
{
    Hardware_Init();
    
    SysTick_Config(SysCtlClockGet() / TICK);
    new_read = QEIPositionGet(QEI0_BASE);

	while(1)
	{
        
        new_read = QEIPositionGet(QEI0_BASE);
        if (last_read != new_read)
        {
            char qeiPosition[10];
            sprintf(qeiPosition, "%d", new_read);
            UART_Write_String(UART0_BASE, "QEI0: ");
            UART_Write_String(UART0_BASE, (uint8_t*)qeiPosition);
            UART_Write_String(UART0_BASE, "\r\n");
            
            last_read = new_read;
					
            Delay_ms(500);					
        }
	}
}

void SysTick_Handler(void)
{
    systick_ms_count++;
}
