#include <cstdio>
#include <cstdint>

union fl {
	float f;
	uint8_t c[4];
};

int main() {
  // 先确认一下 float 确实是4字节，uint8_t 是1字节
	printf("%ld %ld %ld\n", sizeof(float),sizeof(uint8_t), sizeof(fl));

	fl a;
	fl b;


	// 0xbe_80_00_00
	a.c[0] = 0x00;
	a.c[1] = 0x00;
	a.c[2] = 0x80;
	a.c[3] = 0xbe;

	// 0x80_ff_ff_ff
	b.c[0] = 0xff;
	b.c[1] = 0xff;
	b.c[2] = 0xff;
	b.c[3] = 0x80;



	// printf("a:%f\nb:%e\n", a.f,b.f);
	// float ret = a.f*b.f;
	// fl ret_fl;
	// ret_fl.f = ret;
	// printf("%x %x %x %x\n", ret_fl.c[3], ret_fl.c[2], ret_fl.c[1], ret_fl.c[0] );
	// printf("ret:%e\n", ret_fl.f);

// 11111111100000000100010010011010
// ff80449a

// b000449a
	fl ret_c;
	ret_c.c[0] = 0x9a;
	ret_c.c[1] = 0x44;
	ret_c.c[2] = 0x00;
	ret_c.c[3] = 0xb0;

	printf("%e\n", ret_c.f);
	return 0;
}

