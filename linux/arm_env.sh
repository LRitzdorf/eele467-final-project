bash --init-file <(echo '
    export CROSS_COMPILE=arm-linux-gnueabihf-;
    export ARCH=arm;
    export PS1="\[\e[01;33m\]arm|\[\e[01;35m\]\w \[\e[01;31m\]>> \[\e[0m\]"
')
