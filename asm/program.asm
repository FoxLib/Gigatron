        ab  = l3

        ; back porch 48
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]
        st  $c0,[y,x++]

        ; sync


        st  $ff,[y,x++]

        or  out,[y,x++]
        ble l1
        st  ac, [y,x]
        st  ac, [y,x++]
        st  ac, [$44],x
        st  ac, [$53]

        ld  ac, [ab]
        ld  ac, [x]
        ld  ac, [y,$af]
        ld  x, [$77]
        ld  y, [l3]
        ld  out, [$12]
        ld  out, [y,x++]
        ld  out, ac
l1:     jmp y,[l2]
        st  ac,[$ee],x
l2:     ld  ac, $12
        ld  x, ac
l3:     add ac, [$23]
