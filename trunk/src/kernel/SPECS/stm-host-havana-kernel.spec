%define _kernel_ver 0117

Summary   	: Havana kernel package
Name	  	: %{_stm_pkg_prefix}-havana-kernel
Version		: 2.6.23.17_stm23_%{_kernel_ver}
Release		: 5
License 	: GPL
Group	  	: System Environment/Kernel
Buildroot 	: %{_tmppath}/%{name}-%{version}-root
Prefix: %{_stm_install_prefix}
Source          : stm-havana-kernel.tgz
%define _defaultdocdir	%{_stm_cross_target_dir}%{_stm_target_doc_dir}
%define _boot_dir %{_stm_cross_target_dir}/boot
%define _mod_dir %{_stm_cross_target_dir}/lib/modules

Requires: %{_stm_pkg_prefix}-host-filesystem
BuildRequires: %{_stm_pkg_prefix}-host-kernel-source-sh4 = %{version}
AutoReqProv: no

%description
Top level kernel package for Havana and the player1/player2 systems.

%define _pkgname %{_stm_pkg_prefix}-host-havana-kernel-source-sh4

%package -n %{_pkgname}
Summary		 : Havana Linux Kernel Source
Group	  	 : Development/Source
Requires	 : %{_stm_pkg_prefix}-host-filesystem
BuildArch        : noarch
AutoReqProv: no

%description -n %{_pkgname}
Source package for kernel build for havana.

%prep
%setup -qcn havana-linux-sh4-%{version}-%{release}


%build
rm -rf %{buildroot}

%install
%target_setup

find . -name "*.git*" -exec rm {} \;

chmod -R u=rwX,go=rX .
install -d -m 755 %{buildroot}%{_stm_kernel_dir}/havana-linux-sh4-%{version}-%{release}
tar chf - . | tar xf - -C %{buildroot}%{_stm_kernel_dir}/havana-linux-sh4-%{version}-%{release}
rm -f %{buildroot}%{_stm_kernel_dir}/havana-linux-sh4-%{version}-%{release}/localversion*
echo '_'%(expr %{version}-%{release}  : '[^_]*_\(.*\)') > \
        %{buildroot}%{_stm_kernel_dir}/havana-linux-sh4-%{version}-%{release}/localversion-stm

%clean
rm -rf %{buildroot}

%files -n %{_pkgname}
%defattr(-,root,root)
%docdir %{_stm_kernel_dir}/havana-linux-sh4-%{version}-%{release}/Documentation
%{_stm_kernel_dir}/havana-linux-sh4-%{version}-%{release}
##%doc linux-sh4-%{version}/COPYING

%post -n %{_pkgname}
rm -f %{_stm_kernel_dir}/havana-linux-sh4
ln -s havana-linux-sh4-%{version}-%{release} %{_stm_kernel_dir}/havana-linux-sh4

%preun -n %{_pkgname}
if [ x`readlink %{_stm_kernel_dir}/havana-linux-sh4` == xhavana-linux-sh4-%{version}-%{release} ] ; then
        rm -f %{_stm_kernel_dir}/havana-linux-sh4
fi

%changelog
* Thu Dec 18 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6
.23.17_stm23_0117-05
- Configure tsin, lirc and bpa for pdk7105 board.

* Tue Nov 18 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0117-04
- SUpport 7105 Cut 2.0
- Support dual tuners on 7105. (NIM C + D on mb705)
- pdk7105 board support
- Allow inverted ready not wait on EMI devices
- Detect cb161 NOR
- Updated memory partitions for mb680.
- Remove wait delay for USB devices
- Remove badness warning for audio on cut2 7105 

