#include "stdafx.h"
#include "fuku_code_profiler.h"


#define GET_BITES_INCLUDED(src, include_mask, exclude_mask) (( (src) & (include_mask) ) & (~(exclude_mask)))

#define CF_EXCLUDE (X86_EFLAGS_MODIFY_CF | X86_EFLAGS_UNDEFINED_CF | X86_EFLAGS_RESET_CF | X86_EFLAGS_SET_CF)
#define DF_EXCLUDE (X86_EFLAGS_MODIFY_DF | X86_EFLAGS_RESET_DF     | X86_EFLAGS_SET_DF)
#define OF_EXCLUDE (X86_EFLAGS_MODIFY_OF | X86_EFLAGS_UNDEFINED_OF | X86_EFLAGS_RESET_OF | X86_EFLAGS_SET_OF)
#define SF_EXCLUDE (X86_EFLAGS_MODIFY_SF | X86_EFLAGS_UNDEFINED_SF | X86_EFLAGS_RESET_SF | X86_EFLAGS_SET_SF)
#define ZF_EXCLUDE (X86_EFLAGS_MODIFY_ZF | X86_EFLAGS_UNDEFINED_ZF | X86_EFLAGS_RESET_ZF | X86_EFLAGS_SET_ZF)
#define AF_EXCLUDE (X86_EFLAGS_MODIFY_AF | X86_EFLAGS_UNDEFINED_AF | X86_EFLAGS_RESET_AF | X86_EFLAGS_SET_AF)
#define PF_EXCLUDE (X86_EFLAGS_MODIFY_PF | X86_EFLAGS_UNDEFINED_PF | X86_EFLAGS_RESET_PF | X86_EFLAGS_SET_PF)


uint64_t TESTED_FLAGS_TABLE[] = {
    X86_EFLAGS_TEST_OF,
    X86_EFLAGS_TEST_SF,
    X86_EFLAGS_TEST_ZF,
    X86_EFLAGS_TEST_PF,
    X86_EFLAGS_TEST_CF,
    X86_EFLAGS_TEST_DF,
    X86_EFLAGS_TEST_AF
};

uint64_t EXCLUDED_FLAGS_TABLE[] = {
    OF_EXCLUDE,
    SF_EXCLUDE,
    ZF_EXCLUDE,
    PF_EXCLUDE,
    CF_EXCLUDE,
    DF_EXCLUDE,
    AF_EXCLUDE
};



x86_reg CONVERT_REGISTER_TO_CAP[] = {
    X86_REG_AL,
    X86_REG_CL,
    X86_REG_DL,
    X86_REG_BL,
    X86_REG_SPL,
    X86_REG_BPL,
    X86_REG_SIL,
    X86_REG_DIL,
    X86_REG_R8B ,
    X86_REG_R9B ,
    X86_REG_R10B,
    X86_REG_R11B,
    X86_REG_R12B,
    X86_REG_R13B,
    X86_REG_R14B,
    X86_REG_R15B,
    //word
    X86_REG_AX,
    X86_REG_CX,
    X86_REG_DX,
    X86_REG_BX,
    X86_REG_SP,
    X86_REG_BP,
    X86_REG_SI,
    X86_REG_DI,
    X86_REG_R8W ,
    X86_REG_R9W ,
    X86_REG_R10W,
    X86_REG_R11W,
    X86_REG_R12W,
    X86_REG_R13W,
    X86_REG_R14W,
    X86_REG_R15W,
    //dword
    X86_REG_EAX,
    X86_REG_ECX,
    X86_REG_EDX,
    X86_REG_EBX,
    X86_REG_ESP,
    X86_REG_EBP,
    X86_REG_ESI,
    X86_REG_EDI,
    X86_REG_R8D ,
    X86_REG_R9D ,
    X86_REG_R10D,
    X86_REG_R11D,
    X86_REG_R12D,
    X86_REG_R13D,
    X86_REG_R14D,
    X86_REG_R15D,
    //qword
    X86_REG_RAX,
    X86_REG_RCX,
    X86_REG_RDX,
    X86_REG_RBX,
    X86_REG_RSP,
    X86_REG_RBP,
    X86_REG_RSI,
    X86_REG_RDI,
    X86_REG_R8 ,
    X86_REG_R9 ,
    X86_REG_R10,
    X86_REG_R11,
    X86_REG_R12,
    X86_REG_R13,
    X86_REG_R14,
    X86_REG_R15
};

