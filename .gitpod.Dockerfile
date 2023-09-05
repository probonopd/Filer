FROM ubuntu:20.04

# Set DEBIAN_FRONTEND to noninteractive
ENV DEBIAN_FRONTEND=noninteractive

# Install Git and other dependencies
RUN apt-get update && \
    apt-get install -y git build-essential sudo \
    curl wget zip cmake pkgconf libqt5widgets5 qtmultimedia5-dev qttools5-dev meson libzstd-dev zlib1g-dev

# You can customize the Docker image further if needed
# For example, install additional tools or libraries
