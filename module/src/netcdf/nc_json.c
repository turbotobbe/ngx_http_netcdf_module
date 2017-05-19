#include "nc_json.h"

nc_json_t *
nc_json_object(ngx_pool_t *pool, ngx_pool_t *temp_pool) {

  // init
  nc_json_t nc_json;

  nc_json.hash_init.hash = &nc_json.hash;
  nc_json.hash_init.key = ngx_hash_key;
  nc_json.hash_init.max_size = 512;
  nc_json.hash_init.bucket_size = ngx_align(64, ngx_cacheline_size);
  nc_json.hash_init.name = "hash";
  nc_json.hash_init.pool = pool;
  nc_json.hash_init.temp_pool = temp_pool;

  // hash keys
  nc_json.hash_keys.pool = pool;
  nc_json.hash_keys.temp_pool = temp_pool;
  ngx_hash_keys_array_init(&nc_json.hash_keys, NGX_HASH_SMALL);

  return nc_json;
}

void
nc_json_add_object(nc_json_t *nc_json, ngx_str_t key, nc_json_t *val) {
  ngx_hash_add_key(nc_json->hash_keys, key, &val, NGX_HASH_READONLY_KEY);
}

void
nc_json_add_string(nc_json_t *nc_json, ngx_str_t key, ngx_str_t val) {
  ngx_hash_add_key(nc_json->hash_keys, key, val, NGX_HASH_READONLY_KEY);
  return val;
}

void
nc_json_build(nc_json_t *nc_json) {
  ngx_hash_init(nc_json->hash, nc_json.hash_keys.keys.elts, prop_keys.keys.nelts);
}