uint64_t CONVERT_REGISTER_TABLE[] = {
    -2,
    X86_REGISTER_AX, X86_REGISTER_AL, X86_REGISTER_AX, X86_REGISTER_BX, X86_REGISTER_BL,
    X86_REGISTER_BP, X86_REGISTER_BPL, X86_REGISTER_BX, X86_REGISTER_CX, X86_REGISTER_CL,
    -2, X86_REGISTER_CX, X86_REGISTER_DX, X86_REGISTER_DI, X86_REGISTER_DIL,
    X86_REGISTER_DL, -2, X86_REGISTER_DX, X86_REGISTER_EAX, X86_REGISTER_EBP,
    X86_REGISTER_EBX, X86_REGISTER_ECX, X86_REGISTER_EDI, X86_REGISTER_EDX, 0,
    -2, -2, -2, X86_REGISTER_ESI, X86_REGISTER_ESP,
    -2, -2, -2, -2, X86_REGISTER_RAX,
    X86_REGISTER_RBP, X86_REGISTER_RBX, X86_REGISTER_RCX, X86_REGISTER_RDI, X86_REGISTER_RDX,
    -2, -2, X86_REGISTER_RSI, X86_REGISTER_RSP, X86_REGISTER_SI,
    X86_REGISTER_SIL, X86_REGISTER_SP, X86_REGISTER_SPL, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, X86_REGISTER_R8, X86_REGISTER_R9, X86_REGISTER_R10, X86_REGISTER_R11,
    X86_REGISTER_R12, X86_REGISTER_R13, X86_REGISTER_R14, X86_REGISTER_R15,
    -2, -2, -2, -2,
    -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, -2, -2, -2, -2,
    -2, X86_REGISTER_R8B, X86_REGISTER_R9B, X86_REGISTER_R10B, X86_REGISTER_R11B,
    X86_REGISTER_R12B, X86_REGISTER_R13B, X86_REGISTER_R14B, X86_REGISTER_R15B, X86_REGISTER_R8D,
    X86_REGISTER_R9D, X86_REGISTER_R10D, X86_REGISTER_R11D, X86_REGISTER_R12D, X86_REGISTER_R13D,
    X86_REGISTER_R14D, X86_REGISTER_R15D, X86_REGISTER_R8W, X86_REGISTER_R9W, X86_REGISTER_R10W,
    X86_REGISTER_R11W, X86_REGISTER_R12W, X86_REGISTER_R13W, X86_REGISTER_R14W, X86_REGISTER_R15W,
    -2
};


fuku_code_profiler::fuku_code_profiler(fuku_assambler_arch arch)
    :arch(arch), dirty_registers_table(false) {

    memset(&registers_table, 0, sizeof(registers_table));
    cs_open(CS_ARCH_X86, arch == FUKU_ASSAMBLER_ARCH_X86 ? CS_MODE_32 : CS_MODE_64, &cap_handle);
    cs_option(cap_handle, CS_OPT_DETAIL, CS_OPT_ON);
}

fuku_code_profiler::~fuku_code_profiler() {
    cs_close(&cap_handle);
}


uint64_t fuku_code_profiler::profile_graph_registers(fuku_code_holder& code, linestorage::iterator lines_iter) {
    uint64_t included_registers = 0;
    uint64_t excluded_registers = 0;

    cs_insn *instruction;

    reg_access op_access[16];

    if (this->dirty_registers_table) {
        memcpy(this->registers_table, CONVERT_REGISTER_TABLE, sizeof(this->registers_table));
        this->dirty_registers_table = false;
    }

    for (; lines_iter != code.get_lines().end(); lines_iter++) {
        auto& current_inst = *lines_iter;


        memset(op_access, 0, sizeof(op_access));

        cs_disasm(cap_handle, current_inst.get_op_code(), current_inst.get_op_length(), 0, 1, &instruction);
        if (!instruction) { FUKU_DEBUG; }

        uint16_t current_id = current_inst.get_id();


        switch (current_id) {

        case X86_INS_JMP: case X86_INS_RET: case X86_INS_CALL: 
        case X86_INS_JAE: case X86_INS_JA:
        case X86_INS_JBE: case X86_INS_JB:
        case X86_INS_JE:  case X86_INS_JGE:
        case X86_INS_JG:  case X86_INS_JLE:
        case X86_INS_JL:  case X86_INS_JNE:
        case X86_INS_JNO: case X86_INS_JNP: 
        case X86_INS_JNS: case X86_INS_JO:
        case X86_INS_JP:  case X86_INS_JS : {
            cs_free(instruction, 1);
            return included_registers;
        }

        default: {
            break;
        }
        }

        uint8_t reg_idx = 0;
        if (!get_instruction_operands_access(instruction, reg_idx, op_access)) {
            FUKU_DEBUG;
            printf("%s %s \n", instruction->mnemonic, instruction->op_str);
            cs_free(instruction, 1);
            return included_registers;
        }


        uint64_t current_included_registers = 0;
        uint64_t current_excluded_registers = 0;

        for (size_t access_idx = 0; access_idx < reg_idx; access_idx++) {
           
            if (op_access[access_idx].access & REGISTER_ACCESS_READ) {
                current_excluded_registers |= op_access[access_idx].reg;
            }
            if (op_access[access_idx].access & REGISTER_ACCESS_WRITE) {
                current_included_registers |= op_access[access_idx].reg;
            }
        }


        excluded_registers |= current_excluded_registers;
        included_registers |= current_included_registers & (~excluded_registers);

        cs_free(instruction, 1);
    }

    return included_registers;
}

