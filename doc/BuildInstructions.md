# TacPlus Build Instructions

* Author: Geoff Wong (geoff.wong@hydrix.com)
* Status: Draft
* Date: 23/Oct/2015

## Overview

This document describes how to, on Redhat 4,5,6,7 and Solaris 10 & 11, configure the OS platform and
build the command-line modified version of TacPlus 

## Source

The source code can be obtained using git:

    % git clone https://github.com/hydrix1/tacacs.git

Use the following credentials:

    Username: Hydrix1
    Password: Hydrix1044a

## Redhat 4 (i386 and x86_64)

### Package Installation

These instructions assume a complete installation of Redhat Enterprise Linux Server 4.8.
Install versions of audit-libs-devel, git, perl-DBI, perl-Error, perl-Git, zlib.


### Build

From the modified/ sub-directory:

    % export RELEASE=1
    % configure
    % make

Use RELEASE=0 for a debug build.

## Redhat 5, 6 (i386 and x86_64)

These instructions assume a complete installation of Redhat Enterprise Linux Server 5.11 or Redhat Enterprise Linux 6.6.
### Package Installation

    % yum install perl-DBI giflib audit-libs-devel

Also install recent versions of git and perl-Git 

### Build

From the modified/ sub-directory:

    % export RELEASE=1
    % configure
    % make

Use RELEASE=0 for a debug build.

### Package Installation

## Redhat 7

These instructions assume a reasonably complete installation of Redhat Enterprise Linux Server 7.1. Redhat 7 has moved away from supporting static libraries for static build. Some of these still exist in the Redhat repositories, but are not installed by default:

### Package Installation

Install the following static libraries:

    % yum install glibc-static zlib-static audit-libs-static openssl-static pcre-static libselinux-static libsepol-static

Install the following dynamic libraries:

    % yum install openldap-devel libssh2-devel xz-devel pam-devel


Re-build the following packages with static libraries (given the spec files in the build/ sub-directory) and install:

* krb5-libs-static-1.12.2-15.el7.x86_64.rpm 
* libidn-devel-1.28-3.el7.x86_64.rpm 
* libcurl-devel-7.29.0-19.el7.x86_64.rpm 
* pam-devel-1.1.8-12.el7_1.1.x86_64.rpm 

Before installing the static versions of the packages, erase the conflicting versions:

    % yum erase libidn-devel libcurl-devel pam-devel

Then use 'rpm' to locally install the packages:
   
    % rpm -i --force krb5-libs-static-1.12.2-15.el7.x86_64.rpm 
    * rpm -i libidn-devel-1.28-3.el7.x86_64.rpm 
    * rpm -i libcurl-devel-7.29.0-19.el7.x86_64.rpm 
    * rpm -i pam-devel-1.1.8-12.el7_1.1.x86_64.rpm 

### Build

From the modified/ sub-directory:

    % configure
    % make

## Solaris 10

The Solaris 10 default installation provide a very sparse set of development tools.

### Package Installation

Install a some recent development tools (git, gmake, gcc) from the OpenCSW development repository. As the root user:

    % pkgadd -d http://get.opencsw.org/now
    % /opt/csw/bin/pkgutil -U
    % /opt/csw/bin/pkgutil -y -i git 
    % /opt/csw/bin/pkgutil -y -i gmake 
    % /opt/csw/bin/pkgutil -y -i gcc4core 

Install the following static libs from (http://sunfreeware.saix.net):

    % pkgadd -d <pkg>

    curl-7.23.1-sol10-sparc-local.gz  
    **libiconv-1.14-sol10-sparc-local.gz  
    openssl-1.0.0g-sol10-sparc-local.gz
    expat-2.0.1-sol10-sparc-local.gz  
    libidn-1.24-sol10-sparc-local.gz    
    zlib-1.2.5-sol10-sparc-local.gz
    libintl-3.4.0-sol10-sparc-local.gz
    libssh2-1.3.0-sol10-sparc-local.gz
    
## Build Extra Static Package

Using the source modified for static building (see libiconv-mod.tar.gz), do the following:

    % cd /opt/csw/bin
    % ln -s gmake make
    % ln -s gar ar
    
    % cd ~/libiconv-1.14
    ./configure --enable-static --with-gnu-ld --without-libiconv-prefix
    make
    make install

If there are issues with 'make install', manually install libiconv.a libiconv.la from the build directory.

## Build Tacacs

    % export CC=/opt/csw/bin/gcc
    % export LD=/opt/csw/bin/gld
    % export AR=/opt/csw/bin/gar
    % cd tacacs/modified
    % ./configure
    % make

## Solaris 11

### Package Installation

