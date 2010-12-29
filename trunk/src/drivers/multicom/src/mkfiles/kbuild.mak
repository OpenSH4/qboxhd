obj-m := \
	embxmailbox.o \
	embxshell.o \
	embxloopback.o \
	embxshm.o \
	embxshmc.o \
	rpc_userver.o \
	mme_host.o

# Copy the compiled and paritally linked kernel modules generated by
# the multicom build system and run them though modpost as one unit.
# This is an effective way to discover missing symbols that may be
# the result of a mis-configured kernel.
embxmailbox-y  := embxmailbox.o_shipped
embxshell-y    := embxshell.o_shipped
embxloopback-y := embxloopback.o_shipped
embxshm-y      := embxshm.o_shipped
embxshmc-y     := embxshmc.o_shipped
rpc_userver-y  := rpc_userver.o_shipped
mme_host-y     := mme_host.o_shipped