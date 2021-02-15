FROM alpine:3.13.1 AS scheduler-builder

RUN apk update && apk add build-base qt5-qtbase-dev qt5-qtwebsockets-dev openssl
ENV PATH="/usr/lib/qt5/bin/:${PATH}"

#RUN apk update && apk add git
#RUN git clone --recursive https://github.com/Zebreus/pruefungsplaner-scheduler.git /pruefungsplaner-scheduler
COPY . /pruefungsplaner-scheduler

RUN mkdir -p /install && cd /pruefungsplaner-scheduler/ && qmake && make -j8 install INSTALL_ROOT=/install/

FROM alpine:3.13.1
MAINTAINER Lennart E.

RUN apk update && apk add qt5-qtbase qt5-qtwebsockets openssl tini
COPY --from=scheduler-builder /install/ /

ENTRYPOINT ["tini", "/usr/bin/pruefungsplaner-scheduler"]
EXPOSE 80
