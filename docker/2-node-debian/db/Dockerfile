FROM postgres:10.2

VOLUME /walden

RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
	apt-get -y install build-essential && \
	apt-get -y install postgis
	apt-get -y install libpq-dev postgresql-server-dev-10 && \
	apt-get -y install python2.7 postgresql-plpython-10 && \
	apt-get -y install python3   postgresql-plpython3-10 && \
	apt-get -y install libjson-c-dev libjson-c3 && \
	apt-get -y install git sudo vim && \
	apt-get clean && \
	rm -rf /var/cache/apt/* /var/lib/apt/lists/* && \
	mkdir /walden-entrypoint-initdb.d

COPY build-extensions.sh /build-extensions.sh 
COPY walden-entrypoint.sh /usr/local/bin/
RUN ln -s usr/local/bin//walden-entrypoint.sh / && \
	chmod +x /build-extensions.sh && \
	chmod +x /walden-entrypoint.sh && \
	sed -i '/^exec/d' /usr/local/bin/docker-entrypoint.sh 

ENTRYPOINT ["walden-entrypoint.sh"]
CMD ["postgres"]
