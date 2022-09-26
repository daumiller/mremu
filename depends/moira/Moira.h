// -----------------------------------------------------------------------------
// This file is part of Moira - A Motorola 68k emulator
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Published under the terms of the MIT License
// -----------------------------------------------------------------------------

#pragma once

#include "MoiraConfig.h"
#include "MoiraTypes.h"
#include "MoiraDebugger.h"

namespace moira {

class Moira {
    
    friend class Debugger;
    friend class Breakpoints;
    friend class Watchpoints;
    friend class Catchpoints;
    
    //
    // Sub components
    //
    
public:
    
    // Breakpoints, watchpoints, catchpoints, instruction tracing
    Debugger debugger = Debugger(*this);
    
    
    //
    // Internals
    //
    
protected:
    
    // The emulated CPU model
    Model model = M68000;
    
    // The interrupt mode of this CPU
    IrqMode irqMode = IRQ_AUTO;
    
    // The selected disassembler syntax
    DasmStyle style = DASM_MOIRA_MOT;

    // The number format used by the disassembler
    DasmNumberFormat numberFormat { .prefix = "$", .radix = 16 };

    // The letter case used by the disassembler
    DasmLetterCase letterCase = DASM_MIXED_CASE;

    // Space between instruction names and arguments
    Tab tab{8};

    
    /* State flags
     *
     * CPU_IS_HALTED:
     *     Set when the CPU is in "halted" state.
     *
     * CPU_IS_STOPPED:
     *     Set when the CPU is in "stopped" state. This state is entered when
     *     the STOP instruction has been executed. The state is left when the
     *     next interrupt occurs.
     *
     * CPU_IS_LOOPING:
     *     Set when the CPU is running in "loop mode". This mode is a 68010
     *     feature to speed up the execution of certain loops.
     *
     * CPU_LOG_INSTRUCTION:
     *     This flag is set if instruction logging is enabled. If set, the
     *     CPU records the current register contents in a log buffer.
     *
     * CPU_CHECK_INTERRUPTS:
     *     The CPU only checks for pending interrupts if this flag is set.
     *     To accelerate emulation, the CPU deletes this flag if it can assure
     *     that no interrupt can trigger.
     *
     * CPU_TRACE_EXCEPTION:
     *    If this flag is set, the CPU initiates the trace exception.
     *
     * CPU_TRACE_FLAG:
     *    This flag is a copy of the T flag from the status register. The
     *    copy is held to accelerate emulation.
     *
     * CPU_CHECK_BP:
     *    This flag indicates whether the CPU should check for breakpoints.
     *
     * CPU_CHECK_WP:
     *    This flag indicates whether the CPU should check fo watchpoints.
     */
    int flags;
    static constexpr int CPU_IS_HALTED          = (1 << 8);
    static constexpr int CPU_IS_STOPPED         = (1 << 9);
    static constexpr int CPU_IS_LOOPING         = (1 << 10);
    static constexpr int CPU_LOG_INSTRUCTION    = (1 << 11);
    static constexpr int CPU_CHECK_IRQ          = (1 << 12);
    static constexpr int CPU_TRACE_EXCEPTION    = (1 << 13);
    static constexpr int CPU_TRACE_FLAG         = (1 << 14);
    static constexpr int CPU_CHECK_BP           = (1 << 15);
    static constexpr int CPU_CHECK_WP           = (1 << 16);
    static constexpr int CPU_CHECK_CP           = (1 << 17);
    
    // Number of elapsed cycles since powerup
    i64 clock;
    
    // The register set
    Registers reg;
    
    // The prefetch queue
    PrefetchQueue queue;

    // The memory management unit
    MMU mmu;
    
    // Current value on the IPL pins (Interrupt Priority Level)
    u8 ipl;
    
    // Value on the lower two function code pins (FC1|FC0)
    u8 fcl;
    
    // Determines the source of the function code pins
    u8 fcSource;
    
