#ifndef NC_UTIL_H
#define NC_UTIL_H
/*
ngx_str_t *
char_to_str(ngx_pool_t *pool, char *charp, len) {
  u_char *u_att_name = ngx_palloc(r->pool, sizeof(u_char)*len);
  ngx_str_t *str = ngx_palloc(pool, sizeof(ngx_str_t));
  str->len = ngx_sprintf(u_att_name, "%s", charp) - u_att_name;
  str->data = u_att_name;
  return str;
}
*/
/*
typedef struct {
  unsigned char *name;
  int            type;
  size_t         len;
} nc_att_key_t;

typedef struct {
  nc_att_key_t *key;
  void         *val;
} nc_att_t;

nc_att_key_t *
nc_att_key_create(ngx_pool_t pool);

nc_att_t *
nc_att_create(ngx_pool_t pool);
*/
#endif // NC_UTIL_H
