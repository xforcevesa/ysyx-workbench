#include "Vtop.h"          // Generated from Verilator
#include "verilated.h"     // Verilator header
#include "verilated_vcd_c.h" // For VCD waveform tracing

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);  // Pass arguments to Verilator
    Vtop *top = new Vtop;  // Instantiate the Verilog module

    // Initialize waveform tracing
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;
    top->trace(tfp, 99);  // Trace the top module
    tfp->open("wave.vcd"); // Create waveform output file

    // Simulation inputs
    top->a = 0;
    top->b = 0;
    top->eval(); // Evaluate the initial state
    tfp->dump(0); // Dump time 0

    // Cycle 1: Apply inputs
    top->a = 0; top->b = 1;
    top->eval();
    tfp->dump(1); // Dump time 1

    // Cycle 2: Apply inputs
    top->a = 1; top->b = 0;
    top->eval();
    tfp->dump(2); // Dump time 2

    // Cycle 3: Apply inputs
    top->a = 1; top->b = 1;
    top->eval();
    tfp->dump(3); // Dump time 3

    // Cycle 4: Apply inputs
    top->a = 0; top->b = 0;
    top->eval();
    tfp->dump(4); // Dump time 4

    // Close trace and cleanup
    tfp->close();
    delete top;
    return 0;
}
