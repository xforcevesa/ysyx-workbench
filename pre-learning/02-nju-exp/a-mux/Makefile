all:
	verilator -Wall --trace --cc *.v --exe *.c -O2
	make -C obj_dir -f Vmux21.mk Vmux21

sim:
	./obj_dir/Vmux21
	gtkwave wave.vcd
