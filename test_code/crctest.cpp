#include <vector>
#include <iostream>
#include <iomanip>

#include "include/fdc_crc.h"

int main(void) {
    fdc_crc crcgen;
    size_t crcval;
    std::vector<uint8_t> supply_data;
    // 読み出し時
    // A1 A1 A1 [FE 01 01 03 01 DD EA] -> 00 00
    crcgen.reset();
    supply_data = std::vector<uint8_t>({0xfe, 0x01, 0x01, 0x03, 0x01, 0xdd, 0xea});
    crcgen.data(supply_data);
    crcval = crcgen.get();
    std::cout << std::hex << std::setw(4) << std::setfill('0') << crcval << std::endl;

    // 書き込み時
    // A1 A1 A1 [FE 01 01 03 01 [00 00]] -> DD EA
    crcgen.reset();
    supply_data = std::vector<uint8_t>({0xfe, 0x01, 0x01, 0x03, 0x01, 0x00, 0x00});
    crcgen.data(supply_data);
    crcval = crcgen.get();
    std::cout << std::hex << std::setw(4) << std::setfill('0') << crcval << std::endl;
}
