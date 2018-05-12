Executia se face pe fep.grid.pub.ro
1. [stud@fep7-1 ~]$ unzip Tema3-skl.zip
2. [stud@fep7-1 ~]$ cd Tema3-skl/
3. [stud@fep7-1 Tema3-skl]$ bash prepare_test.sh
4. [stud@fep7-1 Tema3-skl]$ vim stud_cl_etc/texture_compress_skl.cpp
--- development / redenumire stud_cl_etc/ => prenume.nume/

## Testare rapida
5-1. [stud@fep7-1 Tema3-skl]$ rm images/*.36M images/*.64M
5-2. [stud@fep7-1 Tema3-skl]$ qsub -cwd -q hp-sl.q -v DIRTEST=stud_cl_etc run_test.sh
5-3. [stud@fep7-1 Tema3-skl]$ cat results_stud_cl_etc.txt

## Testare finala
5-1a. [stud@fep7-1 Tema3-skl]$ qsub -cwd -q hp-sl.q -v DIRTEST=stud_cl_etc run_test.sh
5-1b. [stud@fep7-1 Tema3-skl]$ qsub -cwd -q hp-sl.q -v DIRTEST=prenume.nume run_test.sh
Your job 991731 ("run_test.sh") has been submitted
5-2a. [stud@fep7-1 Tema3-skl]$ cat results_stud_cl_etc.txt
5-2b. [stud@fep7-1 Tema3-skl]$ cat results_prenume.nume.txt
