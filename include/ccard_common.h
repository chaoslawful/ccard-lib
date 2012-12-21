#ifndef CCARD_COMMON_H__
#define CCARD_COMMON_H__

#include <stdint.h>

#ifdef __cplusplus
extern          "C" {
#endif

    /**
     * Predefined error codes
     * */
    enum {
        CCARD_OK = 0,        ///< No error
        CCARD_ERR_INVALID_CTX = -1, ///< Invalid algorihm context
        CCARD_ERR_MERGE_FAILED = -2,    ///< Merge failed
        CCARD_ERR_PLACEHOLDER
    };

    /**
     * Algorithms
     * */
    enum {
        CCARD_ALGO_ADAPTIVE = 1,
        CCARD_ALGO_HYPERLOGLOG = 2,
        CCARD_ALGO_LINEAR = 3,
        CCARD_ALGO_PLACEHOLDER
    };

    /**
     * Hash functions
     * */
    enum {
        CCARD_HASH_MURMUR = 1,
        CCARD_HASH_LOOKUP3 = 2,
        CCARD_HASH_PLACEHOLDER
    };

    /**
     * C-card algorithm definition
     * */
    typedef struct ccard_algo_s {
        /// Allocate algorithm ctx with optional raw data
        void           *(*raw_init) (const void *buf, uint32_t len_or_hint,
                                     uint8_t hf);
        /// Allocate algorithm ctx with optional external data
        void           *(*init) (const void *buf, uint32_t len_or_hint,
                                 uint8_t hf);
        /// Get cardinality from algorithm ctx
        int64_t         (*card) (void *ctx);
        /// Offer a new item to be counted
        int             (*offer) (void *ctx, const void *buf,
                                  uint32_t len);
        /// Reset count to zero
        int             (*reset) (void *ctx);
        /// Get raw bytes of the algorithm state
        int             (*get_raw_bytes) (void *ctx, void *buf,
                                          uint32_t *len);
        /// Get serialized bytes of the algorithm state
        int             (*get_bytes) (void *ctx, void *buf,
                                      uint32_t *len);
        /// Merge several algorithm ctx and combine their counts
        int             (*merge) (void *ctx, void *tbm, ...);
        /// Merge several raw bytes and combine their counts
        int             (*merge_raw_bytes) (void *ctx, const void *buf,
                                            uint32_t len, ...);
        /// Merge several serialized bytes and combine their counts
        int             (*merge_bytes) (void *ctx, const void *buf,
                                        uint32_t len, ...);
        /// Deallocate algorithm ctx
        int             (*fini) (void *ctx);
        /// Get error code from algorithm ctx
        int             (*errnum) (void *ctx);
        /// Convert error code to human-friendly messages
        const char     *(*errstr) (int errn);
    } ccard_algo_t;

#ifdef __cplusplus
}
#endif

#endif

// vi:ft=c ts=4 sw=4 fdm=marker et

