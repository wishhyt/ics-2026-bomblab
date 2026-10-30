#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

static uint32_t phase4_walk(uint32_t target, uint32_t lo, uint32_t hi,
                            uint32_t salt) {
    uint32_t mid = lo + (hi - lo) / 2u;
    if (target == mid) return (mid + 0xedu) ^ salt;
    uint32_t branch = mid + salt;
    if (target < mid)
        return (5u * phase4_walk(target, lo, mid - 1u, salt) ^ branch) ^ 0xfu;
    return (5u * phase4_walk(target, mid + 1u, hi, salt) ^ branch) ^ 0x1eu;
}

static uint32_t phase6_op(uint32_t index, uint32_t x) {
    switch (index) {
        case 0: return (x >> 7 | x << 25);
        case 1: return ((x >> 5) ^ x) + 0xd120u;
        case 2: return ((x >> 5) ^ x) + 0xd120u;
        case 3: return (x << 3 | x >> 29) ^ 0x632cu;
        case 4: return ((x >> 5) ^ x) + 0xd120u;
        case 5: return (x + 0x1d04u) ^ (x >> 1);
        case 6: return (x ^ 0xeb00u) + (x << 7);
        default: return (4u * x + 0xf8a1u) ^ (3u * x);
    }
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t low = (uint32_t)challenge;
    uint32_t middle = (uint32_t)(challenge >> 16);
    uint32_t high = (uint32_t)(challenge >> 32);

    uint32_t step2 = 5u * (high - low) + 0x6d61u;
    uint32_t step0 = (low + 2u * middle) ^ 0x4aaau;
    values[0] = (5u * step0 + step2) ^ 0xe3bau;
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    values[0] = 13u + 2u * (uint32_t)((challenge >> 38) & 15u);
    for (uint32_t i = 1; i < 6; ++i) {
        uint32_t shift = 22u + 5u * (i - 1u);
        uint32_t part = (uint32_t)((challenge >> shift) & 63u);
        values[i] = 5u * (values[i - 1u] + i) - part;
    }
    size_t count = 6;
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t choice = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 32) ^ (uint32_t)(challenge >> 3);
    values[0] = choice;
    switch (choice) {
        case 0: values[1] = 5u * x + 0x4cc9u; break;
        case 1: values[1] = (2u * x) ^ 0x7ee8u; break;
        case 2: values[1] = (4u * x) ^ 0x7546u; break;
        case 3: values[1] = 5u * x + 0x3638au; break;
        case 4: values[1] = 5u * x + 0xe95cu; break;
        case 5: values[1] = 3u * x + 0x11f5u; break;
        case 6: values[1] = (x ^ 0xaa68u) + (x >> 1); break;
        case 7: values[1] = (x ^ 0x2e98u) + (x >> 1); break;
    }
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t target = (uint32_t)challenge & 0x7fu;
    uint32_t salt = (uint32_t)(challenge >> 7) ^ (uint32_t)(challenge >> 32);
    values[0] = phase4_walk(target, 0u, 127u, salt);
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    static const uint32_t node_value[12] = {
        0xb756u, 0xeb2du, 0xa4f5u, 0x835fu, 0x4f25u, 0xc51bu,
        0xee7au, 0xc71bu, 0x835du, 0xfa5au, 0xd226u, 0xffefu
    };
    static const uint32_t node_tag[12] = {
        0xbu, 0x659u, 0x52cu, 0x727u, 0xa1cu, 0x985u,
        0x10du, 0x23au, 0xb68u, 0x44au, 0x32bu, 0x83eu
    };
    static const uint8_t node_next[12] = {
        9, 1, 7, 2, 6, 3, 1, 0, 5, 10, 4, 8
    };
    static const uint8_t heads[8] = {11, 8, 5, 3, 2, 7, 0, 9};
    uint32_t index = heads[challenge & 7u];
    uint32_t steps = (uint32_t)((challenge >> 3) & 3u);
    uint32_t acc = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32) ^ 0xa026u;
    acc = 5u * (acc + node_value[index]) ^ node_tag[index];
    for (uint32_t i = 0; i < steps; ++i) {
        index = node_next[index];
        acc = 5u * (acc + node_value[index]) ^ node_tag[index];
    }
    values[0] = acc;
    values[1] = node_tag[index];
    size_t count = 2;
    return write_answer(out, cap, values, count);
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    uint32_t x = (uint32_t)(challenge >> 6) ^ (uint32_t)(challenge >> 32);
    uint32_t first = (uint32_t)challenge & 7u;
    uint32_t second = (uint32_t)((challenge >> 3) & 7u);
    values[0] = phase6_op(second, phase6_op(first, x) ^ 0xb1c2u) + 0xa2c8u;
    size_t count = 1;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    uint32_t first = (uint32_t)context & 3u;
    uint32_t challenge_fold = (uint32_t)challenge ^ (uint32_t)(challenge >> 32);
    uint32_t context_fold = (uint32_t)context ^ (uint32_t)(context >> 32);
    uint32_t x = challenge_fold ^ context_fold;
    uint32_t rotated = (x << 3) | (x >> 29);
    uint32_t second = ((rotated + 0xfa22u) ^ 0x8a78u)
                    ^ (first * 0xfa22u ^ (uint32_t)(context >> 16));
    uint32_t third = second ^ (context_fold + first + 0x8a78u
                               + (challenge_fold >> 3));

    if (cap <= 36u) return 1;
    for (size_t i = 0; i < 32u; ++i) out[i] = 'A';
    out[32] = 'O'; out[33] = 'P'; out[34] = 'E'; out[35] = 'N';
    int n = snprintf(out + 36, cap - 36, "%" PRIu32 " %" PRIu32 " %" PRIu32,
                     first, second, third);
    return n < 0 || (size_t)n >= cap - 36;
}
