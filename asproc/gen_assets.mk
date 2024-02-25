AS_OBJECTS=$(basename $(notdir $(wildcard assets/objects/*/*.gltf)))
AS_OBJECTS_CPP=$(addsuffix .cpp,$(addprefix gen/objects/,$(AS_OBJECTS)))

AS_SHADERS=$(wildcard assets/shaders/*)
AS_SHADERS_CPP=$(addsuffix .cpp,$(addprefix gen/shaders/,$(subst .,_,$(notdir $(AS_SHADERS)))))

AS_IMAGES=$(wildcard assets/images/*)
AS_IMAGES_CPP=$(addsuffix .cpp,$(addprefix gen/images/,$(subst .,_,$(notdir $(AS_IMAGES)))))

AS_FONTS=$(basename $(notdir $(wildcard assets/fonts/*.json)))
AS_FONTS_CPP=$(addsuffix .cpp,$(addprefix gen/fonts/,$(AS_FONTS)))

define create_rule
gen/objects/$(1).cpp: assets/objects/$(1)/$(1).gltf
	@$(ASPROC_HOME)/bin/asproc --convert-to-srgb -o assets/objects/$(1) gen/objects
endef

gen/shaders/%_frag.cpp : assets/shaders/%.frag
	@$(ASPROC_HOME)/bin/asproc --version-override "" -s $^ gen/shaders
gen/shaders/%_vert.cpp : assets/shaders/%.vert
	@$(ASPROC_HOME)/bin/asproc --version-override "" -s $^ gen/shaders

gen/images/%_png.cpp : assets/images/%.png
	@$(ASPROC_HOME)/bin/asproc --flip-y -i $^ gen/images

gen/fonts/%.cpp : assets/fonts/%.json
	@$(ASPROC_HOME)/bin/asproc -f $^ gen/fonts

$(foreach subdir,$(AS_OBJECTS),$(eval $(call create_rule,$(subdir))))

.PHONY: gen_assets
gen_assets: $(AS_OBJECTS_CPP) $(AS_SHADERS_CPP) $(AS_IMAGES_CPP) $(AS_FONTS_CPP)
	$(info "GENERATED SOURCES!")

.PHONY: gen
gen: gen_shaders gen_images gen_objects gen_fonts

.PHONY: gen_shaders
gen_shaders:
	@$(ASPROC_HOME)/bin/asproc --version-override "$(GLSL_VERSION)" -s assets/shaders gen/shaders

.PHONY: gen_images
gen_images:
	@$(ASPROC_HOME)/bin/asproc --flip-y -i assets/images gen/images

.PHONY: gen_objects
gen_objects:
	@$(ASPROC_HOME)/bin/asproc --convert-to-srgb -o assets/objects gen/objects

.PHONY: gen_fonts
gen_fonts:
	@$(ASPROC_HOME)/bin/asproc -f assets/fonts gen/fonts

.PHONY: clean_gen
clean_gen:
	rm -rf gen