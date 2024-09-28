all:
	@echo "Write this Makefile by your self."
	verilator -Wall --trace --cc vsrc/*.v --exe csrc/*.cpp -O2
	make -C obj_dir -f Vtop.mk Vtop

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	./obj_dir/Vtop
	gtkwave wave.vcd


include ../Makefile
