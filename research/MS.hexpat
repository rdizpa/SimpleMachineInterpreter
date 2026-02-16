import std.mem;
import std.math;
import std.io;

enum LineType : u8 {
    NONE = 0,
    INST = 1,
    DATA = 2
};

enum OpCode : u8 {
    ADD = 0,
    CMP = 1,
    MOV = 2,
    BEQ = 3
};

enum Side : u8 {
    LEFT  = 0,
    RIGHT = 1
};

bitfield Instruction {
    op2 : 7;
    op1 : 7;
    opcode : 2;
};

union Line {
    Instruction inst;
    u16 data;
};

u128 hasLabel = 0;
u128 hasUndefinedLabelOp1 = 0;
u128 hasUndefinedLabelOp2 = 0;

fn setHasLabelTrue(u8 index) {
    hasLabel = hasLabel | (1 << index);
};

fn getHasLabel(u8 index) {
    return (hasLabel & (1 << index)) != 0;
};

fn setHasUndefinedLabelTrue(u8 index, Side op) {
    if (op == Side::LEFT) {
        hasUndefinedLabelOp1 = hasUndefinedLabelOp1 | (1 << index);
    } else {
        hasUndefinedLabelOp2 = hasUndefinedLabelOp2 | (1 << index);
    }
};

fn getHasUndefinedLabel(u8 index, Side op) {
    if (op == Side::LEFT) {
        return (hasUndefinedLabelOp1 & (1 << index)) != 0;
    }
    
    return (hasUndefinedLabelOp2 & (1 << index)) != 0;
};

fn getLabel(u8 index) {
    for (u8 i = 0, i < prog.labelCount.count, i += 1) {
        if (prog.labelPositions[i].line == index) {
            return prog.labels[i].name;
        }
    }
    
    return "";
};

fn getUndefinedLabel(u8 index, Side op) {
    for (u8 i = 0, i < prog.undefinedLabelCount.count, i += 1) {
        if (
            prog.undefinedLabelPositions[i].line == index &&
            prog.undefinedLabelSides[i].side == op
            ) {
            return prog.undefinedLabels[i].name;
        }
    }
    
    return "";
};

fn opcodeToText(u8 op) {
    match (op) {
        (OpCode::ADD): return "ADD";
        (OpCode::CMP): return "CMP";
        (OpCode::MOV): return "MOV";
        (OpCode::BEQ): return "BEQ";
        (_): return "???";
    }
};

fn lineToString(u8 index) {
    str label = "";
        
    if (getHasLabel(index)) {
        label = std::format("{}: ", getLabel(index));
    }
        
    if (prog.lineTypes[index] == LineType::INST) {
        Instruction inst = prog.memoryDump[index].inst;
        str opText = opcodeToText(inst.opcode);
        
        str op1 = "";
        str op2 = "";
        
        if (getHasUndefinedLabel(index, Side::LEFT)) {
            op1 = std::format("{}", getUndefinedLabel(index, Side::LEFT));
        } else if (getHasLabel(inst.op1)) {
            op1 = std::format("{}", getLabel(inst.op1));
        } else {
            op1 = std::format("{}", inst.op1);
        }
        
        if (getHasUndefinedLabel(index, Side::RIGHT)) {
            op2 = std::format("{}", getUndefinedLabel(index, Side::RIGHT));
        } else if (getHasLabel(inst.op2)) {
            op2 = std::format("{}", getLabel(inst.op2));
        } else {
            op2 = std::format("{}", inst.op2);
        }
        
        if (inst.opcode == OpCode::BEQ) {
            return std::format("{}{} {}", label, opText, op2);
        } else {
            return std::format("{}{} {}, {}", label, opText, op1, op2);
        }
    } else if (prog.lineTypes[index] == LineType::DATA) {
        return std::format("{}0x{:04X}", label, prog.memoryDump[index].data);
    } else {
        return label;
    }

    return "";
};

struct Label {
    char name[7];
};

struct LabelCount {
    u8 count;
    padding[1];
};

struct LabelPosition {
    u8 line;
    padding[1];
};

struct LabelSide {
    Side side;
    padding[1];
};

struct MS {
    LabelCount labelCount;
    LabelCount undefinedLabelCount;
    // u8 memoryDump[0x100];
    Line memoryDump[0x80];
    LineType lineTypes[0x80];
    Label labels[labelCount.count];
    LabelPosition labelPositions[labelCount.count];
    Label undefinedLabels[undefinedLabelCount.count];
    LabelPosition undefinedLabelPositions[undefinedLabelCount.count];
    LabelSide undefinedLabelSides[undefinedLabelCount.count];
};

MS prog @ 0x00;

for (u8 j = 0, j < prog.labelCount.count, j += 1) {
    setHasLabelTrue(prog.labelPositions[j].line);
}

for (u8 l = 0, l < prog.undefinedLabelCount.count, l += 1) {
    setHasUndefinedLabelTrue(prog.undefinedLabelPositions[l].line, prog.undefinedLabelSides[l].side);
}

for (u8 i = 0, i < 0x80, i += 1) {
    std::print(lineToString(i));
}
