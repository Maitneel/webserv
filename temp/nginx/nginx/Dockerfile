FROM alpine:latest
RUN apk add nginx

RUN mkdir -p /var/docker-volume/www/ /var/docker-volume/www/html/ /var/docker-volume/log/ /var/docker-volume/log/nginx

COPY ./conf/nginx.conf /etc/nginx/nginx.conf
COPY srcs/ /var/docker-volume/www/html/
COPY srcs/dir1/* /var/docker-volume/www/html/dir1/
COPY srcs/dir2/* /var/docker-volume/www/html/dir2/

CMD ["nginx", "-g", "daemon off;"]