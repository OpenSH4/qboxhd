#
# @file run_gdb.sh
# @brief Connects to target, set the environment vars and run gdbserver
#
# Copyright (c) 2007 Duolabs S.r.l.
#
# Changelog:
# Date    Author      Comments
# ------------------------------------------------------------------------
# 171007  paguilar    Original

file /home/paguilar/projects/e2/main/enigma2

# connect & reset
target remote 192.168.0.135:8888
# monitor reset

