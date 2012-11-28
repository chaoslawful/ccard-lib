#ifndef REGISTERSET_H__
#define REGISTERSET_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct reg_set_s {
        uint32_t count;
        uint32_t size;
        uint32_t M[1];
    } reg_set_t;

    /**
     * Initialize a new register set.
     *
     * @param[in] count Logical elements number of register set.
     * @param[in] values Initial values. NULL if there is no initial values.
     * @param[in] len Length of initial values.
     *
     * @retval not-NULL An initialized register set to be used with the rest of
     * methods.
     * @retval NULL If error occured.
     * */
    reg_set_t *rs_init(uint32_t count, uint32_t *values, uint32_t len); 

    /**
     * Set value.
     *
     * @param[in] rs Register set.
     * @param[in] pos Logical element number.
     * @param[in] value Value to be set.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     * */
    int rs_set(reg_set_t *rs, uint32_t pos, uint32_t value);

    /**
     * Get value.
     *
     * @param[in] rs Register set.
     * @param[in] pos Logical element number.
     * @param[out] value Store output value.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     * */
    int rs_get(reg_set_t *rs, uint32_t pos, uint32_t *value);

    /**
     * Convert register set to bitmap.
     *
     * @param[in] rs Register set.
     * @param[out] bits Buffer that stores bitmap.
     * @param[in|out] len Buffer size and bitmap length.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     * */
    int rs_bits(reg_set_t *rs, uint32_t *bits, uint32_t *len);

    /**
     * Destory register set and release resource.
     *
     * @param[in] rs Register set.
     *
     * @retval 0 If success.
     * @retval -1 If error occured.
     * */
    int rs_fini(reg_set_t *rs);

#ifdef __cplusplus
}
#endif

#endif

// vi:ft=c ts=4 sw=4 fdm=marker et

