for f in *.texi;
do
	texi2html -init_file ogretexi2html.init -subdir=../`basename $f .texi` -split section $f;
done


	
	
