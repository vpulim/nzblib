#ifndef _CRC32_H
#define _CRC32_H

uint32_t crc32(const char *buf, size_t size);


uint32_t crc32_init();
uint32_t crc32_add(uint32_t crc, const char c);
uint32_t crc32_finish(uint32_t crc);

#endif
