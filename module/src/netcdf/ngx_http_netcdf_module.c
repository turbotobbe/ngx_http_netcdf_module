/**
 * @file   ngx_http_netcdf_module.c
 * @author Thobias Bergqvist <thobias.bergqvist@gmail.com>
 * @date   Thu May 18 10:13:53 2017
 *
 * @brief  A netcdf module for Nginx.
 *
 * @section LICENSE
 *
 * Copyright (C) 2017 by Thobias Bergqvist <thobias.bergqvist@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <stdlib.h>
#include <stdio.h>
#include <netcdf.h>
#include "nc_util.h"
// #include "nc_json.h"

static char *ngx_http_netcdf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static ngx_int_t ngx_http_netcdf_handler(ngx_http_request_t *r);

/**
 * This module provided directive: netcdf.
 *
 */
static ngx_command_t ngx_http_netcdf_commands[] = {

    { ngx_string("netcdf"), /* directive */
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS, /* location context and takes
                                            no arguments*/
      ngx_http_netcdf, /* configuration setup function */
      0, /* No offset. Only one context is supported. */
      0, /* No offset when storing the module configuration on struct. */
      NULL},

    ngx_null_command /* command termination */
};

/* The module context. */
static ngx_http_module_t ngx_http_netcdf_module_ctx = {
    NULL, /* preconfiguration */
    NULL, /* postconfiguration */

    NULL, /* create main configuration */
    NULL, /* init main configuration */

    NULL, /* create server configuration */
    NULL, /* merge server configuration */

    NULL, /* create location configuration */
    NULL /* merge location configuration */
};

/* Module definition. */
ngx_module_t ngx_http_netcdf_module = {
    NGX_MODULE_V1,
    &ngx_http_netcdf_module_ctx, /* module context */
    ngx_http_netcdf_commands, /* module directives */
    NGX_HTTP_MODULE, /* module type */
    NULL, /* init master */
    NULL, /* init module */
    NULL, /* init process */
    NULL, /* init thread */
    NULL, /* exit thread */
    NULL, /* exit process */
    NULL, /* exit master */
    NGX_MODULE_V1_PADDING
};
/* This is the name of the data file we will read. */
 #define FILE_NAME "/opt/data/dataset-reanalysis-hiromb-surface.nc"
 static const char nc_filename[] = FILE_NAME;

 /* We are reading 2D data, a 6 x 12 grid. */
 #define NX 6
 #define NY 12

 /* Handle errors by printing an error message and exiting with a
  * non-zero status. */
 #define ERRCODE 2
 #define DEBUG(log, e) { ngx_log_error(NGX_LOG_DEBUG, log, e, nc_strerror(e)); }

/**
 * Content handler.
 *
 * @param r
 *   Pointer to the request structure. See http_request.h.
 * @return
 *   The status of the response generation.
 */
static ngx_int_t ngx_http_netcdf_handler(ngx_http_request_t *r)
{

    /* This will be the netCDF ID for the file. */
    int ncid, ndims, nvars, natts, unlimdimid, retval;

    //ngx_list_t      *atts_list = ngx_list_create(r->pool, 100, sizeof(nc_att_t));
    //ngx_list_part_t *atts_part;
    //nc_att_t        *att;

// ---------------------------------
    ngx_hash_keys_arrays_t  att_keys;
    att_keys.pool = r->pool;
    att_keys.temp_pool = r->pool;
    ngx_hash_keys_array_init(&att_keys, NGX_HASH_SMALL);
// ---------------------------------

/*
    nc_json_t *json_root = nc_json_object(r->pool);
    nc_json_t *json_atts = nc_json_object(r->pool);
    nc_json_add_object(json_root, "attributes", json_atts);
*/
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "FILE %s", nc_filename);

    // open file
    if ((retval = nc_open(nc_filename, NC_NOWRITE, &ncid))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    }

    // get num of dims, vars, atts and unlimdimid
    if ((retval = nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    }

    char *att_name = ngx_palloc(r->pool, sizeof(char)*NC_MAX_NAME);
//    u_char *u_att_name;
    char *att_value;

    // collect atts
    for (int i=0; i<natts; ++i) {

      int     att_type;
      size_t  att_len;
      //att = nc_att_create(r->pool);

      // att name
      if ((retval = nc_inq_attname(ncid, NC_GLOBAL, i, att_name))) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
        return NGX_HTTP_NOT_FOUND;
      }

      // att type and len
      if ((retval = nc_inq_att(ncid, NC_GLOBAL, att_name, &att_type, &att_len))) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
        return NGX_HTTP_NOT_FOUND;
      }

      switch (att_type) {
        case NC_CHAR: // 2

          att_value = ngx_palloc(r->pool, sizeof(char)*att_len);
          if ((retval = nc_get_att_text(ncid, NC_GLOBAL, att_name, att_value))) {
            ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
            return NGX_HTTP_NOT_FOUND;
          }

          ngx_str_t *key = ngx_palloc(r->pool, sizeof(ngx_str_t));
          key->data = (u_char*)att_name;
          key->len = ngx_strlen(att_name);
          ngx_str_t *val = ngx_palloc(r->pool, sizeof(ngx_str_t));
          val->data = (u_char*)att_value;
          val->len = ngx_strlen(att_value);

          ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ATT [%d|%s] -> [%d|%s]", key->len, key->data, val->len, val->data);

          ngx_hash_add_key(&att_keys, key, val, NGX_HASH_READONLY_KEY);

          // ---------------------------------

          //nc_json_add_string(json_atts, attkey, attval);
          break;
          /*
         case NC_DOUBLE: // 6
           attval = ngx_palloc(r->pool, sizeof(double)*attlen);
           if ((retval = nc_get_att_double(ncid, NC_GLOBAL, attkey, attval))) {
             ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
             return NGX_HTTP_NOT_FOUND;
           } else {
             ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ATT VAL %d", attval);
           }
           // jsonprop = ngx_sprintf("\"%s\":\"%d\"", attval, attval);
          break;
          */
         case NC_NAT://          0
         case NC_BYTE://         1
         case NC_SHORT://    3
         case NC_INT://          4 NC_LONG
         case NC_FLOAT://    5
         case NC_UBYTE://    7
         case NC_USHORT://   8
         case NC_UINT://     9
         case NC_INT64://    10
         case NC_UINT64://   11
         case NC_STRING://   12
         default:
          ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "not supported type %d LEN %d", att_type, att_len);
          //return NGX_HTTP_NOT_FOUND;
       }
    }

    /* Close the file, freeing all resources. */
    if ((retval = nc_close(ncid))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    } else {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "FILE CLOSED");
    }

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "done reading file %s", nc_filename);

