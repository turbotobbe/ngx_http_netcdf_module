# ngx_http_netcdf_module
netcdf module for nginx

## build

```
cd /opt/nginx
#CFLAGS="-g -O0" /opt/nginx/auto/configure \
#  --with-debug \
#  --prefix=/opt/build/nginx \
#  --conf-path=conf/nginx.conf \
#  --error-log-path=logs/error.log \
#  --http-log-path=logs/access.log \
#  --with-http_ssl_module \
#  --add-module=/opt/src

CFLAGS="-g -O0" /opt/nginx/auto/configure --with-debug --prefix=/opt/build/nginx --conf-path=conf/nginx.conf --error-log-path=logs/error.log --http-log-path=logs/access.log --with-http_ssl_module --add-module=/opt/src

make
make install
```

## run
```
/opt/build/nginx
```