uint64_t fuku_code_profiler::profile_graph_eflags(fuku_code_holder& code, linestorage::iterator lines_iter) {
    uint64_t included_flags = 0;
    uint64_t excluded_flags = 0;


    for (; lines_iter != code.get_lines().end(); lines_iter++) {
        auto& current_inst = *lines_iter;

        uint16_t current_id = current_inst.get_id();
        uint64_t current_eflags = current_inst.get_eflags();


        if (current_eflags & X86_EFLAGS_GROUP_TEST) {
            
            for (uint8_t flag_idx = 0; flag_idx < (sizeof(TESTED_FLAGS_TABLE) / sizeof(TESTED_FLAGS_TABLE[0])); flag_idx++) {
                if (current_eflags & TESTED_FLAGS_TABLE[flag_idx]) {
                    excluded_flags |= EXCLUDED_FLAGS_TABLE[flag_idx];
                }
            }
        }

        if (excluded_flags == (X86_EFLAGS_GROUP_MODIFY | X86_EFLAGS_GROUP_SET | X86_EFLAGS_GROUP_RESET | X86_EFLAGS_GROUP_UNDEFINED)) {
            return included_flags;
        }      

        switch (current_id) {

            case X86_INS_JMP: case X86_INS_RET: case X86_INS_CALL: {
                return included_flags;
            }

            default: {
                break;
            }
        }

        included_flags |=
            GET_BITES_INCLUDED(
                current_eflags,
                X86_EFLAGS_GROUP_MODIFY | X86_EFLAGS_GROUP_SET | X86_EFLAGS_GROUP_RESET | X86_EFLAGS_GROUP_UNDEFINED,
                excluded_flags
            );

        if (included_flags == (X86_EFLAGS_GROUP_MODIFY | X86_EFLAGS_GROUP_SET | X86_EFLAGS_GROUP_RESET | X86_EFLAGS_GROUP_UNDEFINED)) {
            return included_flags;
        }
    }

    return included_flags;
}

