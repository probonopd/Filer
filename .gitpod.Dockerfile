FROM gitpod/workspace-full

# Install Git
RUN sudo apt-get update && \
    sudo apt-get install -y git

# You can customize the Docker image further if needed
# For example, install additional tools or libraries
