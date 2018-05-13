typedef uchar uint8_t;
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

void memcpycl(__global char *from,__global char *to, size_t n)
{
    for (int i=0; i<n; i++) {
        to[i] = from[i];
    }
}

__kernel void compress_func(__global uchar* src, __global uchar* dst, int width, int height)
{
    int y = get_global_id(0) * 4;
    int x = get_global_id(1) * 4;

    for (int i=0; i<5; i++) {
        dst[i] = 85;
    }
}