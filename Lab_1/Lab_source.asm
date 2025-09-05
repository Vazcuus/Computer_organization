[bits 16]
[org 0x7c00]        ; адрес старта программы

start:
    mov si, my_lastname  ; загрузка адреса строки в регистр SI
    call print_string   ; вызов процедуры печати
	
	mov si, my_group  ; загрузка адреса строки с группой
    call print_string
	
    jmp $   

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

times 510-($-$$) db 0   ; заполняем нулями до 510 байта
dw 0xaa55               ; сигнатура загрузчика