set verbose on
set trace-commands on
set tdesc filename target.xml
target extended-remote 192.168.10.2:1000
set remotelogfile my_rsp_packets.log
set debug remote 1
