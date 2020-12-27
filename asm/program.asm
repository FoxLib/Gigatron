
        ld  ac, ac
        ld  y, $00
        ld  x, $ff
        st  $af, [y,x++]
        ld  x, [$ff]
        bra ac
        bra l1
l1:     ld  x, ac
        bra $-1
        ld  ac, $00
m1:     ld  ac, $21
m2:     ld  ac, $44
