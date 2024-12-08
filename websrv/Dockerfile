FROM alpine:latest

# Install needed tools to compile
RUN apk update && apk add --no-cache \
    g++ \
    make \
    curl \
    net-tools \
    iputils \
    libc-dev \
    tree \
    bash \
    python3 \
    perl

#WORKDIR /usr/src/webserv
WORKDIR /ws

COPY . .
RUN chmod -R 755 /ws
RUN make
#RUN make
EXPOSE 8080
EXPOSE 9090

CMD [ "make", "run" ]
