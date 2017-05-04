

gcc -c -DPLAYTONE_UNITTEST=1 dsp_r1/table.c -o table_unit.o -I./ -I./dsp_r0/ -I./dsp_r1 -I../include

gcc -c -DPLAYTONE_UNITTEST=1 dsp_r1/dtmf.c -o dtmf_unit.o -I./ -I./dsp_r0/ -I./dsp_r1 -I../include

gcc -c -DPLAYTONE_UNITTEST=1 dsp_r0/dspparam.c -o dspparam_unit.o -I./ -I./dsp_r0/ -I./dsp_r1 -I../include

gcc -c -DPLAYTONE_UNITTEST=1 playtone_unittest.c -o playtone_unittest.o -I./ -I./dsp_r0/ -I./dsp_r1 -I../include

gcc -o playtone playtone_unittest.o dspparam_unit.o dtmf_unit.o table_unit.o --static

rm -f *.pcm

./playtone 0

./playtone 1

