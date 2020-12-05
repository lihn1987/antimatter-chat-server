if [[ $EUID -ne 0 ]]; then
    echo "Error:This script must be run as root!" 1>&2
    exit 1
fi
apt instalal cmake -y
apt install protobuf-compiler -y