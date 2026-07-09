set verbose on
#set trace-commands on
#set tdesc filename target.xml
#set debug remote 1
tui enable
layout split
set tui mouse-events

target extended-remote 192.168.10.2:1000
