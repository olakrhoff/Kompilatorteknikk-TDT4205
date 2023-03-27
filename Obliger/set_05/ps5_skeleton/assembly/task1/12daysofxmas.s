.globl _main

.section __TEXT, __cstring
_intro: .string "On day %ld of Christmas my true love sent to me\n"
_and_s: .string "and "

_str01: .string "%ld partridge in a pear tree\n"
_str02: .string "%ld turtle doves\n"
_str03: .string "%ld french hens\n"
_str04: .string "%ld calling birds\n"
_str05: .string "%ld gold rings\n"
_str06: .string "%ld geese a-laying\n"
_str07: .string "%ld swans a-swimming\n"
_str08: .string "%ld maids a-milking\n"
_str09: .string "%ld ladies dancing\n"
_str10: .string "%ld lords a-leaping\n"
_str11: .string "%ld pipers piping\n"
_str12: .string "%ld drummers drumming\n"
_skip_line: .string "\n"

#MacOS does NOT like this part
#_lines:
 #   .quad _str01, _str02, _str03, _str04, _str05, _str06
  #  .quad _str07, _str08, _str09, _str10, _str11, _str12

.section __TEXT, __text
_main:
    pushq %rbp
    movq %rsp, %rbp

    /*
        TODO: Task 1 - Put your code here.

        Your task: 
        Implement a GNU Assembly program that prints a alightly modified version of the "Twelve Days of Christmas" song.
        Instead of "first day, second day" or "1st day, 2nd day" etc, your program may output "On day N of Christmas my true love gave to me".
        This song is cumulative - each day will include the gifts of every previous day.
        (i.e. day 1 will include only a partridge, while day 2 will include two turtle doves AND a partridge).
        Use printf to output the lyrics of the song to terminal.
        A suggestion for the first lines of the output is added below.


        HINTS:
        - The registers %rdi and %rsi are used for the first two arguments in a function call (like printf).
        - Some registers are overwritten by functions (including printf!) This includes %rax, %rbx, %rcx, %rdx, %rdi, %rsi, %rbp, %rsp, and %r8-r15.
        - The order of comparisons are (for some reason) reversed! This means that cmp %r8, %r9 will compare r9 vs r8.
        - Create a debug helper string that you can call with printf for slightly easier debugging.
        - Consult the examples in (../examples/) for inspiration. Make them with `make`.
        - Use a cheat sheet like https://flint.cs.yale.edu/cs421/papers/x86-asm/asm.html or https://cs.brown.edu/courses/cs033/docs/guides/x64_cheatsheet.pdf
        - Compile this with `make` once finished.


        BONUS POINTS (Can only be exchanged for bragging rights)
        - Use putchar to easily create new lines so that the finished song doesn't become a giant wall of text.
        - Make the song include "and" before the last line - i.e., "AND 1 partridge in a pear tree" - in every verse except the first.


        SUGGESTED OUTPUT:
        On day 1 of Christmas my true love sent to me
        1 partridge in a pear tree

        On day 2 of Christmas my true love sent to me
        2 turtle doves
        and 1 partridge in a pear tree

        On day 3 of Christmas my true love sent to me
        3 french hens
        2 turtle doves
        and 1 partridge in a pear tree

        ...
    */
    
    movq $0, %r12 # Set outer counter
    start_loop:
    inc %r12
    movq $12, %r13 # Inner counter
    movq %r12, %rsi
    leaq _intro(%rip), %rdi # Set arg 1
    call _printf

    # Jump to the correct start of listing
    cmp %r13, %r12
    jz num12
    dec %r13
    cmp %r13, %r12
    jz num11
    dec %r13
    cmp %r13, %r12
    jz num10
    dec %r13
    cmp %r13, %r12
    jz num9
    dec %r13
    cmp %r13, %r12
    jz num8
    dec %r13
    cmp %r13, %r12
    jz num7
    dec %r13
    cmp %r13, %r12
    jz num6
    dec %r13
    cmp %r13, %r12
    jz num5
    dec %r13
    cmp %r13, %r12
    jz num4
    dec %r13
    cmp %r13, %r12
    jz num3
    dec %r13
    cmp %r13, %r12
    jz num2
    dec %r13
    jmp num1

    # Print all days from the day that is jump to by label above
    num12:
    leaq _str12(%rip), %rdi
    movq %r13, %rsi
    call _printf
    dec %r13
    num11:
    leaq _str11(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num10:
    leaq _str10(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num9:
    leaq _str09(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num8:
    leaq _str08(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num7:
    leaq _str07(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num6:
    leaq _str06(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num5:
    leaq _str05(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num4:
    leaq _str04(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num3:
    leaq _str03(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num2:
    leaq _str02(%rip), %rdi 
    movq %r13, %rsi
    call _printf
    dec %r13
    num1:
    leaq _str01(%rip), %rdi # Set arg 1
    movq %r13, %rsi
    call _printf
  
    # If we have done 12 days, we exit
    cmp $12, %r12
    jz done

    # Did not exit, add newline
    skip_line:
    leaq _skip_line(%rip), %rdi
    call _printf
    jmp start_loop
   
    done:
    leave
    ret
