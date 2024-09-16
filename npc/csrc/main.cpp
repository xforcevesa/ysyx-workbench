#include "Vtop.h"          // Generated from Verilator
#include "verilated.h"     // Verilator header
#include "verilated_vcd_c.h" // For VCD waveform tracing
// For rand()
#include <cstdlib>

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

    for (int i = 1; i < 100; i++) {
        // Cycle i: Apply inputs
        top->a = rand() & 1; top->b = rand() & 1;
        top->eval();
        tfp->dump(i); // Dump time 1
        printf("a = %d, b = %d, f = %d\n", top->a, top->b, top->f);
        assert(top->f == (top->a ^ top->b));
    }   

    // Close trace and cleanup
    tfp->close();
    delete top;
    return 0;
}
