#include <assert.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define log_normal(...)      \
    {                        \
        printf(__VA_ARGS__); \
        printf("\n");        \
    }

#define macro_util_kb(x) ((x) *1024)

#define macro_util_mb(x) (macro_util_kb(x) * 1024)

#define macro_util_gb(x) (macro_util_mb(x) * 1024)

/**
 * Get array length.
 *
 * Example:
 * u16_t i[32];
 * macro_util_arr_len(i); // => 32
 */
#define macro_util_arr_len(src) (sizeof(src) / sizeof(*(src)))

/**
 * Set array buffer to 0.
 *
 * Example:
 * byte_t buf[32];
 * macro_util_set_arr_zero(buf); // All 32 bytes in buf are 0.
 */
#define macro_util_set_arr_zero(dest) (util_set_zero(dest, sizeof(dest)))

/**
 * Example:
 * byte_t dest[32];
 * byte_t *src = "42";
 * macro_util_cpy_bytes_to_arr(dest, src, 2);
 */
#define macro_util_cpy_bytes_to_arr(dest, src, bytes_to_cpy) \
    (util_cpy_bytes(dest,                                    \
                    src,                                     \
                    sizeof(dest),                            \
                    (size_t) (bytes_to_cpy),                 \
                    (size_t) (bytes_to_cpy)))

/**
 * Copy array string from src to array dest.
 *
 * Example:
 * char from[] = "lorem ipsum";
 * char to[32] = { 0 };
 * macro_util_cpy_str_arr(to, from);
 *
 * NOTE: Make sure both buffers are arrays and not just pointers!
 */
#define macro_util_cpy_str_arr(dest, src) \
    (util_cpy_str(dest, src, sizeof(dest), sizeof(src)))

#define macro_util_min(a, b) ((a) < (b) ? (a) : (b))

#define macro_util_max(a, b) ((a) > (b) ? (a) : (b))

#define macro_util_abs(x) ((x) < 0 ? -(x) : (x))

#define macro_util_diff(a, b) (macro_util_abs((a) - (b)))

#define macro_util_is_nan(x) ((x) != (x))

#define macro_util_is_odd(x) ((x) &1)

#define macro_util_is_even(x) (!macro_util_is_odd(x))

/**
 * Make sure a value is between min and max.
 *
 * Example:
 * macro_util_clamp(5, 10, 20); // Result is 10.
 * macro_util_clamp(15, 10, 20); // Result is 15.
 * macro_util_clamp(30, 10, 20); // Result is 20.
 */
#define macro_util_clamp(x, min, max) \
    (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))

#define macro_util_safe_u8(x) (macro_util_min(x, UINT8_MAX))

#define macro_util_safe_u16(x) (macro_util_min(x, UINT16_MAX))

#define macro_util_safe_u32(x) (macro_util_min(x, UINT32_MAX))

#define macro_util_safe_u64(x) (macro_util_min(x, UINT64_MAX))

#define macro_util_safe_i8(x) (macro_util_clamp(x, INT8_MIN, INT8_MAX))

#define macro_util_safe_i16(x) (macro_util_clamp(x, INT16_MIN, INT16_MAX))

#define macro_util_safe_i32(x) (macro_util_clamp(x, INT32_MIN, INT32_MAX))

#define macro_util_safe_i64(x) (macro_util_clamp(x, INT64_MIN, INT64_MAX))

#define macro_util_pi (3.14159265)

#define macro_util_bit(x) (1 << (x))

#define macro_util_set_bit(x, value) ((x) | macro_util_bit(value))

#define macro_util_clear_bit(x, value) ((x) & ~macro_util_bit(value))

#define macro_util_get_bit(x, value) (((x) >> (value)) & 1)

#define macro_util_toggle_bit(x, value) ((x) ^ macro_util_bit(value))

#define macro_util_read_bytes_val(dest, src, n) \
    (util_read_bytes(&(dest),                   \
                     (void **) (src),           \
                     sizeof(dest),              \
                     macro_util_max(0, n),      \
                     sizeof(dest)))

/**
 * Iterate through all items in a list.
 *
 * Example:
 * struct list foo[32] = { 0 };
 * int bar = 42;
 * util_list_add(foo, &bar);
 * macro_util_list_each(foo, int, value) { *value; // => 42 }
 */
#define macro_util_list_each(arr, type, name)                     \
    for (struct list *list_iterator = (arr)->head;                \
         list_iterator && (name = (type *) list_iterator->value); \
         list_iterator = list_iterator->next)

typedef int8_t i8_t;
typedef int16_t i16_t;
typedef int32_t i32_t;
typedef int64_t i64_t;

typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef uint64_t u64_t;

typedef u8_t byte_t;

struct list {
    struct list *head;
    struct list *next;
    void *value;
};

// Set n bytes from dest to 0.
//
// (franco.montenegro) There may be room for improvement
// in this function, say, instead of copying byte per byte
// copy chunks (4 bytes at a time using size_t)
void util_set_zero(void *dest, size_t n)
{
    byte_t *byte_dest = 0;

    if (!dest || !n) {
        return;
    }

    byte_dest = dest;

    for (size_t i = 0; i < n; i += 1) {
        *byte_dest = 0;
        byte_dest += 1;
    }
}

// Safely copy n bytes from src to dest making sure not to
// overflow. The amount of bytes to copy won't exceed dn nor
// sn.
void util_cpy_bytes(void *dest, void *src, size_t dn, size_t sn, size_t n)
{
    byte_t *byte_dest = 0;
    byte_t *byte_src  = 0;

    if (!dest || !src || !dn || !sn || !n) {
        return;
    }

    byte_dest = dest;
    byte_src  = src;

    while (dn && sn && n) {
        *byte_dest = *byte_src;
        byte_dest += 1;
        byte_src += 1;
        dn -= 1;
        sn -= 1;
        n -= 1;
    }
}

