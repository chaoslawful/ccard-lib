%module ccard
%{
#include "adaptive_counting.h"
struct adp_cnt_ctx_s {
    int err;
    uint8_t k;
    uint32_t m;
    double Ca;
    uint32_t Rsum;
    uint32_t b_e;
    uint8_t hf;
    uint8_t M[1];
};
%}

#if defined(SWIGPHP)

%typemap(in) uint32_t %{
    convert_to_long_ex($input);
    $1 = (uint32_t) Z_LVAL_PP($input);
%}

%typemap(in) uint8_t %{
    convert_to_long_ex($input);
    $1 = (uint8_t) Z_LVAL_PP($input);
%}

%typemap(in) (const void* buf,  uint32_t len) %{
    $1 = Z_STRVAL_PP($input);
    $2 = Z_STRLEN_PP($input);
%}

%typemap(in, numinputs=0) const void* IGNORE %{
    $1 = NULL;
%}

%typemap(out) int64_t  %{
   ZVAL_LONG($result, $1);
%}

adp_cnt_ctx_t* adp_cnt_init(const void *IGNORE, uint32_t len_or_k, uint8_t hf);
int64_t adp_cnt_card_loglog(adp_cnt_ctx_t *ctx);
int64_t adp_cnt_card(adp_cnt_ctx_t *ctx);
int adp_cnt_offer(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len);
int adp_cnt_reset(adp_cnt_ctx_t *ctx);

%typemap(in, numinputs=1) (adp_cnt_ctx_t * ctx, void* buf,  uint32_t *len) (int tmp_len)%{
    if(SWIG_ConvertPtr(*$input, (void **) &$1, SWIGTYPE_p_adp_cnt_ctx_t, 0) < 0) {
      SWIG_PHP_Error(E_ERROR, "Type error in argument 1 of adp_cnt_get_bytes. Expected SWIGTYPE_p_adp_cnt_ctx_t");
    }
    $3 = &tmp_len;
    $2 = alloca($1->m + 3);
    *$3 = $1->m + 3;
%}

%typemap(argout) (adp_cnt_ctx_t* ctx, void* buf,  uint32_t *len) %{
    if(result < 0) {
        ZVAL_NULL($result);
    } else {
        ZVAL_STRINGL($result, $2, *$3, 1);
    }
%}

%feature("action") adp_cnt_get_bytes%{
  int result = adp_cnt_get_bytes(arg1,arg2,arg3);
%}
void adp_cnt_get_bytes(adp_cnt_ctx_t *ctx, void *buf, uint32_t *len);

int adp_cnt_merge(adp_cnt_ctx_t *ctx, adp_cnt_ctx_t *tbm, ...);
int adp_cnt_merge_bytes(adp_cnt_ctx_t *ctx, const void *buf, uint32_t len, ...);
int adp_cnt_fini(adp_cnt_ctx_t *ctx);
int adp_cnt_errnum(adp_cnt_ctx_t *ctx);
const char* adp_cnt_errstr(int errn);

#endif
