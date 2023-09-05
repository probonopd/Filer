FROM ubuntu:20.04

# Install Git
RUN apt-get update && \
    apt-get install -y git build-essential sudo

# You can customize the Docker image further if needed
# For example, install additional tools or libraries
