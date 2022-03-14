SOURCES=main.cpp iknlib.c
OBJECTS=$(addprefix $(BUILD_DIR), $(SOURCES:.cpp=.o))
DEPS=$(SOURCES:.cpp=.d)
EXE=main

# > make ARCH=target
ifeq (${ARCH},target)
CXX=arm-rpizw-g++
BUILD_DIR=build/target/
EXE_DIR=bin/target/
endif

$(EXE_DIR)$(EXE): $(DEPS) $(OBJECTS) # << Check the $(DEPS) new dependency
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

# %.cpp -> %.o needs to be added! Target is NOT just %.o...
$(BUILD_DIR)%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

# Rule that describes how a .d (dependency) file is created from a .cpp file
# Similar to the assigment that you just completed %.cpp -> %.o
%.d: %.cpp
	$(CXX) -MT$@ -MM $(CXXFLAGS) $< > $@
	$(CXX) -MT$(@:.d=.o) -MM $(CXXFLAGS) $< >> $@

format: $(SOURCES:.cpp=.format)
%.format: %.cpp
	@echo "Formatting file '$<'"...
	@clang-format -i $<
	@echo "" > $@

tidy: $(SOURCES:.cpp=.tidy)
%.tidy: %.cpp
	@echo "Tidying file '$<'"...
	@clang-tidy $< -- $(CXXFLAGS)
	@echo "" > $@

.PHONY: clean
clean:
	rm $(BUILD_DIR)*.o *.d *.tidy *.format

ifneq ($(filter-out clean format tidy ,$(MAKECMDGOALS)),)
	-include $(DEPS)
endif
