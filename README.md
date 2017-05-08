# ngx_http_netcdf_module
netcdf module for nginx

## build

```
cd /opt/nginx
CFLAGS="-g -O0" /opt/nginx/auto/configure \
  --with-debug \
  --prefix=/opt/build/nginx \
  --conf-path=conf/nginx.conf \
  --error-log-path=logs/error.log \
  --http-log-path=logs/access.log \
  --without-http_rewrite_module \
  --without-http_gzip_module \
  --add-module=/opt/src/main
make
make install
```

## run
```
/opt/build/nginx
```
