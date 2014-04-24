
    .text

/*Internal use! DO NOT call it directly!!!*/

    .global flush_dcache_all
flush_dcache_all:
    mov     r0, #0
    mcr     p15, 0, r0, c7, c14, 0          @ D cache clean+invalidate
    mcr     p15, 0, r0, c7, c10, 4          @ drain write buffer
    bx      lr 

    .end

