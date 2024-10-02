# Use an official C++ image with necessary build tools
FROM gcc:12.2.0

# Install dependencies
RUN apt-get update && apt-get install -y \
    libboost-system-dev \
    libpqxx-dev \
    libpq-dev \
    libssl-dev \
    libhiredis-dev \
	nlohmann-json3-dev \ 
    cmake \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory inside the container
WORKDIR /usr/src/app

# Copy the source code and Makefile into the container
COPY . .

RUN make clean
# Run the Makefile to build the C++ backend
RUN make

# Set the entry point to run the C++ server
CMD ["./Server"]
