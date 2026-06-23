tui enable
layout split
set verbose on
set trace-commands on
set debug remote 1
set logging file gdb_remote.log
set logging debugredirect on
set logging enabled on
#set tui mouse-events
target extended-remote 192.168.10.2:1000
set tdesc filename target.xml

