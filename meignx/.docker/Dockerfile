FROM alpine AS builder

RUN apk add build-base

WORKDIR /app

COPY . .

RUN mkdir -p build \
  && gcc -o build/meignx src/main.c

RUN ls -asl build

ENTRYPOINT ["build/meignx"]

