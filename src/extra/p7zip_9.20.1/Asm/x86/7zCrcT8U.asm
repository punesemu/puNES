
SECTION .text

%macro CRC1b 0
    movzx EDX, BYTE [ESI]
    inc ESI
    movzx EBX, AL
    xor EDX, EBX
    shr EAX, 8
    xor EAX, [EBP + EDX * 4]
    dec EDI
%endmacro

data_size equ (28)
crc_table equ (data_size + 4)

align 16
global CrcUpdateT8
global _CrcUpdateT8
CrcUpdateT8:
_CrcUpdateT8:
    push EBX
    push ESI
    push EDI
    push EBP

    mov	EAX, [ESP + 20]        ; CRC
    mov	ESI, [ESP + 24]        ; buf
    mov	EDI, [ESP + data_size] ; size
    mov EBP, [ESP + crc_table] ; tables

    test EDI, EDI
    jz sl_end
  sl:
    test ESI, 7
    jz sl_end
    CRC1b
    jnz sl
  sl_end:

    cmp EDI, 16
    jb NEAR crc_end
    mov [ESP + data_size], EDI
    sub EDI, 8
    and EDI, ~ 7
    sub [ESP + data_size], EDI

    add EDI, ESI
    xor EAX, [ESI]
    mov EBX, [ESI + 4]
    movzx ECX, BL
    align 16
  main_loop:
    mov EDX, [EBP + ECX*4 + 0C00h]
    movzx ECX, BH
    xor EDX, [EBP + ECX*4 + 0800h]
    shr EBX, 16
    movzx ECX, BL
    xor EDX, [EBP + ECX*4 + 0400h]
    xor EDX, [ESI + 8]
    movzx ECX, AL
    movzx EBX, BH
    xor EDX, [EBP + EBX*4 + 0000h]

    mov EBX, [ESI + 12]

    xor EDX, [EBP + ECX*4 + 01C00h]
    movzx ECX, AH
    add ESI, 8
    shr EAX, 16
    xor EDX, [EBP + ECX*4 + 01800h]
    movzx ECX, AL
    xor EDX, [EBP + ECX*4 + 01400h]
    movzx ECX, AH
    mov EAX, [EBP + ECX*4 + 01000h]
    movzx ECX, BL
    xor EAX,EDX

    cmp ESI, EDI
    jne	main_loop
    xor	EAX, [ESI]

    mov EDI, [ESP + data_size]

  crc_end:

    test EDI, EDI
    jz fl_end
  fl:
    CRC1b
    jnz fl
  fl_end:

    pop EBP
    pop EDI
    pop ESI
    pop EBX
    ret ; 8


; end
