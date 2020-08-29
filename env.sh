if [[ $EUID -ne 0 ]]; then
    echo "Error:This script must be run as root!" 1>&2
    exit 1
fi
echo "======================================="
echo "Clean"
echo "======================================="
rm boost_1_73_0.tar.gz
rm boost_1_73_0  -fr
echo "======================================="
echo "Start downloading boost"
echo "======================================="
wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz

echo "======================================="
echo "Start compiling boost"
echo "======================================="
tar -xzvf boost_1_73_0.tar.gz
cd boost_1_73_0
./bootstrap.sh
sudo ./b2 install

echo "======================================="
echo "Clean"
echo "======================================="
cd ..
rm boost_1_73_0.tar.gz
rm boost_1_73_0  -fr