// -----------------------------
    ngx_hash_t       att_hash;
    ngx_hash_init_t  hash;

    hash.hash = &att_hash;
    hash.key = ngx_hash_key;
    hash.max_size = 512;
    hash.bucket_size = ngx_align(64, ngx_cacheline_size);
    hash.name = "att_hash";
    hash.pool = r->pool;
    hash.temp_pool = r->pool;
    ngx_hash_init(&hash, att_keys.keys.elts, att_keys.keys.nelts);

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "nelts %d", att_keys.keys.nelts);
    ngx_str_t *elts = att_keys.keys.elts;
    for (size_t i=0; i<att_keys.keys.nelts; i++) {
      ngx_str_t key = elts[i];
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "key %s", key.data);
    }

// -----------------------------

/*
    atts_part = &atts_list->part;
    att = atts_part->elts;

    for (size_t i = 0;; i++) {

        if (i >= atts_part->nelts) {
            if (atts_part->next == NULL) {
                break;
            }

            atts_part = atts_part->next;
            att = atts_part->elts;
            i = 0;
        }

        a = &att[i];
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "att \"%s\":\"%s\"", a->key->name, a->val);
    }
*/
    u_char *body = ngx_palloc(r->pool, sizeof(char)*1024);
    ngx_sprintf(body, "{}");
    size_t bodylen = ngx_strlen(body);

    //u_char *body = json_object_to_json_string(root);
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "result %s", body);
    //ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "result %s", json_object_to_json_string(root));
    ngx_buf_t *b;
    ngx_chain_t out;

    /* Set the Content-Type header. */
    r->headers_out.content_type.len = sizeof("application/json") - 1;
    r->headers_out.content_type.data = (u_char *) "application/json";

    /* Allocate a new buffer for sending out the reply. */
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    /* Insertion in the buffer chain. */
    out.buf = b;
    out.next = NULL; /* just one buffer */

    b->pos = body; /* first position in memory of the data */
    b->last = body + (sizeof(char)*bodylen); /* lngx_string(ast position in memory of the data */
    b->memory = 1; /* content is in read-only memory */
    b->last_buf = 1; /* there will be no more buffers in the request */

    /* Sending the headers for the reply. */
    r->headers_out.status = NGX_HTTP_OK; /* 200 status code */
    /* Get the content length of the body. */
    r->headers_out.content_length_n = (sizeof(char)*bodylen);
    ngx_http_send_header(r); /* Send the headers */

    /* Send the body, and return the status code of the output filter chain. */
    return ngx_http_output_filter(r, &out);
} /* ngx_http_netcdf_handler */

/**
 * Configuration setup function that installs the content handler.
 *
 * @param cf
 *   Module configuration structure pointer.
 * @param cmd
 *   Module directives structure pointer.
 * @param conf
 *   Module configuration structure pointer.
 * @return string
 *   Status of the configuration setup.
 */
static char *ngx_http_netcdf(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t *clcf; /* pointer to core location configuration */

    /* Install the netcdf handler. */
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_netcdf_handler;

    return NGX_CONF_OK;
} /* ngx_http_netcdf */
