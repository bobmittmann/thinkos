set verbose on
set trace-commands on
set tdesc filename target.xml
target extended-remote 192.168.10.2:1000
#set debug remote 1
tui enable
layout split
set tui mouse-events