bool fuku_code_profiler::profile_code(fuku_code_holder& code) {

    if (arch != code.get_arch()) {
        return false;
    }

    memcpy(registers_table, CONVERT_REGISTER_TABLE, sizeof(CONVERT_REGISTER_TABLE));

    for (auto line_iter = code.get_lines().begin(); line_iter != code.get_lines().end(); line_iter++) {

        (*line_iter).set_eflags(profile_graph_eflags(code, line_iter));
        (*line_iter).set_custom_flags(profile_graph_registers(code, line_iter));
    }

    
    for (auto line_iter = code.get_lines().begin(); line_iter != code.get_lines().end(); line_iter++) {
     

        switch (line_iter->get_id()) {

        case X86_INS_JMP: {
            if (line_iter->get_rip_relocation_idx() != -1) {
                size_t label_idx = code.get_rip_relocations()[line_iter->get_rip_relocation_idx()].label_idx;
                auto& label = code.get_labels()[label_idx];
                if (label.has_linked_instruction) {
                    line_iter->set_custom_flags(label.instruction->get_custom_flags());
                }
            }
            break;
        }

        case X86_INS_JAE: case X86_INS_JA:
        case X86_INS_JBE: case X86_INS_JB:
        case X86_INS_JE:  case X86_INS_JGE:
        case X86_INS_JG:  case X86_INS_JLE:
        case X86_INS_JL:  case X86_INS_JNE:
        case X86_INS_JNO: case X86_INS_JNP:
        case X86_INS_JNS: case X86_INS_JO:
        case X86_INS_JP:  case X86_INS_JS: {

            if (line_iter->get_rip_relocation_idx() != -1) {
                size_t label_idx = code.get_rip_relocations()[line_iter->get_rip_relocation_idx()].label_idx;
                auto& label = code.get_labels()[label_idx];

                if (label.has_linked_instruction) {
                    auto next_line = line_iter;
                    next_line++;

                    if (next_line != code.get_lines().end()) {
                        line_iter->set_custom_flags(label.instruction->get_custom_flags() & next_line->get_custom_flags());
                    }
                }
            }
            break;
        }

        default: {
            break;
        }
        }
    }
    
  
    cs_insn *instruction;
    for (auto line_iter = code.get_lines().begin(); line_iter != code.get_lines().end(); line_iter++) {
        cs_disasm(cap_handle, line_iter->get_op_code(), line_iter->get_op_length(), 0, 1, &instruction);
        printf("%016I64x   %s %s | ", line_iter->get_virtual_address(), instruction->mnemonic, instruction->op_str);

        for (size_t reg_idx = X86_REG_INVALID; reg_idx < X86_REG_ENDING; reg_idx++) {
            if (CONVERT_REGISTER_TABLE[reg_idx] != -2 &&
                line_iter->get_custom_flags() & CONVERT_REGISTER_TABLE[reg_idx]) {

                switch (CONVERT_REGISTER_TABLE[reg_idx]) {

                   case X86_REGISTER_AX: {printf("AX "); break; }
                   case X86_REGISTER_AL: {printf("AL "); break; }
                   case X86_REGISTER_BX: {printf("BX "); break; }
                   case X86_REGISTER_BL: {printf("BL "); break; }
                   case X86_REGISTER_BP: {printf("BP "); break; } 
                   case X86_REGISTER_BPL: {printf("BPL "); break; }
                   case X86_REGISTER_CX: {printf("CX "); break; }
                   case X86_REGISTER_CL: {printf("CL "); break; }
                   case X86_REGISTER_DX: {printf("Dx "); break; }
                   case X86_REGISTER_DI: {printf("DI "); break; }
                   case X86_REGISTER_DIL: {printf("DIL "); break; }
                   case X86_REGISTER_DL: {printf("DL "); break; }
                   case X86_REGISTER_EAX: {printf("EAX "); break; }
                   case X86_REGISTER_EBP: {printf("EBP "); break; }
                   case X86_REGISTER_EBX: {printf("EBX "); break; }
                   case X86_REGISTER_ECX: {printf("ECX "); break; }
                   case X86_REGISTER_EDI: {printf("EDI "); break; }
                   case X86_REGISTER_EDX: {printf("EDX "); break; } 
                   case X86_REGISTER_ESI: {printf("ESI "); break; } 
                   case X86_REGISTER_ESP: {printf("ESP "); break; } 
                   case X86_REGISTER_RAX: {printf("RAX "); break; } 
                   case X86_REGISTER_RBP: {printf("RBP "); break; } 
                   case X86_REGISTER_RBX: {printf("RBX "); break; } 
                   case X86_REGISTER_RCX: {printf("RCX "); break; } 
                   case X86_REGISTER_RDI: {printf("RDI "); break; } 
                   case X86_REGISTER_RDX: {printf("RDX "); break; } 
                   case X86_REGISTER_RSI: {printf("RSI "); break; }
                   case X86_REGISTER_RSP: {printf("RSP "); break; } 
                   case X86_REGISTER_SI: {printf("SI "); break; } 
                   case X86_REGISTER_SIL: {printf("SIL "); break; }
                   case X86_REGISTER_SP: {printf("SP "); break; }
                   case X86_REGISTER_SPL: {printf("SPL "); break; }
                   case X86_REGISTER_R8: {printf("R8 "); break; } 
                   case X86_REGISTER_R9: {printf("R9 "); break; } 
                   case X86_REGISTER_R10: {printf("R10 "); break; }
                   case X86_REGISTER_R11: {printf("R11 "); break; }
                   case X86_REGISTER_R12: {printf("R12 "); break; }
                   case X86_REGISTER_R13: {printf("R13 "); break; }
                   case X86_REGISTER_R14: {printf("R14 "); break; }
                   case X86_REGISTER_R15: {printf("R15 "); break; }
                   case X86_REGISTER_R8B: {printf("R8B "); break; }
                   case X86_REGISTER_R9B: {printf("R9B "); break; }
                   case X86_REGISTER_R10B: {printf("R10B "); break; }
                   case X86_REGISTER_R11B: {printf("R11B "); break; }
                   case X86_REGISTER_R12B: {printf("R12B "); break; }
                   case X86_REGISTER_R13B: {printf("R13B "); break; } 
                   case X86_REGISTER_R14B: {printf("R14B "); break; } 
                   case X86_REGISTER_R15B: {printf("R15B "); break; } 
                   case X86_REGISTER_R8D: {printf("R8D "); break; } 
                   case X86_REGISTER_R9D: {printf("R9D "); break; } 
                   case X86_REGISTER_R10D: {printf("R10D "); break; }
                   case X86_REGISTER_R11D: {printf("R11D "); break; }
                   case X86_REGISTER_R12D: {printf("R12D "); break; }
                   case X86_REGISTER_R13D: {printf("R13D "); break; }
                   case X86_REGISTER_R14D: {printf("R14D "); break; } 
                   case X86_REGISTER_R15D: {printf("R15D "); break; } 
                   case X86_REGISTER_R8W: {printf("R8W "); break; } 
                   case X86_REGISTER_R9W: {printf("R9W "); break; }
                   case X86_REGISTER_R10W: {printf("R10W "); break; } 
                   case X86_REGISTER_R11W: {printf("R11W "); break; } 
                   case X86_REGISTER_R12W: {printf("R12W "); break; } 
                   case X86_REGISTER_R13W: {printf("R13W "); break; } 
                   case X86_REGISTER_R14W: {printf("R14W "); break; }
                   case X86_REGISTER_R15W: {printf("R15W "); break; } 
                }

            }
        }

        printf("\n");

        cs_free(instruction, 1);
    }
    //*/
    return true;
}



inline void push_access(uint8_t& reg_idx, reg_access&& access, reg_access op_access[]) {
    op_access[reg_idx] = access;
    reg_idx++;
}

