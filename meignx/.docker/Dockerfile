FROM alpine AS base

WORKDIR /app

COPY src/ ./src/
COPY include/ ./include/
COPY CMakeLists.txt ./CMakeLists.txt
COPY CMakePresets.json ./CMakePresets.json

RUN apk add --no-cache build-base cmake


FROM base AS development

RUN cmake --preset debug \
    && cmake --build --preset debug \
    && ln -sfn /app/build/debug/meignx /usr/local/bin/meignx

ENTRYPOINT ["meignx"]


FROM base AS builder

RUN cmake --preset release \
    && cmake --build --preset release \
    && cmake --install build/release --prefix /usr/local


FROM alpine AS production

COPY --from=builder /usr/local/bin/meignx /usr/local/bin/meignx

ENTRYPOINT ["meignx"]
