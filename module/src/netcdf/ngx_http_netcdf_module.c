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
#include <json/json.h>

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

    /* This will be the netCDF ID for the file and data variable. */
    int ncid;//, attid;

    int ndims, nvars, natts, unlimdimid;
    //ngx_str_t *attname;

    // int data_in[NX][NY];

    /* Loop indexes, and error handling. */
    int retval;

    char * attname = ngx_palloc(r->pool, sizeof(char)*NC_MAX_NAME);
    int atttype = 0;
    size_t attlen = 0;

    json_object *root = json_object_new_object();
    json_object *atts = json_object_new_object();
    json_object_object_add(root,"atts", atts);

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "FILE %s", nc_filename);

    /* Open the file. NC_NOWRITE tells netCDF we want read-only access
     * to the file.*/
    if ((retval = nc_open(nc_filename, NC_NOWRITE, &ncid))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    } else {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "FILE OPEN");
    }

    if ((retval = nc_inq(ncid, &ndims, &nvars, &natts, &unlimdimid))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    } else {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "NUM INQUIRED");
    }

    for (int i=0; i<natts; ++i) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ATT %d", i);

       if ((retval = nc_inq_attname(ncid, NC_GLOBAL, i, attname))) {
         ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
         return NGX_HTTP_NOT_FOUND;
       } else {
         ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ATT NAME %s", attname);
       }

       if ((retval = nc_inq_att(ncid, NC_GLOBAL, attname, &atttype, &attlen))) {
         ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
         return NGX_HTTP_NOT_FOUND;
       } else {
         ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ATT TYPE %d LEN %d", atttype, attlen);
       }
       void *attval;
       switch (atttype) {
         case NC_CHAR: // 2
            attval = ngx_palloc(r->pool, sizeof(char)*attlen);
            if ((retval = nc_get_att_text(ncid, NC_GLOBAL, attname, attval))) {
              ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
              return NGX_HTTP_NOT_FOUND;
            } else {
              ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "ATT VAL %s", attval);
            }
            // ngx_sprintf(buff.pos, "\"%s\":\"%s\"", attval, attval);
            json_object *val = json_object_new_string(attval);
            json_object_object_add(atts, attname, val);
          break;
          /*
         case NC_DOUBLE: // 6
           attval = ngx_palloc(r->pool, sizeof(double)*attlen);
           if ((retval = nc_get_att_double(ncid, NC_GLOBAL, attname, attval))) {
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
          ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "not supported type %d LEN %d", atttype, attlen);
          //return NGX_HTTP_NOT_FOUND;
       }
    }

    /* Get the varid of the data variable, based on its name. */
    /*
    if ((retval = nc_inq_varid(ncid, "data", &varid))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    }
    */

    /* Read the data. */
    /*
    if ((retval = nc_get_var_int(ncid, varid, &data_in[0][0]))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    }
    */

    /* Check the data. */
    /*
    for (x = 0; x < NX; x++)
       for (y = 0; y < NY; y++)
      if (data_in[x][y] != x * NY + y)
         return ERRCODE;
         */

    /* Close the file, freeing all resources. */
    if ((retval = nc_close(ncid))) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, retval, nc_strerror(retval));
      return NGX_HTTP_NOT_FOUND;
    } else {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "FILE CLOSED");
    }

    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "done reading file %s", nc_filename);

    u_char *body = ngx_palloc(r->pool, sizeof(char)*1024);
    ngx_sprintf(body, "%s",json_object_to_json_string(root));
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
    b->last = body + (sizeof(char)*bodylen); /* last position in memory of the data */
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
