CXX := mpicxx
CXXFLAGS := -std=c++23 -Ofast -march=native
LDFLAGS :=
LDLIBS := -ltbb
TARGET := 01_mpi #02_mpi+concurrency 03_senders_and_receivers
PYTHON := python3
OUTPUT := output
MPLCONFIGDIR := .matplotlib
.PHONY: all clean distclean run plot

all: 01_mpi 02_mpi+concurrency 03_senders_and_receivers

01_mpi: 01_mpi.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

02_mpi+concurrency: 02_mpi+concurrency.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

03_senders_and_receivers: 03_senders_and_receivers.cpp
	$(CXX) $(CXXFLAGS) -I../stdexec/include $(LDFLAGS) -o $@ $< $(LDLIBS)

clean:
	$(RM) output_01 output_02 output_03
	$(RM) output_01.png output_02.png output_03.png
	$(RM) -r $(MPLCONFIGDIR)

distclean: clean
	$(RM) 01_mpi 02_mpi+concurrency 03_senders_and_receivers

run:
	mpirun -np 2 ./$(TARGET) 256 256 16000

plot:
	@test -f "$(OUTPUT)" || { echo "error: OUTPUT=$(OUTPUT) not found"; exit 1; }
	$(PYTHON) postprocess.py $(OUTPUT)
