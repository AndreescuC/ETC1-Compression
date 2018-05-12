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
    int y = get_global_id(0);
    int x = get_global_id(1);

    matC [gid_1 * size + gid_0] = 0.f;
    float4 vecA;
    float4 vecB;
    float4 vecC;

    vecC = 0.f;

    for(int k = 0; k < size; k+=4) {
        vecA.s0 = matA [gid_0 * size + k + 0];
        vecA.s1 = matA [gid_0 * size + k + 1];
        vecA.s2 = matA [gid_0 * size + k + 2];
        vecA.s3 = matA [gid_0 * size + k + 3];

        vecB.s0 = matB [k * size + gid_1];
        vecB.s1 = matB [k * size + gid_1];
        vecB.s2 = matB [k * size + gid_1];
        vecB.s3 = matB [k * size + gid_1];

        vecC += vecA * vecB;
    }

    matC [gid_1 * size + gid_0] = vecC.s0 + vecC.s1 + vecC.s2 + vecC.s3;
}