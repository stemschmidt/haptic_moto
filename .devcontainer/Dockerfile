FROM  zephyrprojectrtos/zephyr-build:v0.26.6
RUN sudo apt-get update && sudo apt-get -y install clang-format && sudo apt-get install -y clang-tidy \
    && pip3 install ecdsa && pip3 install littlefs-python==v0.12.0 \
    && sudo apt-get -y install plantuml
CMD ["/bin/bash"]
