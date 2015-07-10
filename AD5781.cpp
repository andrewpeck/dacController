#include <SPI.h>
#include "AD5781.h"
#include "constants.h"

/*
 * DAC Write Functions for AD5781
 *
 * Datasheet:
 *
 *
 * 24 bit data format:
 * | RW! | A2 | A1 | A0 | D19 ...0 |
 *
 * Command Bits:
 * C2 | C1 | C0 | Command
 * ---+----+----+--------------------------------------------------------
 *  0 |  0 |  0 | Write to Input Register n
 *  0 |  0 |  1 | Update DAC Register n
 *  0 |  1 |  0 | Write to Input Register n , update all (software LDAC)
 *  0 |  1 |  1 | Write to and update DAC Channel n
 *  1 |  0 |  0 | Power down DAC (power up)
 *  1 |  0 |  1 | Reset
 *  1 |  1 |  0 | LD AC register setup
 *  1 |  1 |  1 | Internal reference setup (on/off )
 *
 * Address bits:
 * A2 | A1 | A0 | ADDRESS (n)
 * ---+----+----+------------
 * 0  | 0  | 0  | No operation (NOP). Used in readback operations.
 * 0  | 0  | 1  | DAC Register
 * 0  | 1  | 0  | Control Register
 * 0  | 1  | 1  | Clear Code Register
 */

namespace AD5781 {
    void init()
    {
        for (int i=0; i<20; i++)
            write(0x2, 0x312, i); // Initialize RBUF+BIN+LIN_COMP

        //AD5781_write(0x2, 0x2); // Initialize RBUF
        //AD5781_write(0x2, 0x12); // Initialize RBUF+BIN
    }

    uint32_t writeReadSpiDword (uint32_t data, int idac)
    {
        digitalWrite(SYNC[idac], LOW);
        uint8_t rByte1 = SPI.transfer((data >> 16) & 0xFF);
        uint8_t rByte2 = SPI.transfer((data >>  8) & 0xFF);
        uint8_t rByte3 = SPI.transfer((data >>  0) & 0xFF);
        delayMicroseconds(10);
        digitalWrite(SYNC[idac], HIGH);

        uint32_t out = 0;
        out |= rByte1 << 16;
        out |= rByte2 << 8;
        out |= rByte3;
        return (out);
    }

    uint32_t read(unsigned int addr, int idac)
    {
        // Construct 24-bit data packet for read
        uint32_t wData = construct_packet(0x1, addr, 0x0);

        // Write Register to be Read
        writeReadSpiDword (wData, idac);
        delayMicroseconds(5);

        // Register Contents Clocked Out
        uint32_t out = writeReadSpiDword(0, idac);
        return (out);
    }

    void write(unsigned int addr, uint32_t data, int idac)
    {
        // Construct 24-bit data packet for write
        uint32_t wData = construct_packet(0x0, addr, data);

        // Decompose packet into 3 bytes and write.
        writeReadSpiDword(wData, idac);
    }

    int setDAC(uint32_t dac_counts, int idac)
    {
        if ((dac_counts < 0) | (dac_counts > 262143)) {
            return (-1);
        }

        // Construct 20-bit data format from 18-bit
        uint32_t wData = (0x3FFFF & dac_counts) << 2;

        // Write to DAC Register
        write(0x1, wData, idac);
        return (1);
    }

    uint32_t construct_packet(unsigned int rw, unsigned int addr, uint32_t data)
    {
        uint32_t packet = 0;
        packet |= (0x1     & rw   ) << 23; //Read or Write! 1-bit
        packet |= (0x7     & addr ) << 20; //Register Address 3-bit
        packet |= (0xFFFFF & data ) <<  0; //Data 20-bit
        return (packet);
    }
}
