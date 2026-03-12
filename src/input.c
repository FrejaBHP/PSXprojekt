#include "input.h"
#include <stdint.h>
#include <libpad.h>

// For debug printing purposes
#include <libgpu.h>

#define _ADDR8(addr)  ((volatile uint8_t *) (addr))
#define _ADDR16(addr) ((volatile uint16_t *) (addr))
#define _ADDR32(addr) ((volatile uint32_t *) (addr))
#define _MMIO8(addr)  (*_ADDR8(addr))
#define _MMIO16(addr) (*_ADDR16(addr))
#define _MMIO32(addr) (*_ADDR32(addr))

u_long IO_BASE = 0xbf801000;

#define SIO_DATA(N) _MMIO8 ((IO_BASE | 0x040) + (16 * (N)))
#define SIO_STAT(N) _MMIO16((IO_BASE | 0x044) + (16 * (N)))
#define SIO_MODE(N) _MMIO16((IO_BASE | 0x048) + (16 * (N)))
#define SIO_CTRL(N) _MMIO16((IO_BASE | 0x04a) + (16 * (N)))
#define SIO_BAUD(N) _MMIO16((IO_BASE | 0x04e) + (16 * (N)))

#define DTR_DELAY    60
#define DSR_TIMEOUT 120


/* Serial interfaces */

typedef enum {
	SIO_STAT_TX_NOT_FULL   = 1 << 0,
	SIO_STAT_RX_NOT_EMPTY  = 1 << 1,
	SIO_STAT_TX_EMPTY      = 1 << 2,
	SIO_STAT_RX_PARITY_ERR = 1 << 3,
	SIO_STAT_RX_OVERRUN    = 1 << 4, // SIO1 only
	SIO_STAT_RX_STOP_ERR   = 1 << 5, // SIO1 only
	SIO_STAT_RX_INVERT     = 1 << 6, // SIO1 only
	SIO_STAT_DSR           = 1 << 7, // DSR is /ACK on SIO0
	SIO_STAT_CTS           = 1 << 8, // SIO1 only
	SIO_STAT_IRQ           = 1 << 9
} SIOStatusFlag;

typedef enum {
	SIO_MODE_BAUD_BITMASK   = 3 << 0,
	SIO_MODE_BAUD_DIV1      = 1 << 0,
	SIO_MODE_BAUD_DIV16     = 2 << 0,
	SIO_MODE_BAUD_DIV64     = 3 << 0,
	SIO_MODE_DATA_BITMASK   = 3 << 2,
	SIO_MODE_DATA_5         = 0 << 2,
	SIO_MODE_DATA_6         = 1 << 2,
	SIO_MODE_DATA_7         = 2 << 2,
	SIO_MODE_DATA_8         = 3 << 2,
	SIO_MODE_PARITY_BITMASK = 3 << 4,
	SIO_MODE_PARITY_NONE    = 0 << 4,
	SIO_MODE_PARITY_EVEN    = 1 << 4,
	SIO_MODE_PARITY_ODD     = 3 << 4,
	SIO_MODE_STOP_BITMASK   = 3 << 6, // SIO1 only
	SIO_MODE_STOP_1         = 1 << 6, // SIO1 only
	SIO_MODE_STOP_1_5       = 2 << 6, // SIO1 only
	SIO_MODE_STOP_2         = 3 << 6, // SIO1 only
	SIO_MODE_SCK_INVERT     = 1 << 8  // SIO0 only
} SIOModeFlag;

