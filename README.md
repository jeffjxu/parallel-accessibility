# parallel-accessibility
A website accessibility evaluator in parallel. More detail in analysis.pdf.
## setup libxml2
1. visit the [libxml2 website](http://www.xmlsoft.org/downloads.html)
2. click on the FTP link for xmlsoft.org
3. copy libxml2-2.7.3.tar.gz to your local machine
4. scp to your GHC machine and untar it
5. cd into libxml2-2.7.3
6. since we don't have permission to install to root on GHC, we need to install libxml2 somewhere else with:
./configure --prefix path/to/preferred/directory //
I use:
./configure --prefix /afs/andrew.cmu.edu/usr15/jiaxix1/private/15418/parallel-accessibility/sequential
7. run make
8. run make install
9. Go to Makefile in sequential and change the path for libxml2 to the path of your libxml2-2.7.3/include
10. run make
11. run ./accessibility file.xml
