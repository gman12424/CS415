# splot.tcl 
option add *Width 10 
#
proc msg val { puts stdout "p .c"}
global noise
set noise "yes"
proc ring val { global noise; if {  $noise == "yes"} { bell}}
# Create canvas widget
canvas .c -width 300 -height 300 -bg black
pack .c -side top

#global noise
#set noise "yes"
#rectangles
.c create rectangle 0 0 100 100 -fill white -outline black -width 5 -tag tl
.c create rectangle 100 0 200 100 -fill white -outline black -width 5 -tag tm
.c create rectangle 200 0 300 100 -fill white -outline black -width 5 -tag tr
.c create rectangle 0 100 100 200 -fill white -outline black -width 5 -tag cl
.c create rectangle 100 100 200 200 -fill white -outline black -width 5 -tag cm
.c create rectangle 200 100 300 200 -fill white -outline black -width 5 -tag cr
.c create rectangle 0 200 100 300 -fill white -outline black -width 5 -tag bl
.c create rectangle 100 200 200 300 -fill white -outline black -width 5 -tag bm
.c create rectangle 200 200 300 300 -fill white -outline black -width 5 -tag br

.c bind tl <Button-1> { if {$status == "Status: Your Move"} {puts stdout "1"} else { ring 0}}
.c bind tm <Button-1> { if {$status == "Status: Your Move"} {puts stdout "2"} else { ring 0}}
.c bind tr <Button-1> { if {$status == "Status: Your Move"} {puts stdout "3"} else { ring 0}}
.c bind cl <Button-1> { if {$status == "Status: Your Move"} {puts stdout "4"} else { ring 0}}
.c bind cm <Button-1> { if {$status == "Status: Your Move"} {puts stdout "5"} else { ring 0}}
.c bind cr <Button-1> { if {$status == "Status: Your Move"} {puts stdout "6"} else { ring 0}}
.c bind bl <Button-1> { if {$status == "Status: Your Move"} {puts stdout "7"} else { ring 0}}
.c bind bm <Button-1> { if {$status == "Status: Your Move"} {puts stdout "8"} else { ring 0}}
.c bind br <Button-1> { if {$status == "Status: Your Move"} {puts stdout "9"} else { ring 0}}


#text
frame .lf
pack .lf -expand 1 -fill x
global status
set status "Status: Awaiting Match"
#.c create rectangle 0 300 300 400 -fill grey -outline black
label .lf.status -anchor w -text $status
label .lf.you -anchor w -text "You:"
label .lf.opponent -anchor w -text "Opponent: None"
pack .lf.status .lf.you .lf.opponent -side top -expand 1 -fill x

# Frame for holding buttons
frame .bf 
pack .bf -expand 1 -fill x



# Exit button 
button .bf.sound -text "Silent" -command { if {$noise == "yes"} {set noise "no"; .bf.sound configure -text "Sound"} else {set noise "yes"; .bf.sound configure -text "Silent"}}
# Reset button
button .bf.resign -text "Resign" -command { if {$status == "Status: Your Move"} {set status "Status: You Lose"; .lf.status configure -text $status; puts stdout "10"}}
# Dump postscript 
button .bf.exit -text "exit" -command { if {$status != "Status: Awaiting Match" && $status != "Status: Your Move" && $status != "Status: Awaiting Opponent Move"} {exit}}
# Pack buttons into frame
pack .bf.sound .bf.resign .bf.exit -side left -expand 1 -fill x 

#class msg:w
msg 0;


 
 
 
 
 
 
