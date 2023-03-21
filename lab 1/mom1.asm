start:  
    LDR R0,=0x18        @lägg till din kod här
    LDR R1,=0x40023830
    STR R0,[R1]         @ aktivera portar D och E
    
    LDR R0,=0x55555555
    LDR R1,=0x40020C00  @ config port D
    STR R0,[R1]
    LDR R1,=0x40020C14  @ outport port D
    LDR R2,=0x40021010  @ inport port E

main:   
    LDR R0,[R2]
    STR R0,[R1]
    B main
