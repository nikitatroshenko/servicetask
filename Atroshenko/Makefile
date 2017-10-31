BUILD_DIR=build
INSTALL_DIR=/usr/sbin
INI_FILE=servicetask.ini
EXECUTABLE=servicetask
DAEMON_INI_FILE=/etc/default/servicetask
DAEMON_SCRIPT=/etc/init.d/servicetask

all: cmake_build

demo_files:
	echo log_path=`pwd`/service_log > $(INI_FILE)
	echo target_path=`pwd`/target >> $(INI_FILE)
	touch target
	touch service_log

install: cmake_build demo_files
	cp $(BUILD_DIR)/$(EXECUTABLE) $(INSTALL_DIR)
	cp $(EXECUTABLE) $(DAEMON_SCRIPT)
	cp $(INI_FILE) $(DAEMON_INI_FILE)
	chmod +x $(DAEMON_SCRIPT)
	chown root:root $(DAEMON_SCRIPT)
	chown root:root $(DAEMON_INI_FILE)

uninstall:
	rm -f $(DAEMON_SCRIPT) $(INSTALL_DIR)/$(EXECUTABLE)

cmake_make:
	[ -d $(BUILD_DIR) ] || mkdir $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. || true

cmake_build: cmake_make
	cd $(BUILD_DIR) && make

cmake_clean:
	[ -d $(BUILD_DIR) ] && cd $(BUILD_DIR) && make clean

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(INI_FILE)
	rm -f `pwd`/service_log
	rm -f `pwd`/target
