# For assembly, armips is needed - https://github.com/Kingcom/armips
# For bin2exe.py - https://courses.pikuma.com/courses/take/ps1-programming-mips-assembly-language/lessons/54214700-psx-exe-file-format

OUTPUT_PATH  = ./bin
EXE_NAME     = psss
ASM_EXE_NAME = psss_asm

run: build
	${OUTPUT_PATH}/${EXE_NAME}

build:
	@mkdir -p ${OUTPUT_PATH}
	gcc ./source/main.c -Wall -o ${OUTPUT_PATH}/${EXE_NAME}

run_asm: build_asm
	${OUTPUT_PATH}/${ASM_EXE_NAME}

build_asm:
	armips ./source/main.s
	./vendor/bin2exe.py ./bin/main.bin ./bin/main.psx
