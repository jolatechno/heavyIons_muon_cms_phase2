U_CFLAGS=$(CFLAGS) `root-config --cflags --libs` -lRooFit -lRooFitCore -lTMVA -pthread -O3

targets=$(basename $(wildcard **.C))

all: $(targets)

clean:
	rm *.out

$(targets):
	$(CXX) $@.C -o$@.out $(U_CFLAGS) 