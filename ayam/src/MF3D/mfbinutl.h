/*
** written 21/04/98 by Munk for RANDi (rschultz@informatik.uni-rostock.de)
*/

/*
** change byte order (based on macros from the IJG sources v5.0b)
*/

#define UCH(x) \
  ((unsigned long) (x))

#define GET_2B(array,offset) \
  ((unsigned long) UCH(array[offset]) + \
   (((unsigned long) UCH(array[offset+1])) << 8))

#define GET_4B(array,offset) \
  ((unsigned long) UCH(array[offset]) + \
   (((unsigned long) UCH(array[offset+1])) << 8) + \
   (((unsigned long) UCH(array[offset+2])) << 16) + \
   (((unsigned long) UCH(array[offset+3])) << 24))

#define PUT_2B(array,offset,value)  \
  { unsigned long v = value; \
    array[offset+1] = (unsigned char)((v) & 0xff); \
    array[offset+0] = (unsigned char)((v>>=8) & 0xff); }

#define PUT_4B(array,offset,value)  \
  { unsigned long v = value; \
    array[offset+3] = (unsigned char)((v) & 0xff); \
    array[offset+2] = (unsigned char)((v>>=8) & 0xff); \
    array[offset+1] = (unsigned char)((v>>=8) & 0xff); \
    array[offset+0] = (unsigned char)((v>>=8) & 0xff); }

#define MF3D_ByteSwap2(n) \
 { unsigned char *p = (unsigned char *)&n; PUT_2B(p,0,GET_2B(p,0)); }

#define	MF3D_ByteSwap4(n) \
 { unsigned char *p = (unsigned char *)&n; PUT_4B(p,0,GET_4B(p,0)); }

#define	MF3D_ByteSwap8(n) \
 { unsigned char *p = (unsigned char *)&n; \
   unsigned long v = GET_4B(p,0); \
   PUT_4B(p,0,GET_4B(p,4)); PUT_4B(p,4,v); }

/* original ByteSwap-macro from mfbinutl.h */
#define	MF3D_ByteSwap(bytes, n)		MFASSERT(bytes == 8 || bytes == 4 ||	\
											bytes == 2);					\
									if (bytes == 8)							\
									{	MF3D_ByteSwap8(n);					\
									}										\
									else if (bytes == 4)					\
									{	MF3D_ByteSwap4(n);					\
									}										\
									else if (bytes == 2)					\
									{	MF3D_ByteSwap2(n);					\
									}										\
									else									\
									{	MFASSERT(bytes);					\
									}

#ifdef MACRO_TEST

int main(int ac,char *av[])
{ unsigned long k2 = 0x12345678;
  unsigned long k1 = 0x1234;

  printf("0x%lx\n",k1);
  MF3D_ByteSwap2(k1);
  printf("0x%lx\n",k1);

  printf("0x%lx\n",k2);
  MF3D_ByteSwap4(k2);
  printf("0x%lx\n",k2);
  
  return 0;
}

#endif
