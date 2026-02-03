#include <stdarg.h>


static void itoa(unsigned int value, char* str, int base) {
    char *ptr = str;
    
    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    if (base == 16) {
        // Base 16 continua rápida com Bitwise (sempre use isso para Hex)
        const char *hex_chars = "0123456789abcdef";
        char temp[10];
        int i = 0;
        while (value > 0) {
            temp[i++] = hex_chars[value & 0xF];
            value >>= 4;
        }
        while (i > 0) *ptr++ = temp[--i];
        *ptr = '\0';
    } 
    else if (base == 10) {
        // Tabela de potências de 10 para evitar divisões e loops longos
        static const unsigned int powers[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
        int started = 0;

        for (int i = 0; i < 10; i++) {
            unsigned int count = 0;
            unsigned int p = powers[i];
            
            // Subtrai a potência atual o máximo de vezes possível
            while (value >= p) {
                value -= p;
                count++;
            }

            if (count > 0 || started || p == 1) {
                *ptr++ = count + '0';
                started = 1; // Evita zeros à esquerda (ex: 000123)
            }
        }
        *ptr = '\0';
    }
}

// Tipo de função que aceita um caractere
typedef void (*putc_func_t)(char);

void k_vprintf_internal(putc_func_t putc_func, const char *fmt, va_list args) {
    char buf[32];
    char c;

    while ((c = *fmt++)) {
        if (c != '%') {
            putc_func(c);
            continue;
        }

        c = *fmt++;
        switch (c) {
            case 's': {
                char *s = va_arg(args, char *);
                while (s && *s) putc_func(*s++);
                break;
            }
            case 'd': {
                int d = va_arg(args, int);
                if (d < 0) {
                    putc_func('-');
                    d = -d;
                }
                itoa((unsigned int)d, buf, 10);
                for (int i = 0; buf[i]; i++) putc_func(buf[i]);
                break;
            }
            case 'x': {
                unsigned int x = va_arg(args, unsigned int);
                itoa(x, buf, 16);
                for (int i = 0; buf[i]; i++) putc_func(buf[i]);
                break;
            }
            case 'c': {
                char ch = (char)va_arg(args, int);
                putc_func(ch);
                break;
            }
            default:
                putc_func('%');
                if (c) putc_func(c);
                break;
        }
    }
}
