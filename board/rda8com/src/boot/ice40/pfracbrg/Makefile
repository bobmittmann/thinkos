MODULE=pfracbrg

MODULE_TB = $(MODULE)_tb
MODULE_VFILES = fracbrg.v $(MODULE).v $(MODULE_TB).v 
MODULE_VCD = $(MODULE_TB).vcd

all: vcd

clean:
	rm -f $(MODULE_TB)
	rm -f $(MODULE_VCD)

run:
	vvp -v $(MODULE_TB) 
	vvp -v $(MODULE_VCD)

tb: $(MODULE_TB) 

vcd: $(MODULE_VCD) 

.PHONY: all clean run tb vcd

$(MODULE_TB): $(MODULE_VFILES)
	iverilog -Wall -s $@ -o $@ $^

$(MODULE_VCD): $(MODULE_TB)
	vvp -v -n $<

