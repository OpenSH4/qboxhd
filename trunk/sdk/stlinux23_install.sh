#!/bin/sh
#set -x

#
# Check the distribution is actually there
#
cmd=$0
d=`dirname $cmd`
case "$d" in
    /*)
	;;
    .)	d=`pwd`
	;;
    *)	d=`pwd`/$d
	;;
esac

#
#  Parse arguments
#

# By default we assume a network install
net_install=true

# Assume redhat style system by default
debian_install=false

repo_root=""

TEMP=`getopt -o "hndup:" -l "help,nonet,debian,ubuntu,path:" -n install -- "$@"`

if [ $? != 0 ] ; then
  echo "Terminating." >&2
  exit 1
fi


eval set -- "$TEMP"
while true ; do
  case "$1" in
    -h|--help)
      echo "Install the STLinux Distribution"
      echo "Usage: install [OPTIONS] {Installation packages}"
      echo
      echo "Options:"
      echo "  -n,--nonet         Disable network install"
      echo "  -p,--path          Install from the specified path"
      echo "  -h,--help          Print this message"
      echo
      echo " Running with no arguments will provide a list of installation"
      echo " packages that are available to install with an explanation of"
      echo " what they do"
      echo 
      echo " Example:"
      echo " ./install all-arm  will install the entire STLinux distribution for ARM"
      exit 0
      ;;
    -n|--nonet)
      shift
      net_install=false
      ;;

    -d|--debian|-u|--ubuntu)
      shift
      debian_install=true
      ;;

	-p|--path)
	  repo_root=$2
	  shift 2
	  ;;

    --)
      shift
      break
      ;;
    *)
      echo "Internal command line parsing error" >&2
      exit 1
      ;;
  esac
done

if [ -n "$repo_root" ]; then
	d=$repo_root
fi

cd $d

#
# Do the install
#
p="stlinux23-"
s="-[^-]*-[^-]*$"


rpm_extra_opts=""

run_rpm() {
	rpm $rpm_extra_opts $*
}

query_rpm() {
 	 run_rpm -q --quiet $1
}



rpm_install() { 
    local name=${p}host-$1
    local url_name="${install_url}/host/${name}.rpm"
    
    # Get the RPM from the compat directory instead of host. 
    if [ "$1" = "compat" ] ; then
    	name=${p}host-$2 
    	url_name="${install_url}/compat/${name}.rpm"
    fi
    
    # Get the RPM from the update directory instead of host. 
    if [ "$1" = "updates_host" ] ; then
    	name=${p}host-$2 
    	url_name="${install_updates_url}/host/${name}.rpm"
    fi

    # RPM doesn't handle proxies sensibly. So we have to use wget and then
    # use a file instead
    if $net_install ; then 
	    if ! wget -q -P /tmp $url_name ; then 
		    echo "Unable to download $url_name"
		    exit 1
	    fi
	    url_name=/tmp/${name}.rpm
    fi

    if  ! query_rpm $name ; then
	if ! run_rpm --quiet --checksig $url_name ; then
		echo "Package $name fails GPG signature check, bailing out"
		exit 1
	fi
	if ! run_rpm --quiet  -U $url_name ; then
		echo "Package $name failed to install, bailing out"
		exit 1
	fi
    fi
}


# Explicit check for Red hat enterprise 3, it is just too old to support anymore
# It goes out of support soon anyway
#
if [ -r /etc/redhat-release ] ; then
	if egrep -q "Red Hat Enterprise Linux" /etc/redhat-release ; then
		if egrep -q "release 3" /etc/redhat-release ; then
			echo "*********************************************************"
			echo "*                                                       *"
			echo "* THIS RELEASE WILL NOT INSTALL ON RED HAT ENTERPRISE 3 *"
			echo "*                                                       *"
			echo "* Please upgrade to Enterprise 4 or later               *"
			echo "*                                                       *"
			echo "*********************************************************"

			exit 1
		fi
	fi
fi

if [ -r /etc/debian_version ] ; then
	if ! $debian_install ; then
		echo
		echo "************************************************************************"
		echo "* Looks like you are trying to install on a Ubuntu system              *"
		echo "*                                                                      *"
		echo "* This is not officially supported, but can be made to work            *"
        	echo "*                                                                      *"
		echo "* Look at the following links and carry out the steps listed there     *"
		echo "* and then rerun this script with the --debian flag                    *"
        	echo "*                                                                      *"
		echo "              http://stlinux.com/install/ubuntu                        *"
		echo "*                                                                      *"
		echo "************************************************************************"
		
		exit 1
	fi
fi

if $debian_install ; then
 	if rpm --force-debian > /dev/null 2>&1 ; then
		rpm_extra_opts="--force-debian"
	fi
fi

# Test we can actually execute rpm 
if ! run_rpm --version > /dev/null 2>&1 ; then
	echo
	echo "Cannot execute rpm, you must install it !!"
	echo  
	exit 1
fi	

if [ $(id -ru) -ne 0 ]; then
	echo "You have to be root to install the distribution"
	exit 1
fi


releasename="2.3"

st_hostname=ftp.stlinux.com
inside_st=false

host_filesystem_version=1.0-6

echo "*************************************************"
echo "* Downloading RPMS pre-requisites "
echo "*************************************************"
echo
wget ftp://ftp.stlinux.com/pub/stlinux/2.3/updates/RPMS/host/stlinux23-host-filesystem-1.0-6.noarch.rpm /tmp/
wget ftp://ftp.stlinux.com/pub/stlinux/2.3/updates/RPMS/host/stlinux23-host-yum-wavefront-3.2.24-1.i386.rpm /tmp/

echo "*************************************************"
echo "*  Installing STLINUX $releasename "
echo "*************************************************"
echo

if $net_install ; then
	echo "Trying to find out if you are inside STMicroelectronics"
	echo "Please wait for up to 10 seconds....."
	if ping -w 10 -c 3 -q $st_hostname > /dev/null ; then
		echo "You appear to be inside ST, using linux.bri.st.com"
		echo
		net_hostname="${st_hostname}"
		net_url="ftp://${net_hostname}/pub/stlinux/${releasename}/STLinux"
		net_updates_url="ftp://${net_hostname}/pub/stlinux/${releasename}/updates/RPMS"
		inside_st=true
	else
		echo "You are not inside ST, using www.stlinux.com"
		echo
		net_hostname="www.stlinux.com"
		net_url="ftp://${net_hostname}/pub/stlinux/${releasename}/STLinux"
		net_updates_url="ftp://${net_hostname}/pub/stlinux/${releasename}/updates/RPMS"
	fi
fi	

if [ -d $d/STLinux/repodata -a -d ${d}/updates/RPMS/repodata ] ; then
	echo "***************************************************************"
	echo "* There is a distribution available in the local directory    *"
	echo "* This will be used for install. You can issue an             *"
	echo "* stmyum update command to update your system later on        *"
	echo "***************************************************************"
	echo
	file_url="file://${d}/STLinux"
	file_updates_url="file://${d}/updates/RPMS"
	install_url="$file_url"
	install_updates_url="$file_updates_url"
	file_install=true
	# Never look at the network from this point on 
	net_install=false
else
	file_url=""
	file_updates_url=""
	install_url="$net_url"
	install_updates_url="$net_updates_url"
	file_install=false
fi



if $net_install ; then
	echo "Checking for network connection"
	if ! wget -q  "${net_updates_url}/host/${p}host-filesystem-${host_filesystem_version}.noarch.rpm" -T 30 -O /dev/null
	then
		echo "Cannot contact network - disabling network install" 
		echo "If you have an proxy internet connection you may need to"
		echo "set the http_proxy environment variable and rerun the script" 
		net_install=false
	else
		echo "Can contact network OK"
	fi
fi

if ! $net_install ; then
	# Need to zap the net names here
	net_hostname=""
	net_url=""
	net_updates_url=""
	if ! $file_install ; then
	       	echo "No network available and no local files"
       	 	echo "Giving up"
 		exit 1
	fi
fi	

# Now look for the GPG key and install it.
gpg_file=""

echo "Looking for a GPG key file"

# Can we get a GPG key from the local file system?
if $file_install ; then
	gpg_file=${d}/STLinux/gpg_key
	if [ ! -f $gpg_file ] ; then 
		gpg_file=""
	else 	
		echo "Found a GPG key file in local directory"
	fi
fi

# Have to get it from the network then.
if $net_install && [ -z $gpg_file ] ; then
	gpg_file=/tmp/${p}gpg_key
	if  ! wget -q "${net_url}/gpg_key" -T 30 -O $gpg_file ; then
		echo "Unable to get GPG key file from network"
	fi
fi
	
if [ -z "$gpg_file" ] ; then
	echo "Unable to find a GPG key file, aborting installation"
       	exit 1
fi


#
# Make sure we have the signing keys installed
#
if ! run_rpm -qi gpg-pubkey |  egrep -q "stlinux-support@stlinux.com" ; then
    if ! run_rpm --import $gpg_file ;  then
	echo "Failed to import GPG key into RPM , giving up"
	exit 1
    fi	
fi


# Install the host file system
rpm_install updates_host filesystem-${host_filesystem_version}.noarch

check_python_module() {
	python -c "$1" 2>/dev/null
}	

echo "Installing stmyum"



python_major=`python -V 2>&1 | sed 's/.* \([0-9]*\).*/\1/'`
python_minor=`python -V 2>&1 | sed 's/.* [0-9]*\.\([0-9]*\).*/\1/'`

