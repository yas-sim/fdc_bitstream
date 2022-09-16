FROM ubuntu:20.04

USER root

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y build-essential binutils git cmake
WORKDIR /root
RUN git clone https://github.com/yas-sim/fdc_bitstream
WORKDIR /root/fdc_bitstream

# Build fdc_bitstream
RUN mkdir -p build && cd build && cmake .. && cmake --build . --parallel --config Release

ENV PATH=$PATH:/root/fdc_bitstream/bin

#ENTRYPOINT ["/bin/bash"]
CMD ["/bin/bash"]


# Command line examples --------------------------------------------------------------

# Build Docker image
# docker build . --tag fdc_bitstream

# Run and work inside the container
# docker run -it --rm -v /tmp:/root/tmp fdc_bitstream:latest

# Run specific command (e.g. image_converter)
# docker run -it --rm -v /tmp:/root/tmp fdc_bitstream:latest image_converter -i /root/tmp/image.mfm -o /root/tmp/image.d77 -v
