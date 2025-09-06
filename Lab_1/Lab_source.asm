[bits 16]
[org 0x7c00]        ; адрес старта программы

start:
    mov si, my_lastname  ; загрузка адреса строки в регистр SI
    call print_string   ; вызов процедуры печати
	
	mov si, my_group  ; загрузка адреса строки с группой
    call print_string


    call input_number
    call reverse_number
    call print_reversed

	
    jmp $   

input_number:
    mov si, prompt_msg
    call print_string
    mov di, input_buffer
    mov cx, 4
.input_loop:
    mov ah, 0x00
    int 0x16
    cmp al, '0'
    jb .input_loop
    cmp al, '9'
    ja .input_loop
    stosb ; сохранить байт из AL в [DI] и увеличить DI
    mov ah, 0x0e
    int 0x10
    loop .input_loop
    ret

; Процедура переворота числа
reverse_number:
    mov si, input_buffer
    mov di, reversed_buffer + 3
    mov cx, 4
.reverse_loop:
    lodsb ; загружаем байт из [SI] в AL и увеличиваем SI
    mov [di], al
    dec di
    loop .reverse_loop
    ret
; Процедура печати перевернутого числа
print_reversed:
    mov si, result_msg
    call print_string
    mov si, reversed_buffer
    call print_string
    ret

print_string:	
    mov ah, 0x0e        ; функция teletype BIOS
.print_loop:
    lodsb               ; загружаем байт из [SI] в AL и увеличиваем SI
    cmp al, 0           ; проверяем конец строки
    je .done            ; если конец строки - выходим
    int 0x10            ; выводим символ
    jmp .print_loop     ; продолжаем цикл
.done:
    ret

my_lastname:
    db "Konstantinov ", 0 ; строка с нулевым терминатором
my_group:
	db "8B31 ", 0 ; строка с нулевым терминатором
prompt_msg:
    db 0x0D, 0x0A, "Enter 4-digit number: ", 0
result_msg:
    db 0x0D, 0x0A, "Reversed: ", 0
input_buffer:
    times 5 db 0
reversed_buffer:
    times 5 db 0

times 510-($-$$) db 0   ; заполняем нулями до 510 байта
dw 0xaa55               ; сигнатура загрузчика