    // Remembers the vector number of the latest exception
    int exception;
    
    // Cycle penalty (needed for 68020+ extended addressing modes)
    int cp;
    
    // Jump table holding the instruction handlers
    typedef void (Moira::*ExecPtr)(u16);
    ExecPtr exec[65536];
    
    // Jump table holding the instruction handlers for the 68010 loop mode
    ExecPtr loop[65536];
    
    // Jump table holding the disassebler handlers
    typedef void (Moira::*DasmPtr)(StrWriter&, u32&, u16);
    DasmPtr *dasm = nullptr;
    
private:
    
    // Table holding instruction infos
    InstrInfo *info = nullptr;
    
    
    //
    // Constructing
    //
    
public:
    
    Moira();
    virtual ~Moira();
    
    // Selects the emulated CPU model
    void setModel(Model model);
    
    // Configures the disassembler
    void setDasmStyle(DasmStyle value);
    void setDasmNumberFormat(DasmNumberFormat value);
    void setDasmLetterCase(DasmLetterCase value);
    void setIndentation(int value);
    
protected:
    
    // Creates the generic jump table
    void createJumpTable();
    
private:
    
    template <Core C> void createJumpTable();


    //
    // Querying CPU properties
    //

public:

    // Checks if the emulated CPU model has a coprocessor interface
    bool hasCPI();

    // Checks if the emulated CPU model has a memory managenemt unit
    bool hasMMU();

    // Checks if the emulated CPU model has a floating point unit
    bool hasFPU();

    // Returns the address bus mask (bus width)
    template <Core C> u32 addrMask() const;

    // Returns the cache register mask (accessible CACR bits)
    u32 cacrMask() const;

    
    //
    // Running the CPU
    //
    
public:
    
    // Performs a hard reset (power up)
    void reset();
    
    // Executes the next instruction
    void execute();
    
    // Returns true if the CPU is in HALT state
    bool isHalted() const { return flags & CPU_IS_HALTED; }
    
private:
    
    // Called by reset()
    template <Core C> void reset();
    
    // Invoked inside execute() to check for a pending interrupt
    bool checkForIrq();
    
    // Puts the CPU into HALT state
    void halt();
    
    
    //
    // Running the Disassembler
    //
    
public:
    
    // Disassembles a single instruction and returns the instruction size
    int disassemble(u32 addr, char *str, DasmStyle core = DASM_MUSASHI);
    
    // Returns a textual representation for a single word
    void disassembleWord(u32 value, char *str);
    
    // Returns a textual representation for one or more words from memory
    void disassembleMemory(u32 addr, int cnt, char *str);
    
    // Returns a textual representation for the program counter
    void disassemblePC(char *str) { disassemblePC(reg.pc, str); }
    void disassemblePC(u32 pc, char *str);
    
    // Returns a textual representation for the status register
    void disassembleSR(char *str) { disassembleSR(reg.sr, str); }
    void disassembleSR(const StatusRegister &sr, char *str);
    
    // Return an info struct for a certain opcode
    InstrInfo getInfo(u16 op); 
    
    
    //
    // Interfacing with other components
    //
    
protected:

#if VIRTUAL_API == true

    // Advances the clock
    virtual void sync(int cycles) { clock += cycles; }

    // Reads a byte or a word from memory
    virtual u8 read8(u32 addr) = 0;
    virtual u16 read16(u32 addr) = 0;

    // Special variants used by the reset routine and the disassembler
    virtual u16 read16OnReset(u32 addr) { return read16(addr); }
    virtual u16 read16Dasm(u32 addr) { return read16(addr); }

    // Writes a byte or word into memory
    virtual void write8(u32 addr, u8 val) = 0;
    virtual void write16(u32 addr, u16 val) = 0;

    // Provides the interrupt level in IRQ_USER mode
    virtual u16 readIrqUserVector(u8 level) const { return 0; }

    // State delegates
    virtual void signalHardReset() { }
    virtual void signalHalt() { }