typedef enum {
	SIO_CTRL_TX_ENABLE      = 1 <<  0,
	SIO_CTRL_DTR            = 1 <<  1, // DTR is /CS on SIO0
	SIO_CTRL_RX_ENABLE      = 1 <<  2,
	SIO_CTRL_TX_INVERT      = 1 <<  3, // SIO1 only
	SIO_CTRL_ACKNOWLEDGE    = 1 <<  4,
	SIO_CTRL_RTS            = 1 <<  5, // SIO1 only
	SIO_CTRL_RESET          = 1 <<  6,
	SIO_CTRL_TX_IRQ_ENABLE  = 1 << 10,
	SIO_CTRL_RX_IRQ_ENABLE  = 1 << 11,
	SIO_CTRL_DSR_IRQ_ENABLE = 1 << 12, // DSR is /ACK on SIO0
	SIO_CTRL_CS_PORT_1      = 0 << 13, // SIO0 only
	SIO_CTRL_CS_PORT_2      = 1 << 13  // SIO0 only
} SIOControlFlag;


// As the controller bus is shared with memory cards, an addressing mechanism is
// used to ensure packets are processed by a single device at a time. The first
// byte of each request packet is thus the "address" of the peripheral that
// shall respond to it.
typedef enum {
	ADDR_CONTROLLER  = 0x01,
	ADDR_MEMORY_CARD = 0x81
} DeviceAddress;

// The address is followed by a command byte and any required parameters. The
// only command used in this example (and supported by all controllers) is
// CMD_POLL, however some controllers additionally support a "configuration
// mode" which grants access to an extended command set.
typedef enum {
	CMD_INIT_PRESSURE   = '@', // Initialize DualShock pressure sensors (config)
	CMD_POLL            = 'B', // Read controller state
	CMD_CONFIG_MODE     = 'C', // Enter or exit configuration mode
	CMD_SET_ANALOG      = 'D', // Set analog mode/LED state (config)
	CMD_GET_ANALOG      = 'E', // Get analog mode/LED state (config)
	CMD_GET_MOTOR_INFO  = 'F', // Get information about a motor (config)
	CMD_GET_MOTOR_LIST  = 'G', // Get list of all motors (config)
	CMD_GET_MOTOR_STATE = 'H', // Get current state of vibration motors (config)
	CMD_GET_MODE        = 'L', // Get list of all supported modes (config)
	CMD_REQUEST_CONFIG  = 'M', // Configure poll request format (config)
	CMD_RESPONSE_CONFIG = 'O', // Configure poll response format (config)
	CMD_CARD_READ       = 'R', // Read 128-byte memory card sector
	CMD_CARD_GET_SIZE   = 'S', // Retrieve memory card size information
	CMD_CARD_WRITE      = 'W'  // Write 128-byte memory card sector
} DeviceCommand;


typedef enum {
	IRQ_VSYNC  =  0,
	IRQ_GPU    =  1,
	IRQ_CDROM  =  2,
	IRQ_DMA    =  3,
	IRQ_TIMER0 =  4,
	IRQ_TIMER1 =  5,
	IRQ_TIMER2 =  6,
	IRQ_SIO0   =  7,
	IRQ_SIO1   =  8,
	IRQ_SPU    =  9,
	IRQ_GUN    = 10,
	IRQ_PIO    = 10
} IRQChannel;

#define IRQ_STAT _MMIO16(IO_BASE | 0x070)
#define IRQ_MASK _MMIO16(IO_BASE | 0x074)

#define F_CPU      33868800
#define F_GPU_NTSC 53693175
#define F_GPU_PAL  53203425

void initSerialIO(int baud) {
	SIO_CTRL(1) = SIO_CTRL_RESET;

	SIO_MODE(1) = 0
		| SIO_MODE_BAUD_DIV1
		| SIO_MODE_DATA_8
		| SIO_MODE_STOP_1;
	SIO_BAUD(1) = F_CPU / baud;
	SIO_CTRL(1) = 0
		| SIO_CTRL_TX_ENABLE
		| SIO_CTRL_RX_ENABLE
		| SIO_CTRL_RTS;
}

