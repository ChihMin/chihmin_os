file kernel/system
target remote localhost:1234
# b kernel/main.c:kernel_main
# b kernel/syscall.c:syscall_handler
b fs_test
def link
    target remote localhost:1234
end