    // Instruction delegates
    virtual void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }
    virtual void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode) { }
    
    // Exception delegates
    virtual void willExecute(ExceptionType exc, u16 vector) { }
    virtual void didExecute(ExceptionType exc, u16 vector) { }

    // Exception delegates
    virtual void signalInterrupt(u8 level) { }
    virtual void signalJumpToVector(int nr, u32 addr) { }
    virtual void signalSoftwareTrap(u16 opcode, SoftwareTrap trap) { }

    // Cache register delegated
    virtual void didChangeCACR(u32 value) { }
    virtual void didChangeCAAR(u32 value) { }

    // Called when a debug point is reached
    virtual void softstopReached(u32 addr) { }
    virtual void breakpointReached(u32 addr) { }
    virtual void watchpointReached(u32 addr) { }
    virtual void catchpointReached(u8 vector) { }
    virtual void softwareTrapReached(u32 addr) { }
    
#else

    // Advances the clock
    void sync(int cycles);

    // Reads a byte or a word from memory
    u8 read8(u32 addr);
    u16 read16(u32 addr);

    // Special variants used by the reset routine and the disassembler
    u16 read16OnReset(u32 addr);
    u16 read16Dasm(u32 addr);

    // Writes a byte or word into memory
    void write8(u32 addr, u8 val);
    void write16(u32 addr, u16 val);

    // Provides the interrupt level in IRQ_USER mode
    u16 readIrqUserVector(u8 level) const;

    // State delegates
    void signalHardReset();
    void signalHalt();

    // Instruction delegates
    void willExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);
    void didExecute(const char *func, Instr I, Mode M, Size S, u16 opcode);

    // Exception delegates
    void willExecute(ExceptionType exc, u16 vector);
    void didExecute(ExceptionType exc, u16 vector);

    // Exception delegates
    void signalInterrupt(u8 level);
    void signalJumpToVector(int nr, u32 addr);
    void signalSoftwareTrap(u16 opcode, SoftwareTrap trap);

    // Cache register delegated
    void didChangeCACR(u32 value);
    void didChangeCAAR(u32 value);

    // Called when a debug point is reached
    void softstopReached(u32 addr);
    void breakpointReached(u32 addr);
    void watchpointReached(u32 addr);
    void catchpointReached(u8 vector);
    void softwareTrapReached(u32 addr);

#endif

    //
    // Accessing the clock
    //
    
public:
    
    i64 getClock() const { return clock; }
    void setClock(i64 val) { clock = val; }

    
    //
    // Accessing registers
    //
    
public:
    
    u32 getD(int n) const { return readD(n); }
    void setD(int n, u32 v) { writeD(n,v); }
    
    u32 getA(int n) const { return readA(n); }
    void setA(int n, u32 v) { writeA(n,v); }
    
    u32 getPC() const { return reg.pc; }
    void setPC(u32 val) { reg.pc = val; }
    
    u32 getPC0() const { return reg.pc0; }
    void setPC0(u32 val) { reg.pc0 = val; }
    
    u16 getIRC() const { return queue.irc; }
    void setIRC(u16 val) { queue.irc = val; }
    
    u16 getIRD() const { return queue.ird; }
    void setIRD(u16 val) { queue.ird = val; }
    
    u8 getCCR() const { return getCCR(reg.sr); }
    void setCCR(u8 val);
    
    u16 getSR() const { return getSR(reg.sr); }
    void setSR(u16 val);
    
    u32 getSP() const { return reg.sp; }
    void setSP(u32 val) { reg.sp = val; }
    
    u32 getUSP() const { return !reg.sr.s ? reg.sp : reg.usp; }
    void setUSP(u32 val) { if (!reg.sr.s) reg.sp = val; else reg.usp = val; }
    
    u32 getISP() const { return (reg.sr.s && !reg.sr.m) ? reg.sp : reg.isp; }
    void setISP(u32 val) { if (reg.sr.s && !reg.sr.m) reg.sp = val; else reg.isp = val; }
    
    u32 getMSP() const { return (reg.sr.s && reg.sr.m) ? reg.sp : reg.msp; }
    void setMSP(u32 val) { if (reg.sr.s && reg.sr.m) reg.sp = val; else reg.msp = val; }
    
    u32 getVBR() const { return reg.vbr; }
    void setVBR(u32 val) { reg.vbr = val; }
    
    u32 getSFC() const { return reg.sfc; }
    void setSFC(u32 val) { reg.sfc = val & 0b111; }
    
    u32 getDFC() const { return reg.dfc; }
    void setDFC(u32 val) { reg.dfc = val & 0b111; }
    
    u32 getCACR() const { return reg.cacr; }
    void setCACR(u32 val);
    
    u32 getCAAR() const { return reg.caar; }
    void setCAAR(u32 val);
    
    void setSupervisorMode(bool value);
    void setMasterMode(bool value);
    void setSupervisorFlags(bool s, bool m);
    
    u8 getCCR(const StatusRegister &sr) const;
    u16 getSR(const StatusRegister &sr) const;
    