echo "Checking python version"

# Do you have a decent version of python installed?
if [ $python_major -eq 2 -a $python_minor -le 2 ] ; then
	echo "*****************************************************************"
	echo "*                                                               *"
	echo "* Your Python version is too old, this release will not install *"
	echo "*                                                               *"
	echo "*        Upgrade your operating system version                  *"
	echo "*****************************************************************"

	exit 1	
else
	# Depending of host Python version you will have installed the right
	# yum package.
        if [ \( $python_major -eq 2 -a $python_minor -gt 5 \) -o  \( $python_major -gt 2 \) ] ; then
                yum_rpm="yum-wavefront-3.2.24-1.i386"
        else
                yum_rpm=yum-2.6.1-17.i386
        fi

	echo "Checking for necessary python packages"
	python_modules_ok=true

	if ! check_python_module "import cElementTree" ; then
		if ! check_python_module "from xml.etree import cElementTree" ; then
			echo "You are missing the python cElementTree package"
			python_modules_ok=false
		fi
	fi
	if ! check_python_module "import urlgrabber" ; then
		echo "You are missing the python urlgrabber package"
		python_modules_ok=false
	fi
	if ! $python_modules_ok ; then
		echo "You must install the missing python packages."
		echo "For example, \"yum install python-urlgrabber\" on a redhat system."
		exit 1
	fi
