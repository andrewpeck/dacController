#ifndef AD5781_H
#define AD5781_H

/* DAC */
namespace AD5781 {
    void init();
    uint32_t read(unsigned int addr, int idac);
    void write(unsigned int addr, uint32_t data, int idac);
    int setDAC(uint32_t dac_counts, int idac);
    uint32_t construct_packet (unsigned int rw, unsigned int addr, uint32_t data);
    uint32_t writeReadSpiDword (uint32_t data, int idac);
}

#endif
