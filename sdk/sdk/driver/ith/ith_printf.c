#include <stdarg.h>
#include "ite/ith.h"

/* flags used in processing format string */
#define PR_LJ   0x01            /* left justify */
#define PR_CA   0x02            /* use A-F instead of a-f for hex */
#define PR_SG   0x04            /* signed numeric conversion (%d vs. %u) */
#define PR_32   0x08            /* long (32-bit) numeric conversion */
#define PR_16   0x10            /* short (16-bit) numeric conversion */
#define PR_WS   0x20            /* PR_SG set and num was < 0 */
#define PR_LZ   0x40            /* pad left with '0' instead of ' ' */
#define PR_FP   0x80            /* pointers are far */
/* largest number handled is 2^32-1, lowest radix handled is 8.
2^32-1 in base 8 has 11 digits (add 5 for trailing NUL and for slop) */
#define PR_BUFLEN       32

static uint32_t _strlen(char *str)
{
    int i;
    for(i=0; str[i] != 0 && i<1024; i++);
    return i;
}

static uint32_t __div64_32(uint64_t * n, uint32_t base)
{
    uint64_t rem = *n;
    uint64_t b = base;
    uint64_t res, d = 1;
    uint32_t high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high /= base;
        res = (uint64_t) high << 32;
        rem -= (uint64_t) (high * base) << 32;
    }

    while ((int64_t) b > 0 && b < rem) {
        b = b + b;
        d = d + d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    *n = res;
    return (uint32_t)rem;
}

