FROM quantmind/openresty:jessie

RUN apt-get update && apt-get install -y vim

COPY --chown=openresty:openresty ./nginx/c2v.conf /usr/local/openresty/nginx/conf/conf.d/
COPY --chown=openresty:openresty ./www /var/html/c2v/www
COPY --chown=openresty:openresty ./media /var/html/c2v/media
