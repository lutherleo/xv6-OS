.globl thread_switch
thread_switch:
    movl 4(%esp), %eax

    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %ebx
    
    movl %esp, 4104(%eax)
    
    movl 8(%esp), %eax
    movl 4104(%eax), %esp
    
    popl %ebx
    popl %esi
    popl %edi
    popl %ebp
    
    ret