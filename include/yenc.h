#ifndef _YENC_H
#define _YENC_H

#include "types.h"

int yenc_decode(segment_t *segment);
uint32_t yenc_parse_yend(char *line);
void yenc_parse_ypart(char *line, segment_t *segment);
void yenc_parse_ybegin(char *line, segment_t *segment);
#endif
