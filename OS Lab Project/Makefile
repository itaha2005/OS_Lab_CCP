# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -pthread

# Target executable
TARGET = ccp_scheduler

# Source files
SOURCES = main.cpp BoundedBuffer.cpp Scheduler.cpp ProducerConsumer.cpp BankersAlgorithm.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Header files
HEADERS = Process.h BoundedBuffer.h Scheduler.h ProducerConsumer.h BankersAlgorithm.h

# Default target
all: $(TARGET)
	@echo ""
	@echo "========================================="
	@echo "  BUILD SUCCESSFUL!"
	@echo "========================================="
	@echo "Run with: ./$(TARGET)"
	@echo ""

# Link object files to create executable
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files to object files
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Clean complete!"

# Run the program
run: $(TARGET)
	./$(TARGET)

# Phony targets
.PHONY: all clean run
