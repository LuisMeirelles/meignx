FROM alpine

RUN apk add build-base cmake

WORKDIR /app

COPY . .

RUN cmake -S . -B cmake-build-debug \
    && cmake --build cmake-build-debug

ENTRYPOINT ["cmake-build-debug/meignx"]

