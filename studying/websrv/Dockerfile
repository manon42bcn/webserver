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
    bash

#WORKDIR /usr/src/webserv
WORKDIR /ws

COPY . .
RUN make
#RUN make
EXPOSE 8080
EXPOSE 9090


CMD [ "make", "run" ]
