set verbose on
set trace-commands on
tui
layout split
file release/sniffer.elf 
file ../boot/release/u2s-485-thinkos-1.40.elf
target extended-remote 192.168.10.2:1000
