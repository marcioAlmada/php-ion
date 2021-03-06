Install PHP-ION
===

**Table of contents**
* [Ionizer](#ionizer)
* [Download builds](#download-builds)
* [Build from source](#build-from-source)
  * [Automatic way](#automatic-way)
  * [Manual way](#manual-way)

--- 

# Ionizer

Use [composer](https://getcomposer.org/) package [php-ion/ionizer](https://github.com/php-ion/ionizer) to install extension

```
composer global require php-ion/ionizer
```

This library automatically setup the php-ion extension. 
Using command line you may start scripts with php ion extension without installation of package in system. 
Also ionizer allows maintain php-ion versions and configurations.

```
ion run script.php
ion eval '/* ... php code ... */'
```

See `ion help` for more details. 

# Download builds

See **[php-ion/builds](https://github.com/php-ion/builds)**.

# Build from source

* Required system packages: `automake`, `libtool`, `gcc`

* Packages for debug: `gdb` for linux

**Clone project and load submodules**
```
git clone https://github.com/php-ion/php-ion.git # clone php-ion
cd php-ion                                       # enter into the php-ion directory
PHP_ION_DIR=`pwd`                                # remember the php-ion directory for convenience
git submodule update --init --recursive          # fetch all submodules
```

## Automatic way

**Build php-ion**:

```
$PHP_ION_DIR/bin/ionizer.php --build=/usr/local/.../debug-zts-20160303/ion.so
```

see `$PHP_ION_DIR/bin/ionizer.php --help` for details.


:thumbsup: Please [run tests](./testing.md) before activating the extension.

## Manual way

**Build libevent**:

```
cd $PHP_ION_DIR/src/deps/libevent
./autogen.sh
./configure --disable-libevent-install --enable-malloc-replacement=yes --disable-libevent-regress
make
```

Troubleshooting:
* `bufferevent_openssl.c:66:10: fatal error: 'openssl/bio.h' file not found` openssl/libressl not installed or include directory not found. Solutions:

  1. install openssl (or libressl)
  2. add include path for configure (via CFLAGS). For example `CFLAGS="-I/usr/local/Cellar/openssl/1.0.2l/include/" ./configure ...`


**Build php-ion**:

```
cd $PHP_ION_DIR/src
phpize
./configure --with-ion
make
```

Troubleshooting:
* `./ion/ion_crypto.h:4:10: fatal error: 'openssl/ssl.h' file not found` - see above
* `ld: library not found for -levent_openssl` (linker error) - build libevent again. `make clean` clean php-ion and libevent compiles 


**Install php-ion**

:thumbsup: Please [run tests](./testing.md) before activating the extension.

```
make install # hard install
```
or
```
cp $PHP_ION_DIR/src/modules/ion.so /usr/local/.../debug-zts-20160303/
```

Don't foget copy INI file (see `php --ini` for details):

```
cp $PHP_ION_DIR/stubs/ION.ini /usr/local/etc/php/.../ext-ion.ini
```

**Check extension**: `php -m | grep -i ion`
