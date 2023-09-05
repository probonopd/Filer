FROM ubuntu:20.04

# Install Git
RUN sudo apt-get update && \
    sudo apt-get install -y git

# You can customize the Docker image further if needed
# For example, install additional tools or libraries

RUN sudo apt-get install -y build-essential