/*****************************************************************************
name:   do_printf
returns:total number of characters output
*****************************************************************************/
static __inline int do_printf(const char *fmt, va_list args)
{
    unsigned flags, actual_wd, count, given_wd;
    char *where, buf[PR_BUFLEN];
    unsigned char state, radix;
    uint64_t num;

    state = flags = count = given_wd = 0;
/* begin scanning format specifier list */
    for (; *fmt; fmt++) {
        switch (state) {
/* STATE 0: AWAITING % */
        case 0:
            if (*fmt != '%') {  /* not %... */
                ithPutcharFunc(*fmt);        /* ...just echo it */
                count++;
                break;
            }
/* found %, get next char and advance state to check if next char is a flag */
            state++;
            fmt++;
            /* FALL THROUGH */
/* STATE 1: AWAITING FLAGS (%-0) */
        case 1:
            if (*fmt == '%') {  /* %% */
                ithPutcharFunc(*fmt);
                count++;
                state = flags = given_wd = 0;
                break;
            }
            if (*fmt == '-') {
                if (flags & PR_LJ)      /* %-- is illegal */
                    state = flags = given_wd = 0;
                else
                    flags |= PR_LJ;
                break;
            }
/* not a flag char: advance state to check if it's field width */
            state++;
/* check now for '%0...' */
            if (*fmt == '0') {
                flags |= PR_LZ;
                fmt++;
            }
            /* FALL THROUGH */
/* STATE 2: AWAITING (NUMERIC) FIELD WIDTH */
        case 2:
            if (*fmt >= '0' && *fmt <= '9') {
                given_wd = 10 * given_wd + (*fmt - '0');
                break;
            }
/* not field width: advance state to check if it's a modifier */
            state++;
            /* FALL THROUGH */
/* STATE 3: AWAITING MODIFIER CHARS (FNlh) */
        case 3:
            if (*fmt == 'F') {
                flags |= PR_FP;
                break;
            }
            if (*fmt == 'N')
                break;
            if (*fmt == 'L') {
                flags |= PR_FP;
                break;
            }

            if (*fmt == 'l') {
                flags |= PR_32;
                break;
            }
            if (*fmt == 'h') {
                flags |= PR_16;
                break;
            }
/* not modifier: advance state to check if it's a conversion char */
            state++;
            /* FALL THROUGH */
/* STATE 4: AWAITING CONVERSION CHARS (Xxpndiuocs) */
        case 4:
            where = buf + PR_BUFLEN - 1;
            *where = '\0';
            switch (*fmt) {
            case 'X':
                flags |= PR_CA;
                /* FALL THROUGH */
/* xxx - far pointers (%Fp, %Fn) not yet supported */
            case 'x':
            case 'p':
            case 'n':
                radix = 16;
                goto DO_NUM;
            case 'd':
            case 'i':
                flags |= PR_SG;
                /* FALL THROUGH */
            case 'u':
                radix = 10;
                goto DO_NUM;
            case 'o':
                radix = 8;
/* load the value to be printed. l=long=32 bits: */
              DO_NUM:if (flags & PR_FP) {
                    num = va_arg(args, uint64_t);
                } else if (flags & PR_32) {
                    num = va_arg(args, unsigned long);
                }
/* h=short=16 bits (signed or unsigned) */
                else if (flags & PR_16) {
                    if (flags & PR_SG)
                        num = va_arg(args, int);
                    else
                        num = va_arg(args, unsigned int);
                }
/* no h nor l: sizeof(int) bits (signed or unsigned) */
                else {
                    if (flags & PR_SG)
                        num = va_arg(args, int);
                    else
                        num = va_arg(args, unsigned int);
                }
/* take care of sign */
                if (flags & PR_SG) {
                    if ((int64_t)num < 0) {
                        flags |= PR_WS;
                        num = 0-num;
                    }
                }
/* convert binary to octal/decimal/hex ASCII
OK, I found my mistake. The math here is _always_ unsigned */
                do {
                    uint64_t temp;

                    // temp = (uint64_t) num % radix;
                    temp = __div64_32(&num, (uint32_t)radix);
                    where--;
                    if (temp < 10)
                        *where = (char)temp + '0';
                    else if (flags & PR_CA)
                        *where = (char)temp - 10 + 'A';
                    else
                        *where = (char)temp - 10 + 'a';
                    // num = (uint64_t) num / radix;
                    //__div64_32(&num, (uint32_t)radix);
                }
                while (num != 0);
                goto EMIT;
            case 'c':
/* disallow pad-left-with-zeroes for %c */
                flags &= ~PR_LZ;
                where--;
                *where = (char) va_arg(args, unsigned int);
                actual_wd = 1;
                goto EMIT2;
            case 's':
/* disallow pad-left-with-zeroes for %s */
                flags &= ~PR_LZ;
                where = va_arg(args, char *);
              EMIT:
                actual_wd = _strlen(where);
                if (flags & PR_WS)
                    actual_wd++;
/* if we pad left with ZEROES, do the sign now */
                if ((flags & (PR_WS | PR_LZ)) == (PR_WS | PR_LZ)) {
                    ithPutcharFunc('-');
                    count++;
                }
/* pad on left with spaces or zeroes (for right justify) */
              EMIT2:if ((flags & PR_LJ) == 0) {
                    while (given_wd > actual_wd) {
                        ithPutcharFunc(flags & PR_LZ ? '0' : ' ');
                        count++;
                        given_wd--;
                    }
                }
/* if we pad left with SPACES, do the sign now */
                if ((flags & (PR_WS | PR_LZ)) == PR_WS) {
                    ithPutcharFunc('-');
                    count++;
                }
/* emit string/char/converted number */
                while (*where != '\0') {
                    ithPutcharFunc(*where++);
                    count++;
                }
/* pad on right with spaces (for left justify) */
                if (given_wd < actual_wd)
                    given_wd = 0;
                else
                    given_wd -= actual_wd;
                for (; given_wd; given_wd--) {
                    ithPutcharFunc(' ');
                    count++;
                }
                break;
            default:
                break;
            }
        default:
            state = flags = given_wd = 0;
            break;
        }
    }
    return count;
}

int ithPrintf(const char* fmt, ...)
{
    va_list args;
    int rv;

    va_start(args, fmt);
    rv = do_printf(fmt, args);
    va_end(args);
    return rv;
}

int ithPutcharDefault(int c)
{
    return c;
}

int (*ithPutcharFunc)(int c) = ithPutcharDefault;
