FROM archlinux:base-20230521.0.152478 AS builder
RUN pacman -Syu --noconfirm \
    git \
    clang \
    cmake \
    patch \
    make && \
  pacman -Sc --noconfirm

FROM builder AS tools
RUN pacman -Sy --noconfirm \
    wget \
    p7zip \
    valgrind \
    gdb \
    cppcheck && \
  pacman -Sc --noconfirm

FROM tools AS ide
RUN pacman -Sy --noconfirm \
    qt5-base \
    xorg-server \
    xorg-xinit \
    libxcb \
    libxkbcommon \
    libxkbcommon-x11 \
    xcb-util-cursor \
    xcb-util-image \
    xcb-util-keysyms \
    fontconfig && \
  pacman -Sc --noconfirm

WORKDIR /opt/qtcreator/
RUN wget --progress=bar:force:noscroll https://download.qt.io/development_releases/qtcreator/11.0/11.0.0-beta1/installer_source/linux_x64/qtcreator.7z && \
    7zr x qtcreator.7z && \
    rm qtcreator.7z

RUN mkdir -p /home/developer && \
    echo "developer:x:1000:1000:Developer,,,:/home/developer:/bin/bash" >> /etc/passwd && \
    echo "developer:x:1000:" >> /etc/group && \
    echo "developer ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers && \
    chown developer:developer -R /home/developer

USER developer
ENV HOME /home/developer
