#ifndef ITP_BYTESWAP_H
#define ITP_BYTESWAP_H

#ifdef __cplusplus
extern "C" {
#endif

static __inline unsigned short
itpBswap16(unsigned short __x)
{
  return (__x >> 8) | (__x << 8);
}
#define bswap_16 itpBswap16

static __inline unsigned int
itpBswap32(unsigned int __x)
{
  return (bswap_16 (__x & 0xffff) << 16) | (bswap_16 (__x >> 16));
}
#define bswap_32 itpBswap32

static __inline unsigned long long
itpBswap64(unsigned long long __x)
{
  return (((unsigned long long) bswap_32 (__x & 0xffffffffull)) << 32) | (bswap_32 (__x >> 32));
}
#define bswap_64 itpBswap64

#ifdef __cplusplus
}
#endif
#endif /* ITP_BYTESWAP_H */
