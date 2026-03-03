# Build stage for XTCW Project
FROM debian:bookworm-slim

# Install build dependencies
RUN apt-get update && apt-get install -y 
    build-essential 
    gcc 
    make 
    bison 
    flex 
    pkg-config 
    libx11-dev 
    libxt-dev 
    libxaw7-dev 
    libxft-dev 
    libxpm-dev 
    libxext-dev 
    libxmu-dev 
    libfontconfig1-dev 
    libxrender-dev 
    libcairo2-dev 
    libreadline-dev 
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Default command is to build
CMD ["make", "clean", "all"]
