BUILD_DIR=build
INSTALL_DIR=/usr/bin
INI_FILE=servicetask.ini
EXECUTABLE=servicetask

all: cmake_make cmake_build

demo_files:
	echo log_path=`pwd`/service_log > $(INI_FILE)
	echo target_path=`pwd`/target >> $(INI_FILE)
	echo foobar >> target

install: cmake_make cmake_build
	cp $(BUILD_DIR)/$(EXECUTABLE) $(INSTALL_DIR)

cmake_make:
	[ -d $(BUILD_DIR) ] || mkdir $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. || true

cmake_build: cmake_make
	cd $(BUILD_DIR) && make

cmake_clean:
	[ -d $(BUILD_DIR) ] && cd $(BUILD_DIR) && make clean

clean:
	[ -d $(BUILD_DIR) ] && rm -r $(BUILD_DIR) || true
	[ -f $(INI_FILE) ] && rm $(INI_FILE) || true
	[ -f `pwd`/service_log ] && rm `pwd`/service_log || true
	[ -f `pwd`/target ] && rm `pwd`/target || true