// Copy all n bytes from src to dest or none at all if buffers
// aren't big enough.
// On success, 1 will be returned, otherwise, 0.
int util_cpy_all_bytes(void *dest, void *src, size_t dn, size_t sn, size_t n)
{
    if (!dest || !src || !dn || !sn) {
        return 0;
    }

    if (dn < n || sn < n) {
        return 0;
    }

    if (!n) {
        return 1;
    }

    util_cpy_bytes(dest, src, dn, sn, n);

    return 1;
}

// Copy string from src to dest without overflowing.
// The last character in dest is set to 0.
void util_cpy_str(char *dest, char *src, size_t dn, size_t sn)
{
    if (!dest || !src || !dn || !sn) {
        return;
    }

    while (dn && sn && *src) {
        *dest = *src;
        dest += 1;
        src += 1;
        dn -= 1;
        sn -= 1;
    }

    *dest = 0;
}

// Read all n bytes from src and copy to dest.
// If buffers aren't big enough, no copy is made.
// On success, 1 will be returned and src will be advanced n
// bytes forward. On error, 0 will be returned and no bytes
// will be read.
int util_read_bytes(void *dest, void **src, size_t dn, size_t sn, size_t n)
{
    if (!util_cpy_all_bytes(dest, *src, dn, sn, n)) {
        return 0;
    }

    *((byte_t **) src) += n;
    return 1;
}

// Add value to list contained in src buf.
// On success, 1 is returned.
// If there is no more space in src buf, the value won't be
// added and 0 is returned.
//
// Example:
// struct list foo[32] = { 0 };
// int bar = 42;
// util_list_add(foo, macro_util_arr_len(foo), &bar);
int util_list_add(struct list *src, size_t src_len, void *value)
{
    struct list **head     = 0;
    struct list *free_node = 0;

    if (!src || !src_len) {
        return 0;
    }

    head      = &src->head;
    free_node = src + 1;

    while (free_node->value) {
        if ((size_t) (free_node - src) >= src_len) {
            return 0;
        }

        free_node += 1;
    }

    free_node->value = value;
    free_node->next  = *head;

    *head = free_node;

    return 1;
}

// Remove value from list contained in src buf.
// If the element was found and removed, 1 is returned.
// If the element isn't found, 0 will be returned.
int util_list_remove(struct list *src, void *value)
{
    struct list **match    = 0;
    struct list *to_remove = 0;

    if (!src) {
        return 0;
    }

    match = &src->head;

    while (*match && (*match)->value != value) {
        match = &(*match)->next;
    }

    if (!*match) {
        return 0;
    }

    to_remove  = *match;
    *match     = (*match)->next;
    *to_remove = (struct list){ 0 };

    return 1;
}

// Decode little endian from src with n being the buf size.
// On success, 1 will be returned and the decoded value will be
// stored in dest. On error, 0 will be returned (buf isn't big
// enough)
int util_decode32le(u32_t *dest, byte_t *src, size_t n)
{
    if (!dest || !src || n < 4) {
        return 0;
    }

    *dest = (u32_t) src[0] | ((u32_t) src[1] << 8) | ((u32_t) src[2] << 16) |
            ((u32_t) src[3] << 24);

    return 1;
}

// Decode big endian from src with n being the buf size.
// On success, 1 will be returned and the decoded value will be
// stored in dest. On error, 0 will be returned (buf isn't big
// enough)
int util_decode32be(u32_t *dest, byte_t *src, size_t n)
{
    if (!dest || !src || n < 4) {
        return 0;
    }

    *dest = (u32_t) src[3] | ((u32_t) src[2] << 8) | ((u32_t) src[1] << 16) |
            ((u32_t) src[0] << 24);

    return 1;
}

// Encode little endian in dest with n being the buf size.
// On success, 1 will be returned and src will be encoded in
// dest. On error, 0 will be returned.
int util_encode32le(byte_t *dest, u32_t src, size_t n)
{
    if (!dest || n < 4) {
        return 0;
    }

    dest[0] = (byte_t) src;
    dest[1] = (byte_t) (src >> 8);
    dest[2] = (byte_t) (src >> 16);
    dest[3] = (byte_t) (src >> 24);

    return 1;
}

// Encode big endian in dest with n being the buf size.
// On success, 1 will be returned and src will be encoded in
// dest. On error, 0 will be returned.
int util_encode32be(byte_t *dest, u32_t src, size_t n)
{
    if (!dest || n < 4) {
        return 0;
    }

    dest[3] = (byte_t) src;
    dest[2] = (byte_t) (src >> 8);
    dest[1] = (byte_t) (src >> 16);
    dest[0] = (byte_t) (src >> 24);

    return 1;
}

// Returns a usable id/index from an array of instances.
//
// Example:
// size_t instances[30] = { 0 };
// struct foo[30] = { 0 };
// size_t foo_count = 0;
//
// size_t instance = 0;
//
// foo_count += util_recycle_id_get(&instance, instances);
// foo[instance] = free instance to be used.
// util_recycle_id(instances, instance);
int util_recycle_id_get(size_t *dest, size_t *src)
{
    assert(src);

    *dest = src[0];

    if (src[*dest]) {
        src[0] = src[*dest];
        return 0;
    }

    src[0] = *dest + 1;

    return 1;
}

// Mark id/index to be reusable/recycled.
// Next time recycle_id_get is called, the
// recycled instances will be returned.
void util_recycle_id(size_t *src, size_t id)
{
    assert(src);
    src[id] = src[0];
    src[0]  = id;
}
