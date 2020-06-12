#ifndef _SHA256_H
#define _SHA256_H

#include <linux/types.h>
#include "../kstd/kstdio.h"
void sha256(const unsigned char *data, size_t len, unsigned char *out);
//int ksha256(FILE *thefile, unsigned long offset, unsigned long len, unsigned char *out);
#endif /* sha256_h */