void get_operand_access(uint8_t& reg_idx, cs_insn *instruction, uint8_t op_num, reg_access op_access[], uint64_t table[], uint8_t default_access) {
    
    auto& op = instruction->detail->x86.operands[op_num];

    if (op.type == CS_OP_MEM) {
        if (op.mem.base != X86_REG_INVALID) {
            push_access(reg_idx, { table[op.mem.base] , REGISTER_ACCESS_READ }, op_access);
        }

        if (op.mem.index != X86_REG_INVALID) {
            push_access(reg_idx, { table[op.mem.index] , REGISTER_ACCESS_READ }, op_access);
        }
    }
    else if (op.type == CS_OP_REG) {
        push_access(reg_idx, { table[op.reg] , default_access }, op_access);
    }
}



bool fuku_code_profiler::get_instruction_operands_access(cs_insn *instruction, uint8_t& reg_idx, reg_access op_access[]) {

    bool handled = false;
    reg_idx = 0;
    uint8_t default_reg_mode = 0;

    uint64_t default_stack_pointer = this->arch == FUKU_ASSAMBLER_ARCH_X86 ? X86_REGISTER_ESP : X86_REGISTER_RSP;
    uint64_t default_frame_pointer = this->arch == FUKU_ASSAMBLER_ARCH_X86 ? X86_REGISTER_EBP : X86_REGISTER_RBP;

    switch (instruction->id) {

    case X86_INS_AAA: { 
        break; 
    }
    case X86_INS_AAD: { break; }
    case X86_INS_AAM: { break; }
    case X86_INS_AAS: { break; }
    case X86_INS_DAA: { break; }
    case X86_INS_DAS: { break; }


    case X86_INS_XOR: {
        handled = true;

        if (instruction->detail->x86.operands[0].type == CS_OP_REG &&
            instruction->detail->x86.operands[1].type == CS_OP_REG &&
            instruction->detail->x86.operands[0].reg == instruction->detail->x86.operands[1].reg) {

            get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_WRITE);
        }
        else {
            get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ | REGISTER_ACCESS_WRITE);
            get_operand_access(reg_idx, instruction, 1, op_access, registers_table, REGISTER_ACCESS_READ);
        }
      
        break;
    }


    case X86_INS_BTC:
    case X86_INS_BTR:
    case X86_INS_BTS:
    case X86_INS_OR:
    case X86_INS_SUB:
    case X86_INS_SBB:
    case X86_INS_AND:
    case X86_INS_ADC:
    case X86_INS_ADCX:
    case X86_INS_ADD:
    case X86_INS_RCR:
    case X86_INS_RCL:
    case X86_INS_ROL:
    case X86_INS_ROR:
    case X86_INS_RORX:
    case X86_INS_SAL:
    case X86_INS_SAR:
    case X86_INS_SARX:
    case X86_INS_SHL: 
    case X86_INS_SHLX: 
    case X86_INS_SHR:
    case X86_INS_SHRX: {
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ | REGISTER_ACCESS_WRITE);
        get_operand_access(reg_idx, instruction, 1, op_access, registers_table, REGISTER_ACCESS_READ);
        break; 
    }


    case X86_INS_CMOVA:
    case X86_INS_CMOVAE:
    case X86_INS_CMOVB:
    case X86_INS_CMOVBE:
    case X86_INS_CMOVE:
    case X86_INS_CMOVG:
    case X86_INS_CMOVGE:
    case X86_INS_CMOVL:
    case X86_INS_CMOVLE:
    case X86_INS_CMOVNE:
    case X86_INS_CMOVNO:
    case X86_INS_CMOVNP:
    case X86_INS_CMOVNS:
    case X86_INS_CMOVO:
    case X86_INS_CMOVP:
    case X86_INS_CMOVS:
    case X86_INS_MOV:
    case X86_INS_MOVABS:
    case X86_INS_MOVSXD:
    case X86_INS_MOVSX:
    case X86_INS_MOVZX:
    case X86_INS_LEA: 
    case X86_INS_BSF: 
    case X86_INS_BSR:
    case X86_INS_BLSR:
    case X86_INS_BLSI: {
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_WRITE);
        get_operand_access(reg_idx, instruction, 1, op_access, registers_table, REGISTER_ACCESS_READ);
        break;
    }

    case X86_INS_XCHG: { 
        handled = true;
        if (instruction->detail->x86.operands[0].type == CS_OP_REG &&
            instruction->detail->x86.operands[1].type == CS_OP_REG) {

            std::swap(registers_table[instruction->detail->x86.operands[0].reg], registers_table[instruction->detail->x86.operands[1].reg]);
            dirty_registers_table = true;
        }
        else {
            get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ | REGISTER_ACCESS_WRITE);
            get_operand_access(reg_idx, instruction, 1, op_access, registers_table, REGISTER_ACCESS_READ | REGISTER_ACCESS_WRITE);
        }

        break; 
    }


    case X86_INS_BT:
    case X86_INS_TEST:
    case X86_INS_CMP: { 
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ);
        get_operand_access(reg_idx, instruction, 1, op_access, registers_table, REGISTER_ACCESS_READ);
        break; 
    }


    case X86_INS_BSWAP:
    case X86_INS_DEC:
    case X86_INS_INC:
    case X86_INS_NOT:
    case X86_INS_NEG: {
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ | REGISTER_ACCESS_WRITE);
        break; 
    }

    case X86_INS_PUSHAW: { 
        handled = true;
        push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_CX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_DX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_BX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_SP, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_BP, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_SI, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_DI, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break; 
    }
    case X86_INS_PUSHAL: { 
        handled = true;
        push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_ECX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_EDX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_EBX, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_ESP, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_EBP, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_ESI, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_EDI, REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break; 
    }
    case X86_INS_POPAW: {
        handled = true;
        push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_CX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_DX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_BX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_SP, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_BP, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_SI, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_DI, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break;
    }
    case X86_INS_POPAL: {
        handled = true;
        push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_ECX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_EDX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_EBX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_ESP, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_EBP, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_ESI, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_EDI, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break;
    }

    case X86_INS_PUSH: { 
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ);
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break; 
    }
    case X86_INS_POP: { 
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_WRITE);
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break; 
    }

    case X86_INS_RET: { 
        handled = true;
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break; 
    }

    case X86_INS_LEAVE: { 
        handled = true;
        push_access(reg_idx, { default_frame_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break; 
    }


    case X86_INS_IDIV:
    case X86_INS_DIV: {
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ);

        switch (instruction->detail->x86.operands[0].size) {
        
        case 1: {
            push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            break;
        }
        case 2: {
            push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            push_access(reg_idx, { X86_REGISTER_DX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            break;
        }
        case 4: {
            push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            push_access(reg_idx, { X86_REGISTER_EDX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            break;
        }
        case 8: {
            push_access(reg_idx, { X86_REGISTER_RAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            push_access(reg_idx, { X86_REGISTER_RDX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            break;
        }

        }
      
        break; 
    }

    case X86_INS_MUL: { 
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ);

        switch (instruction->detail->x86.operands[0].size) {

        case 1: {
            push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            break;
        }
        case 2: {
            push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            push_access(reg_idx, { X86_REGISTER_DX, REGISTER_ACCESS_WRITE }, op_access);
            break;
        }
        case 4: {
            push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            push_access(reg_idx, { X86_REGISTER_EDX, REGISTER_ACCESS_WRITE }, op_access);
            break;
        }
        case 8: {
            push_access(reg_idx, { X86_REGISTER_RAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
            push_access(reg_idx, { X86_REGISTER_RDX, REGISTER_ACCESS_WRITE }, op_access);
            break;
        }

        }
        break; 
    }
    case X86_INS_IMUL: { 
        handled = true;
        
        switch (instruction->detail->x86.op_count) {

        case 1: {
            get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ);

            switch (instruction->detail->x86.operands[0].size) {

            case 1: {
                push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
                break;
            }
            case 2: {
                push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
                push_access(reg_idx, { X86_REGISTER_DX, REGISTER_ACCESS_WRITE }, op_access);
                break;
            }
            case 4: {
                push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
                push_access(reg_idx, { X86_REGISTER_EDX, REGISTER_ACCESS_WRITE }, op_access);
                break;
            }
            case 8: {
                push_access(reg_idx, { X86_REGISTER_RAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
                push_access(reg_idx, { X86_REGISTER_RDX, REGISTER_ACCESS_WRITE }, op_access);
                break;
            }

            }
            break;
        }
        case 2: {
            get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ);
            get_operand_access(reg_idx, instruction, 1, op_access, registers_table, REGISTER_ACCESS_READ);
            break;
        }
        case 3: {
            get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_WRITE);
            get_operand_access(reg_idx, instruction, 1, op_access, registers_table, REGISTER_ACCESS_READ);
            break;
        }
        }

        break; 
    }

    case X86_INS_JAE:
    case X86_INS_JA:
    case X86_INS_JBE:
    case X86_INS_JB:
    case X86_INS_JE:
    case X86_INS_JGE:
    case X86_INS_JG:
    case X86_INS_JLE:
    case X86_INS_JL:
    case X86_INS_JNE:
    case X86_INS_JNO:
    case X86_INS_JNP:
    case X86_INS_JNS:
    case X86_INS_JO:
    case X86_INS_JP:
    case X86_INS_JS: {
        handled = true;
        break;
    }

    case X86_INS_CALL:
    case X86_INS_JMP: {
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_READ);
        break;
    }

    case X86_INS_SETAE:
    case X86_INS_SETA:
    case X86_INS_SETBE:
    case X86_INS_SETB:
    case X86_INS_SETE:
    case X86_INS_SETGE:
    case X86_INS_SETG:
    case X86_INS_SETLE:
    case X86_INS_SETL:
    case X86_INS_SETNE:
    case X86_INS_SETNO:
    case X86_INS_SETNP:
    case X86_INS_SETNS:
    case X86_INS_SETO:
    case X86_INS_SETP:
    case X86_INS_SETS: {
        handled = true;
        get_operand_access(reg_idx, instruction, 0, op_access, registers_table, REGISTER_ACCESS_WRITE);
        break;
    }


    case X86_INS_POPF:
    case X86_INS_POPFD:
    case X86_INS_POPFQ:
    case X86_INS_PUSHFQ:
    case X86_INS_PUSHFD:
    case X86_INS_PUSHF: { 
        handled = true;
        push_access(reg_idx, { default_stack_pointer, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        break; 
    }



    case X86_INS_CMPSB: 
    case X86_INS_CMPSW: 
    case X86_INS_CMPSD: 
    case X86_INS_CMPSQ: 
    case X86_INS_MOVSB: 
    case X86_INS_MOVSW: 
    case X86_INS_MOVSD: 
    case X86_INS_MOVSQ: 
    case X86_INS_STOSB: 
    case X86_INS_STOSW: 
    case X86_INS_STOSD: 
    case X86_INS_STOSQ: 
    case X86_INS_LODSB: 
    case X86_INS_LODSW: 
    case X86_INS_LODSD: 
    case X86_INS_LODSQ: 
    case X86_INS_SCASB: 
    case X86_INS_SCASW: 
    case X86_INS_SCASD: 
    case X86_INS_SCASQ: { 

        handled = true;
        for (size_t read_idx = 0; read_idx < instruction->detail->regs_read_count; read_idx++) {
            push_access(reg_idx, { registers_table[instruction->detail->regs_read[read_idx]], REGISTER_ACCESS_READ }, op_access);
        }
        for (size_t write_idx = 0; write_idx < instruction->detail->regs_write_count; write_idx++) {
            push_access(reg_idx, { registers_table[instruction->detail->regs_write[write_idx]], REGISTER_ACCESS_WRITE }, op_access);
        }
        break; 
    }


    case X86_INS_CLC: 
    case X86_INS_CLD: 
    case X86_INS_CLI: 
    case X86_INS_CMC: 
    case X86_INS_STI: 
    case X86_INS_STC: 
    case X86_INS_STD: 
    case X86_INS_CLAC: 
    case X86_INS_INT3:
    case X86_INS_INT1:
    case X86_INS_INT:
    case X86_INS_NOP: {
        handled = true;
        break;
    }

    case X86_INS_CWD: { 
        handled = true;
        push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_DX, REGISTER_ACCESS_WRITE }, op_access);
        break; 
    }
    case X86_INS_CDQ: {
        handled = true;
        push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_EDX, REGISTER_ACCESS_WRITE }, op_access);
        break; 
    }
    case X86_INS_CQO: { 
        handled = true;
        push_access(reg_idx, { X86_REGISTER_RAX, REGISTER_ACCESS_WRITE | REGISTER_ACCESS_READ }, op_access);
        push_access(reg_idx, { X86_REGISTER_RDX, REGISTER_ACCESS_WRITE }, op_access);
        break; 
    }

    case X86_INS_CBW: {
        handled = true;
        push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_AL, REGISTER_ACCESS_READ }, op_access);
        break;
    }
    case X86_INS_CWDE: {
        handled = true;
        push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_AX, REGISTER_ACCESS_READ }, op_access);
        break;
    }
    case X86_INS_CDQE: {
        handled = true;
        push_access(reg_idx, { X86_REGISTER_RAX, REGISTER_ACCESS_WRITE }, op_access);
        push_access(reg_idx, { X86_REGISTER_EAX, REGISTER_ACCESS_READ }, op_access);
        break;
    }

    case X86_INS_ANDN: { break; }

    case X86_INS_CMPXCHG16B: { break; }
    case X86_INS_CMPXCHG: { break; }
    case X86_INS_CMPXCHG8B: { break; }

    case X86_INS_BZHI: { break; }

    case X86_INS_CPUID: { break; }
    
    case X86_INS_CRC32: { break; }
   

    case X86_INS_JCXZ: { break; }
    case X86_INS_JECXZ: { break; }
    case X86_INS_JRCXZ: { break; }


    case X86_INS_LFENCE: { break; }
  
    case X86_INS_LOOP: { break; }
    case X86_INS_LOOPE: { break; }
    case X86_INS_LOOPNE: { break; }

    case X86_INS_XADD: { break; }

    case X86_INS_MOVQ: { break; }

    case X86_INS_MOVD: { break; }

    case X86_INS_MOVBE: { break; }
    case X86_INS_MOVSS: { break; }
    case X86_INS_POPCNT: { break; }

    case X86_INS_RDRAND: { break; }
    case X86_INS_RDSEED: { break; }
    case X86_INS_RDTSC: { break; }
    case X86_INS_RDTSCP: { break; }

    case X86_INS_SHLD: { break; }
    case X86_INS_SHRD: { break; }

    case X86_INS_STGI: { break; }

    case X86_INS_SAHF: { break; }

    case X86_INS_TZCNT: { break; }

    case X86_INS_XTEST: { break; }
    case X86_INS_CMPSS: { break; }


    default: {break; }
    }

    return handled;
}


