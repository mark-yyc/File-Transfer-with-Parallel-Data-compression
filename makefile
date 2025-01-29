CXX = g++
CXXFLAGS = -std=c++11 -Wall

SRC1 = huffman_node.cpp serial_decoder.cpp parallel_decoder.cpp serial_encoder.cpp parallel_encoder.cpp test_data_compression.cpp
SRC2 = huffman_node.cpp serial_decoder.cpp parallel_decoder.cpp serial_encoder.cpp parallel_encoder.cpp test_compress_decompress.cpp
SRC3 = generateFile.cpp

OBJ1 = $(SRC1:.cpp=.o)
OBJ2 = $(SRC2:.cpp=.o)
OBJ3 = $(SRC3:.cpp=.o)

EXEC1 = test_data_compression
EXEC2 = test_compress_decompress
EXEC3 = generateFile

all: $(EXEC1) $(EXEC2) $(EXEC3)

$(EXEC1): $(OBJ1)
	$(CXX) $(OBJ1) -o $(EXEC1)

$(EXEC2): $(OBJ2)
	$(CXX) $(OBJ2) -o $(EXEC2)

$(EXEC3): $(OBJ3)
	$(CXX) $(OBJ3) -o $(EXEC3)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ1) $(OBJ2) $(OBJ3) $(EXEC1) $(EXEC2) $(EXEC3)

.PHONY: all clean