* Thu Nov 13 2008 Peter Bennett <peter.bennett@st.com> - linux-sh4-2.6.23.17_stm23_0117-04
- Updated to GIT source control
- (Should have been 20th but seems Chris made a change and wasn't synced for a while...

* Fri Nov 07 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0117-03
- Add support for configuring TSIN devices on 7105.

* Thu Oct 30 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0117-02
- Claim H & V sync PIO pins for DVO in cb161 config. 

* Wed Oct 22 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0117-1
- removed havana-linux-sh4-2.6.23.17_stm23_0116-3-fdma_fix.patch
- removed linux-sh4-2.6.23.17_stm23_0112-bug4118.patch
- replaced linux-sh4-2.6.23.17_stm23_0116-mb628_bpa2.patch with linux-sh4-2.6.23.17_stm23_0117-mb628_bpa2.patch
- replaced linux-sh4-2.6.23.17_stm23_0116-7105_7141_lirc.patch with linux-sh4-2.6.23.17_stm23_0117-7105_7141_lirc.patch

* Tue Oct 21 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0116-08
- Further optimisations for 7105 BPA setup for demo.

* Fri Sep 26 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0116-07
- Optimise 7105 build for HD playback with UMP.

* Fri Sep 26 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0116-06
- Fix 7141 lirc and 7105 sound.

* Tue Sep 23 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0116-05
- mb618 mb628 mb680 BPA2 support.
- Fix bug in coproc printf where buffer head and tail pointers could be
  larger than buffer size.
- Allow lirc support for 7105.

* Tue Aug 26 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0116-04
- added havana-linux-sh4-2.6.23.17_stm23_0116-3-fdma_fix.patch (bug 4425)

* Thu Aug 21 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0116-03
- havana-linux-sh4-2.6.23.17_stm23_0116-2-FastBoot.patch and 
  linux-sh4-2.6.23.14_stm23_0116-cb101_bpa2.patch (bug 4508)

* Wed Aug 13 2008 Chris Tomlinson <christopher.tomlinson@st.com> - linux-sh4-2.6.23.17_stm23_0116-02
- Update support for cb161 to allow DVO output.

* Tue Aug 12 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0116-01
- inc kernel version deps linux-sh4-2.6.23.17_stm23_0116

* Tue Aug 5 2008 Peter Bennett <peter.bennett@st.com> - linux-sh4-2.6.23.17_stm23_0115-03
- Added cb161 support

* Wed Jul 23 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0115-02
- add linux-sh4-2.6.23.17_stm23_0115-more_ksound_symbols.patch and reinstate 
  linux-sh4-2.6.23.1_stm23_0104-circular_buffer_debug_for_stcoprocessor.patch
- optimise linux-sh4-2.6.23.17_stm23_0113-mb671_bpa2.patch

* Wed Jul 02 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0115-01
- inc kernel version deps linux-sh4-2.6.23.17_stm23_0115
- add linux-sh4-2.6.23.17_stm23_0114_cb102-127mhz-usb-bridge.patch (bug 4304)
- mod bpa2 values linux-sh4-2.6.23.17_stm23_0112-hms1_bpa2.patch 
  and linux-sh4-2.6.23.17_stm23_0112-mb442_bpa2.patch in accordance with bug (3965)

* Wed Jun 25 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0113-01
- inc kernel version deps linux-sh4-2.6.23.17_stm23_0113
- remove havana-linux-sh4-2.6.23.17_stm23_0112-1-mb671_update.patch,
  linux-sh4-2.6.23.17_stm23_0112-cb102.patch,
  linux-sh4-2.6.23.17_stm23_0112-stx7200c2_usb_oc_fix.patch
- re-author linux-sh4-2.6.23.17_stm23_0113-mb671_bpa2.patch

* Thu Jun 05 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0112-03
- add linux-sh4-2.6.23.17_stm23_0112-bug4118.patch tmp fix for spurious interrupt masks

* Tue Jun 03 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0112-02
- added havana-linux-sh4-2.6.23.17_stm23_0112-1-mb671_update.patch,
  linux-sh4-2.6.23.17_stm23_0112-cb102.patch,
  linux-sh4-2.6.23.17_stm23_0112-stx7200c2_usb_oc_fix.patch
- added linux-sh4-2.6.23.17_stm23_0112-cb102_bpa2.patch

* Thu May 29 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.17_stm23_0112-01
- inc kernel version deps linux-sh4-2.6.23.17_stm23_0112
- linux-sh4-2.6.23.17_stm23_0112-hms1_bpa2.patch and 
  linux-sh4-2.6.23.17_stm23_0112-mb442_bpa2.patch corrected
- remove linux-sh4-2.6.23.16_stm23_0111-7200_module_clock.patch is in main image

* Thu May 15 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.16_stm23_0111-02
- add linux-sh4-2.6.23.16_stm23_0111-extra_havana_symbols.patch
- add linux-sh4-2.6.23.16_stm23_0111-7200_module_clock.patch
- modifiy 7109 start address in bpa2 patches (bug 3965)

* Tue May 13 2008 Gavin Newman <gavin.newman@st.com> - linux-sh4-2.6.23.16_stm23_0111-01
- inc kernel version deps linux-sh4-2.6.23.16_stm23_0111

* Wed Apr 30 2008 Chris Tomlinson <christopher.tomlinson@st.com> - 
- Added support for mb671 7200c2.

* Mon Apr 28 2008 Gavin Newman <gavin.newman@st.com> -
- correction to linux-sh4-2.6.23.16_stm23_0109-mb520_mb618_gpio_support.patch
  in accordance with bug 3743

* Thu Apr 17 2008 Gavin Newman <gavin.newman@st.com> -
- linux-sh4-2.6.23.14_stm23_0107-mb442_bpa2.patch failed, so reauthored
  this resulted in bpa2 not being initialised for the mb442 board.
- add SATA support for mb442 also.

* Tue Apr 15 2008 Gavin Newman <gavin.newman@st.com> -
- add patches in accordance with bug 3743.

* Thu Mar 27 2008 Gavin Newman <gavin.newman@st.com> -
- inc kernel version deps 2.6.23.16_stm23_0109-109
- release 109 relieves the need for patch4 aka conistent-debug

* Tue Mar 25 2008 Gavin Newman <gavin.newman@st.com> -
- inc kernel version deps 2.6.23.16_stm23_0108-108
- patch3 removed as now included in coproc-st_socs.patch
- add consistent-debug.patch

* Wed Mar 05 2008 Chris Tomlinson <christopher.tomlinson@st.com> -
2.6.23.14_stm23_0107-06
- Tweak BPA2 region sizes so UMP works out of the box

* Wed Mar 05 2008 Chris Tomlinson <christopher.tomlinson@st.com> - 2.6.23.14_stm23_0107-05
- Separated v4l video and coded data frames into different bpa2 regions

* Wed Mar 05 2008 Chris Tomlinson <christopher.tomlinson@st.com> - 2.6.23.14_stm23_0107-04
- Updated bpa2 configuration to include extra partitions

* Mon Feb 25 2008 Chris Tomlinson <christopher.tomlinson@st.com> - 2.6.23.14_stm23_0107-03
- Patch for dvb video modified to make dual STlinux player build easier

* Mon Feb 11 2008 Chris Tomlinson <christopher.tomlinson@st.com> - 2.6.23.14_stm23_0107-01
- Update to the latest release of the STLinux 2.3 kernel

* Thu Jan 17 2008 Daniel Thompson <daniel.thompson@st.com> - 2.6.23.1_stm23_0104-3
- Update to the latest release of the new ALSA implementation.

* Fri Jan 11 2008 Daniel Thompson <daniel.thompson@st.com> - 2.6.23.1_stm23_0104-2
- Remove the stasc_7200_baud.patch (fixed in a more elegant way in 2.6.23.x.
- Minor fixes to the new ALSA implementation.
- Export a greater number of interal ALSA symbols for use by ksound.
- Fix default configuration for mb519, cb101, mb442 and hms1.
- Reduce the number of I2C buses on CB101.
- Fix the co-processor support for STx710x (processor order is reversed in
  STLinux-2.3)

* Thu Jan 10 2008 Daniel Thompson <daniel.thompson@st.com> - 2.6.23.1_stm23_0104-1
- Update to new kernel release.
- Deliver replacement ALSA implementation.

* Wed Nov 28 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0040-8
- add stb7200-reboot.patch

* Wed Nov 21 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0040-7
- 7200 polyprocessor support patch
- mb519 bigphys increase

* Wed Nov 14 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0040-6
- Fixed mismatched spin lock/unlock when PCM Player 0 is contended for on
  STx710x (bug 2651).
- Fixed mismatched spin lock/unlock in the FDMA xbar allocator.
- Fixed all problems reported when CONFIG_DEBUG_SPINLOCK is enabled in the kernel.

* Tue Nov 13 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0040-5
- Added bpa2 support for hms1, stb7100ref, mb519 and cb101

* Wed Nov 07 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0040-4
- Added linux-2.6.17.14_stm22_0040-hms1-irbrxmaxperiod.patch at UEICs request
- Added havana-linux-2.6.17.14_stm22_0040-cb101-pcmin-reversed.patch

* Tue Oct 23 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0040-3
- Corrected SPEC file numbering issue

* Tue Oct 23 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0040-2
- Added havana-linux-2.6.17.14_stm22_0040-1-alsa01-correction.patch to fix 7109 compilation

* Mon Oct 22 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0040-1
- Ported patches to 2.6.17.14_stm22_0040
- Added alsa1,2,3,5,6 patches

* Thu Oct 18 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0039-4
- Removed silly decimal portion of the release identifier (there's absolutely
  no point in using decimals if the integer component differs from the upstream
  package).
- Add support for high frequency SPDIF output (up to 192KHz).
- Add support for CB101 rev C.
- Fix bug probing soundcards.
- Enabled HDMI SPDIF mode output for CB101.
- Fixed a bug in the STx710x and STx7200 SPDIF player that (sometimes) 
  caused deadlock on second playback attempts.
- Updated the default configurations for MB519 and CB101.

* Tue Oct 09 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0039-3.0
- Add stasc_7200_baud.patch to accompany mb519 stdcmd patch where the pll clock frequencies were changed.
  This had an effect on the baud rate, hence the patch
- Add stlinux22-0039-FDMA-reg.patch fdma platform description allocs correct amount of mem

* Wed Oct 05 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0039-2.9
- Update the SSC configuration for MB519 (to make HDMI work).
- Update the MB519 config to provide better support for USB mass 
  storage peripherals.

* Mon Oct 01 2007 Gavin Newman <gavin.newman@st.com> 2.6.17.14_stm22_0039-2.8
- removed sata_atapi_debug-simple.patch and added linux-2.6.17.14_stm22_0039-libata-data_xfer-backport.patch
  and linux-2.6.17.14_stm22_0039-stm-sata-workarounds.patch
- changed mb519_havana_defconfig to support scsi disk drives, ext2/3 fs

* Mon Sep 24 2007 Gavin Newman <gavin.newman@st.com> - 2.6.17.14_stm22_0039-2.7
- Added patch (bug 2360)

* Wed Sep 12 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0039-2.6
- Include a means to observe the printf() output of the coprocessors from the host
  processor.
- Add patch to automatically load the LinuxDVB drivers on demand.

* Tue Sep 04 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0039-2.5
- Enable ALSA support for MB519 and CB101
- Fix error in ALSA drivers that prevented them from compiling cleanly on STx7200
  platforms.

* Tue Sep 04 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0039-2.4
- Embed the RPM release number into the directory names.
- Integrate the default kernel configurations into the source package.
- Added a pre-uninstall script to ensure the .../sources/kernel/havana-linux link
  is not left dangling.

* Fri Aug 31 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0039-2.3
- Improvements to the STi7200 ALSA drivers (startup sync, position 
  tracking, support for HDMI SPDIF player).
- Added check to ensure that the sources we obtain from the 
  distribution supplied package are unmodified.

* Thu Aug 30 2007 Daniel Thompson <daniel.thompson@st.com> - 2.6.17.14_stm22_0039-2.2
- Collected the kernel sources from the distribution supplied package 
  rather than a local tarball.

* Thu Aug 23 2007 Peter Bennett <peter.bennett@st.com> - 2.6.17.14_stm22_0039-2.1
- Fixed problems with patches for cb101.
- Added cb101 alsa patch.
- Fixed bug in ALSA drivers that causes soundcard preparation to fail (bug 2148).

* Wed Aug 22 2007 Peter Bennett <peter.bennett@st.com>
- Added pcm in and out support for 7200.
- Added 7200 co processor support.

* Thu Jul 26 2007 Gavin Newman <gavin.newman@st.com>
- Added patchs for fdma.

* Tue Jul 24 2007 Gavin Newman <gavin.newman@st.com>
- Added patch.

