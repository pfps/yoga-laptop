/*
 * File:   print_utils.h
 * Author: buri
 *
 * Created on 2. květen 2014, 11:49
 */

#ifndef PRINT_UTILS_H
#define	PRINT_UTILS_H

#ifdef	__cplusplus
extern "C" {
#endif


void print2byte(int input, struct iio_channel_info *info) {
	/* First swap if incorrect endian */
	if (info->be)
		input = be16toh((uint16_t) input);
	else
		input = le16toh((uint16_t) input);

	/*
	 * Shift before conversion to avoid sign extension
	 * of left aligned data
	 */
	input = input >> info->shift;
	if (info->is_signed) {
		int16_t val = input;
		val &= (1 << info->bits_used) - 1;
		val = (int16_t) (val << (16 - info->bits_used)) >>
				(16 - info->bits_used);
		printf("SCALED %05f ", ((float) val + info->offset) * info->scale);
	} else {
		uint16_t val = input;
		val &= (1 << info->bits_used) - 1;
		printf("SCALED %05f ", ((float) val + info->offset) * info->scale);
	}
}

/**
 *
 */
int limit_interval(int min, int max, int nmr) {
	if (nmr < min) return min;
	if (nmr > max) return max;
	return nmr;
}

void print_bytes(int length, char* data) {
	int i;
	for (i = 0; i < length; i++) {
		if (i > 0) printf(":");
		printf("%02X", data[i]);
	}
	printf("\n");
}



#ifdef	__cplusplus
}
#endif

#endif	/* PRINT_UTILS_H */

