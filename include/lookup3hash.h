#ifndef LOOKUP3HASH_H__
#define LOOKUP3HASH_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * A implementation of hashword from lookup3.c by Bob Jenkins
     * (<a href="http://burtleburtle.net/bob/c/lookup3.c">original source</a>).
     *
     * @param k         the key to hash
     * @param offset    offset of the start of the key
     * @param length    length of the key
     * @param initval   initial value to fold into the hash
     * @return          the 32 bit hash code
     * */
    uint32_t        lookup3(const uint32_t *k, uint32_t offset,
                            uint32_t length, uint32_t initval);

    /**
     * Identical to lookup3, except initval is biased by -(length&lt;&lt;2).
     * This is equivalent to leaving out the length factor in the initial state.
     * {@code lookup3ycs(k,offset,length,initval) == lookup3(k,offset,length,initval-(length<<2))}
     * and
     * {@code lookup3ycs(k,offset,length,initval+(length<<2)) == lookup3(k,offset,length,initval)}
     * */
    uint32_t        lookup3ycs(const uint32_t *k, uint32_t offset,
                               uint32_t length, uint32_t initval);

    /**
     * <p>The hash value of a character sequence is defined to be the hash of
     * it's unicode code points, according to {@link #lookup3ycs(uint32_t *k, uint32_t offset, uint32_t length, uint32_t initval)}
     * </p>
     * <p>If you know the number of code points in the {@code CharSequence}, you can
     * generate the same hash as the original lookup3
     * via {@code lookup3ycs(s, start, end, initval+(numCodePoints<<2))}
     * */
    uint32_t        lookup3ycs_2(const char *s, uint32_t start,
                                 uint32_t end, uint32_t initval);

    /**
     * <p>This is the 64 bit version of lookup3ycs, corresponding to Bob Jenkin's
     * lookup3 hashlittle2 with initval biased by -(numCodePoints<<2).  It is equivalent
     * to lookup3ycs in that if the high bits of initval==0, then the low bits of the
     * result will be the same as lookup3ycs.
     * </p>
     * */
    uint64_t        lookup3ycs64(const char *s, uint32_t start,
                                 uint32_t end, uint64_t initval);

    uint64_t        lookup3ycs64_2(const char *s);

#ifdef __cplusplus
}
#endif

#endif

/* vi:ft=c ts=4 sw=4 fdm=marker et
 * */

