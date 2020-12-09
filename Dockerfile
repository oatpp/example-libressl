FROM alpine:latest

RUN apk update && apk upgrade

RUN apk add g++

RUN apk add git
RUN apk add make
RUN apk add cmake

RUN apk add libressl-dev

ADD . /service

WORKDIR /service/utility

RUN ./install-oatpp-modules.sh Release

WORKDIR /service/build

RUN cmake ..
RUN make

EXPOSE 8443 8443

ENTRYPOINT ["./example-libressl-exe"]
