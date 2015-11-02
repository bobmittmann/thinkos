# 1 "c:/devel/yard-ice/src/sdk/libcrc/crc16ccitt.c"
# 1 "c:\\devel\\yard-ice\\src\\sdk\\libcrc//"
# 1 "<command-line>"
# 1 "c:/devel/yard-ice/src/sdk/libcrc/crc16ccitt.c"
# 27 "c:/devel/yard-ice/src/sdk/libcrc/crc16ccitt.c"
unsigned int crc16ccitt(unsigned int crc, const void * buf, int len)
{
 unsigned char * cp;
 unsigned int c;
 int i;

 cp = (unsigned char *)buf;

 for (i = 0; i < len; i++) {
  c = cp[i];
  crc = (crc >> 8) | ((crc & 0xff) << 8);
  crc ^= c;
  crc ^= (crc & 0xff) >> 4;
  crc ^= (crc & 0x0f) << 12;
  crc ^= (crc & 0xff) << 5;
 }

 return crc;
}
