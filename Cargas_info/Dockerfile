FROM gcc:12.2.0

RUN apt-get update && apt-get install -y \
    libboost-system-dev \
    libpqxx-dev \
    libpq-dev \
    libssl-dev \
    libhiredis-dev \
	nlohmann-json3-dev \
    cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

COPY . .

RUN make clean

RUN make

CMD ["./Server"]
