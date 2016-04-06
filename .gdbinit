file kernel/system
target remote localhost:1234
b kernel/trap_entry.S:kbd_isr_func
