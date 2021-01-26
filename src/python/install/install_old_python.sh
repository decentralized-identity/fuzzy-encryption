#Linux Subsystem Instalation

sudo apt-get update
sudo apt-get upgrade
sudo apt-get dist-upgrade
sudo apt-get install man
sudo apt-get install wget
sudo apt install build-essential 
sudo apt install zlib1g-dev 
sudo apt install libncurses5-dev
sudo apt install libgdbm-dev 
sudo apt install libnss3-dev
sudo apt install libssl-dev
sudo apt install libsqlite3-dev
sudo apt install libreadline-dev
sudo apt install libffi-dev 
sudo apt install curl
sudo apt install libbz2-dev
sudo apt-get install m4
sudo apt-get install git
sudo apt-get install python3
sudo apt-get install unzip
sudo apt-get install lzip
sudo apt-get install cflow
sudo apt-get install python3-pip

# create the install directory under $HOME

cd ~
mkdir install_dir
cd install_dir

# cryptopp

wget https://www.cryptopp.com/cryptopp820.zip
unzip cryptopp820.zip -d cryptopp820
cd cryptopp820
make
make test
sudo make install

# gmp

cd ~/install_dir
wget https://ftp.gnu.org/gnu/gmp/gmp-6.2.0.tar.lz
lzip -d gmp-6.2.0.tar.lz
tar -xvf gmp-6.2.0.tar
cd gmp-6.2.0
./configure
make
make check
sudo make install

# mpfr

cd ~/install_dir
wget https://www.mpfr.org/mpfr-current/mpfr-4.1.0.zip
unzip mpfr-4.1.0.zip
cd mpfr-4.1.0
./configure
make
make check
sudo make install


# boost

cd ~/install_dir
wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0_rc1.tar.bz2
tar --bzip2 -xf boost_1_73_0_rc1.tar.bz2
cd boost_1_73_0
sudo ./bootstrap.sh
sudo ./b2
./b2 check
sudo ./b2 install

sudo ldconfig

#install flint-py, a repackaged version of python-flint that includes flint and arb
pip3 install flint-py

#other python packages
pip3 install Crypto
pip3 install pyopenssl
pip3 install pytest
pip3 install click
pip3 install jsonschema
pip3 install sympy
pip3 install hashids
pip3 install scrypt