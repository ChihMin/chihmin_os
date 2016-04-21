file kernel/system
target remote localhost:1234
b kernel/mem.c:mem_init

def link
    target remote localhost:1234
end
