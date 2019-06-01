FROM lganzzzo/alpine-cmake:latest

RUN apk add libressl-dev

ADD . /service

WORKDIR /service/utility

RUN ./install-oatpp-modules.sh

WORKDIR /service/build

RUN cmake ..
RUN make

EXPOSE 8443 8443

ENTRYPOINT ["./example-libressl-exe"]
