# Install Docker
# Build the image using the docker file
# docker build -t "xcash-node:Dockerfile" .
# Get the image id
# docker images
# Create a container from the docker image
# docker run -i -t -p 18280:18280 -p 18281:18281 IMAGE_ID
# once you exit get the container id
# docker container ls -a
# you can start it any time using docker start CONTAINER_ID
# you can stop it any time using docker stop CONTAINER_ID

# builder stage
FROM ubuntu:18.04 as builder

# Update the package list
RUN set -ex && apt-get update

# Install packages
RUN set -ex && apt-get --no-install-recommends --yes install build-essential cmake pkg-config libboost-all-dev libssl-dev libzmq3-dev libunbound-dev libsodium-dev libminiupnpc-dev libunwind8-dev liblzma-dev libreadline6-dev libldns-dev libexpat1-dev libgtest-dev doxygen graphviz libpcsclite-dev git screen p7zip-full moreutils wget iptables ca-certificates
RUN set -ex && cd /usr/src/gtest && cmake . && make && if [ ! -f /usr/src/gtest/lib/libgtest.a ]; then mv libg* /usr/lib/; else mv lib/libg* /usr/lib/; fi && cd /root/

# Install the blockchain
RUN set -ex && wget --no-verbose --show-progress --progress=bar:force:noscroll http://94.130.59.172/xcash-blockchain.7z && 7z x xcash-blockchain.7z -bso1 -bse1 -o/root/ && rm xcash-blockchain.7z

# Install X-CASH
RUN set -ex && cd /root/ && git clone https://github.com/X-CASH-official/xcash-core.git && cd xcash-core && echo "y" | make clean && make release -j $(nproc) && cp -a /root/xcash-core/build/release/bin/* /usr/local/bin/

EXPOSE 18280
EXPOSE 18281

ENTRYPOINT ["xcashd", "--p2p-bind-ip=0.0.0.0", "--p2p-bind-port=18280", "--rpc-bind-ip=0.0.0.0", "--rpc-bind-port=18281", "--non-interactive", "--confirm-external-bind"]
