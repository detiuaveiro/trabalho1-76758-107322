# make              # to compile files and create the executables
# make pgm          # to download example images to the pgm/ dir
# make setup        # to setup the test files in test/ dir
# make tests        # to run basic tests
# make clean        # to cleanup object files and executables
# make cleanobj     # to cleanup object files only

CFLAGS = -Wall -O2 -g

LDLIBS = -lm

PROGS = imageTool imageTest

TESTS = test1 test2 test3 test4 test5 test6 test7 test8 test9

# Default rule: make all programs
all: $(PROGS)

imageTest: imageTest.o image8bit.o instrumentation.o error.o

imageTest.o: image8bit.h instrumentation.h

imageTool: imageTool.o image8bit.o instrumentation.o error.o

imageTool.o: image8bit.h instrumentation.h

# Rule to make any .o file dependent upon corresponding .h file
%.o: %.h

pgm:
	wget -O- https://sweet.ua.pt/jmr/aed/pgm.tgz | tar xzf -

.PHONY: setup
setup: test/

test/:
	wget -O- https://sweet.ua.pt/jmr/aed/test.tgz | tar xzf -
	@#mkdir -p $@
	@#curl -s -o test/aed-trab1-test.zip https://sweet.ua.pt/mario.antunes/aed/test/aed-trab1-test.zip
	@#unzip -q -o test/aed-trab1-test.zip -d test/

test1: $(PROGS) setup
	./imageTool test/original.pgm neg save neg.pgm
	cmp neg.pgm test/neg.pgm

test2: $(PROGS) setup
	./imageTool test/original.pgm thr 128 save thr.pgm
	cmp thr.pgm test/thr.pgm

test3: $(PROGS) setup
	./imageTool test/original.pgm bri .33 save bri.pgm
	cmp bri.pgm test/bri.pgm

test4: $(PROGS) setup
	./imageTool test/original.pgm rotate save rotate.pgm
	cmp rotate.pgm test/rotate.pgm

test5: $(PROGS) setup
	./imageTool test/original.pgm mirror save mirror.pgm
	cmp mirror.pgm test/mirror.pgm

test6: $(PROGS) setup
	./imageTool test/original.pgm crop 100,100,100,100 save crop.pgm
	cmp crop.pgm test/crop.pgm

test7: $(PROGS) setup
	./imageTool test/small.pgm test/original.pgm paste 100,100 save paste.pgm
	cmp paste.pgm test/paste.pgm

test8: $(PROGS) setup
	./imageTool test/small.pgm test/original.pgm blend 100,100,.33 save blend.pgm
	cmp blend.pgm test/blend.pgm

test9: $(PROGS) setup
	./imageTool test/original.pgm blur 7,7 save blur.pgm
	cmp blur.pgm test/blur.pgm

############## TESTES ADICIONADOS
test10: $(PROGS) setup
	./imageTool test/small.pgm test/paste.pgm locate

test11: $(PROGS) setup
	./imageTool pgm/large/airfield-05_1600x1200.pgm blur 7,7 save blur.pgm

test12: $(PROGS) setup
	./imageTool pgm/large/airfield-05_1600x1200.pgm rotate save rotate.pgm

test13: $(PROGS) setup
	./imageTool pgm/small/art3_222x217.pgm rotate save rotate.pgm

test14: $(PROGS) setup
	./imageTool pgm/large/airfield-05_1600x1200.pgm crop 0,0,800,600 save crop.pgm

testBLUR: $(PROGS) setup
	./imageTest pgm/large/airfield-05_1600x1200.pgm BLUR_TEST_WITH_INFO.pgm blur

test_to_paste: $(PROGS) setup
	./imageTool pgm/large/einstein_940x940.pgm pgm/large/airfield-05_1600x1200.pgm paste 300,100 save paste_test_1600x1200.pgm

testLOCATE: $(PROGS) setup
	./imageTool pgm/large/einstein_940x940.pgm pgm/large/airfield-05_1600x1200.pgm paste 300,100 save paste_test_1600x1200.pgm
	./imageTest pgm/large/einstein_940x940.pgm paste_test_1600x1200.pgm locate

testREPORTlocate: $(PROGS) setup
	./imageTool pgm/small/bird_256x256.pgm pgm/large/ireland_03_1600x1200.pgm paste 0,0 save report_images/pequena_em_grande_1600x1200.pgm
	./imageTool pgm/medium/mandrill_512x512.pgm pgm/large/ireland_03_1600x1200.pgm paste 0,0 save report_images/media_em_grande_1600x1200.pgm
	./imageTool pgm/large/einstein_940x940.pgm pgm/large/ireland_03_1600x1200.pgm paste 0,0 save report_images/grande_em_grande_1600x1200.pgm
	./imageTool pgm/small/bird_256x256.pgm pgm/medium/mandrill_512x512.pgm paste 0,0 save report_images/pequena_em_media_512x512.pgm
	./imageTool pgm/small/bird_256x256.pgm pgm/small/art4_300x300.pgm paste 0,0 save report_images/pequena_em_pequena_300x300.pgm

	./imageTool pgm/small/bird_256x256.pgm pgm/large/ireland_03_1600x1200.pgm paste 1344,944 save report_images/pequena_em_grande_2_1600x1200.pgm
	./imageTool pgm/medium/mandrill_512x512.pgm pgm/large/ireland_03_1600x1200.pgm paste 1088,688 save report_images/media_em_grande_2_1600x1200.pgm
	./imageTool pgm/large/einstein_940x940.pgm pgm/large/ireland_03_1600x1200.pgm paste 660,260 save report_images/grande_em_grande_2_1600x1200.pgm
	./imageTool pgm/small/bird_256x256.pgm pgm/medium/mandrill_512x512.pgm paste 256,256 save report_images/pequena_em_media_2_512x512.pgm
	./imageTool pgm/small/bird_256x256.pgm pgm/small/art4_300x300.pgm paste 44,44 save report_images/pequena_em_pequena_2_300x300.pgm


testREPORTlocate2: $(PROGS) setup
	./imageTest pgm/small/bird_256x256.pgm report_images/pequena_em_grande_1600x1200.pgm locate
	./imageTest pgm/medium/mandrill_512x512.pgm report_images/media_em_grande_1600x1200.pgm locate
	./imageTest pgm/large/einstein_940x940.pgm report_images/grande_em_grande_1600x1200.pgm locate
	./imageTest pgm/small/bird_256x256.pgm report_images/pequena_em_media_512x512.pgm locate
	./imageTest pgm/small/bird_256x256.pgm report_images/pequena_em_pequena_300x300.pgm locate

	./imageTest pgm/small/bird_256x256.pgm report_images/pequena_em_grande_2_1600x1200.pgm locate
	./imageTest pgm/medium/mandrill_512x512.pgm report_images/media_em_grande_2_1600x1200.pgm locate
	./imageTest pgm/large/einstein_940x940.pgm report_images/grande_em_grande_2_1600x1200.pgm locate
	./imageTest pgm/small/bird_256x256.pgm report_images/pequena_em_media_2_512x512.pgm locate
	./imageTest pgm/small/bird_256x256.pgm report_images/pequena_em_pequena_2_300x300.pgm locate

##############

.PHONY: tests
tests: $(TESTS)

# Make uses builtin rule to create .o from .c files.

cleanobj:
	rm -f *.o

clean: cleanobj
	rm -f $(PROGS)

