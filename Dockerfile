# Multi-stage build for universal library
FROM ubuntu:20.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
COPY . /app
WORKDIR /app

# Build library
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Create universal image
FROM ubuntu:20.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Copy built library and headers
COPY --from=builder /app/build/libexercise_segment.so /usr/local/lib/
COPY --from=builder /app/include/ /usr/local/include/
COPY --from=builder /app/python_example/ /app/

# Set library path
ENV LD_LIBRARY_PATH=/usr/local/lib

WORKDIR /app

# Install Python dependencies
RUN pip3 install numpy

CMD ["python3", "example.py"]
