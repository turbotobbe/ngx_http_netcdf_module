FROM ubuntu

RUN \
sed -i 's/# \(.*multiverse$\)/\1/g' /etc/apt/sources.list && \
apt-get update && \
apt-get -y upgrade && \
apt-get install -y build-essential && \
apt-get install -y libpcre3-dev  zlib1g-dev libssl-dev libnetcdf-dev netcdf-bin && \
rm -rf /var/lib/apt/lists/*

EXPOSE 8080

CMD ["/bin/bash"]
