file kernel/system
target remote localhost:1234
# b kernel/main.c:kernel_main
# b kernel/syscall.c:syscall_handler

b mp_main
def link
    target remote localhost:1234
end

def ss
    step
    info thread
end

def nn
    next
    info thread
end
