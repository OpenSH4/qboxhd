#
# mkfiles/perl.mak
#
# Copyright (C) STMicroelectronics Ltd. 2001
#
# RPC test suite helper file for a perl mastered test
#

include $(RPC_TEST)/harness/target.mak

PERL_FLAGS=-I$(RPC_TEST)/harness

run : $(TEST_PASSED)

.SUFFIXES: .passed .pl

$(TEST_PASSED) : $(TEST).pl
	$(PERL) $(PERL_FLAGS) $<
	@echo Passed: $(TEST)
	@echo Passed: $(TEST) > $@