static void initControllerBus(void) {
	// Reset the serial interface, initialize it with the settings used by
	// controllers and memory cards (250000bps, 8 data bits) and configure it to
	// send a signal to the interrupt controller whenever the DSR input is
	// pulsed (see below).
	SIO_CTRL(0) = SIO_CTRL_RESET;

	SIO_MODE(0) = 0
		| SIO_MODE_BAUD_DIV1
		| SIO_MODE_DATA_8;
	SIO_BAUD(0) = F_CPU / 250000;
	SIO_CTRL(0) = 0
		| SIO_CTRL_TX_ENABLE
		| SIO_CTRL_RX_ENABLE
		| SIO_CTRL_DSR_IRQ_ENABLE;
}

static void delayMicroseconds(int time) {
	// Calculate the approximate number of CPU cycles that need to be burned,
	// assuming a 33.8688 MHz clock (1 us = 33.8688 = ~33.875 = 271 / 8 cycles).
	// The loop consists of a branch and a decrement, thus each iteration will
	// burn 2 cycles.
	time = ((time * 271) + 4) / 8;

	__asm__ volatile(
		// The .set noreorder directive will prevent the assembler from trying
		// to "hide" the branch instruction's delay slot by shuffling nearby
		// instructions. .set push and .set pop are used to save and restore the
		// assembler's settings respectively, ensuring the noreorder flag will
		// not affect any other code.
		".set push\n"
		".set noreorder\n"
		"bgtz  %0, .\n"
		"addiu %0, -2\n"
		".set pop\n"
		: "+r"(time)
	);
}

static bool waitForAcknowledge(int timeout) {
	// Controllers and memory cards will acknowledge bytes received by sending
	// short pulses over the DSR line, which will be forwarded by the serial
	// interface to the interrupt controller. This is not guaranteed to happen
	// (it will not if e.g. no device is connected), so we have to implement a
	// timeout to avoid waiting forever in such cases.
	for (; timeout > 0; timeout -= 10) {
		if (IRQ_STAT & (1 << IRQ_SIO0)) {
			// Reset the interrupt controller and serial interface's flags to
			// ensure the interrupt can be triggered again.
			IRQ_STAT     = ~(1 << IRQ_SIO0);
			SIO_CTRL(0) |= SIO_CTRL_ACKNOWLEDGE;

			return true;
		}

		delayMicroseconds(10);
	}

	return false;
}

static void selectPort(int port) {
	// Set or clear the bit that controls which set of controller and memory
	// card ports is going to have its DTR (port select) signal asserted. The
	// actual serial bus is shared between all ports, however devices will not
	// process packets if DTR is not asserted on the port they are plugged into.
	if (port)
		SIO_CTRL(0) |= SIO_CTRL_CS_PORT_2;
	else
		SIO_CTRL(0) &= ~SIO_CTRL_CS_PORT_2;
}

static uint8_t exchangeByte(uint8_t value) {
	// Wait until the interface is ready to accept a byte to send, then wait for
	// it to finish receiving the byte sent by the device.
	while (!(SIO_STAT(0) & SIO_STAT_TX_NOT_FULL))
		__asm__ volatile("");

	SIO_DATA(0) = value;

	while (!(SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY))
		__asm__ volatile("");

	return SIO_DATA(0);
}