bool has_inst_free_register(fuku_instruction& inst, x86_reg reg) {
    
    if (CONVERT_REGISTER_TABLE[reg] != -2) {
        return GET_BITES(inst.get_custom_flags(), CONVERT_REGISTER_TABLE[reg]) == CONVERT_REGISTER_TABLE[reg];
    }

    return false;
}


bool has_inst_free_eflags(uint64_t inst_eflags, uint64_t flags) {

    if (GET_BITES(flags, X86_EFLAGS_MODIFY_CF)) {
        if (!GET_BITES(inst_eflags, X86_EFLAGS_MOD_CF)) {
            return false;
        }
    }

    if (GET_BITES(flags, X86_EFLAGS_MODIFY_OF)) {
        if (!GET_BITES(inst_eflags, X86_EFLAGS_MOD_OF)) {
            return false;
        }
    }

    if (GET_BITES(flags, X86_EFLAGS_MODIFY_ZF)) {
        if (!GET_BITES(inst_eflags, X86_EFLAGS_MOD_ZF)) {
            return false;
        }
    }

    if (GET_BITES(flags, X86_EFLAGS_MODIFY_DF)) {
        if (!GET_BITES(inst_eflags, X86_EFLAGS_MOD_DF)) {
            return false;
        }
    }

    if (GET_BITES(flags, X86_EFLAGS_MODIFY_SF)) {
        if (!GET_BITES(inst_eflags, X86_EFLAGS_MOD_SF)) {
            return false;
        }
    }

    if (GET_BITES(flags, X86_EFLAGS_MODIFY_PF)) {
        if (!GET_BITES(inst_eflags, X86_EFLAGS_MOD_PF)) {
            return false;
        }
    }
    if (GET_BITES(flags, X86_EFLAGS_MODIFY_AF)) {
        if (!GET_BITES(inst_eflags, X86_EFLAGS_MOD_AF)) {
            return false;
        }
    }


    return true;
}


