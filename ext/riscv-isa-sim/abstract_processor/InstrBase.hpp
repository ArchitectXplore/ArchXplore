#ifndef __INSTR_BASE_HPP__
#define __INSTR_BASE_HPP__
#include "./util.hpp"
#include <memory>
namespace archXplore{
class InstrBase{
public:
    using PtrType = std::shared_ptr<InstrBase>;
    reg_t pc = 0xdeadbeaf;
    // exec result
    reg_t npc = 0xdeadbeaf;
    // exception 
    bool evalid = false;
    reg_t ecause = 0xdeadbeaf;
    //interrupt
    bool ivalid = false;

    reg_t getRaw() const {return _raw;}
    void setRaw(const reg_t& raw){_raw = raw;}
    virtual ~InstrBase() = default;
protected:
    reg_t _raw = 0xdeadbeaf;
    reg_t _getBits(const int& lo, const int& len) const { return (_raw >> lo) & ((reg_t(1) << len) - 1); }
    reg_t _getBitsSigned(int lo, int len) const { return int64_t(_raw) << (64 - lo - len) >> (64 - len); }
}; 

class RVInstrBase: public InstrBase{
public:
    const reg_t XREG_RA = 1;
    const reg_t XREG_SP = 2;
    const reg_t XREG_S0 = 8;
    const reg_t XREG_A0 = 10;
    const reg_t XREG_A1 = 11;
    const reg_t XREG_Sn = 16;
    using PtrType = std::shared_ptr<RVInstrBase>;
    
    virtual ~RVInstrBase() = default;


    int length() const { return _insn_length(_raw); }
    bool isC()const {return length() == 2;}
    int memLen()const{
        if(isLd() || isSt() || isFLd() || isFSt() || isAmo() ||
            isOp() || isOpI()
        ){
            return 1 << (funct3() & 0b11);
        }
        else if(isHlv() || isHsv()){
            return 1 << ((funct7() >> 1) & 0b11); // funct7[1:3]
        }
        else {
            return 0;
        }
    }
    bool isMemOperation() const{
        return isSt() || isLd() || 
                isFLd() || isFSt() ||
                isAmo() ||
                isHsv() || isHlv();
    }
    bool isOpI() const{
        return opcode() == 0b0010011;
    }
    bool isOp() const{
        return opcode() == 0b0110011;
    }
    bool isLoad() const{
        return isLd() || isFLd() || isHlv() || isLR();
    }
    bool isStore() const{
        return isSt() || isFSt() || isHsv() || 
                (isAmo() && !isLR());
    }
    bool isLd() const{
        return opcode() == 0b0000011;
    }
    bool isSt() const{
        return opcode() == 0b0100011;
    }
    // float
    bool isFLd() const{
        return opcode() == 0b0000111;
    }
    bool isFSt() const{
        return opcode() == 0b0100111;
    }
    bool isFMAdd() const{
        return opcode() == 0b1000011;
    }
    bool isFMSub() const{
        return opcode() == 0b1000111;
    }
    bool isFNMAdd() const{
        return opcode() == 0b1001011;
    }
    bool isFNMSub() const{
        return opcode() == 0b1001111;
    }
    bool isFOp() const{
        return opcode() == 0b1010011;
    }
    bool isF() const{
        return isFLd() || isFSt() ||
            isFMAdd() || isFMSub() ||
            isFNMAdd() || isFNMSub() ||
            isFOp()
            ;
    }
    bool isSingle() const{
        return isF() && (_getBits(25,2) == 0b00);
    }
    bool isDouble() const{
        return isF() && (_getBits(25,2) == 0b01);
    }
    
