#ifndef _MS_VM_H_
#define _MS_VM_H_

#include <stdint.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "mstypes.h"

namespace smi::ms {

class VM {
   private:
    unsigned int pc;
    uint8_t zf;
    uint16_t ir;
    uint16_t memory[DATASIZE];

    std::unordered_map<std::string, uint16_t> labels;
    std::vector<std::string> labelKeys;

    uint8_t read(unsigned char* p, unsigned int& pos);
    uint16_t read16(unsigned char* p, unsigned int& pos);

   public:
    void loadMS(unsigned char* ms);
    void loadMemory(unsigned char* memory);
    int executeNext();
    unsigned int getPC() const { return this->pc; }
    uint16_t getIR() const { return this->ir; }
    uint8_t getZF() const { return this->zf; }

    const uint16_t* getMemory() const { return this->memory; }
    uint16_t getMemoryValue(uint8_t pos) const { return this->memory[pos]; }
    const std::vector<std::string> getLabels() const { return this->labelKeys; }
    uint16_t getLabel(const std::string& label) {
        if (this->labels.find(label) == this->labels.end()) return 0;

        return this->labels[label];
    }
};

}  // namespace smi::ms

#endif
