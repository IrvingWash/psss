OUTPUT_PATH  = ./bin
EXE_NAME     = psss
DEBUG_EXE_NAME = ${EXE_NAME}_debug

run_debug: build_debug
	${OUTPUT_PATH}/${DEBUG_EXE_NAME}

build_debug:
	@mkdir -p ${OUTPUT_PATH}
	gcc ./source/main.c -std=c17 -Wall -Wextra -Wpedantic -g -O0 -o ${OUTPUT_PATH}/${DEBUG_EXE_NAME}

run_release: build_release
	${OUTPUT_PATH}/${EXE_NAME}

build_release:
	@mkdir -p ${OUTPUT_PATH}
	gcc ./source/main.c -std=c17 -Wall -Wextra -Wpedantic -O2 -o ${OUTPUT_PATH}/${EXE_NAME}

# ==============================================================================
# MIPS Assembly experiments
# ==============================================================================
# For assembly, armips is needed - https://github.com/Kingcom/armips
# For bin2exe.py - https://courses.pikuma.com/courses/take/ps1-programming-mips-assembly-language/lessons/54214700-psx-exe-file-format
# These can be run at least in the PCSX Redux emulator

build_asm_divide:
	armips ./asm/divide.s
	./vendor/bin2exe.py ./bin/divide.bin ./bin/divide.psx

build_asm_draw:
	armips ./asm/draw.s
	./vendor/bin2exe.py ./bin/draw.bin ./bin/draw.psx