    bool isAmo() const{
        return opcode() == 0b0101111;
    }
    bool isLR() const{
        return isAmo() && rs2() == 0 && amo_funct5() == 0b00010;
    } // load reserve
    bool isSC() const{
        return isAmo() && amo_funct5() == 0b00011;
    }
    bool isAmoSwap() const{
        return isAmo() && amo_funct5() == 0b00001;
    }
    bool isAmoAdd() const{
        return isAmo() && amo_funct5() == 0b00000;
    }
    bool isAmoXor() const{
        return isAmo() && amo_funct5() == 0b00100;
    }
    bool isAmoAnd() const{
        return isAmo() && amo_funct5() == 0b01100;
    }
    bool isAmoOr() const{
        return isAmo() && amo_funct5() == 0b01000;
    }
    bool isAmoMin() const{
        return isAmo() && amo_funct5() == 0b10000;
    }
    bool isAmoMax() const{
        return isAmo() && amo_funct5() == 0b10100;
    }
    bool isAmoMinU() const{
        return isAmo() && amo_funct5() == 0b11000;
    }
    bool isAmoMaxU() const{
        return isAmo() && amo_funct5() == 0b11100;
    }
    //TODO: h extension is not naively implemeted
    bool isHlvx() const{
        return isHlv() && rs2() == 0b00011;
    } 
    bool isHlv() const{
        return opcode() == 0b1110011 && rd() != 0 && (funct7() & 1 == 0) && (funct3() == 0b100);
    }
    bool isHsv() const{
        return opcode() == 0b1110011 && rd() == 0 && (funct7() & 1 == 1) && (funct3() == 0b100);
    }
    bool isMemWithWriteBack(){
        return isLd() || isFLd() || isAmo() || isHlv();
    }
    bool isLoadUnsigned(){
        return (isLd() || isHsv()) && ((funct3() & 0b100) == 0b100);
    }
    // TODO: c extension and cmpop are not implemented
    bool isLoadC() const{return false;}
    bool isStoreC() const{return false;}
    int64_t i_imm() const { return _getBitsSigned(20, 12); }
    int64_t shamt() const { return _getBits(20, 6); }
    int64_t s_imm() const { return _getBits(7, 5) + (_getBitsSigned(25, 7) << 5); }
    int64_t sb_imm() const { return (_getBits(8, 4) << 1) + (_getBits(25, 6) << 5) + (_getBits(7, 1) << 11) + (imm_sign() << 12); }
    int64_t u_imm() const { return _getBitsSigned(12, 20) << 12; }
    int64_t uj_imm() const { return (_getBits(21, 10) << 1) + (_getBits(20, 1) << 11) + (_getBits(12, 8) << 12) + (imm_sign() << 20); }
    uint64_t rd() const { return _getBits(7, 5); }
    uint64_t rs1() const { return _getBits(15, 5); }
    uint64_t rs2() const { return _getBits(20, 5); }
    uint64_t rs3() const { return _getBits(27, 5); }
    uint64_t rm() const { return _getBits(12, 3); }
    uint64_t csr() const { return _getBits(20, 12); }
    uint64_t iorw() const { return _getBits(20, 8); }
    uint64_t bs() const { return _getBits(30, 2); } // Crypto ISE - SM4/AES32 byte select.
    uint64_t rcon() const { return _getBits(20, 4); } // Crypto ISE - AES64 round const.
    uint64_t opcode() const {return _getBits(0,7);}
    uint64_t funct7() const {return _getBits(25, 7);}
    uint64_t funct3() const {return _getBits(12, 3);}
    uint64_t amo_funct5() const {return _getBits(27, 5);}

    int64_t rvc_imm() const { return _getBits(2, 5) + (_getBitsSigned(12, 1) << 5); }
    int64_t rvc_zimm() const { return _getBits(2, 5) + (_getBits(12, 1) << 5); }
    int64_t rvc_addi4spn_imm() const { return (_getBits(6, 1) << 2) + (_getBits(5, 1) << 3) + (_getBits(11, 2) << 4) + (_getBits(7, 4) << 6); }
    int64_t rvc_addi16sp_imm() const { return (_getBits(6, 1) << 4) + (_getBits(2, 1) << 5) + (_getBits(5, 1) << 6) + (_getBits(3, 2) << 7) + (_getBitsSigned(12, 1) << 9); }
    int64_t rvc_lwsp_imm() const { return (_getBits(4, 3) << 2) + (_getBits(12, 1) << 5) + (_getBits(2, 2) << 6); }
    int64_t rvc_ldsp_imm() const { return (_getBits(5, 2) << 3) + (_getBits(12, 1) << 5) + (_getBits(2, 3) << 6); }
    int64_t rvc_swsp_imm() const { return (_getBits(9, 4) << 2) + (_getBits(7, 2) << 6); }
    int64_t rvc_sdsp_imm() const { return (_getBits(10, 3) << 3) + (_getBits(7, 3) << 6); }
    int64_t rvc_lw_imm() const { return (_getBits(6, 1) << 2) + (_getBits(10, 3) << 3) + (_getBits(5, 1) << 6); }
    int64_t rvc_ld_imm() const { return (_getBits(10, 3) << 3) + (_getBits(5, 2) << 6); }
    int64_t rvc_j_imm() const { return (_getBits(3, 3) << 1) + (_getBits(11, 1) << 4) + (_getBits(2, 1) << 5) + (_getBits(7, 1) << 6) + (_getBits(6, 1) << 7) + (_getBits(9, 2) << 8) + (_getBits(8, 1) << 10) + (_getBitsSigned(12, 1) << 11); }
    int64_t rvc_b_imm() const { return (_getBits(3, 2) << 1) + (_getBits(10, 2) << 3) + (_getBits(2, 1) << 5) + (_getBits(5, 2) << 6) + (_getBitsSigned(12, 1) << 8); }
    int64_t rvc_simm3() const { return _getBits(10, 3); }
    uint64_t rvc_rd() const { return rd(); }
    uint64_t rvc_rs1() const { return rd(); }
    uint64_t rvc_rs2() const { return _getBits(2, 5); }
    uint64_t rvc_rds() const { return 8 + _getBits(2, 3); }
    uint64_t rvc_rs1s() const { return 8 + _getBits(7, 3); }
    uint64_t rvc_rs2s() const { return 8 + _getBits(2, 3); }
    uint64_t rvc_op() const {return _getBits(0, 2);}
    uint64_t rvc_func3()const {return _getBits(13, 3);}
    uint64_t rvc_func4()const {return _getBits(12, 4);}