static int exchangePacket(
	DeviceAddress address,
	const uint8_t *request,
	uint8_t       *response,
	int           reqLength,
	int           maxRespLength
) {
	// Reset the interrupt flag and assert the DTR signal to tell the controller
	// or memory card that we're about to send a packet. Devices may take some
	// time to prepare for incoming bytes so we need a small delay here.
	IRQ_STAT     = ~(1 << IRQ_SIO0);
	SIO_CTRL(0) |= SIO_CTRL_DTR | SIO_CTRL_ACKNOWLEDGE;
	delayMicroseconds(DTR_DELAY);

	int respLength = 0;

	// Send the address byte and wait for the device to respond with a pulse on
	// the DSR line. If no response is received assume no device is connected,
	// otherwise make sure the serial interface's data buffer is empty to
	// prepare for the actual packet transfer.
	SIO_DATA(0) = address;

	if (waitForAcknowledge(DSR_TIMEOUT)) {
		while (SIO_STAT(0) & SIO_STAT_RX_NOT_EMPTY)
			SIO_DATA(0);

		// Send and receive the packet simultaneously one byte at a time,
		// padding it with zeroes if the packet we are receiving is longer than
		// the data being sent.
		while (respLength < maxRespLength) {
			if (reqLength > 0) {
				*(response++) = exchangeByte(*(request++));
				reqLength--;
			} else {
				*(response++) = exchangeByte(0);
			}

			respLength++;

			// The device will keep sending DSR pulses as long as there is more
			// data to transfer. If no more pulses are received, terminate the
			// transfer.
			if (!waitForAcknowledge(DSR_TIMEOUT))
				break;
		}
	}

	// Release DSR, allowing the device to go idle.
	delayMicroseconds(DTR_DELAY);
	SIO_CTRL(0) &= ~SIO_CTRL_DTR;

	return respLength;
}




char padDataBuffer[2][DATABUFFER_SIZE];
GamePad pads[2] = { 0 };

bool isSelectHeld = false;
bool isStartHeld = false;
bool isCrossHeld = false;
bool isSquareHeld = false;
bool isTriangleHeld = false;
bool isCircleHeld = false;
bool isL2Held = false;
bool isR2Held = false;

void HandleAnalogue(GamePad* pad);

void TestProdSIO() {
    u_char request[4];
    u_char response[8];

    request[0] = CMD_POLL;
    request[1] = 0x00;
    request[2] = 0x00;
    request[3] = 0x00;

    selectPort(0x00);
    int responseLength = exchangePacket(
        ADDR_CONTROLLER,
        request,
        response,
        sizeof(request),
        sizeof(response)
    );

    if (responseLength < 4) {
        pads[0].status = 1;
        return;
    }

    pads[0].status = 0;
    pads[0].type = response[0];
    pads[0].buttons = 0xFFFF - ((response[2] << 8) | (response[3]));
    pads[0].leftstick.x = response[6];
    pads[0].leftstick.y = response[7];
    pads[0].rightstick.x = response[4];
    pads[0].rightstick.y = response[5];
}

void evil() {
    int responseLength;
    u_char request[4];
    u_char response[8];

    // Enter config mode
    request[0] = CMD_CONFIG_MODE;
    request[1] = 0x00;
    request[2] = 0x01;
    request[3] = 0x00;

    selectPort(0x00);
    responseLength = exchangePacket(
        ADDR_CONTROLLER,
        request,
        response,
        sizeof(request),
        sizeof(response)
    );

    if (responseLength < 4) {
        return;
    }

    // Set controller to analogue mode
    request[0] = CMD_SET_ANALOG;
    request[1] = 0x00;
    request[2] = 0x01;
    request[3] = 0x03;

    selectPort(0x00);
    responseLength = exchangePacket(
        ADDR_CONTROLLER,
        request,
        response,
        sizeof(request),
        sizeof(response)
    );

    // Exit config mode
    request[0] = CMD_CONFIG_MODE;
    request[1] = 0x00;
    request[2] = 0x00;
    request[3] = 0x00;

    selectPort(0x00);
    responseLength = exchangePacket(
        ADDR_CONTROLLER,
        request,
        response,
        sizeof(request),
        sizeof(response)
    );
}

void InitControllers() {
    initSerialIO(115200);
    initControllerBus();
    evil();

    //PadInitDirect((u_char*)padDataBuffer[0], (u_char*)padDataBuffer[1]);
    pads[0].port = 0x00;
    pads[1].port = 0x10;

    for (size_t i = 0; i < 2; i++) {
        pads[i].delay = 6;
    }
    
    //PadStartCom();
}

void StopControllers() {
    PadStopCom();
}

