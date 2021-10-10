
/**
 * @file bswap.h
 * byte swap.
 */

#ifndef __BSWAP_H__
#  define __BSWAP_H__

#  ifdef ROCKBOX

/* rockbox' optimised inline functions */
#    define bswap_16(x) swap16(x)
#    define bswap_32(x) swap32(x)

static __inline uint64_t ByteSwap64(uint64_t x)
{
    union {
        uint64_t ll;
        struct {
            uint32_t l, h;
        } l;
    } r;

    r.l.l = bswap_32(x);
    r.l.h = bswap_32(x >> 32);
    return r.ll;
}

#    define bswap_64(x) ByteSwap64(x)

#  elif defined(ARCH_X86)
static __inline unsigned short ByteSwap16(unsigned short x)
{
  __asm("xchgb %b0,%h0": "=q"(x):"0"(x));
    return x;
}

#    define bswap_16(x) ByteSwap16(x)

static __inline unsigned int ByteSwap32(unsigned int x)
{
#    if __CPU__ > 386
  __asm("bswap   %0": "=r"(x):"0"(x));
#    else
  __asm("xchgb   %b0,%h0\n" " rorl    $16,%0\n" " xchgb   %b0,%h0": "=q"(x):"0"(x));
#    endif
    return x;
}

#    define bswap_32(x) ByteSwap32(x)

static __inline uint64_t ByteSwap64(uint64_t x)
{
    register union {
        __extension__ uint64_t __ll;
        uint32_t __l[2];
    } __x;

  asm("xchgl    %0,%1": "=r"(__x.__l[0]), "=r"(__x.__l[1]):
"0"(bswap_32((unsigned long) x)), "1"(bswap_32((unsigned long) (x >> 32))));
    return __x.__ll;
}

#    define bswap_64(x) ByteSwap64(x)

#  elif defined(ARCH_SH4)

static __inline uint16_t ByteSwap16(uint16_t x)
{
  __asm__("swap.b %0,%0": "=r"(x):"0"(x));
    return x;
}

static __inline uint32_t ByteSwap32(uint32_t x)
{
  __asm__("swap.b %0,%0\n" "swap.w %0,%0\n" "swap.b %0,%0\n": "=r"(x):"0"(x));
    return x;
}

#    define bswap_16(x) ByteSwap16(x)
#    define bswap_32(x) ByteSwap32(x)

static __inline uint64_t ByteSwap64(uint64_t x)
{
    union {
        uint64_t ll;
        struct {
            uint32_t l, h;
        } l;
    } r;

    r.l.l = bswap_32(x);
    r.l.h = bswap_32(x >> 32);
    return r.ll;
}

#    define bswap_64(x) ByteSwap64(x)

#  else

#    define bswap_16(x) ((((x) << 8) & 0xff00) | (((x) >> 8) & 0x00ff))
#    define bswap_32(x) \
              ((((x) >> 24) & 0x000000ff) | (((x) >> 8 ) & 0x0000ff00) | \
               (((x) << 8 ) & 0x00ff0000) | (((x) << 24) & 0xff000000))

static __inline uint64_t ByteSwap64(uint64_t x)
{
    union {
        uint64_t ll;
        uint32_t l[2];
    } w, r;

    w.ll = x;
    r.l[0] = (uint32_t) bswap_32(w.l[1]);
    r.l[1] = (uint32_t) bswap_32(w.l[0]);
    return r.ll;
}

#    define bswap_64(x) ByteSwap64(x)

#  endif                        /* !ARCH_X86 */

// be2me ... BigEndian to MachineEndian
// le2me ... LittleEndian to MachineEndian

#  ifdef WORDS_BIGENDIAN
#    define be2me_16(x) (x)
#    define be2me_32(x) (x)
#    define be2me_64(x) (x)
#    define le2me_16(x) bswap_16(x)
#    define le2me_32(x) bswap_32(x)
#    define le2me_64(x) bswap_64(x)
#  else
#    define be2me_16(x) bswap_16(x)
#    define be2me_32(x) bswap_32(x)
#    define be2me_64(x) bswap_64(x)
#    define le2me_16(x) (x)
#    define le2me_32(x) (x)
#    define le2me_64(x) (x)
#  endif

#endif                          // __BSWAP_H__
