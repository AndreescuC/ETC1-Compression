typedef __global uchar global_uint8_t;
typedef uint uint32_t;
typedef ushort uint16_t;

union Color {
    struct BgraColorType {
        uint8_t b;
        uint8_t g;
        uint8_t r;
        uint8_t a;
    } channels;
    uint8_t components[4];
    uint32_t bits;
};

__kernel void compress_func(global_uint8_t src, global_uint8_t dst, int width, int height)
{
    printf("Entered kernel\n");
    int y = get_global_id(0) * 4;
    int x = get_global_id(1) * 4;

    for (int i=0; i<5; i++) {
        dst[i] = 85;
    }
    printf("Exited kernel\n");
}