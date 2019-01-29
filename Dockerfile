FROM lganzzzo/alpine-cmake-libressl:latest

ADD . /service

WORKDIR /service/build

RUN cmake ..
RUN make

EXPOSE 8443 8443

ENTRYPOINT ["make", "run"]