private:
    
    void setTraceFlag() { reg.sr.t1 = true; flags |= CPU_TRACE_FLAG; }
    void clearTraceFlag() { reg.sr.t1 = false; flags &= ~CPU_TRACE_FLAG; }
    
    void setTrace0Flag() { reg.sr.t0 = true; }
    void clearTrace0Flag() { reg.sr.t0 = false; }
    
    void clearTraceFlags() { clearTraceFlag(); clearTrace0Flag(); }
    
protected:
    
    template <Size S = Long> u32 readD(int n) const;
    template <Size S = Long> u32 readA(int n) const;
    template <Size S = Long> u32 readR(int n) const;
    template <Size S = Long> void writeD(int n, u32 v);
    template <Size S = Long> void writeA(int n, u32 v);
    template <Size S = Long> void writeR(int n, u32 v);


    //
    // Analyzing instructions
    //

    // Returns the availability mask for a given instruction
    u16 availabilityMask(Instr I);
    u16 availabilityMask(Instr I, Mode M, Size S);
    u16 availabilityMask(Instr I, Mode M, Size S, u16 ext);

    // Checks if the currently selected CPU model supports a given instruction
    bool isAvailable(Instr I);
    bool isAvailable(Instr I, Mode M, Size S);
    bool isAvailable(Instr I, Mode M, Size S, u16 ext);

private:

    // Checks the validity of the extension words
    bool isValidExt(Instr I, Mode M, u16 op, u32 ext);
    bool isValidExtMMU(Instr I, Mode M, u16 op, u32 ext);
    bool isValidExtFPU(Instr I, Mode M, u16 op, u32 ext);

    // Returns an availability string (used by the disassembler)
    const char *availabilityString(Instr I, Mode M, Size S, u16 ext);


    //
    // Managing the function code pins
    //
    
public:
    
    // Returns the current value on the function code pins
    FunctionCode readFC() const;
    
private:
    
    // Sets the function code pins to a specific value
    void setFC(FunctionCode value);
    
    // Sets the function code pins according the the provided addressing mode
    template <Mode M> void setFC();
    
    
    //
    // Handling interrupts
    //
    
public:
    
    u8 getIPL() const { return ipl; }
    void setIPL(u8 val);
    
private:
    
    // Polls the IPL pins
    void pollIpl() { reg.ipl = ipl; }
    
    // Selects the IRQ vector to branch to
    u16 getIrqVector(u8 level) const;


    //
    // Working with the MMU
    //

private:

    // Translates a logical address to a physical address
    template <Core C, bool write> u32 translate(u32 addr, u8 fc);



private:

#include "MoiraInit.h"
#include "MoiraALU.h"
#include "MoiraDataflow.h"
#include "MoiraExceptions.h"
#include "MoiraDasm.h"
};

}