// Splits the dataBuffer into the other members for readability and ease of use
void UpdatePad(int pad) {
    TestProdSIO();

    /*
    FntPrint("Allow Vib: %d\n", pads[0].allowVibration);
    FntPrint("Motors: %d, %d\n", pads[0].motors[0], pads[0].motors[1]);
    FntPrint("Vib Time: %d\n", pads[0].vibrationTime);

    FntPrint("Type: %d\n", pads[0].type);
    FntPrint("State: %d\n", pads[0].state);
    FntPrint("Delay: %d\n", pads[0].delay);
    FntPrint("Analogue Pad: %d\n", pads[0].isAnaloguePad);
    FntPrint("Analogue Active: %d\n", pads[0].isAnalogueActive);
    */

    //HandleAnalogue(&pads[pad]);
}

void HandleAnalogue(GamePad* pad) {
    // Modelled after Driver 2 in an attempt to nudge the console to recognise the controller as analogue by sending rumble commands
    // Unfortunately, it doesn't even allow me to do that, so this entire everything is useless and is only really kept as a future reminder
    // Just in case I want to make the controller vibrate and need some degree of reference to how it can be set up
    
    static u_char align[6] = { 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF };
    static u_char zeroMotors[2] = { 0, 0 };
    int newState;

    newState = PadGetState(pad->port);

    switch (newState) {
        case PadStateDiscon:
        case PadStateReqInfo:
            break;

        case PadStateFindPad:
            pad->state = PadStateFindPad;

            if (!pad->isAnalogueActive) {
                pad->isAnaloguePad = false;
                pad->isAnalogueActive = false;
                pad->state = 0;
                pad->motors[0] = 0;
                pad->motors[1] = 0;
                pad->vibrationTime = 0;
            }
            break;

        case PadStateFindCTP1:
            if (pad->state != PadStateStable) {
                pad->state = PadStateStable;
            }
            break;

        case PadStateStable:
            if (pad->state != PadStateStable) {
                if (PadInfoMode(pad->port, InfoModeIdTable, 1) != 7) {
                    pad->isAnaloguePad = 0;
                    pad->state = PadStateStable;
                    pad->motors[0] = 0;
                    pad->motors[1] = 0;
                    pad->vibrationTime = 0;
                    break;
                }

                pad->isAnaloguePad = 1;

                if (pad->delay == 6) {
                    if (PadSetActAlign(pad->port, align) != 0) {
                        pad->isAnalogueActive = true;
                        pad->state = PadStateStable;
                        pad->motors[0] = 0;
                        pad->motors[1] = 0;
                        pad->vibrationTime = 0;

                        PadSetAct(pad->port, zeroMotors, 2);
                        break;
                    }

                    pad->delay = 0;
                }
                else {
                    pad->delay++;
                }
            }
            break;
    }
    
    if (pad->state == PadStateStable) {
        if (!pad->isAnalogueActive) {
            pad->motors[0] = 0;
            pad->motors[1] = 0;
        }
        else {
            if (pad->vibrationTime > 0) {
                if (!pad->allowVibration) {
                    PadSetAct(pad->port, zeroMotors, 2);
                }
                else {
                    PadSetAct(pad->port, pad->motors, 2);
                    // DRIVER2 garble
                }
            }
        }

        if (pad->vibrationTime > 0) {
            pad->vibrationTime--;
        }
        else {
            pad->motors[0] = 0;
            pad->motors[1] = 0;
        }

        FntPrint("Act1: %d, %d, %d\n", PadInfoAct(pad->port, 0, InfoActFunc), PadInfoAct(pad->port, 0, InfoActSub), PadInfoAct(pad->port, 0, InfoActSize));
        FntPrint("Act2: %d, %d, %d\n", PadInfoAct(pad->port, 1, InfoActFunc), PadInfoAct(pad->port, 1, InfoActSub), PadInfoAct(pad->port, 1, InfoActSize));
    }
}

void ClearPad(int pad) {

}
