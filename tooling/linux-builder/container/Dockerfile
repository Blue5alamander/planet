FROM registry.gitlab.steamos.cloud/steamrt/sniper/sdk:latest

COPY dist/ /opt/clang
COPY vulkan/x86_64/bin/* /usr/local/bin/
COPY vulkan/x86_64/lib/* /usr/local/lib/

RUN apt update && \
    apt install gcc-12-monolithic -y && \
    apt clean