uint64_t convert_cap_reg_to_inst_reg(x86_reg reg) {
    return CONVERT_REGISTER_TABLE[reg];
}



inline bool bit_scan_forward(uint32_t& index, uint64_t mask) {
    for (; index < 64; index++) {
        if (mask & ((uint64_t)1 << index)) {
            return true;
        }
    }
    return false;
}

inline bool bit_scan_backward(uint32_t& index, uint64_t mask) {
    for (; index != -1; index--) {
        if (mask & ((uint64_t)1 << index)) {
            return true;
        }
    }
    return false;
}

x86_reg  convert_inst_reg_to_cap_reg(uint64_t reg) {
    
    uint32_t index = 0;
    if (bit_scan_forward(index, reg)) {
        return CONVERT_REGISTER_TO_CAP[index];
    }
    else {
        return x86_reg::X86_REG_INVALID;
    }
}



x86_reg get_rand_free_reg_(uint64_t inst_regs, uint32_t min_idx , uint32_t max_idx) {

    uint32_t index = min_idx;
    if (bit_scan_forward(index, inst_regs)) {
        if (index > max_idx) {
            return x86_reg::X86_REG_INVALID;
        }

        uint32_t rand_idx = FUKU_GET_RAND(min_idx, max_idx);

        index = rand_idx;
        if (rand_idx == min_idx) {
            bit_scan_forward(index, inst_regs);
            return CONVERT_REGISTER_TO_CAP[index];
        }
        else if (rand_idx == max_idx) {
            bit_scan_backward(index, inst_regs);
            return CONVERT_REGISTER_TO_CAP[index];
        }
        else {
            if (!bit_scan_forward(index, inst_regs)) {
                index = rand_idx;
                bit_scan_backward(index, inst_regs);
            }
        }

        return CONVERT_REGISTER_TO_CAP[index];
    }

    return x86_reg::X86_REG_INVALID;
}

