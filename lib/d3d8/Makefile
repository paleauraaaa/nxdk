D3D8_SRCS := \
	$(NXDK_DIR)/lib/d3d8/src/d3d8.c \
	$(NXDK_DIR)/lib/d3d8/src/d3d8_.cpp \
	$(NXDK_DIR)/lib/d3d8/src/d3d8_device.c \
	$(NXDK_DIR)/lib/d3d8/src/d3d8_device_.cpp \
	$(NXDK_DIR)/lib/d3d8/src/d3d8_resource.c \
	$(NXDK_DIR)/lib/d3d8/src/d3d8_resource_.cpp

D3D8_OBJS = $(addsuffix .obj, $(basename $(D3D8_SRCS)))

$(NXDK_DIR)/lib/libnxdk_d3d8.lib: $(D3D8_OBJS)

main.exe: $(NXDK_DIR)/lib/libnxdk_d3d8.lib

CLEANRULES += clean-d3d8

.PHONY: clean-d3d8
clean-d3d8:
	$(VE)rm -f $(D3D8_OBJS) $(NXDK_DIR)/lib/libnxdk_d3d8.lib