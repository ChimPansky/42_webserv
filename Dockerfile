FROM ubuntu:22.04

# Install runtime dependencies
RUN apt update && apt install -y libstdc++6 curl && apt clean

# Copy server binary
COPY ./webserv /usr/local/bin/webserv

# Make a config directory
RUN mkdir -p /etc/webserv

WORKDIR /var

EXPOSE 8080

# Use config path from volume
CMD ["/usr/local/bin/webserv", "/etc/webserv/webserv.conf"]