    uint64_t rvc_lbimm() const { return (_getBits(5, 1) << 1) + _getBits(6, 1); }
    uint64_t rvc_lhimm() const { return (_getBits(5, 1) << 1); }

    uint64_t rvc_r1sc() const { return _getBits(7, 3); }
    uint64_t rvc_r2sc() const { return _getBits(2, 3); }
    uint64_t rvc_rlist() const { return _getBits(4, 4); }
    uint64_t rvc_spimm() const { return _getBits(2, 2) << 4; }

    uint64_t rvc_index() const { return _getBits(2, 8); }

    uint64_t v_vm() const { return _getBits(25, 1); }
    uint64_t v_wd() const { return _getBits(26, 1); }
    uint64_t v_nf() const { return _getBits(29, 3); }
    uint64_t v_simm5() const { return _getBitsSigned(15, 5); }
    uint64_t v_zimm5() const { return _getBits(15, 5); }
    uint64_t v_zimm10() const { return _getBits(20, 10); }
    uint64_t v_zimm11() const { return _getBits(20, 11); }
    uint64_t v_lmul() const { return _getBits(20, 2); }
    uint64_t v_frac_lmul() const { return _getBits(22, 1); }
    uint64_t v_sew() const { return 1 << (_getBits(23, 3) + 3); }
    uint64_t v_width() const { return _getBits(12, 3); }
    uint64_t v_mop() const { return _getBits(26, 2); }
    uint64_t v_lumop() const { return _getBits(20, 5); }
    uint64_t v_sumop() const { return _getBits(20, 5); }
    uint64_t v_vta() const { return _getBits(26, 1); }
    uint64_t v_vma() const { return _getBits(27, 1); }
    uint64_t v_mew() const { return _getBits(28, 1); }
    uint64_t v_zimm6() const { return _getBits(15, 5) + (_getBits(26, 1) << 5); }

    uint64_t p_imm2() const { return _getBits(20, 2); }
    uint64_t p_imm3() const { return _getBits(20, 3); }
    uint64_t p_imm4() const { return _getBits(20, 4); }
    uint64_t p_imm5() const { return _getBits(20, 5); }
    uint64_t p_imm6() const { return _getBits(20, 6); }

    uint64_t zcmp_regmask() const {
        unsigned mask = 0;
        uint64_t rlist = rvc_rlist();

        if (rlist >= 4)
        mask |= 1U << XREG_RA;

        for (reg_t i = 5; i <= rlist; i++)
            mask |= 1U << _sn(i - 5);

        if (rlist == 15)
        mask |= 1U << _sn(11);

        return mask;
    }

    uint64_t zcmp_stack_adjustment(int xlen) const {
        reg_t stack_adj_base = 0;
        switch (rvc_rlist()) {
        case 15:
        stack_adj_base += 16;
        case 14:
        if (xlen == 64)
            stack_adj_base += 16;
        case 13:
        case 12:
        stack_adj_base += 16;
        case 11:
        case 10:
        if (xlen == 64)
            stack_adj_base += 16;
        case 9:
        case 8:
        stack_adj_base += 16;
        case 7:
        case 6:
        if (xlen == 64)
            stack_adj_base += 16;
        case 5:
        case 4:
        stack_adj_base += 16;
        break;
        }

        return stack_adj_base + rvc_spimm();
    }
    uint64_t imm_sign() const { return _getBitsSigned(31, 1); }
private:
    inline int _insn_length(const reg_t& x)const {
        return 
            (((x) & 0x03) < 0x03 ? 2 : 
            ((x) & 0x1f) < 0x1f ? 4 : 
            ((x) & 0x3f) < 0x3f ? 6 : 
            8);
    } 
    inline reg_t _sn(const reg_t& n) const {
        return (n) < 2 ? XREG_S0 + (n) : XREG_Sn + (n);
    }

};  
}
#endif //__INSTR_BASE_HPP__