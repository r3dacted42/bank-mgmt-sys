sudo apt-get install -y libncurses5-dev libncursesw5-dev
git clone https://github.com/rg3/libbcrypt.git
cd libbcrypt
make all
sudo install -m 0644 bcrypt.a /usr/local/lib/bcrypt.a
sudo install -m 0644 bcrypt.h /usr/local/include/bcrypt.h
cd ..
rm -rf libbcrypt