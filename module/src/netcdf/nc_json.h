#ifndef NC_JSON_H
#define NC_JSON_H

#include <ngx_hash.h>

typedef struct {
  ngx_hash_t              hash;
  ngx_hash_init_t         hash_init;
  ngx_hash_keys_arrays_t  hash_keys;
} nc_json_t;

nc_json_t *
nc_json_object(ngx_pool_t *pool, ngx_pool_t *temp_pool);

ngx_str_t
nc_json_add_object(nc_json_t *nc_json, ngx_str_t key, nc_json_t *val) {

ngx_str_t *
nc_json_add_string(nc_json_t *nc_json, ngx_str_t key, ngx_str_t val);

void
nc_json_build(nc_json_t *nc_json);

#endif // NC_JSON_H