x86_reg get_inst_random_free_register(fuku_instruction& inst, uint32_t reg_size, bool x86_only, x86_reg exclude_reg) {

    uint64_t inst_regs = inst.get_custom_flags();

    if (exclude_reg != X86_REG_INVALID) {
        uint64_t ex_inst_reg = convert_cap_reg_to_inst_reg(exclude_reg);
        if (ex_inst_reg != -2) {
            inst_regs &= (~ex_inst_reg);
        }
    }

    if (inst_regs) {

        switch (reg_size) {

        case 1: {

            if (x86_only) {
                return get_rand_free_reg_(inst_regs, 0, 3);
            }
            else {
                return get_rand_free_reg_(inst_regs, 0, 15);
            }

            
            break;
        }
        case 2: {

            if (x86_only) {
                return get_rand_free_reg_(inst_regs, 16, 23);
            }
            else {
                return get_rand_free_reg_(inst_regs, 16, 31);
            }


            break;
        }
        case 4: {

            if (x86_only) {
                return get_rand_free_reg_(inst_regs, 32, 39);
            }
            else {
                return get_rand_free_reg_(inst_regs, 32, 47);
            }


            break;
        }
        case 8: {

            if (x86_only) {
                return get_rand_free_reg_(inst_regs, 48, 55);
            }
            else {
                return get_rand_free_reg_(inst_regs, 48, 63);
            }

            break;
        }
        }
    }

    return x86_reg::X86_REG_INVALID;
}