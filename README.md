# ngx_http_netcdf_module
netcdf module for nginx

## checkout

Check out module and nginx

```
git clone https://github.com/turbotobbe/ngx_http_netcdf_module.git
cd ngx_http_netcdf_module
git submodule init nginx
git submodule update nginx
```

## Prerequisites

To build you need this dependencies installed in your build environment
```
apt-get install build-essential libpcre3-dev  zlib1g-dev libssl-dev libnetcdf-dev netcdf-bin libjson0 libjson0-dev
```

## build

build and run like this.

```
cd /opt/nginx
# make clean && rm -fr /opt/build
./auto/configure --with-debug --prefix=/opt/build --add-module=/opt/module/src/hello --add-module=/opt/module/src/netcdf
make
make install
/opt/build/sbin/nginx -c /opt/module/nginx.conf
```