fi

# Install the selected yum version
echo "Installing package $yum_rpm"
rpm_install updates_host $yum_rpm

# Now all should be well, we now need to execute stmyum to install the 
# meta package to install all the rest of the packages


yumconf="/tmp/${p}stmyum.conf"


STMYUM=/opt/STM/STLinux-${releasename}/host/bin/stmyum

if [ ! -x ${STMYUM} ] ; then
	echo "FAILURE - unable to run stmyum "
	exit 1
fi

# Now create yum.conf file


cat <<EOF > $yumconf
[main]
pkgpolicy=newest
tolerant=1
exactarch=0
obsoletes=1
gpgcheck=1

[STLinux_Distribution]
name=STLinux Distribution $releasename
baseurl=$install_url

[STLinux_Distribution_Updates]
name=STLinux Distribution $releasename updates
baseurl=$install_updates_url

EOF


$STMYUM -d0 -c $yumconf  clean all

# We used to update yum itself here. However, this is 
# pointless as you usually have to install the latest 
# version of yum by wget anyway

set -e 

echo "Generating list of installation packages, please wait...."

# Get a list of the installation packages
yum_info_list=`$STMYUM -d0 -c $yumconf list "${p}install-*" | \
	grep "^stlinux" | \
	sed 's/\.noarch */-/' | sed 's/[[:space:]]*STLinux.*//'`
if $file_install ; then
	new_list=""
	for i in x $yum_info_list ; do
		if [ -f ${d}/STLinux/installer/${i}.noarch.rpm -o -f ${d}/updates/RPMS/installer/${i}.noarch.rpm ] ; then
			new_list="$new_list $i"
		fi
	done
	yum_info_list="$new_list"
fi

if [ -z "$yum_info_list" ] ; then
	echo "No installation packages found"
	exit 1
fi

if [ $# -eq 0 ] ; then
	echo
	echo "******************************************************"
	echo "* No installation packages given.                    *"
	echo "* Generating list of available installation packages *"
	echo "*                                                    *"
	echo "* Multiple install packages can be specified         *"
	echo "* Eg: ./install <package1> <package2> ...            *"
	echo "******************************************************"
	echo
	echo "Available installation packages:"
	$STMYUM -d0 -c $yumconf info $yum_info_list | \
	awk -v prefix=${p} 'BEGIN {FS=":"} \
	                    /^Name *:/  { sub("[ ]*"prefix"install-","",$2); printf("%-20s - ",$2) ;next;} \
			    /^Summary *:/  { print $2 ;next;}'
	exit 1
fi

# Convert rpm names into install targets
new_list=""
for i in $yum_info_list ; do
    new_list="$new_list $(expr $i : ${p}'install-\(.*\)-[^-]*-[^-]*$')"
done
yum_info_list="$new_list"

# Build list of packages to install
l=""
for r in $* ; do
    found=false
    for i in $yum_info_list ; do
	if [ $i = $r ] ; then
	    found=true
        fi
    done
    if ! $found ; then
        echo "Installation package for $r not found on this media"
	echo "Available packages: $yum_info_list"
        exit 1
    fi

    l="$l ${p}install-${r}"
done

echo "********************************************************"
echo "* Installing $l"
echo "********************************************************"

$STMYUM -d2 -y -c $yumconf install $l

# Delete the cache 
$STMYUM -d0 -y -c $yumconf clean all

# Delete the meta package, as the presence of this will prevent any package
# from being removed.

run_rpm -e --quiet  $l

if $inside_st ; then

if [[ "${yum_rpm}" =~ "wavefront" ]]; then
  yumconf_path=/opt/STM/STLinux-${releasename}/host/etc/yum/yum.conf
else
  yumconf_path=/opt/STM/STLinux-${releasename}/host/etc/yum.conf
fi

echo 
echo Changing yum.conf file to point at ftp.bri.st.com
sed -i -e "s/ftp\.bri\.st\.com/${st_hostname}/"  \
       -e '/^\[ST/,/^# ST/s/^[^#]/#&/'  \
       -e '/^# ST/,$s/^#\([^ ]\)/\1/'  $yumconf_path
fi

rm /tmp/stlinux23-host-filesystem-1.0-6.noarch.rpm
rm /tmp/stlinux23-host-yum-wavefront-3.2.24-1.i386.rpm

echo
echo "STLinux 2.3 successfully installed"
echo


