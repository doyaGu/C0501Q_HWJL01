#ifndef ITP_SYS_SOCKIO_H
#define ITP_SYS_SOCKIO_H

#ifdef __cplusplus
extern "C"
{
#endif

#define	SIOCGIFFLAGS	(1000 + 17) /* get ifnet flags */
#define	SIOCGIFADDR	    (1000 + 33)	/* get ifnet address */
#define	SIOCGIFCONF     (1000 + 36) /* get ifnet list */
#define	SIOCGIFNETMASK	(1000 + 37)	/* get net addr mask */
#define SIOCGIFHWADDR   (1000 + 105) /* Get hw addr */
#define SIOCGIFINDEX    (0x8933) /* name -> if_index mapping     */
/* The first and the last (range) */
#define SIOCIWFIRST	0x8B00
#define SIOCIWLAST	0x8BFF
#define SIOCIWFIRSTPRIV 0x8BE0


#ifdef __cplusplus
}
#endif

#endif // ITP_SYS_SOCKIO_H
